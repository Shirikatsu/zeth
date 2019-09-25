#ifndef __ZETH_BLAKE2S_COMP_SETUP_TCC__
#define __ZETH_BLAKE2S_COMP_SETUP_TCC__

namespace libzeth
{
// All constants come from https://blake2.net/blake2.pdf appendix A.2
template<typename FieldT> void BLAKE2s_256_comp<FieldT>::setup_constants()
{
    IV[0] = {
        0, 1, 1, 0, 1, 0, 1, 0, // 6A
        0, 0, 0, 0, 1, 0, 0, 1, // 09
        1, 1, 1, 0, 0, 1, 1, 0, // E6
        0, 1, 1, 0, 0, 1, 1, 1  // 67
    };

    IV[1] = {
        1, 0, 1, 1, 1, 0, 1, 1, // BB
        0, 1, 1, 0, 0, 1, 1, 1, // 67
        1, 0, 1, 0, 1, 1, 1, 0, // AE
        1, 0, 0, 0, 0, 1, 0, 1  // 85
    };

    IV[2] = {
        0, 0, 1, 1, 1, 1, 0, 0, // 3C
        0, 1, 1, 0, 1, 1, 1, 0, // 6E
        1, 1, 1, 1, 0, 0, 1, 1, // F3
        0, 1, 1, 1, 0, 0, 1, 0  // 72
    };

    IV[3] = {
        1, 0, 1, 0, 0, 1, 0, 1, // A5
        0, 1, 0, 0, 1, 1, 1, 1, // 4F
        1, 1, 1, 1, 0, 1, 0, 1, // F5
        0, 0, 1, 1, 1, 0, 1, 0  // 3A
    };

    IV[4] = {
        0, 1, 0, 1, 0, 0, 0, 1, // 51
        0, 0, 0, 0, 1, 1, 1, 0, // 0E
        0, 1, 0, 1, 0, 0, 1, 0, // 52
        0, 1, 1, 1, 1, 1, 1, 1  // 7F
    };

    IV[5] = {
        1, 0, 0, 1, 1, 0, 1, 1, // 9B
        0, 0, 0, 0, 0, 1, 0, 1, // 05
        0, 1, 1, 0, 1, 0, 0, 0, // 68
        1, 0, 0, 0, 1, 1, 0, 0  // 8C

    };

    IV[6] = {
        0, 0, 0, 1, 1, 1, 1, 1, // 1F
        1, 0, 0, 0, 0, 0, 1, 1, // 83
        1, 1, 0, 1, 1, 0, 0, 1, // D9
        1, 0, 1, 0, 1, 0, 1, 1  // AB
    };

    IV[7] = {
        0, 1, 0, 1, 1, 0, 1, 1, // 5B
        1, 1, 1, 0, 0, 0, 0, 0, // E0
        1, 1, 0, 0, 1, 1, 0, 1, // CD
        0, 0, 0, 1, 1, 0, 0, 1  // 19
    };

    sigma[0] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    sigma[1] = {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3};
    sigma[2] = {11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4};
    sigma[3] = {7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8};
    sigma[4] = {9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13};
    sigma[5] = {2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9};
    sigma[6] = {12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11};
    sigma[7] = {13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10};
    sigma[8] = {6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5};
    sigma[9] = {10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0};
}

template<typename FieldT> void BLAKE2s_256_comp<FieldT>::setup_h()
{
    // parameter block, size set to 32 bytes, fanout and depth set to serial
    // mode
    std::array<std::array<FieldT, 32>, 8> parameter_block;
    parameter_block[0] = {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1};
    parameter_block[1] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    parameter_block[2] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    parameter_block[3] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    parameter_block[4] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    parameter_block[5] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    parameter_block[6] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    parameter_block[7] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    for (size_t i = 0; i < 8; i++) {
        std::array<FieldT, 32> pb_swapped =
            swap_byte32_endianness(parameter_block[i]);
        std::array<FieldT, 32> IVi = IV[i];
        h[i] = binary_field_xor(pb_swapped, IVi);
    }
}

template<typename FieldT>
void BLAKE2s_256_comp<FieldT>::setup_counter(size_t len_input_block)
{
    std::vector<FieldT> length_bits =
        convert_to_binary<FieldT>(len_input_block);
    size_t bit_size = length_bits.size();
    size_t padding = 64 - bit_size;

    // initialize counters to size of blocks
    t[0] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    for (size_t i = 0; int(i) < std::min(int(32), int(bit_size)); i++) {
        t[0][32 - i - 1] = length_bits[bit_size - i - 1];
    }

    t[1] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    if (bit_size > 32) {
        for (size_t i = 0; i < bit_size - 32; i++) {
            t[1][padding + i] = length_bits[i];
        }
    }
}

template<typename FieldT> void BLAKE2s_256_comp<FieldT>::setup_v()
{
    for (size_t i = 0; i < 8; i++) {
        std::vector<FieldT> temp_field_vector(h[i].begin(), h[i].end());
        v[0][i].fill_with_field_elements(this->pb, temp_field_vector);
    }

    for (size_t i = 8; i < 12; i++) {
        std::vector<FieldT> temp_field_vector(
            IV[i - 8].begin(), IV[i - 8].end());
        v[0][i].fill_with_field_elements(this->pb, temp_field_vector);
    }

    std::array<FieldT, 32> temp_field_xored = binary_field_xor(IV[4], t[0]);
    std::vector<FieldT> temp_field_vector12(
        temp_field_xored.begin(), temp_field_xored.end());
    v[0][12].fill_with_field_elements(this->pb, temp_field_vector12);

    temp_field_xored = binary_field_xor(IV[5], t[1]);
    std::vector<FieldT> temp_field_vector13(
        temp_field_xored.begin(), temp_field_xored.end());
    v[0][13].fill_with_field_elements(this->pb, temp_field_vector13);

    // We do only one compression, f0 is set to xFF
    std::array<FieldT, 32> xFF = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    temp_field_xored = binary_field_xor(IV[6], xFF);
    std::vector<FieldT> temp_field_vector14(
        temp_field_xored.begin(), temp_field_xored.end());
    v[0][14].fill_with_field_elements(this->pb, temp_field_vector14);

    // We are in sequential mode, f1 is set to x00
    std::array<FieldT, 32> x00 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    temp_field_xored = binary_field_xor(IV[7], x00);
    std::vector<FieldT> temp_field_vector15(
        temp_field_xored.begin(), temp_field_xored.end());
    v[0][15].fill_with_field_elements(this->pb, temp_field_vector15);
}

template<typename FieldT> void BLAKE2s_256_comp<FieldT>::setup_gadgets()
{
    for (size_t i = 0; i < rounds; i++) {
        g_arrays[i].emplace_back(
            this->pb,
            v[i][0],
            v[i][4],
            v[i][8],
            v[i][12],
            block[sigma[i % rounds][0]],
            block[sigma[i % rounds][1]],
            v_temp[i][0],
            v_temp[i][4],
            v_temp[i][8],
            v_temp[i][12],
            FMT(this->annotation_prefix, " g_primitive_1_round_%zu", i));

        g_arrays[i].emplace_back(
            this->pb,
            v[i][1],
            v[i][5],
            v[i][9],
            v[i][13],
            block[sigma[i % rounds][2]],
            block[sigma[i % rounds][3]],
            v_temp[i][1],
            v_temp[i][5],
            v_temp[i][9],
            v_temp[i][13],
            FMT(this->annotation_prefix, " g_primitive_2_round_%zu", i));

        g_arrays[i].emplace_back(
            this->pb,
            v[i][2],
            v[i][6],
            v[i][10],
            v[i][14],
            block[sigma[i % rounds][4]],
            block[sigma[i % rounds][5]],
            v_temp[i][2],
            v_temp[i][6],
            v_temp[i][10],
            v_temp[i][14],
            FMT(this->annotation_prefix, " g_primitive_3_round_%zu", i));

        g_arrays[i].emplace_back(
            this->pb,
            v[i][3],
            v[i][7],
            v[i][11],
            v[i][15],
            block[sigma[i % rounds][6]],
            block[sigma[i % rounds][7]],
            v_temp[i][3],
            v_temp[i][7],
            v_temp[i][11],
            v_temp[i][15],
            FMT(this->annotation_prefix, " g_primitive_4_round_%zu", i));

        g_arrays[i].emplace_back(
            this->pb,
            v_temp[i][0],
            v_temp[i][5],
            v_temp[i][10],
            v_temp[i][15],
            block[sigma[i % rounds][8]],
            block[sigma[i % rounds][9]],
            v[i + 1][0],
            v[i + 1][5],
            v[i + 1][10],
            v[i + 1][15],
            FMT(this->annotation_prefix, " g_primitive_5_round_%zu", i));

        g_arrays[i].emplace_back(
            this->pb,
            v_temp[i][1],
            v_temp[i][6],
            v_temp[i][11],
            v_temp[i][12],
            block[sigma[i % rounds][10]],
            block[sigma[i % rounds][11]],
            v[i + 1][1],
            v[i + 1][6],
            v[i + 1][11],
            v[i + 1][12],
            FMT(this->annotation_prefix, " g_primitive_6_round_%zu", i));

        g_arrays[i].emplace_back(
            this->pb,
            v_temp[i][2],
            v_temp[i][7],
            v_temp[i][8],
            v_temp[i][13],
            block[sigma[i % rounds][12]],
            block[sigma[i % rounds][13]],
            v[i + 1][2],
            v[i + 1][7],
            v[i + 1][8],
            v[i + 1][13],
            FMT(this->annotation_prefix, " g_primitive_7_round_%zu", i));

        g_arrays[i].emplace_back(
            this->pb,
            v_temp[i][3],
            v_temp[i][4],
            v_temp[i][9],
            v_temp[i][14],
            block[sigma[i % rounds][14]],
            block[sigma[i % rounds][15]],
            v[i + 1][3],
            v[i + 1][4],
            v[i + 1][9],
            v[i + 1][14],
            FMT(this->annotation_prefix, " g_primitive_8_round_%zu", i));
    }

    for (size_t i = 0; i < 8; i++) {
        std::vector<FieldT> temp_field_vector(h[i].begin(), h[i].end());
        xor_vector.emplace_back(
            this->pb,
            v[rounds][i],
            v[rounds][8 + i],
            temp_field_vector,
            output_bytes[i],
            FMT(this->annotation_prefix, " xor_output_%zu", i));
    }
}

} // namespace libzeth

#endif // __ZETH_BLAKE2S_COMP_SETUP_TCC__