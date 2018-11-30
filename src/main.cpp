#include <MinMaxStreamer.hpp>
#include <Partition.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>

auto main(int argc, char const* argv[])
    -> int
{
    namespace po = boost::program_options;

    po::options_description description{"HYPE - Hypergraph Partitioner using Neighbourhood Heuristics"};


    // clang-format off
    description.add_options()
        ("help,h", "display this help message")

        ("raw,r",
         po::bool_switch()->default_value(false),
         "output raw numbers to make it easier to redirect output into files")

        ("input,i",
         po::value<std::string>(),
         "input hypergraph file")

        ("balancing,b",
         po::value<double>()->default_value(0.05),
         "a value between 1 and 0, to which unbalancing will be bound")

        ("partitions,p",
         po::value<std::size_t>(),
         "number of partitions");
    // clang-format on

    po::variables_map vm{};
    po::store(po::command_line_parser(argc, argv).options(description).run(), vm);
    po::notify(vm);

    if(!vm.count("input")
       || !vm.count("partitions")
       || !vm.count("balancing")
       || vm.count("help")) {

        std::cout << description;
        return 0;
    }

    auto input_path = vm["input"].as<std::string>();
    auto partitions = vm["partitions"].as<std::size_t>();
    auto balancing = vm["balancing"].as<double>();
    auto raw = vm["raw"].as<bool>();

    if(!raw) {
        std::cout << "----------------------------------------------------------------------------\n"
                  << "Partitioning Graph: "
                  << input_path
                  << "\n"
                  << "into "
                  << partitions
                  << " partitions\n"
                  << "\n"
                  << "\nStarting Hypergraph Streaming"
                  << "\n"
                  << "----------------------------------------------------------------------------\n";
    }

    auto begin = std::chrono::steady_clock::now();

    mmstream::MinMaxStreamer streamer{partitions};

    streamer.parseGraph(input_path, balancing);

    auto parts = std::move(streamer.getPartitions());

    auto end = std::chrono::steady_clock::now();

    auto partitioning_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
            .count();

    //start parallel futures
    auto soed_fut = mmstream::getSumOfExteralDegrees(parts);
    auto vtx_balance_fut = mmstream::getVertexBalancing(parts);
    auto edge_balance_fut = mmstream::getEdgeBalancing(parts);
    auto edge_cut_fut = mmstream::getHyperedgeCut(parts);
    auto k_minus_1_fut = mmstream::getKminus1Metric(parts);

    //wait for all results
    auto soed = soed_fut.get();
    auto vtx_balance = vtx_balance_fut.get();
    auto edge_balance = edge_balance_fut.get();
    auto edge_cut = edge_cut_fut.get();
    auto k_minus_1 = k_minus_1_fut.get();

    if(raw) {
        std::cout << partitions
                  << "\t\t"
                  << soed
                  << "\t\t"
                  << vtx_balance
                  << "\t\t"
                  << edge_balance
                  << "\t\t"
                  << edge_cut
                  << "\t\t"
                  << k_minus_1
                  << "\t\t"
                  << partitioning_time
                  << std::endl;

    } else {
        std::cout << "sum of external degrees: " << soed << "\n"
                  << "Hyperedges cut: " << edge_cut << "\n"
                  << "K-1: " << k_minus_1 << "\n"
                  << "node balancing: " << vtx_balance << "\n"
                  << "edge balancing: " << edge_balance << "\n"
                  << "streaming time: " << partitioning_time << "\n"
                  << std::endl;
    }
}
