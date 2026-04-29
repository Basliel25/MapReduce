#include "MapReduce.h"

static hashtable_t *partitions;
static int num_partitions;
static Partitioner partitioner;

void MR_Emit(char *key, char *value) {
    int partition_num = partitioner(key, num_partitions);
    //Acquire hastable lock
    pthread_mutex_lock(&partitions[partition_num].partition_lock);

    int bucket_num = partitioner(key, partitions[partition_num].num_buckets);
    entry_t **current_entry_ptr = &partitions[partition_num].buckets[bucket_num];
    
    // Key search, if not found
    // find insertion point
    // Insertion sort
    while (*current_entry_ptr != NULL) {
        int cmp = strcmp((*current_entry_ptr)->key, key);
        if (cmp == 0) {
            // Key found
            kv_node_t *new_node = malloc(sizeof(kv_node_t));
            new_node->value = strdup(value);
            // Prepend value for O(1) insertion
            new_node->next = (*current_entry_ptr)->values;
            (*current_entry_ptr)->values = new_node;
            
            pthread_mutex_unlock(&partitions[partition_num].partition_lock);
            return;
        } else if (cmp > 0) {
            break;
        }
        current_entry_ptr = &((*current_entry_ptr)->next);
    }

    // If Key does not exist
    entry_t *new_entry = malloc(sizeof(entry_t));
    new_entry->key = strdup(key);
    new_entry->cursor = NULL;
    new_entry->next = *current_entry_ptr;
    
    kv_node_t *new_node = malloc(sizeof(kv_node_t));
    new_node->value = strdup(value);
    new_node->next = NULL;
    new_entry->values = new_node;
    
    *current_entry_ptr = new_entry;
    
    pthread_mutex_unlock(&partitions[partition_num].partition_lock);
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
