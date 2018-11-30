#pragma once

#include <future>
#include <unordered_set>
#include <vector>

namespace mmstream {

class Partition
{
public:
    Partition(Partition&&) = default;
    Partition(const Partition&) = default;
    Partition() = delete;
    Partition(std::size_t);

    auto operator=(Partition &&)
        -> Partition& = default;
    auto operator=(const Partition&)
        -> Partition& = default;

    auto addNode(uint64_t node,
                 const std::vector<uint64_t>& edges)
        -> void;
    auto addNode(uint64_t node,
                 const std::unordered_set<uint64_t>& edges)
        -> void;
    auto commonTopics(const std::vector<uint64_t>& edges)
        -> std::int64_t;
    auto hasEdge(uint64_t edge) const
        -> bool;
    auto getNodes() const
        -> const std::unordered_set<uint64_t>&;
    auto getNodes()
        -> std::unordered_set<uint64_t>&;
    auto getEdges() const
        -> const std::unordered_set<uint64_t>&;
    auto getEdges()
        -> std::unordered_set<uint64_t>&;
    auto clear()
        -> void;
    auto getId() const
        -> std::size_t;
    auto numberOfEdges() const
        -> std::size_t;
    auto numberOfNodes() const
        -> std::size_t;
    auto externalDegree(const std::vector<Partition>& parts) const
        -> std::future<std::size_t>;

private:
    std::size_t id_;
    std::unordered_set<uint64_t> nodes_;
    std::unordered_set<uint64_t> edges_;
};

auto getSumOfExteralDegrees(const std::vector<Partition>& partitions)
    -> std::future<std::size_t>;

auto getHyperedgeCut(const std::vector<Partition>& partitions)
    -> std::future<std::size_t>;

auto getEdgeBalancing(const std::vector<Partition>& partitions)
    -> std::future<double>;

auto getVertexBalancing(const std::vector<Partition>& partitions)
    -> std::future<double>;

auto getKminus1Metric(const std::vector<Partition>& partitions)
    -> std::future<std::size_t>;

} // namespace mmstream
