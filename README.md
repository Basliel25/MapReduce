# MapReduce

A small C implementation of a single-node MapReduce framework with a
pthread-based mapper and reducer pool, partitioned hash tables, and a
sorted dispatch order per partition. Contains with a `wordcount` driver.

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

## Architecture notes

- `num_partitions == num_reducers`. Each reducer owns one partition.
- `MR_Emit` is thread-safe per partition via a `pthread_mutex_t`.
- `MR_Teardown` (called at the end of `MR_Run`) frees every value node,
  entry, bucket array, and partition mutex. Verified leak-free under
  `valgrind --leak-check=full`.
