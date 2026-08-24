/*
 * Copyright (c) 2024 Hisilicon Technologies Co., Ltd.
 * Hikptool is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 */

#ifndef SH_JSON_H
#define SH_JSON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Distinct node kinds used by the DOM. */
enum sh_json_kind {
	SH_JSON_NIL = 0,   /* no kind assigned yet / null literal */
	SH_JSON_NO = 1,    /* JSON false */
	SH_JSON_YES = 2,   /* JSON true */
	SH_JSON_NUM = 3,   /* JSON number */
	SH_JSON_BUF = 4,   /* JSON string */
	SH_JSON_SEQ = 5,   /* JSON array */
	SH_JSON_MAP = 6    /* JSON object */
};

/*
 * One node of the JSON document tree.
 *  - fwd/bck: next / previous sibling inside the same parent;
 *  - head:    first child (members of an object, elements of an array);
 *  - name:    member name (meaningful only inside an object);
 *  - str:     owned payload for a string node;
 *  - ival / dval: integer and floating point views for a number node.
 */
typedef struct sh_json {
	struct sh_json *fwd;
	struct sh_json *bck;
	struct sh_json *head;
	int kind;
	char *name;
	char *str;
	long ival;
	double dval;
} sh_json;

/* Parse a JSON document. Returns NULL on any syntax or memory error. */
sh_json *sh_json_parse(const char *text);

/* Recursively release a node together with all of its descendants. */
void sh_json_delete(sh_json *node);

/* Find a member by name; comparison is case-insensitive. */
sh_json *sh_json_get_item(const sh_json *object, const char *name);
/* Find a member by name; comparison is case-sensitive. */
sh_json *sh_json_get_item_cs(const sh_json *object, const char *name);

/* Fetch the index-th element of an array (0-based). */
sh_json *sh_json_item_at(const sh_json *array, int index);
/* Return the number of children; 0 if the node is not an array. */
int sh_json_item_count(const sh_json *array);

/* Create a fresh object, array, string or number node. */
sh_json *sh_json_create_obj(void);
sh_json *sh_json_create_arr(void);
sh_json *sh_json_create_str(const char *value);
sh_json *sh_json_create_num(double value);

/*
 * Attach item to object under name. On success the object takes ownership
 * of item and the node is returned; on failure item is kept by the caller
 * and NULL is returned. The member name is duplicated.
 */
sh_json *sh_json_attach(sh_json *object, const char *name, sh_json *item);
/* Same as sh_json_attach but reports success as 1 / 0. */
int sh_json_attach_checked(sh_json *object, const char *name, sh_json *item);
/* Append item as a new element of array. Returns 1 on success, 0 on error. */
int sh_json_push(sh_json *array, sh_json *item);

/* Create a string member under name and return the created node. */
sh_json *sh_json_put_str(sh_json *object, const char *name, const char *value);
/* Create a string member under name, report success as 1 / 0. */
int sh_json_put_str_checked(sh_json *object, const char *name,
			    const char *value);
/* Create a number member under name and return the created node. */
sh_json *sh_json_put_num(sh_json *object, const char *name, double value);

/* Serialize a tree into a newly allocated, NUL-terminated string. */
char *sh_json_write(const sh_json *node);

#ifdef __cplusplus
}
#endif

#endif /* SH_JSON_H */