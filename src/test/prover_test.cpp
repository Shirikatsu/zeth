#include "gtest/gtest.h"

#include <libff/common/default_types/ec_pp.hpp>
#include <libsnark/common/default_types/r1cs_ppzksnark_pp.hpp>
#include <libsnark/common/default_types/r1cs_gg_ppzksnark_pp.hpp>
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark.hpp>
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_gg_ppzksnark/r1cs_gg_ppzksnark.hpp>

// Header to use the merkle tree data structure to keep a local merkle tree
#include <src/types/merkle_tree.hpp> //<libsnark/common/data_structures/merkle_tree.hpp>

// Have access to a chrono to measure the rough time of execution of a set of instructions
#include <chrono>
#include "snarks_alias.hpp"
// Import only the core components of the SNARK (not the API components)
#include "snarks_core_imports.hpp"
#include "libsnark_helpers/libsnark_helpers.hpp"
#include "circuits/mimc/mimc_hash.hpp" //"circuits/sha256/sha256_ethereum.hpp"
#include "circuit-wrapper.hpp"
#include "util.hpp"

using namespace libzeth;

typedef libff::default_ec_pp ppT;
typedef libff::Fr<ppT> FieldT; // Should be alt_bn128 in the CMakeLists.txt
typedef MiMC_hash_gadget<FieldT> HashT; // We use our hash function to do the tests

namespace {

bool TestValidJS2In2Case1(
    CircuitWrapper<FieldT, HashT, 2, 2> &prover,
    libzeth::keyPairT<ppT> keypair
) {
    // --- General setup for the tests --- //
    libff::print_header("test JS 2-2: IN => vpub_in = 0x0, note1 = 0x2F0000000000000F, note2 = 0x0 || OUT => vpub_out = 0x1700000000000007, note1 = 0x1800000000000008, note2 = 0x0");

    libff::enter_block("[START] Instantiate merkle tree for the tests", true);
    // Create a merkle tree to run our tests
    // Note: `make_unique` should be C++14 compliant, but here we use c++11, so we instantiate our unique_ptr manually
    std::unique_ptr<merkle_tree<FieldT, HashT>> test_merkle_tree = std::unique_ptr<merkle_tree<FieldT, HashT>>(
        new merkle_tree<FieldT, HashT>(
            ZETH_MERKLE_TREE_DEPTH
        )
    );
    libff::leave_block("[END] Instantiate merkle tree for the tests", true);

    // --- Test 1: Generate a valid proof for commitment inserted at address 1 -- //
    libff::enter_block("[BEGIN] Create JSInput", true);
    // Create the zeth note data for the commitment we will insert in the tree (commitment to spend in this test)
    /*
    bits384 trap_r_bits384 = get_bits384_from_vector(hexadecimal_str_to_binary_vector("0F000000000000FF00000000000000FF00000000000000FF00000000000000FF00000000000000FF00000000000000FF"));
    bits64 value_bits64 = get_bits64_from_vector(hexadecimal_str_to_binary_vector("2F0000000000000F"));
    bits256 a_sk_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("FF0000000000000000000000000000000000000000000000000000000000000F"));
    bits256 rho_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("FFFF000000000000000000000000000000000000000000000000000000009009"));
    bits256 a_pk_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("6461f753bfe21ba2219ced74875b8dbd8c114c3c79d7e41306dd82118de1895b"));
    bits256 nf_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("69f12603c2cfb2acf6f80a8f72cbdeb4417a6b8c7290e793c4d22830c4b35c5f"));
    bits256 cm_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("823d19485c94f74b4739ba7d17e4b434693086a996fa2e8d1438a91b1c220331"));
    libff::bit_vector address_bits = {1, 0, 0, 0}; // 4 being the value of ZETH_MERKLE_TREE_DEPTH
    const size_t address_commitment = 1;
    */

    FieldT r_trap = FieldT("6576838732374919021860119342200398901974877797242970520445052250557344565821"); 
    FieldT r_mask = FieldT("12946791413528024759839394340318236878559158148001437182189040772047964059643"); 
    FieldT a_sk = FieldT("18834251028175908666459239027856614524890385928194459012149074634190864282942");
    FieldT a_pk = FieldT("7557284417121959865806721773315070721481656811910187747772194493889443679515");
    FieldT rho = FieldT("1050857222747641138499486657636501128934992461797459043826425853190223953952");
    FieldT nf = FieldT("16718990446402804328289195783834079890551903281540894658644001381677667818395");
    FieldT value = FieldT("100"); // 3386706919782613007 is 2F0000000000000F in decimal
    FieldT cm = FieldT("21075862836612025410946586494502715375244302209109240154340545056954086062438");
    libff::bit_vector address_bits = {1, 0, 0, 0}; // 4 being the value of ZETH_MERKLE_TREE_DEPTH
    const size_t address_commitment = 1;


    // We insert the commitment to the zeth note in the merkle tree
    //test_merkle_tree->set_value(address_commitment, libff::bit_vector(get_vector_from_bits256(cm_bits256)));
    test_merkle_tree->set_value(address_commitment, cm);
    //libff::bit_vector updated_root_value = test_merkle_tree->get_root();
    FieldT updated_root_value = test_merkle_tree->get_root();
    //std::vector<libsnark::merkle_authentication_node> path = test_merkle_tree->get_path(address_commitment);
    std::vector<FieldT> path = test_merkle_tree->get_path(address_commitment);


    // JS Inputs: 1 note of value > 0 to spend, and a dummy note
    /*
    ZethNote note_input(
        a_pk_bits256,
        value_bits64,
        rho_bits256,
        trap_r_bits384
    );
    */
   FZethNote<FieldT> note_input(
        a_pk, 
        value, 
        rho, 
        r_trap,
        r_mask
    );

    /*
    ZethNote note_dummy_input(
        a_pk_bits256,
        get_bits64_from_vector(hexadecimal_str_to_binary_vector("0000000000000000")),
        get_bits256_from_vector(hexadecimal_digest_to_binary_vector("AAAA00000000000000000000000000000000000000000000000000000000EEEE")),
        trap_r_bits384
    );
    */
    FZethNote<FieldT> note_dummy_input(
        a_pk, 
        FieldT("0000000000000000"), 
        FieldT("6845108050456603036310667214894676007661663921399154479307840696887919990996"),  // rho_dummy = mimic_hash([4], sha3("Clearmatics"))
        r_trap,
        r_mask
    );

    /*
    JSInput input(
        path,
        address_commitment,
        get_bitsAddr_from_vector(address_bits),
        note_input,
        a_sk_bits256,
        nf_bits256
    );
    */
    FJSInput input(
        path,
        address_commitment,
        get_bitsAddr_from_vector(address_bits),
        note_input,
        a_sk,
        nf
    );

    // We keep the same path and address as the previous commitment
    // We don't care since this coin is zero-valued and the merkle auth path check
    // Doesn't count in such case
    /*
    JSInput input_dummy(
        path,
        address_commitment,
        get_bitsAddr_from_vector(address_bits),
        note_dummy_input,
        a_sk_bits256,
        nf_bits256
    );
    */
    FJSInput input_dummy(
        path,
        address_commitment,
        get_bitsAddr_from_vector(address_bits),
        note_dummy_input,
        a_sk,
        nf
    );


    std::array<FJSInput, 2> inputs;
    inputs[0] = input;
    inputs[1] = input_dummy;
    libff::leave_block("[END] Create JSInput", true);

    libff::enter_block("[BEGIN] Create JSOutput/ZethNote", true);

    /*/
    bits64 value_out_bits64 = get_bits64_from_vector(hexadecimal_str_to_binary_vector("1800000000000008"));
    bits256 a_pk_out_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("7777f753bfe21ba2219ced74875b8dbd8c114c3c79d7e41306dd82118de1895b"));
    bits256 rho_out_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("1111000000000000000000000000000000000000000000000000000000009777"));
    bits384 trap_r_out_bits384 = get_bits384_from_vector(hexadecimal_str_to_binary_vector("11000000000000990000000000000099000000000000007700000000000000FF00000000000000FF0000000000000777"));
    */
    FieldT a_sk_out = FieldT("4047591473000155590199171927915978796573140621771266280705379796913645161555"); // mimic_hash([-1], sha3("Clearmatics"))
    FieldT a_pk_out = FieldT("1570964467829526078663761333427980033315285096730121923127414543972819455975");
    FieldT r_trap_out = FieldT("3121287842287349864642297846963883646477840388236905026425392648441319037621"); // mimic_hash([-2], sha3("Clearmatics"))
    FieldT r_mask_out = FieldT("19253552993580840822052270218222026004974786985939386817973146641031673406634"); // mimic_hash([-3], sha3("Clearmatics"))
    FieldT rho_out = FieldT("10448869983030339500740742410361707713409326656173533049846269061232406471931"); // mimic_hash([-4], sha3("Clearmatics"))
    FieldT value_out = FieldT("75"); // 1729382256910270472 is 1800000000000008 in decimal


    /* 
   ZethNote note_output(
        a_pk_out_bits256,
        value_out_bits64,
        rho_out_bits256,
        trap_r_out_bits384
    );
    ZethNote note_dummy_output(
        a_pk_out_bits256,
        get_bits64_from_vector(hexadecimal_str_to_binary_vector("0000000000000000")),
        rho_out_bits256,
        trap_r_out_bits384
    );    
    */

   FZethNote<FieldT> note_output(
        a_pk_out,
        value_out,
        rho_out,
        r_trap_out,
        r_mask_out
    );

    FZethNote<FieldT> note_dummy_output(
        a_pk_out,
        FieldT("0000000000000000"),
        rho_out,
        r_trap_out,
        r_mask_out
    );
 


    //bits64 value_pub_out_bits64 = get_bits64_from_vector(hexadecimal_str_to_binary_vector("1700000000000007"));
    FieldT value_pub_out = FieldT("25"); // 1657324662872342535 is "1700000000000007" in decimal
    std::array<FZethNote<FieldT>, 2> outputs;
    outputs[0] = note_output;
    outputs[1] = note_dummy_output;
    libff::leave_block("[END] Create JSOutput/ZethNote", true);

    libff::enter_block("[BEGIN] Generate proof", true);
    extended_proof<ppT> ext_proof = prover.prove(
        updated_root_value,
        inputs,
        outputs,
        FieldT("0000000000000000"), // vpub_in = 0
        value_pub_out,
        keypair.pk
    );
    libff::leave_block("[END] Generate proof", true);


    libff::enter_block("[BEGIN] Verify proof", true);
    // Get the verification key
    libzeth::verificationKeyT<ppT> vk = keypair.vk;
    bool res = libzeth::verify(ext_proof, vk);
    libff::leave_block("[END] Verify proof", true);

    return res;
}


bool TestValidJS2In2Case2(
    CircuitWrapper<FieldT, HashT, 2, 2> &prover,
    libzeth::keyPairT<ppT> keypair
) {
    libff::print_header("Starting test: IN => v_pub = 0, note1 = 0x2F0000000000000F, note2 = 0x0 || OUT => v_pub = 0x000000000000000B, note1 = 0x1A00000000000002, note2 = 0x1500000000000002");

    libff::enter_block("[START] Instantiate merkle tree for the tests", true);
    // Create a merkle tree to run our tests
    // Note: `make_unique` should be C++14 compliant, but here we use c++11, so we instantiate our unique_ptr manually
    std::unique_ptr<merkle_tree<FieldT, HashT>> test_merkle_tree = std::unique_ptr<merkle_tree<FieldT, HashT>>(
        new merkle_tree<FieldT, HashT>(
            ZETH_MERKLE_TREE_DEPTH
        )
    );
    libff::leave_block("[END] Instantiate merkle tree for the tests", true);

    // --- Test 1: Generate a valid proof for commitment inserted at address 1 -- //
    libff::enter_block("[BEGIN] Create JSInput", true);

    /*
    // Create the zeth note data for the commitment we will insert in the tree (commitment to spend in this test)
    bits384 trap_r_bits384 = get_bits384_from_vector(hexadecimal_str_to_binary_vector("0F000000000000FF00000000000000FF00000000000000FF00000000000000FF00000000000000FF00000000000000FF"));
    bits256 a_sk_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("FF0000000000000000000000000000000000000000000000000000000000000F"));
    bits256 rho_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("FFFF000000000000000000000000000000000000000000000000000000009009"));
    bits256 a_pk_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("6461f753bfe21ba2219ced74875b8dbd8c114c3c79d7e41306dd82118de1895b"));
    bits256 nf_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("69f12603c2cfb2acf6f80a8f72cbdeb4417a6b8c7290e793c4d22830c4b35c5f"));

    // We compute the commitment to insert it in the merkle tree we use for the tests
    bits256 cm_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("823d19485c94f74b4739ba7d17e4b434693086a996fa2e8d1438a91b1c220331"));
    libff::bit_vector address_bits = {1, 0, 0, 0}; // 4 being the value of ZETH_MERKLE_TREE_DEPTH
    const size_t address_commitment = 1;
    */

    FieldT r_trap = FieldT("6576838732374919021860119342200398901974877797242970520445052250557344565821"); 
    FieldT r_mask = FieldT("12946791413528024759839394340318236878559158148001437182189040772047964059643"); 
    FieldT a_sk = FieldT("18834251028175908666459239027856614524890385928194459012149074634190864282942");
    FieldT a_pk = FieldT("7557284417121959865806721773315070721481656811910187747772194493889443679515");
    FieldT rho = FieldT("1050857222747641138499486657636501128934992461797459043826425853190223953952");
    FieldT nf = FieldT("16718990446402804328289195783834079890551903281540894658644001381677667818395");
    FieldT value = FieldT("100"); // 3386706919782613007 is 2F0000000000000F in decimal
    FieldT cm = FieldT("21075862836612025410946586494502715375244302209109240154340545056954086062438");
    libff::bit_vector address_bits = {1, 0, 0, 0}; // 4 being the value of ZETH_MERKLE_TREE_DEPTH
    const size_t address_commitment = 1;



    // We insert the commitment to the zeth note in the merkle tree
    test_merkle_tree->set_value(address_commitment, cm);
    FieldT updated_root_value = test_merkle_tree->get_root();
    std::vector<merkle_authentication_node> path = test_merkle_tree->get_path(address_commitment);

    // JS Inputs
    FZethNote<FieldT>note_input1(
        a_pk,
        FieldT("100"), // value associated with the commitment cm_bits256
        rho,
        r_trap,
        r_mask
    );
    FZethNote<FieldT>note_input2(
        a_pk,
        FieldT("0000000000000000"),
        rho,
        r_trap,
        r_mask
    );
    FJSInput input1(
        path,
        address_commitment,
        get_bitsAddr_from_vector(address_bits),
        note_input1,
        a_sk,
        nf
    );
    // We keep the same path and address as the previous commitment
    // We don't care since this coin is zero-valued and the merkle auth path check
    // Doesn't count in such case
    FJSInput input2(
        path,
        address_commitment,
        get_bitsAddr_from_vector(address_bits),
        note_input2,
        a_sk,
        nf
    );
    std::array<FJSInput, 2> inputs;
    inputs[0] = input1;
    inputs[1] = input2;
    libff::leave_block("[END] Create JSInput", true);

    libff::enter_block("[BEGIN] Create JSOutput/ZethNote", true);
    /*
    bits256 a_pk_out_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("7777f753bfe21ba2219ced74875b8dbd8c114c3c79d7e41306dd82118de1895b"));
    bits256 rho_out_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("1111000000000000000000000000000000000000000000000000000000009777"));
    bits384 trap_r_out_bits384 = get_bits384_from_vector(hexadecimal_str_to_binary_vector("11000000000000990000000000000099000000000000007700000000000000FF00000000000000FF0000000000000777"));
    */

    FieldT a_sk_out = FieldT("4047591473000155590199171927915978796573140621771266280705379796913645161555"); // mimic_hash([-1], sha3("Clearmatics"))
    FieldT a_pk_out = FieldT("1570964467829526078663761333427980033315285096730121923127414543972819455975");
    FieldT r_trap_out = FieldT("3121287842287349864642297846963883646477840388236905026425392648441319037621"); // mimic_hash([-2], sha3("Clearmatics"))
    FieldT r_mask_out = FieldT("19253552993580840822052270218222026004974786985939386817973146641031673406634"); // mimic_hash([-3], sha3("Clearmatics"))
    FieldT rho_out = FieldT("10448869983030339500740742410361707713409326656173533049846269061232406471931"); // mimic_hash([-4], sha3("Clearmatics"))
    FieldT value_out_1 = FieldT("70"); // to change to 1A00000000000002
    FieldT value_out_2 = FieldT("20"); // to change to 1500000000000002



    FZethNote<FieldT>note_output1(
        a_pk_out,
        value_out_1,
        rho_out,
        r_trap_out,
        r_mask_out
    );
    FZethNote<FieldT>note_output2(
        a_pk_out,
        value_out_2,
        rho_out,
        r_trap_out,
        r_mask_out
    );
    std::array<FZethNote<FieldT>, 2> outputs;
    outputs[0] = note_output1;
    outputs[1] = note_output2;
    libff::leave_block("[END] Create JSOutput/ZethNote", true);

    libff::enter_block("[BEGIN] Generate proof", true);
    // RHS = 0x1A00000000000002 + 0x1500000000000002 + 0x000000000000000B = 2F0000000000000F (LHS) so far 100 = 70 + 20 + 10
    extended_proof<ppT> ext_proof = prover.prove(
        updated_root_value,
        inputs,
        outputs,
        FieldT("0000000000000000"), // vpub_in = 0x0
        FieldT("10"), // to change to vpub_out = 000000000000000B
        keypair.pk
    );
    libff::leave_block("[END] Generate proof", true);

    libff::enter_block("[BEGIN] Verify proof", true);
    // Get the verification key
    libzeth::verificationKeyT<ppT> vk = keypair.vk;
    bool res = libzeth::verify(ext_proof, vk);
    libff::leave_block("[END] Verify proof", true);

    return res;
}


bool TestValidJS2In2Case3(
    CircuitWrapper<FieldT, HashT,2, 2> &prover,
    libzeth::keyPairT<ppT> keypair
) {
    // --- General setup for the tests --- //
    libff::print_header("Starting test: IN => v_pub = 0x0000000000000010, note1 = 0x2F0000000000000F, note2 = 0x0 || OUT => v_pub = 0x000000000000000B, note1 = 0x1A00000000000012, note2 = 0x1500000000000002");

    libff::enter_block("[START] Instantiate merkle tree for the tests", true);
    // Create a merkle tree to run our tests
    // Note: `make_unique` should be C++14 compliant, but here we use c++11, so we instantiate our unique_ptr manually
    std::unique_ptr<merkle_tree<FieldT, HashT>> test_merkle_tree = std::unique_ptr<merkle_tree<FieldT, HashT>>(
        new merkle_tree<FieldT, HashT>(
            ZETH_MERKLE_TREE_DEPTH
            )
    );
    libff::leave_block("[END] Instantiate merkle tree for the tests", true);

    // --- Test 1: Generate a valid proof for commitment inserted at address 1 -- //
    libff::enter_block("[BEGIN] Create JSInput", true);
    // Create the zeth note data for the commitment we will insert in the tree (commitment to spend in this test)
    /*
    bits384 trap_r_bits384 = get_bits384_from_vector(hexadecimal_str_to_binary_vector("0F000000000000FF00000000000000FF00000000000000FF00000000000000FF00000000000000FF00000000000000FF"));
    bits256 a_sk_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("FF0000000000000000000000000000000000000000000000000000000000000F"));
    bits256 rho_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("FFFF000000000000000000000000000000000000000000000000000000009009"));
    bits256 a_pk_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("6461f753bfe21ba2219ced74875b8dbd8c114c3c79d7e41306dd82118de1895b"));
    bits256 nf_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("69f12603c2cfb2acf6f80a8f72cbdeb4417a6b8c7290e793c4d22830c4b35c5f"));

    // We compute the commitment to insert it in the merkle tree we use for the tests
    bits256 cm_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("823d19485c94f74b4739ba7d17e4b434693086a996fa2e8d1438a91b1c220331"));
    libff::bit_vector address_bits = {1, 0, 0, 0}; // 4 being the value of ZETH_MERKLE_TREE_DEPTH
    const size_t address_commitment = 1;
    */

    FieldT r_trap = FieldT("6576838732374919021860119342200398901974877797242970520445052250557344565821"); 
    FieldT r_mask = FieldT("12946791413528024759839394340318236878559158148001437182189040772047964059643"); 
    FieldT a_sk = FieldT("18834251028175908666459239027856614524890385928194459012149074634190864282942");
    FieldT a_pk = FieldT("7557284417121959865806721773315070721481656811910187747772194493889443679515");
    FieldT rho = FieldT("1050857222747641138499486657636501128934992461797459043826425853190223953952");
    FieldT nf = FieldT("16718990446402804328289195783834079890551903281540894658644001381677667818395");
    FieldT value = FieldT("100"); // 3386706919782613007 is 2F0000000000000F in decimal
    FieldT cm = FieldT("21075862836612025410946586494502715375244302209109240154340545056954086062438");
    libff::bit_vector address_bits = {1, 0, 0, 0}; // 4 being the value of ZETH_MERKLE_TREE_DEPTH
    const size_t address_commitment = 1;



    // We insert the commitment to the zeth note in the merkle tree
    test_merkle_tree->set_value(address_commitment, cm);
    FieldT updated_root_value = test_merkle_tree->get_root();
    std::vector<merkle_authentication_node> path = test_merkle_tree->get_path(address_commitment);

    // JS Inputs
    FZethNote<FieldT> note_input1(
        a_pk,
        FieldT("100"), // value associated with the commitment cm_bits256
        rho,
        r_trap,
        r_mask
    );
    FZethNote<FieldT> note_input2(
        a_pk,
        FieldT("0000000000000000"),
        rho,
        r_trap,
        r_mask
    );
    FJSInput input1(
        path,
        address_commitment,
        get_bitsAddr_from_vector(address_bits),
        note_input1,
        a_sk,
        nf
    );
    // We keep the same path and address as the previous commitment
    // We don't care since this coin is zero-valued and the merkle auth path check
    // Doesn't count in such case
    FJSInput input2(
        path,
        address_commitment,
        get_bitsAddr_from_vector(address_bits),
        note_input2,
        a_sk,
        nf
    );
    std::array<FJSInput, 2> inputs;
    inputs[0] = input1;
    inputs[1] = input2;
    libff::leave_block("[END] Create JSInput", true);

    libff::enter_block("[BEGIN] Create JSOutput/ZethNote", true);
    FieldT a_sk_out = FieldT("4047591473000155590199171927915978796573140621771266280705379796913645161555"); // mimic_hash([-1], sha3("Clearmatics"))
    FieldT a_pk_out = FieldT("1570964467829526078663761333427980033315285096730121923127414543972819455975");
    FieldT r_trap_out = FieldT("3121287842287349864642297846963883646477840388236905026425392648441319037621"); // mimic_hash([-2], sha3("Clearmatics"))
    FieldT r_mask_out = FieldT("19253552993580840822052270218222026004974786985939386817973146641031673406634"); // mimic_hash([-3], sha3("Clearmatics"))
    FieldT rho_out = FieldT("10448869983030339500740742410361707713409326656173533049846269061232406471931"); // mimic_hash([-4], sha3("Clearmatics"))
    FieldT value_out_1 = FieldT("70"); // to change to 1A00000000000002
    FieldT value_out_2 = FieldT("20"); // to change to 1500000000000002

    FZethNote<FieldT> note_output1(
        a_pk_out,
        value_out_1,
        rho_out,
        r_trap_out,
        r_mask_out
    );
    FZethNote<FieldT> note_output2(
        a_pk_out,
        value_out_2,
        rho_out,
        r_trap_out,
        r_mask_out
    );

    std::array<FZethNote<FieldT>, 2> outputs;
    outputs[0] = note_output1;
    outputs[1] = note_output2;
    libff::leave_block("[END] Create JSOutput/ZethNote", true);

    libff::enter_block("[BEGIN] Generate proof", true);
    // (RHS) 0x1A00000000000012 + 0x1500000000000002 + 0x000000000000000B = 2F0000000000000F + 0x0000000000000010 + 0x0 (LHS)
    extended_proof<ppT> ext_proof = prover.prove(
        updated_root_value,
        inputs,
        outputs,
        FieldT("10"), // v_pub_in = 0x0000000000000010
        FieldT("20"), // v_pub_out = 0x000000000000000B
        keypair.pk
    );
    libff::leave_block("[END] Generate proof", true);

    libff::enter_block("[BEGIN] Verify proof", true);
    // Get the verification key
    libzeth::verificationKeyT<ppT> vk = keypair.vk;
    bool res = libzeth::verify(ext_proof, vk);
    libff::leave_block("[END] Verify proof", true);

    return res;
}


bool TestValidJS2In2Deposit(
    CircuitWrapper<FieldT, HashT, 2, 2> &prover,
    libzeth::keyPairT<ppT> keypair
) {
    // --- General setup for the tests --- //
    libff::print_header("Starting test: IN => v_pub = 0x6124FEE993BC0000, note1 = 0x0, note2 = 0x0 || OUT => v_pub = 0x0, note1 = 0x3782DACE9D900000, note2 = 0x29A2241AF62C0000");

    libff::enter_block("[START] Instantiate merkle tree for the tests", true);
    // Create a merkle tree to run our tests
    // Note: `make_unique` should be C++14 compliant, but here we use c++11, so we instantiate our unique_ptr manually
    std::unique_ptr<merkle_tree<FieldT, HashT>> test_merkle_tree = std::unique_ptr<merkle_tree<FieldT, HashT>>(
        new merkle_tree<FieldT, HashT>(
            ZETH_MERKLE_TREE_DEPTH
            )
    );
    libff::leave_block("[END] Instantiate merkle tree for the tests", true);

    // --- Test 1: Generate a valid proof for commitment inserted at address 1 -- //
    libff::enter_block("[BEGIN] Create JSInput", true);
    // Create the zeth note data for the commitment we will insert in the tree (commitment to spend in this test)
    /*
    bits384 trap_r_bits384 = get_bits384_from_vector(hexadecimal_str_to_binary_vector("0F000000000000FF00000000000000FF00000000000000FF00000000000000FF00000000000000FF00000000000000FF"));
    bits256 a_sk_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("FF0000000000000000000000000000000000000000000000000000000000000F"));
    bits256 rho_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("FFFF000000000000000000000000000000000000000000000000000000009009"));
    bits256 a_pk_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("6461f753bfe21ba2219ced74875b8dbd8c114c3c79d7e41306dd82118de1895b"));
    bits256 nf_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("69f12603c2cfb2acf6f80a8f72cbdeb4417a6b8c7290e793c4d22830c4b35c5f"));

    // We compute a commitment to insert it in the merkle tree we use for the tests
    bits256 cm_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("823d19485c94f74b4739ba7d17e4b434693086a996fa2e8d1438a91b1c220331"));
    libff::bit_vector address_bits = {1, 0, 0, 0}; // 4 being the value of ZETH_MERKLE_TREE_DEPTH
    const size_t address_commitment = 1;
    */

    FieldT r_trap = FieldT("6576838732374919021860119342200398901974877797242970520445052250557344565821"); 
    FieldT r_mask = FieldT("12946791413528024759839394340318236878559158148001437182189040772047964059643"); 
    FieldT a_sk = FieldT("18834251028175908666459239027856614524890385928194459012149074634190864282942");
    FieldT a_pk = FieldT("7557284417121959865806721773315070721481656811910187747772194493889443679515");
    FieldT rho = FieldT("1050857222747641138499486657636501128934992461797459043826425853190223953952");
    FieldT nf = FieldT("16718990446402804328289195783834079890551903281540894658644001381677667818395");
    FieldT value = FieldT("0000000000000000"); // 3386706919782613007 is 2F0000000000000F in decimal
    FieldT cm = FieldT("19331960042315155497530170019921951736965260654332349247509836147634461830589");
    libff::bit_vector address_bits = {1, 0, 0, 0}; // 4 being the value of ZETH_MERKLE_TREE_DEPTH
    const size_t address_commitment = 1;

    // We insert the commitment to the zeth note in the merkle tree
    test_merkle_tree->set_value(address_commitment, cm);
    FieldT updated_root_value = test_merkle_tree->get_root();
    std::vector<merkle_authentication_node> path = test_merkle_tree->get_path(address_commitment);

    // JS Inputs
    FZethNote<FieldT> note_input1(
        a_pk,
        value,
        rho,
        r_trap,
        r_mask
    );
    FZethNote<FieldT> note_input2(
        a_pk,
        value,
        rho,
        r_trap,
        r_mask
    );
    FJSInput input1(
        path,
        address_commitment,
        get_bitsAddr_from_vector(address_bits),
        note_input1,
        a_sk,
        nf
    );
    // We keep the same path and address as the previous commitment
    // We don't care since this coin is zero-valued and the merkle auth path check
    // Doesn't count in such case

    FJSInput input2(
        path,
        address_commitment,
        get_bitsAddr_from_vector(address_bits),
        note_input2,
        a_sk,
        nf
    );
    std::array<FJSInput, 2> inputs;
    inputs[0] = input1;
    inputs[1] = input2;
    libff::leave_block("[END] Create JSInput", true);

    libff::enter_block("[BEGIN] Create JSOutput/ZethNote", true);
    /*
    bits256 a_pk_out_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("7777f753bfe21ba2219ced74875b8dbd8c114c3c79d7e41306dd82118de1895b"));
    bits256 rho_out_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("1111000000000000000000000000000000000000000000000000000000009777"));
    bits384 trap_r_out_bits384 = get_bits384_from_vector(hexadecimal_str_to_binary_vector("11000000000000990000000000000099000000000000007700000000000000FF00000000000000FF0000000000000777"));
    */
    FieldT a_sk_out = FieldT("4047591473000155590199171927915978796573140621771266280705379796913645161555"); // mimic_hash([-1], sha3("Clearmatics"))
    FieldT a_pk_out = FieldT("1570964467829526078663761333427980033315285096730121923127414543972819455975");
    FieldT r_trap_out = FieldT("3121287842287349864642297846963883646477840388236905026425392648441319037621"); // mimic_hash([-2], sha3("Clearmatics"))
    FieldT r_mask_out = FieldT("19253552993580840822052270218222026004974786985939386817973146641031673406634"); // mimic_hash([-3], sha3("Clearmatics"))
    FieldT rho_out = FieldT("10448869983030339500740742410361707713409326656173533049846269061232406471931"); // mimic_hash([-4], sha3("Clearmatics"))
    FieldT value_out_1 = FieldT("80"); // to change to 1A00000000000002
    FieldT value_out_2 = FieldT("20"); // to change to 1500000000000002

    FZethNote<FieldT> note_output1(
        a_pk_out,
        value_out_1,
        rho_out,
        r_trap_out,
        r_mask_out
    );
    FZethNote<FieldT> note_output2(
        a_pk_out,
        value_out_2,
        rho_out,
        r_trap_out,
        r_mask_out
    );
    std::array<FZethNote<FieldT>, 2> outputs;
    outputs[0] = note_output1;
    outputs[1] = note_output2;
    libff::leave_block("[END] Create JSOutput/ZethNote", true);

    libff::enter_block("[BEGIN] Generate proof", true);
    // RHS = 0x0 + 0x3782DACE9D900000 + 0x29A2241AF62C0000 = 0x6124FEE993BC0000 (LHS)
    extended_proof<ppT> ext_proof = prover.prove(
        updated_root_value,
        inputs,
        outputs,
        FieldT("100"), // v_pub_in = 0x6124FEE993BC0000
        FieldT("00"), // v_pub_out = 0x000000000000000B
        keypair.pk
    );
    libff::leave_block("[END] Generate proof", true);

    libff::enter_block("[BEGIN] Verify proof", true);
    // Get the verification key
    libzeth::verificationKeyT<ppT> vk = keypair.vk;
    bool res = libzeth::verify(ext_proof, vk);

    ext_proof.dump_primary_inputs();
    libff::leave_block("[END] Verify proof", true);

    return res;
}


bool TestInvalidJS2In2(
    CircuitWrapper<FieldT, HashT, 2, 2> &prover,
    libzeth::keyPairT<ppT> keypair
) {
    // --- General setup for the tests --- //
    libff::print_header("Starting test: IN => v_pub = 0xFA80001400000000, note1 = 0x0, note2 = 0x0 || OUT => v_pub = 0x0, note1 = 0x8530000A00000001, note2 = 0x7550000A00000000");

    libff::enter_block("[START] Instantiate merkle tree for the tests", true);
    // Create a merkle tree to run our tests
    // Note: `make_unique` should be C++14 compliant, but here we use c++11, so we instantiate our unique_ptr manually
    std::unique_ptr<merkle_tree<FieldT, HashT>> test_merkle_tree = std::unique_ptr<merkle_tree<FieldT, HashT>>(
        new merkle_tree<FieldT, HashT>(
            ZETH_MERKLE_TREE_DEPTH
            )
    );
    libff::leave_block("[END] Instantiate merkle tree for the tests", true);

    // --- Test 1: Generate a valid proof for commitment inserted at address 1 -- //
    libff::enter_block("[BEGIN] Create JSInput", true);
    // Create the zeth note data for the commitment we will insert in the tree (commitment to spend in this test)
    /*
    bits384 trap_r_bits384 = get_bits384_from_vector(hexadecimal_str_to_binary_vector("0F000000000000FF00000000000000FF00000000000000FF00000000000000FF00000000000000FF00000000000000FF"));
    bits256 a_sk_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("FF0000000000000000000000000000000000000000000000000000000000000F"));
    bits256 rho_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("FFFF000000000000000000000000000000000000000000000000000000009009"));
    bits256 a_pk_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("6461f753bfe21ba2219ced74875b8dbd8c114c3c79d7e41306dd82118de1895b"));
    bits256 nf_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("69f12603c2cfb2acf6f80a8f72cbdeb4417a6b8c7290e793c4d22830c4b35c5f"));

    // We compute a commitment to insert it in the merkle tree we use for the tests
    bits256 cm_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("823d19485c94f74b4739ba7d17e4b434693086a996fa2e8d1438a91b1c220331"));
    libff::bit_vector address_bits = {1, 0, 0, 0}; // 4 being the value of ZETH_MERKLE_TREE_DEPTH
    const size_t address_commitment = 1;
    */

    FieldT r_trap = FieldT("6576838732374919021860119342200398901974877797242970520445052250557344565821"); 
    FieldT r_mask = FieldT("12946791413528024759839394340318236878559158148001437182189040772047964059643"); 
    FieldT a_sk = FieldT("18834251028175908666459239027856614524890385928194459012149074634190864282942");
    FieldT a_pk = FieldT("7557284417121959865806721773315070721481656811910187747772194493889443679515");
    FieldT rho = FieldT("1050857222747641138499486657636501128934992461797459043826425853190223953952");
    FieldT nf = FieldT("16718990446402804328289195783834079890551903281540894658644001381677667818395");
    FieldT value = FieldT("0000000000000000"); // 3386706919782613007 is 2F0000000000000F in decimal
    FieldT cm = FieldT("19331960042315155497530170019921951736965260654332349247509836147634461830589");
    libff::bit_vector address_bits = {1, 0, 0, 0}; // 4 being the value of ZETH_MERKLE_TREE_DEPTH
    const size_t address_commitment = 1;


    // We insert the commitment to the zeth note in the merkle tree
    test_merkle_tree->set_value(address_commitment, cm);
    FieldT updated_root_value = test_merkle_tree->get_root();
    std::vector<merkle_authentication_node> path = test_merkle_tree->get_path(address_commitment);

    // JS Inputs
    FZethNote<FieldT> note_input1(
        a_pk,
        FieldT("0000000000000000"),
        rho,
        r_trap,
        r_mask
    );
    FZethNote<FieldT> note_input2(
        a_pk,
        FieldT("0000000000000000"),
        rho,
        r_trap,
        r_mask
    );

    FJSInput input1(
        path,
        address_commitment,
        get_bitsAddr_from_vector(address_bits),
        note_input1,
        a_sk,
        nf
    );
    // We keep the same path and address as the previous commitment
    // We don't care since this coin is zero-valued and the merkle auth path check
    // Doesn't count in such case

    FJSInput input2(
        path,
        address_commitment,
        get_bitsAddr_from_vector(address_bits),
        note_input2,
        a_sk,
        nf
    );
    std::array<FJSInput, 2> inputs;
    inputs[0] = input1;
    inputs[1] = input2;
    libff::leave_block("[END] Create JSInput", true);

    libff::enter_block("[BEGIN] Create JSOutput/ZethNote", true);
    /*
    bits256 a_pk_out_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("7777f753bfe21ba2219ced74875b8dbd8c114c3c79d7e41306dd82118de1895b"));
    bits256 rho_out_bits256 = get_bits256_from_vector(hexadecimal_digest_to_binary_vector("1111000000000000000000000000000000000000000000000000000000009777"));
    bits384 trap_r_out_bits384 = get_bits384_from_vector(hexadecimal_str_to_binary_vector("11000000000000990000000000000099000000000000007700000000000000FF00000000000000FF0000000000000777"));
    */
    FieldT a_sk_out = FieldT("4047591473000155590199171927915978796573140621771266280705379796913645161555"); // mimic_hash([-1], sha3("Clearmatics"))
    FieldT a_pk_out = FieldT("1570964467829526078663761333427980033315285096730121923127414543972819455975");
    FieldT r_trap_out = FieldT("3121287842287349864642297846963883646477840388236905026425392648441319037621"); // mimic_hash([-2], sha3("Clearmatics"))
    FieldT r_mask_out = FieldT("19253552993580840822052270218222026004974786985939386817973146641031673406634"); // mimic_hash([-3], sha3("Clearmatics"))
    FieldT rho_out = FieldT("10448869983030339500740742410361707713409326656173533049846269061232406471931"); // mimic_hash([-4], sha3("Clearmatics"))
    FieldT value_out_1 = FieldT("80"); // to change to 1A00000000000002
    FieldT value_out_2 = FieldT("70"); // to change to 1500000000000002


    FZethNote<FieldT> note_output1(
        a_pk_out,
        value_out_1, // 0x8530000A00000000 = 9.597170848876199937 ETH
        rho_out,
        r_trap_out,
        r_mask_out
    );
    FZethNote<FieldT> note_output2(
        a_pk_out,
        value_out_2, // 0x7550000A00000000 = 8.453256543524093952 ETH
        rho_out,
        r_trap_out,
        r_mask_out
    );
    std::array<FZethNote<FieldT>, 2> outputs;
    outputs[0] = note_output1;
    outputs[1] = note_output2;
    libff::leave_block("[END] Create JSOutput/ZethNote", true);

    libff::enter_block("[BEGIN] Generate proof", true);
    // LHS = 0xFA80001400000000 (18.050427392400293888 ETH) =/= 0x8530000A00000001 (9.597170848876199937 ETH) + 0x7550000A00000000 (8.453256543524093952 ETH) = RHS
    // LHS = 18.050427392400293888 ETH
    // RHS = 18.050427392400293889 ETH (1 wei higher than LHS)
    extended_proof<ppT> ext_proof = prover.prove(
        updated_root_value,
        inputs,
        outputs,
        FieldT("100"), // vpub_in = 0xFA80001400000000 = 18.050427392400293888 ETH
        FieldT("0000000000000000"), // vpub_out = 0x0
        keypair.pk
    );
    libff::leave_block("[END] Generate proof", true);

    libff::enter_block("[BEGIN] Verify proof", true);
    // Get the verification key
    libzeth::verificationKeyT<ppT> vk = keypair.vk;
    bool res = libzeth::verify(ext_proof, vk);
    libff::leave_block("[END] Verify proof", true);

    return res;
}


TEST(MainTests, ProofGenAndVerifJS2to2) {
    // Run the trusted setup once for all tests, and keep the keypair in memory for the duration of the tests
    CircuitWrapper<FieldT, HashT, 2, 2> proverJS2to2;
    libzeth::keyPairT<ppT> keypair = proverJS2to2.generate_trusted_setup();
    bool res = false;

    res = TestValidJS2In2Case1(proverJS2to2, keypair);
    std::cout << "[TestValidJS2In2Case1] Expected (True), Obtained result: " << res << std::endl;
    ASSERT_TRUE(res);

    
    res = TestValidJS2In2Case2(proverJS2to2, keypair);
    std::cout << "[TestValidJS2In2Case2] Expected (True), Obtained result: " << res << std::endl;
    ASSERT_TRUE(res);
    
    res = TestValidJS2In2Case3(proverJS2to2, keypair);
    std::cout << "[TestValidJS2In2Case3] Expected (True), Obtained result: " << res << std::endl;
    ASSERT_TRUE(res);

    res = TestValidJS2In2Deposit(proverJS2to2, keypair);
    std::cout << "[TestValidJS2In2Deposit] Expected (True), Obtained result: " << res << std::endl;
    ASSERT_TRUE(res);

    
    // The following test is expected to throw an exception because the LHS =/= RHS
    try {
        res = TestInvalidJS2In2(proverJS2to2, keypair);
        std::cout << "[TestValidJS2In2Deposit] Expected (False), Obtained result: " << res << std::endl;
        ASSERT_TRUE(res);
    } catch (const std::invalid_argument& e) {
	  std::cerr << "Invalid argument exception: " << e.what() << '\n';
    }
    
}

} // namespace

int main(int argc, char **argv) {
    ppT::init_public_params(); // /!\ WARNING: Do once for all tests. Do not forget to do this !!!!
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
