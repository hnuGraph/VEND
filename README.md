# Vend  **: Vertex Encoding for Edge Nonexistence Determination**

## Introduction

​	We propose a novel technique of vertex encoding for edge nonexistence determination(VEND, for short) to avoid no-result edge query executions , which will certainly improve performance of graph database and accelerate related graph searches.

## Build

Linux

graph storage is  based on a database, so firstly make sure that the environment supports  

1. create a static library of rocksdb 

   ```shell
   $ git clone https://github.com/facebook/rocksdb
   $ make static_lib
   ```

2. modify the dataset information

   the  basic properties of the testing dataset  are available on  **/src/include/common/config.h**

   ```c++
   //  src/include/common/config.h
   static constexpr int K_SIZE = 5;
   static constexpr uint32_t VERTEX_SIZE = 1791489;       
   static constexpr uint32_t DEGREE = 28;
      make sure to correct the dataset details before testing 
   ```

3. build and run  

   Then run the following commands to build

   ```shell
   $ mkdir build 
   $ cd build 
   $ cmake ..
   $ make 
   ```

   example scripts are available on /scripts/execute.sh 

   the following  is properties of the scripts,  you can modify them as you wish 

   ```bash
   param_k=5		
   data_path="../resource/as-skitter.txt"
   vend_prefix=""					// where the encode is stored 
   output_path=""					// result path
   pair_path_prefix=""				// query and insert edges location 
   ```

   Next, run the script to start the vend 



## Codebase 

**db engine**

Provide the database interface, you can rewrite it to build the graph based on another database rather than rocksdb.

```
dbengine/dbengin.h    dbengine/rocksdb.h
```

**encode**

Constructing the Vertex  encoding for the graph, which contains four algorithms,  

1. range based encoding    /encode/single/range_encode.h 
2. hash based encoding    /encode/single/hash_encode.h
3. bloom filter bit encoding    /encode/bfilter/bfilter_bit_encode.h
4. bloom filter int encoding    /encode/bfilter/bfilter_int_encode.h
5. hybrid encoding                   /encode/single/hybrida_encode.h

**vend**

build the encoding  and test one query edge is whether a ne-pair(edge nonexistence) or not

**graph**

build the whole graph, and provides  three operations: query, insertion, and deletion  

**execution**

	1. score:  return the number of certain ne-pairs under each encoding algorithm when querying.
	2. query :  return the query cost time  
	3. delete :  return the time taken for encoding deletion 
	4. insert :  return the time taken for encoding insertion

**utils**

contains two major classes: bitset and timer 

```
utils/bitset.h       // the basic element for encoding 
utils/timer          // provides timer function for each execution 
```



##  Datasets

We provide 3 datasets in our experiment 

1. As-Skitter dataset        [[link](http://snap.stanford.edu/data/as-Skitter.html)]
2. Wiki-topcats dataset   [[link](http://snap.stanford.edu/data/wiki-topcats.html)]
3.  Orkut dataset              [[link](http://snap.stanford.edu/data/com-Orkut.html)]



**Summary of Datasets**

| **Datasets** |      **Type**       | **Vertexes** |  **Edges**  | **Average Degree** |
| :----------: | :-----------------: | :----------: | :---------: | :----------------: |
|  As-Skitter  |  Internet topology  |  1,696,415   | 11,095,298  |         13         |
| Wiki-topcats | Wikipedia hyperlink |  1,791,489   | 25,444,207  |         28         |
|    Orkut     |   Social network    |  3,072,441   | 117,185,083 |         76         |
=======
1. As-Skitter dataset   [link]: http://snap.stanford.edu/data/wiki-topcats.html

2. Wiki-topcats dataset [link]:http://snap.stanford.edu/data/wiki-topcats.html

3.  Orkut dataset       [link]: http://snap.stanford.edu/data/wiki-topcats.html

