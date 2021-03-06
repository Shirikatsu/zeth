#!/usr/bin/env python3

# Copyright (c) 2015-2019 Clearmatics Technologies Ltd
#
# SPDX-License-Identifier: LGPL-3.0+

from __future__ import annotations
import zeth.contracts as contracts
import zeth.constants as constants
from zeth.ownership import OwnershipPublicKey, OwnershipSecretKey, \
    OwnershipKeyPair, ownership_key_as_hex
from zeth.encryption import EncryptionPublicKey, EncryptionSecretKey, \
    EncryptionKeyPair, generate_encryption_keypair, encode_encryption_public_key
import zeth.signing as signing

from zeth.zksnark import IZKSnarkProvider, GenericProof
from zeth.utils import EtherValue, get_trusted_setup_dir, \
    hex_digest_to_binary_string, digest_to_binary_string, encode_abi, encrypt, \
    decrypt, int64_to_hex, encode_message_to_bytes, compute_merkle_path
from zeth.prover_client import ProverClient
from api.util_pb2 import ZethNote, JoinsplitInput
import api.prover_pb2 as prover_pb2

import os
import json
from Crypto import Random
from hashlib import blake2s, sha256
from typing import Tuple, Dict, List, Iterable, Callable, Optional, Any


# Value of a single unit (in Wei) of vpub_in and vpub_out.  Use Szabos (10^12
# Wei).
ZETH_PUBLIC_UNIT_VALUE = 1000000000000


ZERO_UNITS_HEX = "0000000000000000"


# JoinSplit Signature Keys definitions
JoinsplitSigVerificationKey = signing.SigningVerificationKey
JoinsplitSigSecretKey = signing.SigningSecretKey
JoinsplitSigKeyPair = signing.SigningKeyPair


ComputeHSigCB = Callable[[bytes, bytes, JoinsplitSigVerificationKey], bytes]


class ZethAddressPub:
    """
    Public half of a zethAddress.  addr_pk = (a_pk and k_pk)
    """
    def __init__(self, a_pk: OwnershipPublicKey, k_pk: EncryptionPublicKey):
        self.a_pk: OwnershipPublicKey = a_pk
        self.k_pk: EncryptionPublicKey = k_pk


class ZethAddressPriv:
    """
    Secret addr_sk, consisting of a_sk and k_sk
    """
    def __init__(self, a_sk: OwnershipSecretKey, k_sk: EncryptionSecretKey):
        self.a_sk: OwnershipSecretKey = a_sk
        self.k_sk: EncryptionSecretKey = k_sk


class ZethAddress:
    """
    Secret and public keys for both ownership and encryption (referrred to as
    "zethAddress" in the paper).
    """
    def __init__(
            self,
            a_pk: OwnershipPublicKey,
            k_pk: EncryptionPublicKey,
            a_sk: OwnershipSecretKey,
            k_sk: EncryptionSecretKey):
        self.addr_pk = ZethAddressPub(a_pk, k_pk)
        self.addr_sk = ZethAddressPriv(a_sk, k_sk)

    @staticmethod
    def from_key_pairs(
            ownership: OwnershipKeyPair,
            encryption: EncryptionKeyPair) -> ZethAddress:
        return ZethAddress(
            ownership.a_pk,
            encryption.k_pk,
            ownership.a_sk,
            encryption.k_sk)


class JoinsplitInputNote:
    """
    A ZethNote, along with the nullifier and location in Merkle tree.
    """

    def __init__(self, note: ZethNote, nullifier: str, merkle_location: int):
        self.note = note
        self.nullifier = nullifier
        self.merkle_location = merkle_location


def to_zeth_units(value: EtherValue) -> int:
    """
    Convert a quantity of ether / token to Zeth units
    """
    return int(value.wei / ZETH_PUBLIC_UNIT_VALUE)


def create_zeth_notes(
        phi: str,
        hsig: bytes,
        output0: Tuple[OwnershipPublicKey, int],
        output1: Tuple[OwnershipPublicKey, int]
) -> Tuple[ZethNote, ZethNote]:
    """
    Create two ordered ZethNotes. This function is used to generate new output
    notes.
    """
    (recipient0, value0) = output0
    (recipient1, value1) = output1

    rho0 = _compute_rho_i(phi, hsig, 0)
    trap_r0 = trap_r_randomness()
    note0 = ZethNote(
        apk=ownership_key_as_hex(recipient0),
        value=int64_to_hex(value0),
        rho=rho0.hex(),
        trap_r=trap_r0)

    rho1 = _compute_rho_i(phi, hsig, 1)
    trap_r1 = trap_r_randomness()
    note1 = ZethNote(
        apk=ownership_key_as_hex(recipient1),
        value=int64_to_hex(value1),
        rho=rho1.hex(),
        trap_r=trap_r1)

    return note0, note1


def parse_zeth_note(zeth_note_grpc_obj: ZethNote) -> Dict[str, str]:
    note_json = {
        "a_pk": zeth_note_grpc_obj.apk,
        "value": zeth_note_grpc_obj.value,
        "rho": zeth_note_grpc_obj.rho,
        "trap_r": zeth_note_grpc_obj.trap_r,
    }
    return note_json


def zeth_note_obj_from_parsed(parsed_zeth_note: Dict[str, str]) -> ZethNote:
    note = ZethNote(
        apk=parsed_zeth_note["a_pk"],
        value=parsed_zeth_note["value"],
        rho=parsed_zeth_note["rho"],
        trap_r=parsed_zeth_note["trap_r"]
    )
    return note


def compute_commitment(zeth_note_grpc_obj: ZethNote) -> str:
    """
    Used by the recipient of a payment to recompute the commitment and check
    the membership in the tree to confirm the validity of a payment
    """
    # inner_k = blake2s(a_pk || rho)
    inner_k = blake2s(
        encode_abi(
            ['bytes32', 'bytes32'],
            [bytes.fromhex(zeth_note_grpc_obj.apk),
             bytes.fromhex(zeth_note_grpc_obj.rho)])
    ).hexdigest()

    # outer_k = blake2s(r || [inner_k]_128)
    first_128bits_inner_comm = inner_k[0:128]
    outer_k = blake2s(
        encode_abi(
            ['bytes', 'bytes'],
            [bytes.fromhex(zeth_note_grpc_obj.trap_r),
             bytes.fromhex(first_128bits_inner_comm)])).hexdigest()

    # cm = blake2s(outer_k || 0^192 || value_v)
    front_pad = "000000000000000000000000000000000000000000000000"
    cm = blake2s(
        encode_abi(
            ["bytes32", "bytes32"],
            [bytes.fromhex(outer_k),
             bytes.fromhex(front_pad + zeth_note_grpc_obj.value)])
    ).hexdigest()
    return cm


def compute_nullifier(
        zeth_note: ZethNote,
        spending_authority_ask: OwnershipSecretKey) -> bytes:
    """
    Returns nf = blake2s(1110 || [a_sk]_252 || rho)
    """
    binary_ask = digest_to_binary_string(spending_authority_ask)
    first_252bits_ask = binary_ask[:252]
    left_leg_bin = "1110" + first_252bits_ask
    left_leg = int(left_leg_bin, 2).to_bytes(32, byteorder='big')
    blake_hash = blake2s()
    blake_hash.update(left_leg)
    blake_hash.update(bytes.fromhex(zeth_note.rho))
    return blake_hash.digest()


def create_joinsplit_input(
        merkle_path: List[str],
        address: int,
        note: ZethNote,
        a_sk: OwnershipSecretKey,
        nullifier: bytes) -> JoinsplitInput:
    return JoinsplitInput(
        merkle_path=merkle_path,
        address=address,
        note=note,
        spending_ask=ownership_key_as_hex(a_sk),
        nullifier=nullifier.hex())


def write_verification_key(
        vk_obj: prover_pb2.VerificationKey,
        zksnark: IZKSnarkProvider) -> None:
    """
    Writes the verification key (object) in a json file
    """
    vk_json = zksnark.parse_verification_key(vk_obj)
    setup_dir = get_trusted_setup_dir()
    filename = os.path.join(setup_dir, "vk.json")
    with open(filename, 'w') as outfile:
        json.dump(vk_json, outfile)


def get_dummy_rho() -> str:
    return bytes(Random.get_random_bytes(32)).hex()


def get_dummy_input_and_address(
        a_pk: OwnershipPublicKey) -> Tuple[int, ZethNote]:
    """
    Create a zeth note and address, for use as circuit inputs where there is no
    real input.
    """
    dummy_note = ZethNote(
        apk=ownership_key_as_hex(a_pk),
        value=ZERO_UNITS_HEX,
        rho=get_dummy_rho(),
        trap_r=trap_r_randomness())
    dummy_note_address = 7
    return (dummy_note_address, dummy_note)


def compute_joinsplit2x2_inputs(
        mk_root: str,
        input0: Tuple[int, ZethNote],
        mk_path0: List[str],
        input1: Tuple[int, ZethNote],
        mk_path1: List[str],
        sender_ask: OwnershipSecretKey,
        output0: Tuple[OwnershipPublicKey, int],
        output1: Tuple[OwnershipPublicKey, int],
        public_in_value_zeth_units: int,
        public_out_value_zeth_units: int,
        sign_vk: JoinsplitSigVerificationKey,
        compute_h_sig_cb: Optional[ComputeHSigCB] = None
) -> prover_pb2.ProofInputs:
    """
    Create a ProofInput object for joinsplit parameters
    """
    (input_address0, input_note0) = input0
    (input_address1, input_note1) = input1

    input_nullifier0 = compute_nullifier(input_note0, sender_ask)
    input_nullifier1 = compute_nullifier(input_note1, sender_ask)
    js_inputs: List[JoinsplitInput] = [
        create_joinsplit_input(
            mk_path0, input_address0, input_note0, sender_ask, input_nullifier0),
        create_joinsplit_input(
            mk_path1, input_address1, input_note1, sender_ask, input_nullifier1)
    ]

    # Use the specified or default h_sig computation
    compute_h_sig_cb = compute_h_sig_cb or compute_h_sig
    h_sig = compute_h_sig_cb(
        input_nullifier0,
        input_nullifier1,
        sign_vk)
    phi = _transaction_randomness()

    output_note0, output_note1 = create_zeth_notes(
        phi,
        h_sig,
        output0,
        output1)

    js_outputs = [
        output_note0,
        output_note1
    ]

    return prover_pb2.ProofInputs(
        mk_root=mk_root,
        js_inputs=js_inputs,
        js_outputs=js_outputs,
        pub_in_value=int64_to_hex(public_in_value_zeth_units),
        pub_out_value=int64_to_hex(public_out_value_zeth_units),
        h_sig=h_sig.hex(),
        phi=phi)


class ZethClient:
    """
    Context for zeth operations
    """
    def __init__(
            self,
            prover_client: ProverClient,
            mixer_instance: Any,
            zksnark: IZKSnarkProvider):
        self._prover_client = prover_client
        self._mixer_instance = mixer_instance
        self._zksnark = zksnark

    def get_merkle_tree(self) -> List[bytes]:
        return contracts.get_merkle_tree(self._mixer_instance)

    def joinsplit(
            self,
            mk_root: str,
            mk_tree: List[bytes],
            mk_tree_depth: int,
            sender_ownership_keypair: OwnershipKeyPair,
            sender_eth_address: str,
            inputs: List[Tuple[int, ZethNote]],
            outputs: List[Tuple[ZethAddressPub, EtherValue]],
            v_in: EtherValue,
            v_out: EtherValue,
            tx_payment: EtherValue,
            compute_h_sig_cb: Optional[ComputeHSigCB] = None
    ) -> contracts.MixResult:
        assert len(inputs) <= constants.JS_INPUTS
        assert len(outputs) <= constants.JS_OUTPUTS

        sender_a_sk = sender_ownership_keypair.a_sk
        sender_a_pk = sender_ownership_keypair.a_pk
        inputs = \
            inputs + \
            [get_dummy_input_and_address(sender_a_pk)
             for _ in range(constants.JS_INPUTS - len(inputs))]
        mk_paths = \
            [compute_merkle_path(addr, mk_tree_depth, mk_tree)
             for addr, _ in inputs]

        # Generate output notes and proof.  Dummy outputs are constructed with
        # value 0 to an invalid ZethAddressPub, formed from the senders
        # a_pk, and an ephemeral k_pk.
        dummy_k_pk = generate_encryption_keypair().k_pk
        dummy_addr_pk = ZethAddressPub(sender_a_pk, dummy_k_pk)
        outputs = \
            outputs + \
            [(dummy_addr_pk, EtherValue(0))
             for _ in range(constants.JS_OUTPUTS - len(outputs))]
        outputs_with_a_pk = \
            [(zeth_addr.a_pk, to_zeth_units(value))
             for (zeth_addr, value) in outputs]
        (output_note1, output_note2, proof_json, signing_keypair) = \
            self.get_proof_joinsplit_2_by_2(
                mk_root,
                inputs[0],
                mk_paths[0],
                inputs[1],
                mk_paths[1],
                sender_a_sk,
                outputs_with_a_pk[0],
                outputs_with_a_pk[1],
                to_zeth_units(v_in),
                to_zeth_units(v_out),
                compute_h_sig_cb)

        # Encrypt the notes
        outputs_and_notes = zip(outputs, [output_note1, output_note2])
        output_notes_with_k_pk = \
            [(note, zeth_addr.k_pk)
             for ((zeth_addr, _), note) in outputs_and_notes]
        (sender_eph_pk, ciphertexts) = encrypt_notes(output_notes_with_k_pk)

        # Sign
        signature = joinsplit_sign(
            signing_keypair, sender_eph_pk, ciphertexts, proof_json)

        return self.mix(
            sender_eph_pk,
            ciphertexts[0],
            ciphertexts[1],
            proof_json,
            signing_keypair.vk,
            signature,
            sender_eth_address,
            tx_payment.wei,
            4000000)

    def mix(
            self,
            pk_sender: EncryptionPublicKey,
            ciphertext1: bytes,
            ciphertext2: bytes,
            parsed_proof: GenericProof,
            vk: JoinsplitSigVerificationKey,
            sigma: int,
            sender_address: str,
            wei_pub_value: int,
            call_gas: int) -> contracts.MixResult:
        return contracts.mix(
            self._mixer_instance,
            pk_sender,
            ciphertext1,
            ciphertext2,
            parsed_proof,
            vk,
            sigma,
            sender_address,
            wei_pub_value,
            call_gas,
            self._zksnark)

    def get_proof_joinsplit_2_by_2(
            self,
            mk_root: str,
            input0: Tuple[int, ZethNote],
            mk_path0: List[str],
            input1: Tuple[int, ZethNote],
            mk_path1: List[str],
            sender_ask: OwnershipSecretKey,
            output0: Tuple[OwnershipPublicKey, int],
            output1: Tuple[OwnershipPublicKey, int],
            public_in_value_zeth_units: int,
            public_out_value_zeth_units: int,
            compute_h_sig_cb: Optional[ComputeHSigCB] = None
    ) -> Tuple[ZethNote, ZethNote, Dict[str, Any], JoinsplitSigKeyPair]:
        """
        Query the prover server to generate a proof for the given joinsplit
        parameters.
        """
        signing_keypair = signing.gen_signing_keypair()
        proof_input = compute_joinsplit2x2_inputs(
            mk_root,
            input0,
            mk_path0,
            input1,
            mk_path1,
            sender_ask,
            output0,
            output1,
            public_in_value_zeth_units,
            public_out_value_zeth_units,
            signing_keypair.vk,
            compute_h_sig_cb)
        proof_obj = self._prover_client.get_proof(proof_input)
        proof_json = self._zksnark.parse_proof(proof_obj)

        # We return the zeth notes to be able to spend them later
        # and the proof used to create them
        return (
            proof_input.js_outputs[0],  # pylint: disable=no-member
            proof_input.js_outputs[1],  # pylint: disable=no-member
            proof_json,
            signing_keypair)


def encrypt_notes(
        notes: List[Tuple[ZethNote, EncryptionPublicKey]]
) -> Tuple[EncryptionPublicKey, List[bytes]]:
    """
    Encrypts a set of output notes to be decrypted by the respective receivers.
    Returns the senders (ephemeral) public key (encoded as bytes) and the
    ciphertexts corresponding to each note.
    """
    # generate ephemeral ec25519 key
    eph_enc_key_pair = generate_encryption_keypair()
    eph_sk = eph_enc_key_pair.k_sk
    eph_pk = eph_enc_key_pair.k_pk

    def _encrypt_note(out_note: ZethNote, pub_key: EncryptionPublicKey) -> bytes:
        out_note_str = json.dumps(parse_zeth_note(out_note))
        return encrypt(out_note_str, pub_key, eph_sk)

    ciphertexts = [_encrypt_note(note, pk) for (note, pk) in notes]
    return (eph_pk, ciphertexts)


def receive_notes(
        addrs_and_ciphertexts: List[Tuple[int, bytes]],
        sender_k_pk: EncryptionPublicKey,
        receiver_k_sk: EncryptionSecretKey) -> Iterable[Tuple[int, ZethNote]]:
    """
    Given the receivers secret key, and the event data from a transaction
    (encrypted notes), decrypt any that are intended for the receiver.
    """
    for address, ciphertext in addrs_and_ciphertexts:
        try:
            plaintext = decrypt(ciphertext, sender_k_pk, receiver_k_sk)
            yield address, zeth_note_obj_from_parsed(json.loads(plaintext))
        except Exception as e:
            print(f"receive_notes: error: {e}")
            continue


def _encode_proof_and_inputs(proof_json: GenericProof) -> Tuple[bytes, bytes]:
    """
    Given a proof object, compute the hash of the properties excluding "inputs",
    and the hash of the "inputs".
    """

    proof_elements: List[int] = []
    for key in proof_json.keys():
        if key != "inputs":
            proof_elements.extend(proof_json[key])
    return (
        encode_message_to_bytes(proof_elements),
        encode_message_to_bytes(proof_json["inputs"]))


def joinsplit_sign(
        signing_keypair: JoinsplitSigKeyPair,
        sender_eph_pk: EncryptionPublicKey,
        ciphertexts: List[bytes],
        proof_json: GenericProof,
) -> int:
    """
    Generate a signature on the hash of the ciphertexts, proofs and
    primary inputs. This is used to solve transaction malleability.  We chose
    to sign the hash and not the values themselves for modularity (to use the
    same code regardless of whether GROTH16 or PGHR13 proof system is chosen),
    and sign the hash of the ciphers and inputs for consistency.
    """
    assert len(ciphertexts) == constants.JS_INPUTS

    # The message to sign consists of (in order):
    #   - senders public encryption key
    #   - ciphertexts
    #   - proof elements
    #   - public input elements
    h = sha256()
    h.update(encode_encryption_public_key(sender_eph_pk))
    for ciphertext in ciphertexts:
        h.update(ciphertext)

    proof_bytes, pub_inputs_bytes = _encode_proof_and_inputs(proof_json)
    h.update(proof_bytes)
    h.update(pub_inputs_bytes)
    message_digest = h.digest()
    return signing.sign(signing_keypair.sk, message_digest)


def compute_h_sig(
        nf0: bytes,
        nf1: bytes,
        sign_vk: JoinsplitSigVerificationKey) -> bytes:
    """
    Compute h_sig = sha256(nf0 || nf1 || sign_vk)
    Flatten the verification key
    """
    h = sha256()
    h.update(nf0)
    h.update(nf1)
    h.update(signing.encode_vk_to_bytes(sign_vk))
    return h.digest()


def trap_r_randomness() -> str:
    """
    Compute randomness "r" as 48 random bytes
    """
    return bytes(Random.get_random_bytes(48)).hex()


def _compute_rho_i(phi: str, hsig: bytes, i: int) -> bytes:
    """
    Returns rho_i = blake2s(0 || i || 10 || [phi]_252 || hsig)
    See: Zcash protocol spec p. 57, Section 5.4.2 Pseudo Random Functions
    """
    # [SANITY CHECK] make sure i is in the interval [0, JS_INPUTS]
    # Since we only allow for 2 input notes in the joinsplit
    assert i < constants.JS_INPUTS

    blake_hash = blake2s()

    # Append PRF^{rho} tag to a_sk
    binary_phi = hex_digest_to_binary_string(phi)
    first_252bits_phi = binary_phi[:252]
    left_leg_bin = "0" + str(i) + "10" + first_252bits_phi
    blake_hash.update(int(left_leg_bin, 2).to_bytes(32, byteorder='big'))
    blake_hash.update(hsig)
    return blake_hash.digest()


def _h_sig_randomness() -> bytes:
    """
    Compute the signature randomness "randomSeed", used for computing h_sig
    """
    return bytes(Random.get_random_bytes(32))


def _transaction_randomness() -> str:
    """
    Compute the transaction randomness "phi", used for computing the new rhoS
    """
    return bytes(Random.get_random_bytes(32)).hex()
