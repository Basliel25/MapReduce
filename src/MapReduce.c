#include "MapReduce.h"

static hashtable_t *partitions;
static int num_partitions;
static Partitioner partitioner;

void MR_Emit(char *key, char *value) {

    int partiton_num = partitioner(key, num_partitions);
    partitions[partition_num]->bucket_lock;
    if(partitions[partition_num] != NULL) {}

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
    num_partitions = num_reducers;
    if (partition != NULL) {partitioner = partition;}
    else {partitioner = MR_DefaultHashPartition;}

    partitions = malloc(sizeof(hashtable_t)*num_partitions);
    for(int i = 0; i < num_partitions; i++) {
        partitions[i].num_buckets = 101;
        partitions[i].buckets = calloc(101, sizeof(entry_t));
        pthread_mutex_init(&partitions[i].bucket_lock, NULL);
    }
}
char *MR_Getter(char *key, int partition_number) {return NULL;}
