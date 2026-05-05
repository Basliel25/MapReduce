#ifndef MAPREDUCE
#define MAPREDUCE
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "Interface.h"

/***********************
 *** Data Structures ***
***********************/

/**
 * @brief A node inside a bucket of the hashtable
 */
typedef struct kv_node_t {
    char *value; /**<- The value to be stored in the node*/
    struct kv_node_t *next; /**<- Pointer to next Node*/
} kv_node_t;

/**
 * @brief A bucket of linked lists grouped by hash
 */
typedef struct entry_t {
    char *key;
    kv_node_t *values; /**<- Linked List of value nodes.*/
    kv_node_t *cursor; /**<- Pointer to current cursor for getter*/
    struct entry_t *next; /**<- Pointer to next list in bucket*/
} entry_t;

/**
 * @brief The hastable containing all buckets
 */
typedef struct {
    entry_t **buckets; /**<- Pointer to buckets*/
    int num_buckets; /**<- Total buckets in hashtable*/
    pthread_mutex_t partition_lock; /**<- Lock to protect partitions*/
} hashtable_t;

/**
 * @brief Shared work queue of input file names for mapper threads
 */
typedef struct {
    char **files;          /**<- Array of file name pointers (from argv)*/
    int size;              /**<- Total number of files in the queue*/
    int next;              /**<- Index of next file to be claimed*/
    pthread_mutex_t lock;  /**<- Lock protecting next index*/
} file_queue_t;

/**
 * @brief Inserting key-value pair to the appropriate bucket
 * @param int partition: The appropriate hashed bucket number
 * @param char *key: The key to be added to a bucket
 * @param char *value: The value to be associated to the key
 */
void MR_Insert(int partition, char *key, char *value);

/**
 * @brief Mapper worker thread routine. Pulls files from the
 *        shared file_queue_t and invokes the user Mapper on each
 *        until the queue is exhausted.
 * @param void *arg: Unused, required by pthread_create signature
 */
void *MR_MapperWorker(void *arg);

/**
 * @brief Reducer worker thread routine. Iterates every entry in the
 *        assigned partition, resets the per-entry cursor, and invokes
 *        the user Reducer once per key with MR_Getter as the value
 *        accessor.
 * @param void *arg: Pointer to an int holding the partition number
 *                   this reducer is responsible for
 */
void *MR_ReducerWorker(void *arg);

/**
 * @brief qsort comparator that orders entry_t pointers
 *        lexicographically by their key field.
 * @param const void *a: Pointer to an entry_t* element
 * @param const void *b: Pointer to an entry_t* element
 * @return Negative if a<b, zero if equal, positive if a>b
 */
int MR_EntryKeyCmp(const void *a, const void *b);

/**
 * @brief Frees all memory owned by the partitions array: every
 *        kv_node_t (and its strdup'd value), every entry_t (and its
 *        strdup'd key), each partition's bucket array, destroys the
 *        partition mutex, then frees the partitions array itself.
 */
void MR_Teardown(void);
/**
 * @brief Emitting Function
 * @param char *key: The key to be added to a bucket
 * @param char *value: The value to be associated to the key
 */
void MR_Emit(char *key, char *value);

/**
 * @brief Hashing Function
 * @param char *key: The key to be hashed
 * @param int num_partitions: The number of partions
 *                            in the hashtable
 */
unsigned long MR_DefaultHashPartition(char *key, int num_partitions);

/**
 * @brief Map and Reduce Execution
 * @param int argc: Number of arguments passed
 * @param char *argv[]: Arguments
 * @param Mapper map: The mapper function implemented by user
 * @param int num_mappers: Number of mapping threads
 * @param Reducer reduce: The reducer function implemented by user
 * @param int num_reducers: Number of reducing threads
 * @param Partitioner partition: A different hasing function if NULL uses MR_DefaultHashPartition
 */
void MR_Run(int argc, char *argv[], 
	    Mapper map, int num_mappers, 
	    Reducer reduce, int num_reducers, 
	    Partitioner partition);

/**
 * @brief Getter Function, walks buckets to look for values
 * @param char *key: The key to be hashed
 * @param int partition_number: The partiton the value is located in 
 */
char *MR_Getter(char *key, int partition_number);
#endif // MAPREDUCE
