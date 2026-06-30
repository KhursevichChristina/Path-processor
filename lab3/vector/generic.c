#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "generic.h"

// Вспомогательная функция для изменения размера
static bool needToResize(Vector *vector, bool *increase)
{
    if (!vector) { // O(1)
        return false;
    } 

    if (vector->size >= vector->capacity) { // O(1)
        *increase = true; // O(1)
        return true;
    }

    if (vector->size > 0 && vector->size <= vector->capacity / 4) { // O(1)
        *increase = false; // O(1)
        return true;
    }

    return false;

    /*
    Оценка сверху - O(1), число операций не зависит от размера входа
    Точная оценка - Θ(1)
    Оценка снизу - Ω(1)
    */
}

// Определяем увеличивать размер или уменьшать
static int resize(Vector *vector, bool increase)
{
    if (!vector) { // O(1)
        return -1;
    }
    
    size_t new_capacity; // O(1)
    if (increase) { // O(1)
        new_capacity = vector->capacity * 2; // O(1)
    }
    else {
        new_capacity = vector->capacity / 2; // O(1)
        if (new_capacity < MIN_SIZE) { // O(1)
            new_capacity = MIN_SIZE; // O(1)
        }
    }

    void* new_vector = malloc(new_capacity * vector->elem_size); // O(1)
    if (!new_vector) { // O(1)
        return -1;
    }

    if (vector->size > 0) { // O(1)
        memcpy(new_vector, vector->data, vector->size * vector->elem_size); // O(n), n = vector->size
    }

    void* old_data = vector->data; // O(1)
    
    vector->data = new_vector; // O(1)
    vector->capacity = new_capacity; // O(1)

    free(old_data); // O(1)
    
    return 0;

    /*
    Оценка сверху - O(n)
    Точная оценка - Θ(n)
    Оценка снизу - Ω(1), пустой вектор
    */
}

Vector *createVector(size_t elem_size)
{
    if (elem_size == 0) {
        return NULL;
    }

    Vector *vector =  (Vector*) malloc(sizeof(Vector));
    if (!vector) {
        return NULL;
    }

    vector->elem_size = elem_size;
    vector->size = 0;
    vector->capacity = MIN_SIZE;
    vector->data = malloc(elem_size * MIN_SIZE);

    if (!vector->data) {
        free(vector);
        return NULL;
    }

    return vector;
}

int appendVectorItem(Vector *vector, void *el)
{
    if (!vector || !el) { // O(1)
        return -1;
    }

    bool increase; // O(1)
    if (needToResize(vector, &increase)) { // O(1)
        if (increase) { // O(1)
            if (resize(vector, increase) != 0) { // O(n), n = vector->size
                return -1;
            }
        }
    }

    void* new_el = (char*)vector->data + (vector->size * vector->elem_size); // O(1)
    memcpy(new_el, el, vector->elem_size); // O(1), точно знаем размер копируемого элемента
    vector->size++; // O(1)

    return 0;

    /*
    Оценка сверху - O(n)
    Точная оценка - Θ(1), чаще всего добавления без увеличения размера
    Оценка снизу - Ω(1), добавление без увеличения размера вектора
    */
}

void *getVectorItem(Vector *vector, size_t index)
{
    if (!vector) { // O(1)
        return NULL;
    }
    if (index >= vector->size) { // O(1)
        return NULL;
    }

    void* pointer = (char*)vector->data + (index * vector->elem_size); // O(1)
    return pointer;

    /*
    Оценка сверху - O(1), не зависит от размера входа, константное время
    Точная оценка - Θ(1)
    Оценка снизу - Ω(1)
    */
}

int setVectorItem(Vector *vector, size_t index, void *value)
{
    if (!vector || !value) { // O(1)
        return -1;
    }
    if (index >= vector->size) { // O(1)
        return -1;
    }

    void* pointer = (char*)vector->data + (index * vector->elem_size); // O(1)
    memcpy(pointer, value, vector->elem_size); // O(1), точно знаем размер копируемого элемента
    
    return 0;

    /*
    Оценка сверху - O(1), не зависит от размера входа, константное время
    Точная оценка - Θ(1)
    Оценка снизу - Ω(1)
    */  
}

void *popVectorItem(Vector *vector, size_t index)
{
    if (!vector) { // O(1)
        return NULL;
    }
    if (index >= vector->size) { // O(1)
        return NULL;
    }

    void* pop_item = malloc(vector->elem_size); // O(1)
    if (!pop_item) { // O(1)
        return NULL;
    }

    void* pointer = (char*)vector->data + (index * vector->elem_size); // O(1)
    memcpy(pop_item, pointer, vector->elem_size); // O(1), точно знаем размер копируемого элемента

    if (index < vector->size - 1) { // O(1)
        void* new_position = (char*)vector->data + (index * vector->elem_size); // O(1)
        void* old_position = (char*)vector->data + ((index + 1) * vector->elem_size); // O(1)
        size_t size_data = (vector->size - index - 1) * vector->elem_size; // O(1)
        memmove(new_position, old_position, size_data); // O(k), k = vector->size - index - 1
    }

    vector->size--; // O(1)

    bool increase; // O(1)
    if (needToResize(vector, &increase)) { // O(1)
        if (!increase){ // O(1)
            resize(vector, increase); // O(n)
        }
    }

    return pop_item;
    /*
    Оценка сверху - O(n)
    Точная оценка - Θ(n)
    Оценка снизу - Ω(1), удаление последнего элемента без изменений размера вектора
    */  
}

long int findVectorItem(Vector *vector, void *value, EqualsFunc cmp)
{
    if (!vector || !value || !cmp){ // O(1)
        return -1;
    }

    for (long int i = 0; i < vector->size; i++) { // O(n)
        void* current_item = (char*)vector->data + (i * vector->elem_size); // O(n) * O(1)
        if (cmp(current_item, value)) { // O(n) * O(1)
            return i;
        }
    }
    
    return -1;

    /*
    Оценка сверху - O(n)
    Точная оценка - Θ(n)
    Оценка снизу - Ω(1), найденный элемент на первой позиции
    */  
}

int vectorFree(Vector *vector)
{
    if (!vector) {
        return -1;
    }

    void *data_to_free = vector->data;
    
    free(vector);
    
    if (data_to_free) {
        free(data_to_free);
    }
    
    return 0;
}