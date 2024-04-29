#include "utils.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"

void *append_to_array(void *array, const void *elem, size_t size)
{
    DEF_ARR(uint8_t) *arr = array;

    if (arr->size + 1 > arr->alloc) {
        arr->alloc = (arr->alloc) ? arr->alloc * 2 : 2;
        void *ptr = reallocarray(arr->arr, arr->alloc + 1, size);
        if (ptr == NULL)
            return NULL;
        arr->arr = ptr;
    }
    memcpy(arr->arr + (arr->size * size), elem, size);
    arr->size += 1;
    return array;
}

char *append_to_buffer(char_array_t *array, const char *str, size_t lenght)
{
    if (array == NULL || str == NULL)
        return NULL;

    if (array->alloc < array->size + lenght) {
        while (array->alloc < array->size + lenght)
            array->alloc = (array->alloc) ? array->alloc * 2 : 2;
        void *ptr = reallocarray(array->arr, array->alloc + 1, sizeof(char));
        if (ptr == NULL)
            return NULL;
        array->arr = ptr;
    }

    memcpy(array->arr + array->size, str, lenght);
    array->size += lenght;
    array->arr[array->size] = '\0';
    return array->arr;
}
