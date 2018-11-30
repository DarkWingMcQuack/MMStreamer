[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

# MMStreamer

This is an implementation of a Hypergraph partitioner using MinMax Streaming as described in [this](http://papers.nips.cc/paper/5897-streaming-min-max-hypergraph-partitioning.pdf) paper.
Instead of balancing the partitions in terms of edges, this partitioner currently uses the nodes and the maximim unbalancing factor of the cut can be specified by a given parameter.

## How to build 
To build MMStreamer, make sure you have [Boost](https://www.boost.org/), [CMake](https://cmake.org) and a C++17 compatible
compiler, such as clang or gcc installed.
Tested were GCC7.3 and Clang 7. Use one of those two to build without any issues.

Then run the following to build MMStreamer:
```sh
git clone https://github.com/DarkWingMcQuack/MMStreamer
cd MMStreamer
mkdir build && cd build
cmake ..
make
```

To use Clang as compiler, run `cmake -DUSE_CLANG=ON ..` instead of `cmake ..`.

## How to Use
To start the partitioner, follow the commands provided in the main file. The following parameters can be set:

Parameter | Effect
----------- | -----------
`help,h` | display help message
`raw,r` | if set, output is formatted in csv to make it easier to plot directly. If not set, the output is more verbose.
`input,i`| input hypergraph file
`partitions,p` | number of partitions
`balancing,b`| a double between 0 and 1 which will be used as the maximum unbalancing constraint

## Input Format

MMStreamer expects a file in the edgelist format.
In the edgelist format each line models a node followed by a comma 
seperated list of edges it is connected to. 
For example:
```
4: 1, 2, 3, 4, 5
2: 1
5: 3, 5, 7
```
This would be a hypergraph with node `4` connected to edges `1,2,3,4,5` and so on.
