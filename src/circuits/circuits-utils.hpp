// Copyright (c) 2015-2019 Clearmatics Technologies Ltd
//
// SPDX-License-Identifier: LGPL-3.0+

#ifndef __ZETH_CIRCUITS_CIRCUITS_UTILS_HPP__
#define __ZETH_CIRCUITS_CIRCUITS_UTILS_HPP__

#include "types/bits.hpp"

#include <libsnark/gadgetlib1/pb_variable.hpp>

namespace libzeth
{

template<typename FieldT>
libsnark::linear_combination<FieldT> packed_addition(
    libsnark::pb_variable_array<FieldT> input);
template<typename FieldT>
libsnark::pb_variable_array<FieldT> from_bits(
    std::vector<bool> bits, const libsnark::pb_variable<FieldT> &ZERO);
template<typename FieldT, size_t BitLen>
std::array<FieldT, BitLen> binary_field_xor(
    std::array<FieldT, BitLen> A, std::array<FieldT, BitLen> B);
template<typename FieldT> std::vector<FieldT> convert_to_binary(size_t n);

} // namespace libzeth
#include "circuits/circuits-utils.tcc"

#endif // __ZETH_CIRCUITS_CIRCUITS_UTILS_HPP__