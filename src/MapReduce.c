#include "MapReduce.h"

static hashtable_t *partitons;
static int num_partitions;
static Partitioner partitioner;

void MR_Emit(char *key, char *value) {

    // ON CALL this function stores the key value pair to appropriate
    // partiton
    // Initalizes the partion array
    // Selects a partition for each key using the hashing value
    // unsigned long partition = MR_DefaultHashPartition(key, num_partions)
    // Acquire Lock
    // int idx = partiton of value % num_buckets
    // walks bucket[idx] to find the specific key
    // if found appends value to the bucket
    // if not create a new node in bucket[idx]
    // Release Lock
}

unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
   unsigned long hash = 5381;
   int c;
   while((c = *key++) != '\0')
       hash = hash * 33 + c;
   return hash % num_partitions;
}

void MR_Run(int argc, char *argv[], 
        Mapper map, int num_mappers, 
        Reducer reduce, int num_reducers, 
        Partitioner partition) {
    // Initalize global variables
    // hashtable_t hash_partitions[num_partitions] number of partitons is number of reducers 
    // emit called with value key pair
}

char *MR_Getter(char *key, int partition_number) {return NULL;}
