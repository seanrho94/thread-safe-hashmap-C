## Hashy

# The Thread Safe Kind

Note: You can ignore reference to **size** in put and get functions.

Your hash map will need to be able to store any type in conjunction with the function pointers provided during construction. You have been given an empty struct which you will need to complete. While the key and value can be any type, the hash and comparison functions must work on the key values. Use the hash, comparison and destruct function pointers to map data, resolve collisions and deallocate memory.

It is recommended that you use a dynamic array to support the entries within your hash map.

Keep in mind, You will also need to ensure that the hash map is thread safe and performant. You will need to apply fine grained locking to the data structure to ensure that the hash map can support concurrent insertions and removals. Retrieving and placing an element should be constant time on average ( O(1) ) operation.

In this instance, it is advised you construct a hash map where each entry mapped by a hash is its own list (separate chaining). If more than one distinct key results in the same hash, the list should contain these keys at the same location.

Note: Open-addressing will hinder your performance progress.

You will need to implement the following functions for your hash map:

## Creation

To ensure we have a hashmap which can be used with any kind of hashing function and that we have a comparison function and destruction function for complex objects. The hash function will not involve the compression component, you will need to compress the hash to fit your implementation. If the hash, cmp or destruction function pointers are NULL, this function should return NULL.

/**

-   Creates a new hash map with an associated hash function,
-   comparison function and destructor function. 
- struct hashmap_ hash_map_new(size_t (_hash)(void_), int (cmp)(void_,void*), void (_key_destruct)(void_), void (_value_destruct)(void_));

## Put

The put functions will insert a key and value into the hashmap, in the event the key exists within the hashmap, the value will be replaced, since the data structure owns the data, you will need to free the data. It is recommended you record the size variable with the entry.

/**

-   Puts an entry in the hashmap, associating a key with a value
-   If the key does not exists, a new entry should be created and the key
-   and value assumed to be owned by the hash map.
-   If an entry exists, old entry should be removed (hash_map_remove_entry). 
- void hash_map_put_entry_move(struct hash_map_ map, void* k, void* v);

## Remove

If the key exists with the hashmap, it will be removed as well as a the value, if the key does not exist, nothing will be removed.

/**

-   Your hash map must remove an entry from the hash map using the key
-   If the key is not present in this map, the function will not make
-   any changes to the hash map.
-   If the key exists, it will remove the entry and value.
- void hash_map_remove_entry(struct hash_map_ map, void* k);

## Get

Your hashmap should retrieve the value given the key. In the event the key does not exist, your function should return NULL.

/**

-   This will retrieve an entry based on the key within the map itself
-   It will return a reference of associated with the key so it can be modified in place.
-   Note: Since the access of the reference is in a separate context.
-   any modifications through the returned object will not be MT-Safe. 
- void_ hash_map_get_value_ref(struct hash_map* map, void* k);

## Destroy

This will destroy the current entries and the map itself. The destruct function will be called on each object to ensure that it is freed.

/**

-   Destroys all entries within the hash map, using the destructor function
-   It will also free the map after all entries have been removed.
-   If the map is NULL, nothing will occur. 
- void hash_map_destroy(struct hash_map_ map);
