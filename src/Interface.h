#ifndef INTERFACE
#define INTERFACE

/** Interface provided by the MapReduce 
 * as a framework for implementation.
 */
typedef char *(*Getter)(char *key, int partition_number);
typedef void (*Mapper)(char *file_name);
typedef void (*Reducer)(char *key, Getter get_func, int partition_number);
typedef unsigned long (*Partitioner)(char *key, int num_partitions);


#endif // MAPREDUCE
