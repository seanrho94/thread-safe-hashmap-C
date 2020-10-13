#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "hashmap.h"

// Print hashmap
void dump_hashmap(struct hash_map* map) {
    hash_node_t* current_node;
    printf("---Used Entries: %d/%d---\n", map->n_used_entries, map->n_entries);

    for (int table_idx = 0; table_idx < map->n_entries; table_idx++) {
        current_node = map->table[table_idx]->head;

        printf("Entry[%d]: ", table_idx);
        
        while (current_node != NULL) {
            printf("(key: %s|val: %s)->", (char *)current_node->key, (char *)current_node->value);
            current_node = current_node->next;
        }
        if(current_node == NULL){
            printf("NULL");
        }
        printf("\n");
    }
    return;
}

// My hash function
size_t hash(void* k) {
    char* key = (char *)k;
    size_t sum = 0;

    for (int i = 0; i < strlen(k); i++){
        sum += key[i];
    }

    return sum;
}

// My cmp function.
int cmp(void* k1, void* k2) {
    char* key1 = (char *)k1;
    char* key2 = (char *)k2;

    if(strcmp(k1, k2) == 0){
        return 1;
    }

    return 0;
}

// My key destruct function.
void key_destruct(void* k){

}

// My value destruct function.
void value_destruct(void* v){
    
}

// Init entry.
hash_entry_t* entry_init() {
    hash_entry_t* new_entry = (hash_entry_t*)malloc(sizeof(hash_entry_t));
    new_entry->head = NULL;

    return new_entry;
}

struct hash_map* hash_map_new(size_t (*hash)(void*), int (*cmp)(void*,void*), void (*key_destruct)(void*), void (*value_destruct)(void*)) {
    // return NULL if any of these function pointers are NULL.
    if(hash == NULL || cmp == NULL || key_destruct == NULL || value_destruct == NULL){
        return NULL;
    }

    struct hash_map* new_hash_map = (struct hash_map*)malloc(sizeof(struct hash_map));
    
    new_hash_map->n_entries = INITIAL_SIZE;
    new_hash_map->n_used_entries = 0;
    // Initilase mutex.
	pthread_mutex_init(&new_hash_map->map_lock, NULL);
    new_hash_map->hash = hash;
    new_hash_map->cmp = cmp;
    new_hash_map->key_destruct = key_destruct;
    new_hash_map->value_destruct = value_destruct;
    new_hash_map->table = (hash_entry_t**)malloc(sizeof(hash_entry_t*) * new_hash_map->n_entries);

    // Initialise entries.
    for (int table_idx = 0; table_idx < new_hash_map->n_entries; table_idx++) {
        new_hash_map->table[table_idx] = entry_init();
    }
	
	return new_hash_map;
}

// Init node.
hash_node_t* node_init (void* k, void* v) {
    hash_node_t* new_node = (hash_node_t *)malloc(sizeof(hash_node_t));
    new_node->key = k;
    new_node->value = v;
    new_node->next = NULL;

    return new_node;
}

// Re-hashing (Re-arrange) entries in the original hash table into new hash table after resizing.
void re_hashing(struct hash_map* map, hash_entry_t **temp_table, void* k, void* v) {
    int table_idx = map->hash(k) % map->n_entries;
    hash_node_t* new_node = node_init(k, v);
    hash_node_t* current_node = temp_table[table_idx]->head;

    // If the entry is empty.
    if (temp_table[table_idx]->head == NULL) {
        temp_table[table_idx]->head = new_node;
    //  If the entry is not empty.
    }else {
        // The new node's next is current head.
        new_node->next = temp_table[table_idx]->head;
        // Change current head to the new node.
        temp_table[table_idx]->head = new_node;
    }
}

void original_hash_table_destroy(struct hash_map* map, hash_entry_t* entry) {
    hash_node_t* current_node = entry->head;
    hash_node_t* temp_node;

    while (current_node != NULL) {
        temp_node = current_node->next;
        free(current_node);
        current_node = temp_node;
    }
	free(entry);
}

// Increase the size of the hash table when the load factor(useage) is 75%.
void resize_hash_map (struct hash_map* map) {
    hash_node_t* current_node;

    // Inrease the size of the hast table.
    int old_n_entries = map->n_entries;
    map->n_entries *= 4;
    // Reallocate memories for new entries after resizing the hash table.
    map->table = (hash_entry_t**)realloc(map->table, sizeof(hash_entry_t*) * map->n_entries);
    for (int table_idx = old_n_entries; table_idx < map->n_entries; table_idx++) {
        map->table[table_idx] = entry_init();
    }

    // Allocate memory for new hash table.
    hash_entry_t** new_table = (hash_entry_t**)malloc(sizeof(hash_entry_t*) * map->n_entries);
    // Allocate memories for new entries in the new hash table.
    for (int table_idx = 0; table_idx < map->n_entries; table_idx++) {
        new_table[table_idx] = entry_init();
    }

    // Re-hash (re-arrange) the entries of the original hash table into the new hash table. 
    for (int table_idx = 0; table_idx < map->n_entries; table_idx++) {
        current_node = map->table[table_idx]->head;
        while (current_node != NULL) {
            re_hashing(map, new_table, current_node->key, current_node->value);
            current_node = current_node->next;
        }
    }

    // Remove the original hash table and its entries.
    for (int table_idx = 0; table_idx < map->n_entries; table_idx++) {
		original_hash_table_destroy(map, map->table[table_idx]);
	}
    free(map->table);

    // The original hash table becomes the new hash table.
    map->table = new_table;
}

void hash_map_put_entry_move(struct hash_map* map, void* k, void* v) {
    // Lock the map
    pthread_mutex_lock(&map->map_lock); 
    
    // If map OR key OR value is NULL, do nothing.
    if (map == NULL || k == NULL || v == NULL) {
        return;
    }

    int table_idx = map->hash(k) % map->n_entries;
    hash_node_t* new_node = node_init(k, v);
    hash_node_t* current_node = map->table[table_idx]->head;

    // In the event the key exists within the hashmap, the node(entry) should be removed and a new node(entry) will be added.
    while(current_node != NULL) {
        if(map->cmp(current_node->key, k) == 1){
            hash_map_remove_entry(map, current_node->key);
            break;
        }
        current_node = current_node->next;
    }

    // If the entry is empty.
    if (map->table[table_idx]->head == NULL) {
        map->table[table_idx]->head = new_node;
        map->n_used_entries++; 
    //  If the entry is not empty.
    }else {
        // The new node's next is current head.
        new_node->next = map->table[table_idx]->head;
        // Change current head to the new node.
        map->table[table_idx]->head = new_node;
        map->n_used_entries++; 
    }

    // If the current usage of the hash table (load factor) is about 75%, increase its size.
    float load_factor = (float)map->n_used_entries/map->n_entries;
    if (load_factor >= 0.75) {
        resize_hash_map(map);
    }   

    //unlock the map
    pthread_mutex_unlock(&map->map_lock); 
}

void hash_map_remove_entry(struct hash_map* map, void* k) {
    // IF map OR  Key is NULL, do nothing.
    if (map == NULL || k == NULL) {
        return;
    }

    int table_idx = map->hash(k) % map->n_entries;
    hash_node_t* current_node = map->table[table_idx]->head;
    hash_node_t* prev_node;

    // If there is nothing in the entry.
    if(current_node == NULL){
        return;
    }

    // If entry is not empty.
    while (current_node != NULL) {
        // Iterate through the linked list and compare keys.
        if (map->cmp(current_node->key, k) == 1){
            // If the found key is head, move the head ptr  to current node's next. 
            if (current_node == map->table[table_idx]->head) {
                map->table[table_idx]->head = current_node->next;
            // If the found key is not head, change the prev node's next to the current node's next.
            }else {
                prev_node->next = current_node->next;
            }
            // Free the objects (key and values) within the node and free node as well.
            map->key_destruct(current_node->key);
            map->value_destruct(current_node->value);
            map->n_used_entries--;
            free(current_node);

            return;
        }
        prev_node = current_node;
        current_node = current_node->next;
    }

    return;
}

void* hash_map_get_value_ref(struct hash_map* map, void* k) {
    // If map OR Key is NULL, do nothing.
    if (map == NULL || k == NULL) {
        return NULL;
    }

    int table_idx = map->hash(k) % map->n_entries;
    hash_node_t* current_node = map->table[table_idx]->head;
    
    // Lock the map.
    pthread_mutex_lock(&map->map_lock); 
    
    while (current_node != NULL) {
        // Return 1 if two keys are equal.
        if(map->cmp(current_node->key, k) == 1){
            //printf("%s\n", current_node->value);
            pthread_mutex_unlock(&map->map_lock);
            return current_node->value;
        }
        current_node = current_node->next;
    }

    // Unlock the map.
    pthread_mutex_unlock(&map->map_lock);
    return NULL;
}

// Destroy every entries in the hash table.
void entry_destory(struct hash_map* map, hash_entry_t* entry) {
    hash_node_t* current_node = entry->head;
    hash_node_t* temp_node;

    while (current_node != NULL) {
        temp_node = current_node->next;
        map->key_destruct(current_node->key);
        map->value_destruct(current_node->value);
        free(current_node);
        current_node = temp_node;
    }
	free(entry);
}

void hash_map_destroy(struct hash_map* map) {
    // If map is NULL, do nothing.
    if(map == NULL){
        return;
    }
    // Lock the map.
    pthread_mutex_lock(&map->map_lock); 
	
    for (int table_idx = 0; table_idx < map->n_entries; table_idx++) {
		entry_destory(map, map->table[table_idx]);
	}

	free(map->table);
    // Unlock the map.
    pthread_mutex_unlock(&map->map_lock);
	free(map);
}

int main(){
 struct hash_map* new_map = hash_map_new(hash, cmp, key_destruct, value_destruct);
    
    // Put test
    hash_map_put_entry_move(new_map, "Sean", "0432539279");
    hash_map_put_entry_move(new_map, "Jenny", "0430278410");
    hash_map_put_entry_move(new_map, "John", "0405622229");
    hash_map_put_entry_move(new_map, "Eric", "0420880574");
    hash_map_put_entry_move(new_map, "Eliot", "0434504096");
    hash_map_put_entry_move(new_map, "Jay", "0426085800");
    hash_map_put_entry_move(new_map, "San", "0452517242");
    hash_map_put_entry_move(new_map, "Brian", "0451101208");
    hash_map_put_entry_move(new_map, "Edward", "0410890664");
    hash_map_put_entry_move(new_map, "Samuel", "0425481436");
    hash_map_put_entry_move(new_map, "Louie", "0426080682");
    hash_map_put_entry_move(new_map, "Kevin", "0451249618");
    hash_map_put_entry_move(new_map, "Kevin", "1234567899"); // Testing for inserting same key. The value is expeceted to be changed in the hash map.
    hash_map_put_entry_move(new_map, NULL, NULL); // Testing for NULL values
    dump_hashmap(new_map);

    
    /* Remove test
    hash_map_remove_entry(new_map, "San");
    hash_map_remove_entry(new_map, "Jay");
    hash_map_remove_entry(new_map, "Brian");
    hash_map_remove_entry(new_map, "John");
    hash_map_remove_entry(new_map, "Kevin");
    hash_map_remove_entry(new_map, "Kevin"); // Kevin is removed before.
    hash_map_remove_entry(new_map, "Not in hashmap"); // This key does not exist in the hashmap.
    hash_map_remove_entry(NULL, NULL); // Testing for NULL values
    printf("\n");
    dump_hashmap(new_map);
    */
    
    /* Get test
    hash_map_get_value_ref(new_map, "Sean");
    hash_map_get_value_ref(new_map, "Jenny");
    hash_map_get_value_ref(new_map, NULL);
    hash_map_get_value_ref(new_map, "Not in hashmap");
    */
    
    hash_map_destroy(new_map);
    return 0;
}
