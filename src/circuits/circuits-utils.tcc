// Copyright (c) 2015-2019 Clearmatics Technologies Ltd
//
// SPDX-License-Identifier: LGPL-3.0+

#ifndef __ZETH_CIRCUITS_CIRCUITS_UTILS_TCC__
#define __ZETH_CIRCUITS_CIRCUITS_UTILS_TCC__

#include <libsnark/gadgetlib1/pb_variable.hpp>
#include <vector>

namespace libzeth
{

// This define directive is useless/redundant, as ONE is defined here:
// libsnark/gadgetlib1/pb_variable.hpp#74
#ifdef ONE
#undef ONE
#endif
#define ONE libsnark::pb_variable<FieldT>(0)
//
// We know that a pb_variable takes an index in the constructor:
// See: libsnark/gadgetlib1/pb_variable.hpp#29
// Then the pb_variable can be allocated on the protoboard
// See here for the allocation function: libsnark/gadgetlib1/pb_variable.tcc#19
// This function calls the allocation function of the protoboard:
// libsnark/gadgetlib1/protoboard.tcc#38 This function basically allocates the
// variable on the protoboard at the index defined by the variable
// "next_free_var". It then returns the index the variable was allocated at,
// and, we can see in libsnark/gadgetlib1/pb_variable.tcc#19 that the index of
// the variable is given by the index where the variable was allocated on the
// protoboard. MOREOVER, we see in: libsnark/gadgetlib1/protoboard.tcc#19 (the
// constructor of the protoboard) that "next_free_var = 1;" to account for
// constant 1 term. Thus, the variable at index 0 on the protoboard is the
// constant_term variable, which value is FieldT::one() (which basically is the
// multiplicative identity of the field FieldT) Thus we are safe here. The ONE
// is well equal to the value FieldT::one()

template<typename FieldT>
libsnark::linear_combination<FieldT> packed_addition(
    libsnark::pb_variable_array<FieldT> inputs)
{
    // We use `inputs.rbegin(), inputs.rend()` otherwise the resulting linear
    // combination is built by interpreting our bit string as little endian.
    // Thus here, we make sure our binary string is interpreted correctly.
    return libsnark::pb_packing_sum<FieldT>(
        libsnark::pb_variable_array<FieldT>(inputs.rbegin(), inputs.rend()));
};

// Takes a vector of boolean values, and convert this vector of boolean values
// into a vector of FieldT::zero() and FieldT:one()
template<typename FieldT>
libsnark::pb_variable_array<FieldT> from_bits(
    std::vector<bool> bits, const libsnark::pb_variable<FieldT> &ZERO)
{
    libsnark::pb_variable_array<FieldT> acc;
    for (size_t i = 0; i < bits.size(); i++) {
        bool bit = bits[i];
        acc.emplace_back(bit ? ONE : ZERO);
    }

    return acc;
};

// XOR 2 binary strings
template<typename FieldT, size_t BitLen>
std::array<FieldT, BitLen> binary_field_xor(
    std::array<FieldT, BitLen> A, std::array<FieldT, BitLen> B)
{
    for (size_t i = 0; i < BitLen; i++) {
        if ((A[i] - FieldT("1")) * A[i] != 0 ||
            (B[i] - FieldT("1")) * B[i] != 0) {
            throw std::domain_error("Invalid value (should be 0 or 1)");
        }
    }

    std::array<FieldT, BitLen> xor_array;
    xor_array.fill(FieldT("0"));
    for (size_t i = 0; i < BitLen; i++) {
        xor_array[i] = A[i] + B[i] - FieldT(2) * A[i] * B[i]; // c is carry
    }

    return xor_array;
}

template<typename FieldT> std::vector<FieldT> convert_to_binary(size_t n)
{
    std::vector<FieldT> res;

    if (n / 2 != 0) {
        std::vector<FieldT> temp = convert_to_binary<FieldT>(n / 2);
        res.insert(res.end(), temp.begin(), temp.end());
    }

    if (n % 2 == 1) {
        res.push_back(FieldT(1));
    } else {
        res.push_back(FieldT(0));
    }

    return res;
}

} // namespace libzeth

#endif // __ZETH_CIRCUITS_CIRCUITS_UTILS_TCC__
