#ifndef __ZETH_CIRCUITS_COMMITMENT_HPP__
#define __ZETH_CIRCUITS_COMMITMENT_HPP__

// DISCLAIMER:
// Content Taken and adapted from Zcash
// https://github.com/zcash/zcash/blob/master/src/zcash/circuit/commitment.tcc

#include <libsnark/gadgetlib1/gadget.hpp>
#include <libsnark/gadgetlib1/gadgets/hashes/hash_io.hpp>

namespace libzeth
{

template<typename FieldT, typename HashT>
class COMM_gadget : libsnark::gadget<FieldT>
{
private:
    std::shared_ptr<libsnark::block_variable<FieldT>> block;
    // Hash gadget used as a commitment
    std::shared_ptr<HashT> hasher;
    // blake2sCompress(x || y)
    std::shared_ptr<libsnark::digest_variable<FieldT>> result;

public:
    COMM_gadget(
        libsnark::protoboard<FieldT> &pb,
        libsnark::pb_variable_array<FieldT> x,
        libsnark::pb_variable_array<FieldT> y,
        std::shared_ptr<libsnark::digest_variable<FieldT>> result,
        const std::string &annotation_prefix = "COMM_gadget");
    void generate_r1cs_constraints();
    void generate_r1cs_witness();
};

template<typename FieldT>
libsnark::pb_variable_array<FieldT> get128bits(
    libsnark::pb_variable_array<FieldT> &inner_k);

// As mentioned in Zerocash extended paper, page 22
// Right side of the hash inputs to generate cm is: 0^192 || value_v (64 bits)
template<typename FieldT>
libsnark::pb_variable_array<FieldT> getRightSideCMCOMM(
    libsnark::pb_variable<FieldT> &ZERO,
    libsnark::pb_variable_array<FieldT> &value_v);

// TODO: Implement the COMM_k_gadget as a 2 hash rounds in order to directly get
// the value of the commitment_k without needing 2 distinct gadgets for this.
//
// See Zerocash extended paper, page 22
// The commitment k is computed as
// k = blake2sCompress(r || [blake2sCompress(a_pk || rho)]_128)
// where we define the right part as being the inner commitment of k:
// inner_k = blake2sCompress(a_pk || rho)
template<typename FieldT, typename HashT>
class COMM_inner_k_gadget : public COMM_gadget<FieldT, HashT>
{
public:
    COMM_inner_k_gadget(
        libsnark::protoboard<FieldT> &pb,
        libsnark::pb_variable_array<FieldT>
            &a_pk, // public address key, 256 bits
        libsnark::pb_variable_array<FieldT> &rho, // 256 bits
        std::shared_ptr<libsnark::digest_variable<FieldT>>
            result, // blake2sCompress(a_pk || rho)
        const std::string &annotation_prefix = "COMM_inner_k_gadget");
};

// See Zerocash extended paper, page 22
// The commitment k is computed as
// k = blake2sCompress(r || [blake2sCompress(a_pk || rho)]_128)
// where we define outer_k as being the outer commitment of k:
// outer_k = blake2sCompress(r || [inner_commitment]_128)
// We denote by trap_r the trapdoor r
template<typename FieldT, typename HashT>
class COMM_outer_k_gadget : public COMM_gadget<FieldT, HashT>
{
public:
    COMM_outer_k_gadget(
        libsnark::protoboard<FieldT> &pb,
        libsnark::pb_variable_array<FieldT> &trap_r, // 384 bits
        libsnark::pb_variable_array<FieldT>
            &inner_k, // 256 bits, but we only keep 128 bits out of it
        std::shared_ptr<libsnark::digest_variable<FieldT>>
            result, // blake2sCompress(trap_r || [inner_k]_128)
        const std::string &annotation_prefix = "COMM_outer_k_gadget");
};

// cm = blake2sCompress(outer_k || 0^192 || value_v)
template<typename FieldT, typename HashT>
class COMM_cm_gadget : public COMM_gadget<FieldT, HashT>
{
public:
    COMM_cm_gadget(
        libsnark::protoboard<FieldT> &pb,
        libsnark::pb_variable<FieldT> &ZERO,
        libsnark::pb_variable_array<FieldT> &outer_k, // 256 bits
        libsnark::pb_variable_array<FieldT> &value_v, //  64 bits
        std::shared_ptr<libsnark::digest_variable<FieldT>>
            result, // blake2sCompress(outer_k || 0^192 || value_v)
        const std::string &annotation_prefix = "COMM_cm_gadget");
};

} // namespace libzeth
#include "circuits/commitments/commitment.tcc"

#endif // __ZETH_CIRCUITS_COMMITMENT_HPP__