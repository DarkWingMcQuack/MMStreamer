#include <Partition.hpp>
#include <algorithm>
#include <numeric>


mmstream::Partition::Partition(std::size_t id)
    : id_(id) {}

auto mmstream::Partition::getNodes() const
    -> const std::unordered_set<uint64_t>&
{
    return nodes_;
}

auto mmstream::Partition::getNodes()
    -> std::unordered_set<uint64_t>&
{
    return nodes_;
}

auto mmstream::Partition::getEdges() const
    -> const std::unordered_set<uint64_t>&
{
    return edges_;
}

auto mmstream::Partition::getEdges()
    -> std::unordered_set<uint64_t>&
{
    return edges_;
}

auto mmstream::Partition::getId() const
    -> std::size_t
{
    return id_;
}

auto mmstream::Partition::hasEdge(uint64_t elem) const
    -> bool
{
    auto iter = edges_.find(elem);
    return iter != edges_.end();
}

auto mmstream::Partition::addNode(uint64_t elem,
                                  const std::vector<uint64_t>& edges)
    -> void
{
    nodes_.insert(elem);
    edges_.insert(std::begin(edges),
                  std::end(edges));
}

auto mmstream::Partition::addNode(uint64_t elem,
                                  const std::unordered_set<uint64_t>& edges)
    -> void
{
    nodes_.insert(elem);
    edges_.insert(std::begin(edges),
                  std::end(edges));
}

auto mmstream::Partition::numberOfNodes() const
    -> std::size_t
{
    return nodes_.size();
}

auto mmstream::Partition::numberOfEdges() const
    -> std::size_t
{
    return edges_.size();
}

auto mmstream::Partition::clear()
    -> void
{
    nodes_.clear();
    edges_.clear();
}

auto mmstream::Partition::commonTopics(const std::vector<uint64_t>& edges)
    -> std::int64_t
{
    return std::count_if(std::cbegin(edges),
                         std::cend(edges),
                         [this](auto edge) {
                             auto iter = this->edges_.find(edge);
                             return iter != this->edges_.end();
                         });
}

auto mmstream::Partition::externalDegree(const std::vector<Partition>& parts) const
    -> std::future<std::size_t>
{
    return std::async(
        std::launch::async,
        [this, &parts]()
            -> std::size_t {
            return std::accumulate(std::cbegin(edges_),
                                   std::cend(edges_),
                                   0,
                                   [&parts, this](auto init, auto&& edge)
                                       -> std::size_t {
                                       for(auto&& part : parts) {
                                           if(part.getId() == getId())
                                               continue;
                                           if(part.hasEdge(edge))
                                               return init + 1;
                                       }
                                       return init;
                                   });
        });
}

auto mmstream::getSumOfExteralDegrees(const std::vector<Partition>& partitions)
    -> std::future<std::size_t>
{
    return std::async(std::launch::async,
                      [&partitions]() -> std::size_t {
                          //start futures
                          std::vector<std::future<std::size_t>> fut_vec;
                          for(auto&& part : partitions) {
                              fut_vec.emplace_back(part.externalDegree(partitions));
                          }
                          //collect futures and accumulate the results
                          return std::accumulate(std::begin(fut_vec),
                                                 std::end(fut_vec),
                                                 0,
                                                 [](auto init, auto&& fut) {
                                                     return init + fut.get();
                                                 });
                      });
}


auto mmstream::getHyperedgeCut(const std::vector<Partition>& partitions)
    -> std::future<std::size_t>
{
    return std::async(std::launch::async,
                      [&partitions]() -> std::size_t {
                          std::unordered_set<int64_t> edges_cut;
                          for(auto&& part : partitions) {
                              const auto& edges = part.getEdges();

                              for(auto&& edge : edges) {
                                  for(auto&& nested_part : partitions) {
                                      if(part.getId() == nested_part.getId()) {
                                          continue;
                                      }

                                      if(nested_part.hasEdge(edge)) {
                                          edges_cut.insert(edge);
                                      }
                                  }
                              }
                          }

                          return edges_cut.size();
                      });
}


auto mmstream::getEdgeBalancing(const std::vector<Partition>& partitions)
    -> std::future<double>
{
    return std::async(
        std::launch::async,
        [&partitions] {
            //get iterator to the biggest and smallest partition
            auto [smallest_iter, biggest_iter] =
                std::minmax_element(std::cbegin(partitions),
                                    std::cend(partitions),
                                    [](auto&& lhs, auto&& rhs) {
                                        return lhs.numberOfEdges()
                                            < rhs.numberOfEdges();
                                    });

            auto biggest = biggest_iter->numberOfEdges();
            auto smallest = smallest_iter->numberOfEdges();

            return (biggest - smallest)
                / static_cast<double>(biggest);
        });
}

auto mmstream::getVertexBalancing(const std::vector<Partition>& partitions)
    -> std::future<double>
{
    return std::async(
        std::launch::async,
        [&partitions] {
            //get iterator to the biggest and smallest partition
            auto [smallest_iter, biggest_iter] =
                std::minmax_element(std::cbegin(partitions),
                                    std::cend(partitions),
                                    [](auto&& lhs, auto&& rhs) {
                                        return lhs.numberOfNodes()
                                            < rhs.numberOfNodes();
                                    });


            auto biggest = biggest_iter->numberOfNodes();
            auto smallest = smallest_iter->numberOfNodes();

            return (biggest - smallest)
                / static_cast<double>(biggest);
        });
}


auto mmstream::getKminus1Metric(const std::vector<Partition>& partitions)
    -> std::future<std::size_t>
{
    return std::async(
        std::launch::async,
        [&partitions]()
            -> std::size_t {
            //fill up a set to determine the number of edges in
            //the hypergraph
            std::unordered_set<std::uint64_t> edges;
            for(auto&& part : partitions) {
                edges.insert(std::cbegin(part.getEdges()),
                             std::cend(part.getEdges()));
            }

            return std::accumulate(std::cbegin(partitions),
                                   std::cend(partitions),
                                   0,
                                   [](auto init, auto&& part) {
                                       return init + part.numberOfEdges();
                                   })
                - edges.size();
        });
}
