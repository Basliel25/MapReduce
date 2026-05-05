# MapReduce

A small C implementation of a single-node MapReduce framework with a
pthread-based mapper and reducer pool, partitioned hash tables, and a
sorted dispatch order per partition. Contains a `wordcount` driver.
Read more [Here](https://basz-website.basgug25.workers.dev/projects/map-reduce/).

## Architecture

```
src/
  Interface.h     # Mapper / Reducer / Partitioner / Getter typedefs
  MapReduce.h     # Public API + internal data structures
  MapReduce.c     # Framework implementation
  wordcount.c     # Example driver: counts whitespace-separated tokens
Makefile
Dockerfile
```

## Build

```
make
```

Produces `./MapReduce` linked against the `wordcount` driver.

## Run

```
./MapReduce file1.txt file2.txt ...
```

Output is one `<word> <count>` line per unique key, sorted lexicographically
within each partition. Pipe through `sort` for a globally sorted view.

## Docker

```
docker build -t mapreduce:latest .
docker run --rm -v "$PWD/inputs:/data" mapreduce:latest /data/file1 /data/file2
```

The runtime image includes `valgrind` for in-container leak checking.

## Public API

```c
void  MR_Run(int argc, char *argv[],
             Mapper map, int num_mappers,
             Reducer reduce, int num_reducers,
             Partitioner partition);
void  MR_Emit(char *key, char *value);
char *MR_Getter(char *key, int partition_number);
unsigned long MR_DefaultHashPartition(char *key, int num_partitions);
```

A user driver implements `Map` and `Reduce` and calls `MR_Run` from `main`.
Pass `MR_DefaultHashPartition` (or `NULL`) to use the built-in DJB2 hash;
supply a custom `Partitioner` for different key distribution.
