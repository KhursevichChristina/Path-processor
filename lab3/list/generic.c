#include "generic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GenericList *createList(size_t elem_size)
{
    if (elem_size == 0) {
        printf("Error: elem_size cannot be zero\n");
        exit(EXIT_FAILURE);
    }
    
    GenericList *list = (GenericList*) malloc(sizeof(GenericList));

    if (!list) {
        printf("Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    
    list->elem_size = elem_size;
    list->head = NULL;
    
    return list;
}

void appendItem(GenericList *list, void *data)
{
    if (!list || !data) { // O(1)
        printf("Error: Invalid arguments to appendItem\n");
        exit(EXIT_FAILURE);
    }
    
    Node *new_node = (Node*) malloc(sizeof(Node)); // O(1)
    if (!new_node) { // O(1)
        printf("Error: Memory allocation failed for Node\n");
        exit(EXIT_FAILURE);
    }
    
    new_node->data = malloc(list->elem_size); // O(1)
    if (!new_node->data) { // O(1)
        printf("Error: Memory allocation failed for node data\n");
        free(new_node); // O(1)
        exit(EXIT_FAILURE);
    }
    
    memcpy(new_node->data, data, list->elem_size); // O(1), точно знаем размер копируемого элемента
    new_node->next = NULL; // O(1)
    
    if (list->head == NULL) { // O(1)
        list->head = new_node; // O(1)
        return;
    }

    Node *current = list->head; // O(1)
    while (current->next != NULL) { // O(n)
        current = current->next; // O(n) * O(1)
    }

    current->next = new_node; // O(1)

    /*
    Оценка сверху - O(n)
    Точная оценка - Θ(n)
    Оценка снизу - Ω(1), добавление в пустой список
    */
}

int findItem(GenericList *list, void *value, EqualsFunc cmp)
{
    if (!list || !value || !cmp) { // O(1)
        return (unsigned int)-1;
    }

    Node *current = list->head; // O(1)
    unsigned int index = 0; // O(1)

    while (current != NULL) { // O(n)
        if (cmp(current->data, value)) { // O(n) * O(1)
            return index;
        }
        current = current->next; // O(n) * O(1)
        index++; // O(n) * O(1)
    }

    return (unsigned int)-1;

    /*
    Оценка сверху - O(n)
    Точная оценка - Θ(n)
    Оценка снизу - Ω(1), найденный элемент первый
    */
}

void *popItem(GenericList *list, size_t index)
{
    if (!list || list->head == NULL) { // O(1)
        return NULL;
    }

    size_t current_index = 0; // O(1)
    Node *current_node = list->head; // O(1)
    Node *previous = NULL; // O(1)

    while (current_node != NULL && current_index < index) { // O(k), k = min(n, index)
        previous = current_node; // O(k) * O(1)
        current_node = current_node->next; // O(k) * O(1)
        current_index++; // O(k) * O(1)
    }

    if (current_node == NULL) { // O(1)
        return NULL;
    }

    void *data_copy = malloc(list->elem_size); // O(1)
    if (!data_copy) { // O(1)
        printf("Error: Memory allocation failed for data copy\n");
        return NULL;
    }
    memcpy(data_copy, current_node->data, list->elem_size); // O(1), точно знаем размер копируемого элемента

    if (previous == NULL) { // O(1)
        list->head = current_node->next; // O(1)
    }
    else {
        previous->next = current_node->next; // O(1)
    }

    free(current_node->data); // O(1)
    free(current_node); // O(1)

    return data_copy;

    /*
    Оценка сверху - O(n)
    Точная оценка - Θ(n)
    Оценка снизу - Ω(1), удаление первого элемента
    */
}

void freeList(GenericList *list)
{
    if (!list) {
        return;
    }

    Node *current = list->head;
    while (current != NULL) {
        Node *next = current->next;
        free(current->data);
        free(current);
        current = next;
    }

    free(list);
}

unsigned int listLength(GenericList *list)
{
    if (!list) { // O(1)
        return 0;
    }

    unsigned int count = 0; // O(1)
    Node *current = list->head; // O(1)

    while (current != NULL) { // O(n)
        count++; // O(n) * O(1)
        current = current->next; // O(n) * O(1)
    }

    return count;

    /*
    Оценка сверху - O(n)
    Точная оценка - Θ(n)
    Оценка снизу - Ω(1), количество элементов пустого списка
    */
}
