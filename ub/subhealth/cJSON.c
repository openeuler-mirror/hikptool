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

#include "cJSON.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
/* Internal malloc/free wrappers */
static void *(*cJSON_malloc)(size_t sz) = malloc;
static void (*cJSON_free)(void *ptr) = free;

void cJSON_InitHooks(cJSON_Hooks *hooks)
{
	if (!hooks)
		return;
	cJSON_malloc = (hooks->malloc_fn) ? hooks->malloc_fn : malloc;
	cJSON_free = (hooks->free_fn) ? hooks->free_fn : free;
}

/* Internal allocation helpers */
static unsigned char *cJSON_strdup(const char *str)
{
	size_t len;
	unsigned char *copy;

	if (!str)
		return NULL;
	len = strlen(str) + 1;
	copy = (unsigned char *)cJSON_malloc(len);
	if (copy)
		memcpy(copy, str, len);
	return copy;
}

/* ========================================================================
 * Parser
 * ======================================================================== */

typedef struct {
	const unsigned char *json;
	size_t position;
} parse_buffer;

/* Skip whitespace */
static void skip_whitespace(parse_buffer *buf)
{
	const unsigned char *p = buf->json + buf->position;
	while (*p && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n'))
		p++;
	buf->position = (size_t)(p - buf->json);
}

/* Create a new cJSON item */
static cJSON *cJSON_New_Item(void)
{
	cJSON *node = (cJSON *)cJSON_malloc(sizeof(cJSON));
	if (node)
		memset(node, 0, sizeof(cJSON));
	return node;
}

/* Parse functions - forward declarations */
static cJSON *parse_value(parse_buffer *buf);
static cJSON *parse_object(parse_buffer *buf);
static cJSON *parse_array(parse_buffer *buf);
static cJSON *parse_string(parse_buffer *buf);
static cJSON *parse_number(parse_buffer *buf);

/*
 * Parser core - parse any JSON value
 */
static cJSON *parse_value(parse_buffer *buf)
{
	const unsigned char *p;
	cJSON *item;

	skip_whitespace(buf);
	p = buf->json + buf->position;

	if (!*p)
		return NULL;

	switch (*p) {
	case '"':
		return parse_string(buf);
	case '{':
		return parse_object(buf);
	case '[':
		return parse_array(buf);
	case 't':
		if (strncmp((const char *)p, "true", 4) == 0) {
			buf->position += 4;
			item = cJSON_New_Item();
			if (item)
				item->type = cJSON_True;
			return item;
		}
		break;
	case 'f':
		if (strncmp((const char *)p, "false", 5) == 0) {
			buf->position += 5;
			item = cJSON_New_Item();
			if (item)
				item->type = cJSON_False;
			return item;
		}
		break;
	case 'n':
		if (strncmp((const char *)p, "null", 4) == 0) {
			buf->position += 4;
			item = cJSON_New_Item();
			if (item)
				item->type = cJSON_NULL;
			return item;
		}
		break;
	default:
		if (*p == '-' || (*p >= '0' && *p <= '9'))
			return parse_number(buf);
		break;
	}
	return NULL;
}

/* Parse an object */
static cJSON *parse_object(parse_buffer *buf)
{
	cJSON *item, *child;
	const unsigned char *p;

	item = cJSON_New_Item();
	if (!item)
		return NULL;
	item->type = cJSON_Object;

	/* Skip '{' */
	buf->position++;
	skip_whitespace(buf);

	p = buf->json + buf->position;
	if (*p == '}') {
		buf->position++;
		return item;
	}

	while (*p) {
		cJSON *key, *value;

		skip_whitespace(buf);
		p = buf->json + buf->position;

		/* parse key */
		if (*p != '"')
			goto fail;
		key = parse_string(buf);
		if (!key)
			goto fail;

		skip_whitespace(buf);
		p = buf->json + buf->position;
		if (*p != ':') {
			cJSON_Delete(key);
			goto fail;
		}
		buf->position++;

		/* parse value */
		value = parse_value(buf);
		if (!value) {
			cJSON_Delete(key);
			goto fail;
		}

		/* Set the key string on the value node */
		value->string = key->valuestring;
		key->valuestring = NULL;
		cJSON_Delete(key);

		/* Link into child list */
		if (!item->child)
			item->child = value;
		else {
			child = item->child;
			while (child->next)
				child = child->next;
			child->next = value;
			value->prev = child;
		}

		skip_whitespace(buf);
		p = buf->json + buf->position;
		if (*p == '}') {
			buf->position++;
			return item;
		}
		if (*p != ',')
			goto fail;
		buf->position++;
	}

fail:
	cJSON_Delete(item);
	return NULL;
}

/* Parse an array */
static cJSON *parse_array(parse_buffer *buf)
{
	cJSON *item, *child;
	const unsigned char *p;

	item = cJSON_New_Item();
	if (!item)
		return NULL;
	item->type = cJSON_Array;

	buf->position++;
	skip_whitespace(buf);

	p = buf->json + buf->position;
	if (*p == ']') {
		buf->position++;
		return item;
	}

	while (*p) {
		cJSON *value;

		value = parse_value(buf);
		if (!value)
			goto fail;

		if (!item->child)
			item->child = value;
		else {
			child = item->child;
			while (child->next)
				child = child->next;
			child->next = value;
			value->prev = child;
		}

		skip_whitespace(buf);
		p = buf->json + buf->position;
		if (*p == ']') {
			buf->position++;
			return item;
		}
		if (*p != ',')
			goto fail;
		buf->position++;
	}

fail:
	cJSON_Delete(item);
	return NULL;
}

/* Parse a string */
static cJSON *parse_string(parse_buffer *buf)
{
	cJSON *item;
	const unsigned char *p, *start;
	size_t len, i;
	unsigned char *out;

	item = cJSON_New_Item();
	if (!item)
		return NULL;
	item->type = cJSON_String;

	buf->position++; /* skip opening " */
	p = buf->json + buf->position;
	start = p;

	/* Find end of string */
	while (*p && *p != '"') {
		if (*p == '\\') {
			p++;
			if (!*p)
				goto fail;
		}
		p++;
	}

	len = (size_t)(p - start);
	out = (unsigned char *)cJSON_malloc(len + 1);
	if (!out)
		goto fail;

	for (i = 0, p = start; p < start + len; p++) {
		if (*p == '\\') {
			p++;
			if (p >= start + len) {
				cJSON_free(out);
				goto fail;
			}
			switch (*p) {
			case '\\': out[i++] = '\\'; break;
			case '"':  out[i++] = '"'; break;
			case '/':  out[i++] = '/'; break;
			case 'b':  out[i++] = '\b'; break;
			case 'f':  out[i++] = '\f'; break;
			case 'n':  out[i++] = '\n'; break;
			case 'r':  out[i++] = '\r'; break;
			case 't':  out[i++] = '\t'; break;
			case 'u':  /* skip \uXXXX for simplicity - keep as-is */
				out[i++] = '\\'; out[i++] = 'u';
				break;
			default:
				out[i++] = *p; break;
			}
		} else {
			out[i++] = *p;
		}
	}
	out[i] = '\0';
	item->valuestring = (char *)out;

	if (*p == '"')
		buf->position = (size_t)(p - buf->json) + 1;
	else
		goto fail;

	return item;

fail:
	if (item) {
		if (item->valuestring)
			cJSON_free(item->valuestring);
		cJSON_free(item);
	}
	return NULL;
}

/* Parse a number */
/* Parse a number */
static cJSON *parse_number(parse_buffer *buf)
{
	cJSON *item;
	const unsigned char *p;
	char *endptr;

	item = cJSON_New_Item();
	if (!item)
		return NULL;

	item->type = cJSON_Number;
	p = buf->json + buf->position;

	errno = 0;
	item->valuedouble = strtod((const char *)p, &endptr);

	if (endptr == (const char *)p || errno == ERANGE) {
		cJSON_free(item);
		return NULL;
	}

	if (item->valuedouble >= (double)INT_MAX)
		item->valueint = INT_MAX;
	else if (item->valuedouble <= (double)INT_MIN)
		item->valueint = INT_MIN;
	else
		item->valueint = (int)item->valuedouble;

	buf->position =
		(size_t)((const unsigned char *)endptr - buf->json);
	return item;
}

/* Public parse entry */
cJSON *cJSON_ParseWithOpts(const char *value, const char **return_parse_end, int require_null_terminated)
{
	parse_buffer buf;
	cJSON *item;

	if (!value || *value == '\0')
		return NULL;

	memset(&buf, 0, sizeof(buf));
	buf.json = (const unsigned char *)value;
	buf.position = 0;

	item = parse_value(&buf);

	if (!item) {
		if (return_parse_end)
			*return_parse_end = value;
		return NULL;
	}

	if (return_parse_end)
		*return_parse_end = value + buf.position;

	/* Check trailing content */
	if (require_null_terminated) {
		skip_whitespace(&buf);
		if (*(buf.json + buf.position)) {
			cJSON_Delete(item);
			return NULL;
		}
	}

	return item;
}

cJSON *cJSON_Parse(const char *value)
{
	return cJSON_ParseWithOpts(value, NULL, 1);
}

/* ========================================================================
 * Delete
 * ======================================================================== */

/*
 * 删除 cJSON 实体及其所有子实体。
 * 采用标准 cJSON 语义：删除以 item 为起点的整条兄弟链表，
 * 并对每个节点的子节点递归删除。
 */
void cJSON_Delete(cJSON *item)
{
	cJSON *next;

	while (item) {
		next = item->next;
		cJSON_Delete(item->child);
		if (item->string)
			cJSON_free(item->string);
		if (item->valuestring && !(item->type & cJSON_StringIsConst))
			cJSON_free(item->valuestring);
		cJSON_free(item);
		item = next;
	}
}

/* ========================================================================
 * Query
 * ======================================================================== */

static cJSON *get_object_item(const cJSON *object, const char *name, int case_sensitive)
{
	cJSON *child;

	if (!object || !name)
		return NULL;

	child = object->child;
	while (child) {
		if (child->string) {
			int match;
			if (case_sensitive)
				match = (strcmp(child->string, name) == 0);
			else
				match = (strcasecmp(child->string, name) == 0);
			if (match)
				return child;
		}
		child = child->next;
	}
	return NULL;
}

cJSON *cJSON_GetObjectItem(const cJSON *object, const char *string)
{
	return get_object_item(object, string, 0);
}

cJSON *cJSON_GetObjectItemCaseSensitive(const cJSON *object, const char *string)
{
	return get_object_item(object, string, 1);
}

int cJSON_HasObjectItem(const cJSON *object, const char *string)
{
	return cJSON_GetObjectItem(object, string) != NULL;
}

cJSON *cJSON_GetArrayItem(const cJSON *array, int index)
{
	cJSON *child;

	if (!array || array->type != cJSON_Array || index < 0)
		return NULL;

	child = array->child;

	while (child && index > 0) {
		child = child->next;
		index--;
	}
	return child;
}

int cJSON_GetArraySize(const cJSON *array)
{
	cJSON *child;
	int size = 0;

	if (!array || array->type != cJSON_Array)
		return 0;

	child = array->child;
	while (child) {
		size++;
		child = child->next;
	}
	return size;
}

const char *cJSON_GetStringValue(const cJSON *item)
{
	if (!item || item->type != cJSON_String)
		return NULL;
	return item->valuestring;
}

double cJSON_GetNumberValue(const cJSON *item)
{
	if (!item || item->type != cJSON_Number)
		return 0.0;
	return item->valuedouble;
}

/* ========================================================================
 * Create
 * ======================================================================== */

cJSON *cJSON_CreateObject(void)
{
	cJSON *item = cJSON_New_Item();
	if (!item)
		return NULL;
	item->type = cJSON_Object;
	return item;
}

cJSON *cJSON_CreateArray(void)
{
	cJSON *item = cJSON_New_Item();
	if (!item)
		return NULL;
	item->type = cJSON_Array;
	return item;
}

cJSON *cJSON_CreateString(const char *string)
{
	cJSON *item = cJSON_New_Item();
	if (!item)
		return NULL;
	item->type = cJSON_String;
	item->valuestring = (char *)cJSON_strdup(string ? string : "");
	if (!item->valuestring) {
		cJSON_free(item);
		return NULL;
	}
	return item;
}

cJSON *cJSON_CreateNumber(double num)
{
	cJSON *item = cJSON_New_Item();

	if (!item)
		return NULL;

	item->type = cJSON_Number;
	item->valuedouble = num;

	if (num >= (double)INT_MAX)
		item->valueint = INT_MAX;
	else if (num <= (double)INT_MIN)
		item->valueint = INT_MIN;
	else
		item->valueint = (int)num;

	return item;
}

cJSON *cJSON_CreateBool(int b)
{
	cJSON *item = cJSON_New_Item();
	if (!item)
		return NULL;
	item->type = b ? cJSON_True : cJSON_False;
	return item;
}

cJSON *cJSON_CreateNull(void)
{
	cJSON *item = cJSON_New_Item();
	if (!item)
		return NULL;
	item->type = cJSON_NULL;
	return item;
}

/* ========================================================================
 * Add to Object/Array
 * ======================================================================== */
/*
 * Add an unattached item to an object.
 *
 * On success, ownership of item is transferred to object.
 * On failure, item remains owned by the caller.
 */
static int add_item_to_object_internal(cJSON *object, const char *string,
				       cJSON *item)
{
	char *key_copy;

	if (!object || !string || !item)
		return 0;

	/*
	 * Duplicate the key before modifying or attaching item.
	 * If allocation fails, item remains owned by the caller.
	 */
	key_copy = (char *)cJSON_strdup(string);
	if (!key_copy)
		return 0;

	item->string = key_copy;

	if (!object->child) {
		object->child = item;
	} else {
		cJSON *child = object->child;

		while (child->next)
			child = child->next;

		child->next = item;
		item->prev = child;
	}

	return 1;
}

void cJSON_AddItemToObject(cJSON *object, const char *string,
			   cJSON *item)
{
	(void)add_item_to_object_internal(object, string, item);
}

int cJSON_AddItemToObjectChecked(cJSON *object, const char *string,
				 cJSON *item)
{
	return add_item_to_object_internal(object, string, item);
}

void cJSON_AddItemToArray(cJSON *array, cJSON *item)
{
	if (!array || !item)
		return;

	if (!array->child) {
		array->child = item;
	} else {
		cJSON *child = array->child;

		while (child->next)
			child = child->next;

		child->next = item;
		item->prev = child;
	}
}

cJSON *cJSON_AddStringToObject(cJSON *object, const char *name,
			       const char *string)
{
	cJSON *item;

	if (!object || !name || !string)
		return NULL;

	item = cJSON_CreateString(string);
	if (!item)
		return NULL;

	if (!add_item_to_object_internal(object, name, item)) {
		cJSON_Delete(item);
		return NULL;
	}

	return item;
}

int cJSON_AddStringToObjectChecked(cJSON *object, const char *name,
				   const char *string)
{
	return cJSON_AddStringToObject(object, name, string) != NULL;
}

cJSON *cJSON_AddNumberToObject(cJSON *object, const char *name,
			       double number)
{
	cJSON *item;

	if (!object || !name)
		return NULL;

	item = cJSON_CreateNumber(number);
	if (!item)
		return NULL;

	if (!add_item_to_object_internal(object, name, item)) {
		cJSON_Delete(item);
		return NULL;
	}

	return item;
}

/* ========================================================================
 * Print
 * ======================================================================== */

typedef struct {
	char *buffer;
	size_t length;
	size_t offset;
} printbuffer;

static int ensure(printbuffer *pb, size_t needed)
{
	size_t newlen;
	char *newbuf;

	if (!pb->buffer) {
		pb->length = needed > 256 ? needed : 256;
		pb->buffer = (char *)cJSON_malloc(pb->length);
		pb->offset = 0;
		if (!pb->buffer)
			return 0;
		return 1;
	}

	if (pb->offset + needed < pb->length)
		return 1;

	newlen = pb->length * 2;
	if (newlen < pb->offset + needed)
		newlen = pb->offset + needed;

	newbuf = (char *)cJSON_malloc(newlen);
	if (!newbuf)
		return 0;
	memcpy(newbuf, pb->buffer, pb->offset);
	cJSON_free(pb->buffer);
	pb->buffer = newbuf;
	pb->length = newlen;
	return 1;
}

static void print_value(const cJSON *item, int depth, int fmt, printbuffer *pb);

/* Print a string value (with escaping) */
static void print_string(const char *str, printbuffer *pb)
{
	const char *ptr;
	size_t len;

	len = strlen(str);
	if (!ensure(pb, len * 2 + 3))
		return;

	pb->buffer[pb->offset++] = '"';

	for (ptr = str; *ptr; ptr++) {
		switch (*ptr) {
		case '\\': pb->buffer[pb->offset++] = '\\'; pb->buffer[pb->offset++] = '\\'; break;
		case '"':  pb->buffer[pb->offset++] = '\\'; pb->buffer[pb->offset++] = '"'; break;
		case '\b': pb->buffer[pb->offset++] = '\\'; pb->buffer[pb->offset++] = 'b'; break;
		case '\f': pb->buffer[pb->offset++] = '\\'; pb->buffer[pb->offset++] = 'f'; break;
		case '\n': pb->buffer[pb->offset++] = '\\'; pb->buffer[pb->offset++] = 'n'; break;
		case '\r': pb->buffer[pb->offset++] = '\\'; pb->buffer[pb->offset++] = 'r'; break;
		case '\t': pb->buffer[pb->offset++] = '\\'; pb->buffer[pb->offset++] = 't'; break;
		default:
			pb->buffer[pb->offset++] = *ptr;
			break;
		}
	}

	pb->buffer[pb->offset++] = '"';
	pb->buffer[pb->offset] = '\0';
}

/* Forward declaration for recursion */
static void print_value(const cJSON *item, int depth, int fmt, printbuffer *pb)
{
	int i;
	char num_buf[64];
	cJSON *child;

	if (!item) {
		if (!ensure(pb, 5))
			return;
		memcpy(pb->buffer + pb->offset, "null", 4);
		pb->offset += 4;
		return;
	}

	switch (item->type) {
	case cJSON_NULL:
		if (!ensure(pb, 5))
			return;
		memcpy(pb->buffer + pb->offset, "null", 4);
		pb->offset += 4;
		return;

	case cJSON_False:
		if (!ensure(pb, 6))
			return;
		memcpy(pb->buffer + pb->offset, "false", 5);
		pb->offset += 5;
		return;

	case cJSON_True:
		if (!ensure(pb, 5))
			return;
		memcpy(pb->buffer + pb->offset, "true", 4);
		pb->offset += 4;
		return;

	case cJSON_Number:
		if (fabs(item->valuedouble - (double)item->valueint) < DBL_EPSILON)
			snprintf(num_buf, sizeof(num_buf), "%d", item->valueint);
		else
			snprintf(num_buf, sizeof(num_buf), "%g", item->valuedouble);
		print_string(num_buf, pb);
		if (pb->offset >= 2) {
			/* Remove quotes around number - shift characters */
			memmove(pb->buffer + pb->offset - strlen(num_buf) - 2,
				pb->buffer + pb->offset - strlen(num_buf) - 1,
				strlen(num_buf) + 1);
			pb->offset -= 2;
			pb->buffer[pb->offset] = '\0';
		}
		return;

	case cJSON_String:
		if (item->valuestring)
			print_string(item->valuestring, pb);
		else {
			if (!ensure(pb, 3))
				return;
			pb->buffer[pb->offset++] = '"';
			pb->buffer[pb->offset++] = '"';
			pb->buffer[pb->offset] = '\0';
		}
		return;

	case cJSON_Array:
		if (!ensure(pb, 2))
			return;
		pb->buffer[pb->offset++] = '[';
		child = item->child;
		while (child) {
			print_value(child, depth + 1, fmt, pb);
			child = child->next;
			if (child) {
				if (!ensure(pb, 2))
					return;
				pb->buffer[pb->offset++] = ',';
				if (fmt)
					pb->buffer[pb->offset++] = ' ';
			}
		}
		if (!ensure(pb, 2))
			return;
		pb->buffer[pb->offset++] = ']';
		pb->buffer[pb->offset] = '\0';
		return;

	case cJSON_Object:
		if (!ensure(pb, 2))
			return;
		pb->buffer[pb->offset++] = '{';
		if (fmt)
			pb->buffer[pb->offset++] = '\n';
		child = item->child;
		while (child) {
			if (fmt) {
				if (!ensure(pb, depth * 4 + 4))
					return;
				for (i = 0; i < depth + 1; i++) {
					pb->buffer[pb->offset++] = ' ';
					pb->buffer[pb->offset++] = ' ';
				}
			}
			/* Print key */
			if (child->string)
				print_string(child->string, pb);
			if (!ensure(pb, (fmt ? 2 : 1)))
				return;
			pb->buffer[pb->offset++] = ':';
			if (fmt)
				pb->buffer[pb->offset++] = ' ';
			/* Print value */
			print_value(child, depth + 1, fmt, pb);
			child = child->next;
			if (child) {
				if (!ensure(pb, 1))
					return;
				pb->buffer[pb->offset++] = ',';
			}
			if (fmt) {
				if (!ensure(pb, 1))
					return;
				pb->buffer[pb->offset++] = '\n';
			}
		}
		if (fmt && item->child) {
			if (!ensure(pb, depth * 2 + 2))
				return;
			for (i = 0; i < depth; i++) {
				pb->buffer[pb->offset++] = ' ';
				pb->buffer[pb->offset++] = ' ';
			}
		}
		if (!ensure(pb, 1))
			return;
		pb->buffer[pb->offset++] = '}';
		pb->buffer[pb->offset] = '\0';
		return;

	default:
		return;
	}
}

char *cJSON_Print(const cJSON *item)
{
	printbuffer pb;
	char *result;

	memset(&pb, 0, sizeof(pb));
	print_value(item, 0, 1, &pb);

	if (!pb.buffer) {
		result = (char *)cJSON_strdup("null");
		return result;
	}

	/* Add null terminator */
	if (!ensure(&pb, 1)) {
		cJSON_free(pb.buffer);
		return (char *)cJSON_strdup("null");
	}
	pb.buffer[pb.offset] = '\0';

	result = pb.buffer;
	return result;
}

char *cJSON_PrintUnformatted(const cJSON *item)
{
	printbuffer pb;
	char *result;

	memset(&pb, 0, sizeof(pb));
	print_value(item, 0, 0, &pb);

	if (!pb.buffer) {
		result = (char *)cJSON_strdup("null");
		return result;
	}

	if (!ensure(&pb, 1)) {
		cJSON_free(pb.buffer);
		return (char *)cJSON_strdup("null");
	}
	pb.buffer[pb.offset] = '\0';

	result = pb.buffer;
	return result;
}
