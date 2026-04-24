#include "MapReduce.h"

static hashtable_t *partitions;
static int num_partitions;
static Partitioner partitioner;

void MR_Emit(char *key, char *value) {

    int partition_num = partitioner(key, num_partitions);
    
    // Acquire Lock for partition
    pthread_mutex_lock(&partitions[partition_num].partition_lock);

    // Access Appropriate bucket
    int bucket_num = partitioner(key, partitions[partition_num].num_buckets);
    entry_t *entry = partitions[partition_num].buckets[bucket_num];
    // In  the case that bucket is populated and Key-Value pair exists
    while(entry != NULL) {
        if(strcmp(entry->key, key) == 0) {
            //Create new Node and add value
            kv_node_t *new_entry = malloc(sizeof(kv_node_t));
            new_entry->value = strdup(value);
            new_entry->next = NULL;

            // Crawl to end of list and append
            kv_node_t *current = entry->values;
            while(current->next != NULL) current = current->next;
            current->next = new_entry;
            return;
        }
        entry = entry->next;
    }
    // In the case the bucket is empty

    entry_t *new_entry = malloc(sizeof(entry_t));
    new_entry->key = key;
    new_entry->next = NULL;

    kv_node_t *new_node = malloc(sizeof(kv_node_t));
    new_node->value = strdup(value);
    new_node->next = NULL;
    new_entry->values = new_node;

    partitions[partition_num].buckets[bucket_num] = new_entry;
    
    // Relase Lock

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
    partitioner = (partition != NULL) ? partition : MR_DefaultHashPartition;

    partitions = malloc(sizeof(hashtable_t)*num_partitions);
    for(int i = 0; i < num_partitions; i++) {
        partitions[i].num_buckets = 101;
        partitions[i].buckets = calloc(101, sizeof(entry_t));
        pthread_mutex_init(&partitions[i].partition_lock, NULL);
    }
}
char *MR_Getter(char *key, int partition_number) {return NULL;}
