/*
 * Copyright (c) 2009-2017 Dave Gamble and cJSON contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef cJSON__h
#define cJSON__h

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* cJSON Types */
#define cJSON_Invalid (0)
#define cJSON_False   (1 << 0)
#define cJSON_True    (1 << 1)
#define cJSON_NULL    (1 << 2)
#define cJSON_Number  (1 << 3)
#define cJSON_String  (1 << 4)
#define cJSON_Array   (1 << 5)
#define cJSON_Object  (1 << 6)
#define cJSON_Raw     (1 << 7)

#define cJSON_IsReference   256
#define cJSON_StringIsConst 512

/* The cJSON structure */
typedef struct cJSON {
    struct cJSON *next;
    struct cJSON *prev;
    struct cJSON *child;

    int type;

    char *valuestring;
    int valueint;
    double valuedouble;

    char *string;
} cJSON;

/* Hooks for allocation */
typedef struct cJSON_Hooks {
    void *(*malloc_fn)(size_t sz);
    void (*free_fn)(void *ptr);
} cJSON_Hooks;

/* Supply malloc/realloc/free functions to cJSON */
void cJSON_InitHooks(cJSON_Hooks *hooks);

/* Parse a JSON string */
cJSON *cJSON_Parse(const char *value);
cJSON *cJSON_ParseWithOpts(const char *value, const char **return_parse_end, int require_null_terminated);

/* Delete a cJSON entity and all subentities */
void cJSON_Delete(cJSON *item);

/* Get object item by key (case-insensitive) */
cJSON *cJSON_GetObjectItem(const cJSON *object, const char *string);
/* Get object item by key (case-sensitive) */
cJSON *cJSON_GetObjectItemCaseSensitive(const cJSON *object, const char *string);

/* Check if object has an item */
int cJSON_HasObjectItem(const cJSON *object, const char *string);

/* Get array item by index */
cJSON *cJSON_GetArrayItem(const cJSON *array, int index);
int    cJSON_GetArraySize(const cJSON *array);

/* Get string value */
const char *cJSON_GetStringValue(const cJSON *item);

/* Get number value */
double cJSON_GetNumberValue(const cJSON *item);

/* Create objects/arrays/values */
cJSON *cJSON_CreateObject(void);
cJSON *cJSON_CreateArray(void);
cJSON *cJSON_CreateString(const char *string);
cJSON *cJSON_CreateNumber(double num);
cJSON *cJSON_CreateBool(int b);
cJSON *cJSON_CreateNull(void);

/* Add items to objects/arrays */
void cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item);
void cJSON_AddItemToArray(cJSON *array, cJSON *item);

/*
 * Add an item to an object and report whether it succeeded.
 *
 * Return:
 *   1 - success; ownership of item is transferred to object
 *   0 - failure; caller still owns item
 */
int cJSON_AddItemToObjectChecked(cJSON *object, const char *string,
                 cJSON *item);

/* For convenience, adds item and returns reference to it */
cJSON *cJSON_AddStringToObject(cJSON *object, const char *name, const char *string);
cJSON *cJSON_AddNumberToObject(cJSON *object, const char *name, double number);

/*
 * Create and add a string item to an object.
 *
 * Return:
 *   1 - success
 *   0 - failure; the internally created item has been released
 */
int cJSON_AddStringToObjectChecked(cJSON *object, const char *name,
                   const char *string);
/* Print/format a cJSON tree */
char *cJSON_Print(const cJSON *item);
char *cJSON_PrintUnformatted(const cJSON *item);

#ifdef __cplusplus
}
#endif

#endif /* cJSON__h */
