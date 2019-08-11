
- [About](#about)
- [Design goals](#design-goals)
- [Build](#build)
- [Usage](#usage)

## About

This is a fork of [xizeroplus/matching-algorithms](https://github.com/xizeroplus/matching-algorithm) with lots of additional capabilities. Like the original project, it is content-based publish-subscribe algorithms framework aimed at researchers or anyone interested in this area. It might also be suitable as a starting point for a library for a production system; right now, it is simply intended to compare the speed of different algorithms under different load conditions. It is my hope that this version is easier to maintain and extend, and makes it easier to conduct research in this area in the future.

## Design goals

- **DRY code**. A lot of code in the original branch was repeated with slight modifications, making it difficult to maintain and extend.
- **JSON config files** A simple, declarative way to interact with the program. 
- **CSV output** This can be then used by Matlab, Python or some other high-level language to analyse / visualize performance data.
- **Parallelism** In the future, I wish to some of the algorithms so that they scale horizontally.

## Build

Before building, make sure the dependencies [JSON for Modern C++](https://github.com/nlohmann/json) and [Boost](https://www.boost.org/) are installed on your system. 

Use CMake in the usual way:

```bash
mkdir build
cd build
cmake ..
make
make install
```

## Usage

Modify the json files in the ./config/ directory and run. 

Some notes on the ./config/config.json file:
 - **algos** is a list where the only options are "htree", "opIndex", "rein", "siena", "tama" and "subscriptionForest". Anything else gets ignored. Each of these algorithms is run individually on the same (generated) subscriptions and events.
 - **attribute_distribution, value_distribution**: these should be either 0 (for uniform distribution) or 1 (for zipf).
 - **number_of_subscriptions** and **number_of_dimensions** are also lists of the form (start, end, step). These are passed directly to the for-loops in the main program. For example, *"number_of_subscriptions": [1000, 10000, 10000]*, will test subscriptions from 1,000 to 10,000 in steps of 1,000. Setting up lots of ranges will result in lots of experiments and lots of data, so in practice it is best to only write one range per run. For example: *"number_of_subscriptions": [1000, 10000, 10000]* and 	*"number_of_dimensions": [20,20,10]*, will only test 10 loops for the varying amount of subscriptions while keeping the dimensions fixed at $d=20$.

Additionally, some of the algorithms might require some parameters to work, such as number of levels and cells in the case of H-Tree. These can be set modifying the files in ./config/algos/

If opIndex or some other algorithm causes "Segmentation fault: 11", it is probably an issue with memory. This is likely because of too many subscriptions, or because some of the algorithm parameters.