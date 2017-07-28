# ACTS test framework

A test framework for the ACTS development: **absolutely not** intended
for production usage.

## Build requirements

The framework can be build in different configurations. The default setup
requires a C++14 compatible compiler with OpenMP support, the ACTS library
with Core and MaterialPlugin (default configuration), and
[ROOT >= 6.08](https://root.cern.ch/).

Additional code can be activated/deactivated by defining one or more of the
following options

*   USE_DD4HEP
*   USE_GEANT4
*   USE_OPENMP (on by default)
*   USE_PYTHIA8

during the configuration step, e.g. as `cmake -DUSE_DD4HEP=on ...`. Depending
on the selected options, additional ACTS plugins and external software
must be available. For details please see the `CMakeLists.txt` file.

## ACTS Submodule

To keep track of the correct combination of framework and ACTS version,
a copy of the ACTS library is provided as a git submodule in the `acts`
directory. Please have a look at the
[git-submodule documentation][git-book-submodule] for an introduction to
how git submodules work.

If you clone the framework repository you have to either clone with the
`--recursive` option or run the following commands afterwards

    git submodule init
    git submodule update

to download a copy of the specified version. The framework repository
only stores the remote path and the commit id of the used ACTS version.
On your branch you can update it to the specific branch that you are
working on, but please switch to a tagged-version or the master branch
before merging back.

The ACTS version defined by the submodule is also used to run the
continous integration on Gitlab and you can the scripts are therefore
run automatically with your specific version.

## Guidelines for writing an algorithm

All examples in the framework are based on a simple event loop, i.e. a logic
that executes a set of event readers, event writers, and event algorithms for
each event. To simplify interactions between different algorithms please follow
the following guidelines:

*   An algorithm must implement the `name`, `initialize`, `finalize`, and
    `execute` methods defined in the `IAlgorithm` interface. `initialize` and
    `finalize` are executed exactly once, before and after all events were
    processed. All computationally expensive one-time code should run in the
    `initialize` method and not already in the constructor. The `execute`
    method for each event may be called in parallel.
*   Simple algorithms that have no internal state can use the `SimpleAlgorithm`
    as a base class and only need to implement the `execute` method.
*   Communication between different algorithms should only happen via the event
    store provided by the `AlgorithmContext`. Output collections or objects
    should be transfered to the store at the end of the execution. To allow
    to run the same algorithm with multiple configurations the names of
    input and output objects should be configurable for an algorithm.
*   Strictly separate computation from input and output. Input and output
    algorithms must be implemented using the `IReader` and `IWriter` interface.
    They should only read in the data from file and add them to the event store
    or read objects from the event store and write them to file. Again,
    names of the objects that are read/written should be configurable.


[git-book-submodules]: https://git-scm.com/book/en/v2/Git-Tools-Submodules
