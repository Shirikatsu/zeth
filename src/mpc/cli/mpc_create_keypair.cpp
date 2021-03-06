// Copyright (c) 2015-2019 Clearmatics Technologies Ltd
//
// SPDX-License-Identifier: LGPL-3.0+

#include "snarks/groth16/mpc/phase2.hpp"
// This comment preserves include order under clang-format.
#include "circuits/blake2s/blake2s_comp.hpp"
#include "mpc_common.hpp"
#include "snarks/groth16/mpc/mpc_utils.hpp"
#include "snarks/groth16/mpc/powersoftau_utils.hpp"
#include "util.hpp"

#include <vector>

using namespace libzeth;
namespace po = boost::program_options;

namespace
{

// Usage:
//  mpc create-keypair [<option>]
//      <powersoftau_file>
//      <linear_combination_file>
//      <phase2_challenge_file>
//      <keypair_output_file>
//
// Options:
//  -h,--help           This message
//  --pot-degree        powersoftau degree (assumed to match linear comb)
class mpc_create_keypair : public subcommand
{
private:
    std::string powersoftau_file;
    std::string lin_comb_file;
    std::string phase2_challenge_file;
    std::string keypair_out_file;
    size_t powersoftau_degree;

public:
    mpc_create_keypair()
        : subcommand("create-keypair", "Create a full keypair from MPC output")
        , powersoftau_file()
        , lin_comb_file()
        , phase2_challenge_file()
        , keypair_out_file()
        , powersoftau_degree(0)
    {
    }

private:
    void initialize_suboptions(
        po::options_description &options,
        po::options_description &all_options,
        po::positional_options_description &pos) override
    {
        options.add_options()(
            "pot-degree",
            po::value<size_t>(),
            "powersoftau degree (assumed to match linear comb)");
        all_options.add(options).add_options()(
            "powersoftau_file", po::value<std::string>(), "powersoftau file")(
            "linear_combination_file",
            po::value<std::string>(),
            "linear combination file")(
            "phase2_challenge_file",
            po::value<std::string>(),
            "phase2 final challenge file")(
            "keypair_out_file",
            po::value<std::string>(),
            "keypair output file");
        pos.add("powersoftau_file", 1)
            .add("linear_combination_file", 1)
            .add("phase2_challenge_file", 1)
            .add("keypair_out_file", 1);
    }

    void parse_suboptions(const po::variables_map &vm) override
    {
        if (0 == vm.count("powersoftau_file")) {
            throw po::error("powersoftau_file not specified");
        }
        if (0 == vm.count("linear_combination_file")) {
            throw po::error("linear_combination_file not specified");
        }
        if (0 == vm.count("phase2_challenge_file")) {
            throw po::error("phase2_challenge_file not specified");
        }
        if (0 == vm.count("keypair_out_file")) {
            throw po::error("keypair_out_file not specified");
        }

        powersoftau_file = vm["powersoftau_file"].as<std::string>();
        lin_comb_file = vm["linear_combination_file"].as<std::string>();
        phase2_challenge_file = vm["phase2_challenge_file"].as<std::string>();
        keypair_out_file = vm["keypair_out_file"].as<std::string>();
        powersoftau_degree =
            vm.count("pot-degree") ? vm["pot-degree"].as<size_t>() : 0;
    }

    void subcommand_usage() override
    {
        std::cout << "Usage:\n"
                  << "  " << subcommand_name << " [<options>]  \\\n"
                  << "        <powersoftau_file> <linear_combination_file> \\\n"
                  << "        <phase2_challenge_file> <keypair_out_file>\n\n";
    }

    int execute_subcommand() override
    {
        if (verbose) {
            std::cout << "powersoftau_file: " << powersoftau_file << "\n"
                      << "lin_comb_file: " << lin_comb_file << "\n"
                      << "phase2_challenge_file: " << phase2_challenge_file
                      << "\n"
                      << "powersoftau_degree: " << powersoftau_degree << "\n"
                      << "out_file: " << keypair_out_file << std::endl;
        }

        // Load all data
        // TODO: Load just degree from lin_comb data, then load everything
        // in parallel.
        libff::enter_block("Load linear combination data");
        libff::print_indent();
        std::cout << lin_comb_file << std::endl;
        srs_mpc_layer_L1<ppT> lin_comb =
            read_from_file<srs_mpc_layer_L1<ppT>>(lin_comb_file);
        libff::leave_block("Load linear combination data");

        libff::enter_block("Load powers of tau");
        libff::print_indent();
        std::cout << powersoftau_file << std::endl;
        srs_powersoftau<ppT> pot = [this, &lin_comb]() {
            std::ifstream in(
                powersoftau_file, std::ios_base::binary | std::ios_base::in);
            const size_t pot_degree =
                powersoftau_degree ? powersoftau_degree : lin_comb.degree();
            return powersoftau_load(in, pot_degree);
        }();
        libff::leave_block("Load powers of tau");

        libff::enter_block("Load phase2 data");
        libff::print_indent();
        std::cout << phase2_challenge_file << std::endl;
        srs_mpc_phase2_challenge<ppT> phase2 =
            read_from_file<srs_mpc_phase2_challenge<ppT>>(
                phase2_challenge_file);
        libff::leave_block("Load phase2 data");

        // Compute circuit
        libff::enter_block("Generate QAP");
        libsnark::protoboard<FieldT> pb;
        init_protoboard(pb);
        libsnark::r1cs_constraint_system<FieldT> cs =
            pb.get_constraint_system();
        const libsnark::qap_instance<FieldT> qap =
            libsnark::r1cs_to_qap_instance_map(cs, true);
        libff::leave_block("Generate QAP");

        libsnark::r1cs_gg_ppzksnark_keypair<ppT> keypair =
            mpc_create_key_pair<ppT>(
                std::move(pot),
                std::move(lin_comb),
                std::move(phase2.accumulator),
                std::move(cs),
                qap);

        // Write keypair to a file
        libff::enter_block("Writing keypair file");
        if (!libff::inhibit_profiling_info) {
            libff::print_indent();
            std::cout << keypair_out_file << std::endl;
        }
        {
            std::ofstream out(
                keypair_out_file, std::ios_base::binary | std::ios_base::out);
            mpc_write_keypair(out, keypair);
        }
        libff::leave_block("Writing keypair file");

        return 0;
    }
};

} // namespace

// Subcommand instance
subcommand *mpc_create_keypair_cmd = new mpc_create_keypair();
