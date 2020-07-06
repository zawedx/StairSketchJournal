# Stair Sketch

Implementation and experiments for Stair BF, Stair CM, Stair CU



### Build & Run

```
g++ main.cpp -o main -O2 -std=c++11
./main
```



### Files

`adacm.hpp` - Implementation of Time Adaptive Sketches

`bloom_filter.hpp` - Implementation of Bloom Filter

`cm_sketch.hpp` - Implementation of CM Sketch

`common.hpp` - Simple Definitions

`cu_sketch.hpp` - Implementation of CU Sketch

`file_reader.hpp` - Read data from datasets

`hash.hpp` - Implementation of BOBHash

`hokusai.hpp` - Implementation of item aggregation sketches

`main.cpp` - Experiments

`pbf.hpp` - Implementation of Persistent Bloom Filter

`stair_bf.hpp/stair_cm.hpp/stair_cu.hpp` - Implementation of Stair BF/CM/CU

`test.hpp` - Test Functions

`utils.hpp` - Implementation of bitset and hash map