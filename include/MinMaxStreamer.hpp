#pragma once

#include <Partition.hpp>
#include <future>
#include <unordered_set>
#include <vector>

namespace mmstream {

class MinMaxStreamer
{

public:
    MinMaxStreamer(std::size_t partitions);

    auto parseGraph(const std::string& path,
                    double balancing)
        -> void;

    auto addElement(uint64_t node,
                    const std::vector<uint64_t>& edges,
                    double balancing)
        -> void;

    auto getPartitions() const
        -> const std::vector<Partition>&;

private:
    std::vector<Partition> partitions_;
};

} // namespace mmstream
