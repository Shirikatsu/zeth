#ifndef __ZETH_COMMITMENT_CIRCUITS_HPP__
#define __ZETH_COMMITMENT_CIRCUITS_HPP__

// DISCLAIMER:
// Content Taken and adapted from Zcash
// https://github.com/zcash/zcash/blob/master/src/zcash/circuit/commitment.tcc

#include <libsnark/gadgetlib1/gadget.hpp>
#include "circuits/mimc/mimc_hash.hpp"

namespace libzeth {


// See Zerocash extended paper, page 22
// The commitment k is computed as k = sha256(r || [sha256(a_pk || rho)]_128)
// where we define the left part: inner_k = sha256(a_pk || rho) as being the inner commitment of k

// MiMC update: We decided to replace sha256 by mimc_hash to work everywhere in F_p.
// As mimc_hash is based on the mimc encryption function and the Miyagushi-Preneel construct,
// we need to set the hash iv as well as the encryption key. Furthermore, the inputs are now field elements.
// We decided to use the same IV (sha3("Clearmatics")), set in the .tcc, and encryption key (b"mimc"), set by default in mimc_hash.tcc.
// As for the inputs, we decided to replace the || operation by "," and []_128 operation by masking rho with a random element

template<typename FieldT>
class cm_gadget : public libsnark::gadget<FieldT> {
    libsnark::pb_variable<FieldT> a_pk;
    libsnark::pb_variable<FieldT> rho;
    libsnark::pb_variable<FieldT> r_trap;
    libsnark::pb_variable<FieldT> r_mask;
    libsnark::pb_variable<FieldT> masked;
    libsnark::pb_variable<FieldT> k_outer;
    libsnark::pb_variable<FieldT> value;

    // We use pointers for the hasher as some inputs needs to be allocated (masked, k_outer)
    // before we can initialize them
    std::unique_ptr<MiMC_hash_gadget<FieldT>> inner_hasher;
    std::unique_ptr<MiMC_hash_gadget<FieldT>> outer_hasher;
    std::unique_ptr<MiMC_hash_gadget<FieldT>> final_hasher;

public:
    cm_gadget(libsnark::protoboard<FieldT>& pb,
                        libsnark::pb_variable<FieldT>& a_pk,
                        libsnark::pb_variable<FieldT>& rho,
                        libsnark::pb_variable<FieldT>& r_trap,
                        libsnark::pb_variable<FieldT>& r_mask,
                        libsnark::pb_variable<FieldT>& value,
                        const std::string &annotation_prefix = "cm_gadget");

    void generate_r1cs_constraints ();

	void generate_r1cs_witness ();

    const libsnark::pb_variable<FieldT> result() const;

    const libsnark::pb_variable<FieldT> k() const;
};


} // libzeth
#include "circuits/commitments/commitments.tcc"

#endif // __ZETH_COMMITMENT_CIRCUITS_HPP__
