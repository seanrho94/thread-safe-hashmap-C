#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdlib.h>
#include <pthread.h>
#define INITIAL_SIZE 32

// Additional Struct.
typedef struct hash_node {
    void *key;
    void *value;
    struct hash_node *next;
}hash_node_t;

// Additional Struct.
typedef struct hash_entry {
    hash_node_t *head;
}hash_entry_t;

// Given Struct that we need to modify.
struct hash_map {
    int n_entries; // Total Number of entries in the hash table.
    int n_used_entries; // Number of entries that are currently used in the hash table.
    hash_entry_t **table;
    pthread_mutex_t map_lock;
    size_t (*hash)(void*); // Hash function pointer.
    int (*cmp)(void*,void*); // Compare (Key & Value) function pointer.
    void (*key_destruct)(void*); // Key detruct function pointer.
    void (*value_destruct)(void*); // Value destruct function pointer.
};

struct hash_map* hash_map_new(size_t (*hash)(void*), int (*cmp)(void*,void*),
    void (*key_destruct)(void*), void (*value_destruct)(void*));

void hash_map_put_entry_move(struct hash_map* map, void* k, void* v);

void hash_map_remove_entry(struct hash_map* map, void* k);

void* hash_map_get_value_ref(struct hash_map* map, void* k);

void hash_map_destroy(struct hash_map* map);

#endif
