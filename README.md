
- [About](#about)
- [Design goals](#design-goals)
- [Build](#build)
- [Usage](#usage)
- [Troubleshooting](#troubleshooting)

## About

This project aims to assist researchers in developing and testing some of the best-known content-based publish-subscribe algorithms, such as TAMA, Siena, REIN, OpIndex, H-Tree or ACForest (the latter being the reason why I took the trouble to develop this project).

Once the installation is completed, one can do two things with this project. Firstly, it can be used to confirm the veracity of some claimed results in a paper. To do this, simply create a configuration file with whatever parameters the authors use and run. The result will be LATEX markup and a pdf file. The most common experiments and plots are supported. Secondly, it can be used as a starting point to develop other CPS algorithms. If you have a good idea for an algorithm, you may clone this codebase and extend the algorithms by creating a new class under 'algorithms' and inheriting from the 'Broker' superclass.

The actual algorithms engine is written in C++ and partly inherited from [xizeroplus/matching-algorithms](https://github.com/xizeroplus/matching-algorithm), although extensively rewritten. The data-augmentation procedure uses R to leverage its extensive statistical libraries, and then Python is used to glue everything together and automate the workflow.

Several datasets are included with the project which can be used to model real-world scenarios in the manner described in the ACForest paper. The original synthetic data generators are also included.

## Design goals

- **Standarization** The whole workflow is automated, and everything is specified in configuration files, in a manner similar to Docker or Ansible.
- **JSON config files** A simple, declarative way to interact with the program. 
- **CSV output** This can be then used by Matlab, Python or some other high-level language to analyse / visualize performance data.

## Build

Before building, make sure the dependencies [JSON for Modern C++](https://github.com/nlohmann/json) and [Boost](https://www.boost.org/) are installed on your system. 

Some parameters must be set before compilation, since many of these algorithms rely on arrays that must be allocated with some size. Hence, modify the *params.h* file depending on requirements.

Then use CMake in the usual way:

```bash
mkdir build
cd build
cmake ..
make
make install
```

The above also provided as a bash script *make.sh*.

## Usage

#### Configuration

Modify the json files in the ./config/ directory and run. 

Some notes on the ./config/config.json file:
 - **algos** is a list where the only options are "H-Tree", "OpIndex", "REIN", "Siena", "TAMA", "Subscription Forest" and "SCTree". Anything else gets ignored. Each of these algorithms is run individually on the same (generated) subscriptions and events.
 - **attribute_distribution, value_distribution**: these should be either 0 (for uniform distribution) or 1 (for zipf).
 - **number_of_subscriptions** and **number_of_dimensions** are also lists of the form (start, end, step). These are passed directly to the for-loops in the main program. For example, *"number_of_subscriptions": [1000, 10000, 10000]*, will test subscriptions from 1,000 to 10,000 in steps of 1,000. Setting up lots of ranges will result in lots of experiments and lots of data, so in practice it is best to only write one range per run. For example: *"number_of_subscriptions": [1000, 10000, 10000]* and 	*"number_of_dimensions": [20,20,10]*, will only test 10 loops for the varying amount of subscriptions while keeping the dimensions fixed at $d=20$.

Additionally, some of the algorithms might require some parameters to work, such as number of levels and cells in the case of H-Tree (these are parameters which don't need to be set before compilation). These can be set modifying the files in *./config/algos/*

#### Extending

#### Visualization

Upon running the program, the experiments will be run and results recorded in timestamped CSV files in the *./output* directory. A copy of the configuration file will also be saved for future reference on the exact parameters used on the experiments.

The csv files can be plotted using Python. A short plotting procedure is included in *procedures.py.* Include it in a Jupyter notebook like this:

```python
from procedures import *
%matplotlib inline
plt.rcParams["figure.figsize"] = [18, 9] // Or whatever dimensions are desired.
```

Then call the *plot(timestamp, x,y, discard=[])* function. It will look for a csv file in first in './output/save' and then in './output' ( hence interesting csv's can be saved to a 'save' subdirectory so they are not mixed with other experiments). *x* and *y* refer to data used to populate the axes. *discard* takes in a list of algorithm names to discard from the dataset.

 Here are are some examples of the syntax:

```python
plot(1565832155,"Subscriptions", "Mean event match time")

plot(1565832155,"Subscriptions", "Mean event match time", discard=["Siena", "REIN"])

plot(1565711188,"Dimensions", "Mean insert subscription time")
```



## Troubleshooting

If some  algorithm causes "Segmentation fault: 11", it is probably an issue with memory. Hence, check the params.h file and make sure the MAX_ATTS, MAX_SUBS and other settings don't exceed whatever is in the config file. 
