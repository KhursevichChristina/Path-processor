#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "../../lab3/vector/generic.h"
#include "generic.h"

#define SLOT_STATE_SIZE 1
#define MAX_FULLNESS 0.5

static int generate_second_hash(int hash) {
    int second_hash = hash * 131;
    second_hash = (second_hash & INT_MAX);
    if (second_hash % 2 == 0) {
        second_hash++;
    }
    return second_hash;
}

static size_t find_slot(HashTable *table, void *key, HashFunc hash, CmpFunc cmp) {
    int first_hash = hash(key);
    int second_hash = generate_second_hash(first_hash);
    size_t index = first_hash % table->capacity;
    size_t step = second_hash % table->capacity;

    if (step == 0) {
        step = 1;
    }

    size_t first_available_index = table->capacity;

    for (size_t i = 0; i < table->capacity; i++) {
        size_t current_index = (index + i * step) % table->capacity;
        char* slots = table->values->data;
        size_t slot_size = SLOT_STATE_SIZE + table->key_size + table->val_size;
        unsigned char* slot_state = (unsigned char*)(slots + current_index * slot_size);
        if (!slot_state) {
            break;
        }
        if (*slot_state == SLOT_OCCUPIED) {
            void *current_key = (unsigned char*)(slots + current_index * slot_size + SLOT_STATE_SIZE);
            if (cmp(current_key, key)) {
                return current_index;
            }
        }
        else if (*slot_state == SLOT_EMPTY) {
            if (first_available_index != table->capacity) {
                return first_available_index;
            }
            return current_index;
        }
        else if (*slot_state == SLOT_DELETED) {
            if (first_available_index == table->capacity) {
                first_available_index = current_index;
            }
        }
    }
    return first_available_index;
}

int HashInt(const void *key)
{
    if (!key) {
        return 0;
    }
    
    int key_hash = *(const int*)key;
    if (key_hash >= 0) {
        return key_hash;
    }
    else {
        return -key_hash;
    }
}

int HashLongLong(const void *key)
{
    if (!key) {
        return 0;
    }
    
    long long key_hash = *(const long long*)key;
    if (key_hash >= 0) {
        return key_hash;
    }
    else {
        return -key_hash;
    }
}

int HashString(const void *key)
{
    if (!key) {
        return 0;
    }

    int key_hash = 5381;
    const char *string = (const char*)key;
    int c;

    while ((c = *string++)) {
        key_hash = ((key_hash << 5) + key_hash) + c;
    }
    return (int)(key_hash & INT_MAX);
}

HashTable *createHashTable(size_t key_size, size_t val_size)
{
    if (key_size == 0 || val_size == 0) {
        return NULL;
    }

    HashTable *hash_table = malloc(sizeof(HashTable));
    if (!hash_table) {
        return NULL;
    }

    size_t slot_size = SLOT_STATE_SIZE + key_size + val_size;
    hash_table->values = createVector(slot_size);
    if (!hash_table->values) {
        free(hash_table);
        return NULL;
    }

    unsigned char empty_state = SLOT_EMPTY;
    for (size_t i = 0; i < TABLE_MIN_SIZE; i++) {
        if (appendVectorItem(hash_table->values, &empty_state) != 0) {
            vectorFree(hash_table->values);
            free(hash_table);
            return NULL;
        }
    }

    hash_table->capacity = TABLE_MIN_SIZE;
    hash_table->key_size = key_size;
    hash_table->size = 0;
    hash_table->val_size = val_size;

    return hash_table;
}

void setItemHashTable(HashTable *table, void *key, void *data, HashFunc hash, CmpFunc cmp)
{
    if (!table || !key || !data || !hash || !cmp) {
        return;
    }

    double fullness = (double)table->size / table->capacity;
    if (fullness > MAX_FULLNESS) {
        rehashHashTable(table, hash, cmp);
    }

    size_t slot_index = find_slot(table, key, hash, cmp);
    
    char* slots = table->values->data;
    size_t slot_size = SLOT_STATE_SIZE + table->key_size + table->val_size;
    unsigned char* slot_state = (unsigned char*)(slots + slot_index * slot_size);
 
    if (*slot_state == SLOT_OCCUPIED) {
        void* value_pointer = (unsigned char*)(slots + slot_index * slot_size + SLOT_STATE_SIZE + table->key_size);
        memcpy(value_pointer, data, table->val_size);
    } 
    else {
        *slot_state = SLOT_OCCUPIED;
        void* key_pointer = (unsigned char*)(slots + slot_index * slot_size + SLOT_STATE_SIZE);
        memcpy(key_pointer, key, table->key_size);
        void* value_pointer = (unsigned char*)(slots + slot_index * slot_size + SLOT_STATE_SIZE + table->key_size);
        memcpy(value_pointer, data, table->val_size);
        table->size++;
    }
}

void rehashHashTable(HashTable *table, HashFunc hash, CmpFunc cmp)
{
    if (!table || !hash || !cmp) {
        return;
    }

    Vector *old_values = table->values;
    size_t old_capacity = table->capacity;
    size_t new_capacity = old_capacity * 2;
    size_t slot_size = SLOT_STATE_SIZE + table->key_size + table->val_size;

    table->values = createVector(slot_size);
    if (!table->values) {
        table->values = old_values;
        return;
    }

    unsigned char empty_state = SLOT_EMPTY;
    for (size_t i = 0; i < new_capacity; i++) {
        if (appendVectorItem(table->values, &empty_state) != 0) {
            vectorFree(table->values);
            table->values = old_values;
            return;
        }
    }

    table->capacity = new_capacity;
    table->size = 0;

    char* old_slots = old_values->data;
    for (size_t i = 0; i < old_capacity; i++) {
        unsigned char* old_slot_state = (unsigned char*)(old_slots + i * slot_size);

        if (*old_slot_state == SLOT_OCCUPIED) {
            unsigned char* key_pointer = (unsigned char*)(old_slots + i * slot_size + SLOT_STATE_SIZE);
            unsigned char* values_pointer = (unsigned char*)(old_slots + i * slot_size + SLOT_STATE_SIZE + table->key_size);
            setItemHashTable(table, key_pointer, values_pointer, hash, cmp);
        }
    }

    vectorFree(old_values);
}

void *getItemHashTable(HashTable *table, void *key, HashFunc hash, CmpFunc cmp)
{
    if (!table || !key || !hash || !cmp) {
        return NULL;
    }

    size_t slot_index = find_slot(table, key, hash, cmp);
    
    if (slot_index == table->capacity) {
        return NULL;
    }

    char* slots = table->values->data;
    size_t slot_size = SLOT_STATE_SIZE + table->key_size + table->val_size;
    unsigned char* slot_state = (unsigned char*)(slots + slot_index * slot_size);

    if (*slot_state == SLOT_OCCUPIED) {
        void* value_pointer = (unsigned char*)(slots + slot_index * slot_size + SLOT_STATE_SIZE + table->key_size);
        return value_pointer;
    }

    return NULL;
}

void *popItemHashTable(HashTable *table, void *key, HashFunc hash, CmpFunc cmp)
{
    if (!table || !key || !hash || !cmp) {
        return NULL;
    }

    size_t slot_index = find_slot(table, key, hash, cmp);
    
    if (slot_index == table->capacity) {
        return NULL;
    }

    char* slots = table->values->data;
    size_t slot_size = SLOT_STATE_SIZE + table->key_size + table->val_size;
    unsigned char* slot_state = (unsigned char*)(slots + slot_index * slot_size);

    if (*slot_state == SLOT_OCCUPIED) {
        void* value_pointer = (unsigned char*)(slots + slot_index * slot_size + SLOT_STATE_SIZE + table->key_size);
        void* copy_value = malloc(table->val_size);
        if (!copy_value) {
            return NULL;
        }
        memcpy(copy_value, value_pointer, table->val_size);
        *slot_state = SLOT_DELETED;
        table->size--;
        return copy_value;
    }

    return NULL;
}

unsigned long int getCollisionCount(HashTable *table, HashFunc hash)
{
    if (!table || !hash) {
        return 0;
    }

    char* slots = table->values->data;
    size_t slot_size = SLOT_STATE_SIZE + table->key_size + table->val_size;
    unsigned long int count_collision = 0;

    for (size_t i = 0; i < table->capacity; i++) {
        unsigned char* slot_state = (unsigned char*)(slots + i * slot_size);
        if (*slot_state == SLOT_OCCUPIED) {
            void* key_pointer = (unsigned char*)(slots + i * slot_size + SLOT_STATE_SIZE);
            size_t first_key_hash = hash(key_pointer) % table->capacity;
            if (first_key_hash != i) {
                count_collision++;
            }
        }
    }

    return count_collision;
}

void freeHashTable(HashTable *table)
{
    if (!table) {
        return;
    }

    if (table->values) {
        vectorFree(table->values);
    }
    
    free(table);
}