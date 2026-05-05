#include "MapReduce.h"

static hashtable_t *partitions;
static int num_partitions;
static Partitioner partitioner;

static file_queue_t file_queue;
static Mapper user_mapper;
static Reducer user_reducer;

void *MR_MapperWorker(void *arg) {
    (void)arg;
    while (1) {
        pthread_mutex_lock(&file_queue.lock);
        if (file_queue.next >= file_queue.size) {
            pthread_mutex_unlock(&file_queue.lock);
            return NULL;
        }
        char *file = file_queue.files[file_queue.next++];
        pthread_mutex_unlock(&file_queue.lock);

        user_mapper(file);
    }
}

int MR_EntryKeyCmp(const void *a, const void *b) {
    const entry_t *ea = *(const entry_t * const *)a;
    const entry_t *eb = *(const entry_t * const *)b;
    return strcmp(ea->key, eb->key);
}

void *MR_ReducerWorker(void *arg) {
    int partition_number = *(int *)arg;
    hashtable_t *partition = &partitions[partition_number];

    // Pass 1: count entries across all buckets in this partition
    int count = 0;
    for (int b = 0; b < partition->num_buckets; b++) {
        for (entry_t *e = partition->buckets[b]; e != NULL; e = e->next) {
            count++;
        }
    }
    if (count == 0) return NULL;

    // Pass 2: collect entry pointers
    entry_t **sorted = malloc(sizeof(entry_t *) * count);
    int idx = 0;
    for (int b = 0; b < partition->num_buckets; b++) {
        for (entry_t *e = partition->buckets[b]; e != NULL; e = e->next) {
            sorted[idx++] = e;
        }
    }

    // Sort lexicographically by key, then dispatch to user reducer
    qsort(sorted, count, sizeof(entry_t *), MR_EntryKeyCmp);
    for (int i = 0; i < count; i++) {
        sorted[i]->cursor = sorted[i]->values;
        user_reducer(sorted[i]->key, MR_Getter, partition_number);
    }

    free(sorted);
    return NULL;
}

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

    // Initialize shared file queue from argv (skip argv[0])
    file_queue.files = &argv[1];
    file_queue.size = argc - 1;
    file_queue.next = 0;
    pthread_mutex_init(&file_queue.lock, NULL);
    user_mapper = map;

    // Spawn mapper threads and wait for completion
    pthread_t *mapper_threads = malloc(sizeof(pthread_t) * num_mappers);
    for (int i = 0; i < num_mappers; i++) {
        pthread_create(&mapper_threads[i], NULL, MR_MapperWorker, NULL);
    }
    for (int i = 0; i < num_mappers; i++) {
        pthread_join(mapper_threads[i], NULL);
    }
    free(mapper_threads);
    pthread_mutex_destroy(&file_queue.lock);

    // Spawn reducer threads, one per partition
    user_reducer = reduce;
    pthread_t *reducer_threads = malloc(sizeof(pthread_t) * num_reducers);
    int *reducer_ids = malloc(sizeof(int) * num_reducers);
    for (int i = 0; i < num_reducers; i++) {
        reducer_ids[i] = i;
        pthread_create(&reducer_threads[i], NULL, MR_ReducerWorker, &reducer_ids[i]);
    }
    for (int i = 0; i < num_reducers; i++) {
        pthread_join(reducer_threads[i], NULL);
    }
    free(reducer_threads);
    free(reducer_ids);
}
char *MR_Getter(char *key, int partition_number) {
    hashtable_t *partition = &partitions[partition_number];
    int bucket_num = partitioner(key, partition->num_buckets);

    // Locate the entry for this key within the bucket chain
    entry_t *entry = partition->buckets[bucket_num];
    while (entry != NULL) {
        int cmp = strcmp(entry->key, key);
        if (cmp == 0) break;
        if (cmp > 0) return NULL; // bucket chain is sorted; key absent
        entry = entry->next;
    }
    if (entry == NULL) return NULL;

    // Advance the per-entry cursor and return the current value
    if (entry->cursor == NULL) return NULL;
    char *value = entry->cursor->value;
    entry->cursor = entry->cursor->next;
    return value;
}
