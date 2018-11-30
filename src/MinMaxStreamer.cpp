#include <MinMaxStreamer.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <vector>

using mmstream::MinMaxStreamer;
using mmstream::Partition;

MinMaxStreamer::MinMaxStreamer(std::size_t partitions)
{
    // fill _partitions with n empty partitions
    for(std::size_t i{0}; i < partitions; ++i) {
        partitions_.emplace_back(i);
    }
}

namespace {

template<class InputIterator,
         class OutputIterator,
         class UnaryOperation,
         class UnaryPredicate>
OutputIterator transform_if(InputIterator first,
                            InputIterator last,
                            OutputIterator result,
                            UnaryOperation op,
                            UnaryPredicate test)
{
    while(first != last) {
        if(test(*first))
            *result++ = op(*first);
        ++first;
    }
    return result;
}

} // namespace

auto MinMaxStreamer::addElement(uint64_t node,
                                const std::vector<uint64_t>& edges,
                                double balancing)
    -> void
{
    //get the size of the smallest partition
    //in terms of edges
    auto smallest =
        std::min_element(std::cbegin(partitions_),
                         std::cend(partitions_),
                         [](auto&& lhs, auto&& rhs) {
                             return lhs.numberOfNodes()
                                 < rhs.numberOfNodes();
                         })
            ->numberOfNodes();

    std::vector<
        std::pair<std::reference_wrapper<Partition>,
                  std::int64_t>>
        pair_vec;

    //map every partition to (partition&, common_topics)
    transform_if(std::begin(partitions_),
                 std::end(partitions_),
                 std::back_inserter(pair_vec),
                 [&](auto& part) {
                     auto ref = std::ref(part);
                     auto common_topics =
                         part.commonTopics(edges);

                     return std::pair{ref, common_topics};
                 },
                 [&](auto&& part) {
                     return part.numberOfNodes() <= smallest * (1 + balancing);
                 });


    //choose best partition in terms of common topics
    auto best_partition =
        std::max_element(std::cbegin(pair_vec),
                         std::cend(pair_vec),
                         [](auto&& lhs, auto&& rhs) {
                             return lhs.second < rhs.second;
                         });

    //add the node with its topics to the choosen partition
    best_partition->first.get().addNode(node, edges);
}

namespace {

template<class Function>
auto generate_graph_parser(Function&& func)
{
    namespace x3 = boost::spirit::x3;
    namespace fusion = boost::fusion;

    auto line = (x3::uint64 >> ':') > (x3::uint64 % ',');
    auto empty_node = x3::uint64;

    return +(line[std::forward<Function>(func)] | empty_node);
}

} // namespace

auto MinMaxStreamer::parseGraph(const std::string& path,
                                double balancing)
    -> void
{
    namespace x3 = boost::spirit::x3;
    namespace fusion = boost::fusion;

    auto process_line = [this, balancing](auto&& ctx) {
        std::vector<uint64_t> edge_list;
        uint64_t vtx;
        auto tup = std::tie(vtx, edge_list);

        fusion::move(std::move(x3::_attr(std::move(ctx))), tup);

        this->addElement(vtx, edge_list, balancing);
    };

    std::ifstream edge_file(path);
    if(!edge_file) {
        std::cout << "file: " << path << "not found\n";
        return;
    }

    boost::spirit::istream_iterator file_iterator(edge_file >> std::noskipws),
        eof;

    x3::phrase_parse(file_iterator,
                     eof,
                     generate_graph_parser(process_line),
                     x3::space);
}


auto MinMaxStreamer::getPartitions() const
    -> const std::vector<Partition>&
{
    return partitions_;
}
