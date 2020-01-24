// Copyright (c) 2015-2019 Clearmatics Technologies Ltd
//
// SPDX-License-Identifier: LGPL-3.0+

#include "bits.hpp"

namespace libzeth
{

bits384 get_bits384_from_vector(std::vector<bool> vect)
{
    return dump_vector_in_array<384>(vect);
}

bits256 get_bits256_from_vector(std::vector<bool> vect)
{
    return dump_vector_in_array<256>(vect);
}

bits64 get_bits64_from_vector(std::vector<bool> vect)
{
    return dump_vector_in_array<64>(vect);
}

bits_addr get_bits_addr_from_vector(std::vector<bool> vect)
{
    return dump_vector_in_array<ZETH_MERKLE_TREE_DEPTH>(vect);
}

std::vector<bool> get_vector_from_bits384(bits384 arr)
{
    return dump_array_in_vector<384>(arr);
}

std::vector<bool> get_vector_from_bits256(bits256 arr)
{
    return dump_array_in_vector<256>(arr);
}

std::vector<bool> get_vector_from_bits64(bits64 arr)
{
    return dump_array_in_vector<64>(arr);
}

std::vector<bool> get_vector_from_bits32(bits32 arr)
{
    return dump_array_in_vector<32>(arr);
}

std::vector<bool> get_vector_from_bits_addr(bits_addr arr)
{
    return dump_array_in_vector<ZETH_MERKLE_TREE_DEPTH>(arr);
}

} // namespace libzeth
