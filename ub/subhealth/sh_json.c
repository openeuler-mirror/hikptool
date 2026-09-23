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

#include "sh_json.h"
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

/* ------------------------------------------------------------------ *
 * Small allocation / duplication helpers
 * ------------------------------------------------------------------ */

static char *json_dup(const char *src, size_t len)
{
	char *copy;
	size_t n;

	if (src == NULL)
		return NULL;
	n = len + 1;
	copy = (char *)malloc(n);
	if (copy != NULL) {
		memcpy(copy, src, len);
		copy[len] = '\0';
	}
	return copy;
}

static char *json_strdup(const char *src)
{
	if (src == NULL)
		return NULL;
	return json_dup(src, strlen(src));
}

static sh_json *json_new_node(int kind)
{
	sh_json *node = (sh_json *)calloc(1, sizeof(sh_json));

	if (node != NULL)
		node->kind = kind;
	return node;
}

/* ------------------------------------------------------------------ *
 * Recursive deletion
 * ------------------------------------------------------------------ */

void sh_json_delete(sh_json *node)
{
	sh_json *cursor;

	while (node != NULL) {
		cursor = node->fwd;
		sh_json_delete(node->head);
		free(node->name);
		free(node->str);
		free(node);
		node = cursor;
	}
}

/* ------------------------------------------------------------------ *
 * Parser
 * ------------------------------------------------------------------ */

struct json_scan {
	const char *pos;      /* current read position      */
	const char *limit;    /* one past the last byte     */
};

static sh_json *parse_value(struct json_scan *scan);

static void scan_skip_space(struct json_scan *scan)
{
	while (scan->pos < scan->limit) {
		char ch = *scan->pos;

		if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
			scan->pos++;
		else
			break;
	}
}

static sh_json *parse_string_node(struct json_scan *scan)
{
	const char *start;
	const char *walk;
	char *out;
	size_t out_len;
	size_t cap;
	size_t i;

	/* The caller has verified "*scan->pos == '\"' ". */
	start = ++scan->pos;
	walk = start;

	/* Locate the terminating quote, honouring backslash escapes. */
	while (walk < scan->limit && *walk != '"') {
		if (*walk == '\\') {
			walk++;
			if (walk >= scan->limit)
				return NULL;
		}
		walk++;
	}
	if (walk >= scan->limit)
		return NULL;

	/*
	 * The output can never be longer than the source interval between the
	 * quotes, so a buffer of that exact size is guaranteed to be enough.
	 */
	out_len = (size_t)(walk - start);
	cap = out_len + 1;
	out = (char *)malloc(cap);
	if (out == NULL)
		return NULL;

	i = 0;
	for (walk = start; walk < start + out_len; walk++) {
		unsigned char ch = (unsigned char)*walk;

		if (ch != '\\') {
			out[i++] = (char)ch;
			continue;
		}
		walk++;
		if (walk >= start + out_len) {
			free(out);
			return NULL;
		}
		switch (*walk) {
		case '\\':
			out[i++] = '\\';
			break;
		case '"':
			out[i++] = '"';
			break;
		case '/':
			out[i++] = '/';
			break;
		case 'b':
			out[i++] = '\b';
			break;
		case 'f':
			out[i++] = '\f';
			break;
		case 'n':
			out[i++] = '\n';
			break;
		case 'r':
			out[i++] = '\r';
			break;
		case 't':
			out[i++] = '\t';
			break;
		case 'u':
			/* Keep the escape sequence verbatim (no unicode decode). */
			out[i++] = '\\';
			out[i++] = 'u';
			break;
		default:
			out[i++] = *walk;
			break;
		}
	}
	out[i] = '\0';
	/* Advance the scanner past the closing quote. */
	scan->pos = walk + 1;

	sh_json *node = json_new_node(SH_JSON_BUF);

	if (node == NULL) {
		free(out);
		return NULL;
	}
	node->str = out;
	return node;
}

static sh_json *parse_number_node(struct json_scan *scan)
{
	const char *begin = scan->pos;
	char *end = NULL;
	double value;
	sh_json *node;

	errno = 0;
	value = strtod(begin, &end);
	if (end == begin || errno == ERANGE)
		return NULL;

	node = json_new_node(SH_JSON_NUM);
	if (node == NULL)
		return NULL;
	node->dval = value;
	if (value >= (double)LONG_MAX)
		node->ival = LONG_MAX;
	else if (value <= (double)LONG_MIN)
		node->ival = LONG_MIN;
	else
		node->ival = (long)value;

	scan->pos = end;
	return node;
}

static sh_json *parse_array_node(struct json_scan *scan)
{
	sh_json *array = json_new_node(SH_JSON_SEQ);

	if (array == NULL)
		return NULL;
	scan->pos++; /* consume '[' */

	scan_skip_space(scan);
	if (scan->pos < scan->limit && *scan->pos == ']') {
		scan->pos++;
		return array;
	}

	for (;;) {
		sh_json *elem;

		scan_skip_space(scan);
		elem = parse_value(scan);
		if (elem == NULL) {
			sh_json_delete(array);
			return NULL;
		}
		if (array->head == NULL) {
			array->head = elem;
		} else {
			sh_json *last = array->head;

			while (last->fwd != NULL)
				last = last->fwd;
			last->fwd = elem;
			elem->bck = last;
		}

		scan_skip_space(scan);
		if (scan->pos >= scan->limit) {
			sh_json_delete(array);
			return NULL;
		}
		if (*scan->pos == ']') {
			scan->pos++;
			return array;
		}
		if (*scan->pos != ',') {
			sh_json_delete(array);
			return NULL;
		}
		scan->pos++;
	}
}

static sh_json *parse_object_node(struct json_scan *scan)
{
	sh_json *object = json_new_node(SH_JSON_MAP);

	if (object == NULL)
		return NULL;
	scan->pos++; /* consume '{' */

	scan_skip_space(scan);
	if (scan->pos < scan->limit && *scan->pos == '}') {
		scan->pos++;
		return object;
	}

	for (;;) {
		sh_json *key_node;
		sh_json *value_node;
		char *member;

		scan_skip_space(scan);
		if (scan->pos >= scan->limit || *scan->pos != '"') {
			sh_json_delete(object);
			return NULL;
		}
		key_node = parse_string_node(scan);
		if (key_node == NULL) {
			sh_json_delete(object);
			return NULL;
		}
		member = key_node->str;
		key_node->str = NULL;
		sh_json_delete(key_node);

		scan_skip_space(scan);
		if (scan->pos >= scan->limit || *scan->pos != ':') {
			free(member);
			sh_json_delete(object);
			return NULL;
		}
		scan->pos++;

		scan_skip_space(scan);
		value_node = parse_value(scan);
		if (value_node == NULL) {
			free(member);
			sh_json_delete(object);
			return NULL;
		}
		value_node->name = member;

		if (object->head == NULL) {
			object->head = value_node;
		} else {
			sh_json *last = object->head;

			while (last->fwd != NULL)
				last = last->fwd;
			last->fwd = value_node;
			value_node->bck = last;
		}

		scan_skip_space(scan);
		if (scan->pos >= scan->limit) {
			sh_json_delete(object);
			return NULL;
		}
		if (*scan->pos == '}') {
			scan->pos++;
			return object;
		}
		if (*scan->pos != ',') {
			sh_json_delete(object);
			return NULL;
		}
		scan->pos++;
	}
}

static int json_match_kw(struct json_scan *scan, const char *word)
{
	size_t len = strlen(word);
	const char *p = scan->pos;

	if ((size_t)(scan->limit - p) < len ||
	    memcmp(p, word, len) != 0)
		return 0;
	scan->pos += len;
	return 1;
}

static sh_json *parse_value(struct json_scan *scan)
{
	sh_json *node;

	scan_skip_space(scan);
	if (scan->pos >= scan->limit)
		return NULL;

	switch (*scan->pos) {
	case '{':
		return parse_object_node(scan);
	case '[':
		return parse_array_node(scan);
	case '"':
		return parse_string_node(scan);
	case 't':
		if (json_match_kw(scan, "true")) {
			node = json_new_node(SH_JSON_YES);
			return node;
		}
		return NULL;
	case 'f':
		if (json_match_kw(scan, "false")) {
			node = json_new_node(SH_JSON_NO);
			return node;
		}
		return NULL;
	case 'n':
		if (json_match_kw(scan, "null")) {
			node = json_new_node(SH_JSON_NIL);
			return node;
		}
		return NULL;
	default:
		if (*scan->pos == '-' ||
		    (*scan->pos >= '0' && *scan->pos <= '9'))
			return parse_number_node(scan);
		return NULL;
	}
}

sh_json *sh_json_parse(const char *text)
{
	struct json_scan scan;
	sh_json *root;

	if (text == NULL || text[0] == '\0')
		return NULL;

	scan.pos = text;
	scan.limit = text + strlen(text);

	root = parse_value(&scan);
	if (root == NULL)
		return NULL;

	/* Reject trailing non-whitespace content. */
	scan_skip_space(&scan);
	if (scan.pos < scan.limit) {
		sh_json_delete(root);
		return NULL;
	}
	return root;
}

/* ------------------------------------------------------------------ *
 * Query
 * ------------------------------------------------------------------ */

static int json_name_equal(const char *lhs, const char *rhs, int sensitive)
{
	if (sensitive)
		return strcmp(lhs, rhs) == 0;
	return strcasecmp(lhs, rhs) == 0;
}

static sh_json *get_member(const sh_json *object, const char *name,
			   int sensitive)
{
	sh_json *child;

	if (object == NULL || name == NULL)
		return NULL;

	for (child = object->head; child != NULL; child = child->fwd) {
		if (child->name != NULL &&
		    json_name_equal(child->name, name, sensitive))
			return child;
	}
	return NULL;
}

sh_json *sh_json_get_item(const sh_json *object, const char *name)
{
	return get_member(object, name, 0);
}

sh_json *sh_json_get_item_cs(const sh_json *object, const char *name)
{
	return get_member(object, name, 1);
}

sh_json *sh_json_item_at(const sh_json *array, int index)
{
	sh_json *child;

	if (array == NULL || array->kind != SH_JSON_SEQ || index < 0)
		return NULL;
	for (child = array->head; child != NULL && index > 0;
	     child = child->fwd)
		index--;
	return child;
}

int sh_json_item_count(const sh_json *array)
{
	sh_json *child;
	int count = 0;

	if (array == NULL || array->kind != SH_JSON_SEQ)
		return 0;
	for (child = array->head; child != NULL; child = child->fwd)
		count++;
	return count;
}

/* ------------------------------------------------------------------ *
 * Builders
 * ------------------------------------------------------------------ */

sh_json *sh_json_create_obj(void)
{
	return json_new_node(SH_JSON_MAP);
}

sh_json *sh_json_create_arr(void)
{
	return json_new_node(SH_JSON_SEQ);
}

sh_json *sh_json_create_str(const char *value)
{
	sh_json *node = json_new_node(SH_JSON_BUF);

	if (node == NULL)
		return NULL;
	node->str = json_strdup((value != NULL) ? value : "");
	if (node->str == NULL) {
		sh_json_delete(node);
		return NULL;
	}
	return node;
}

sh_json *sh_json_create_num(double value)
{
	sh_json *node = json_new_node(SH_JSON_NUM);

	if (node == NULL)
		return NULL;
	node->dval = value;
	if (value >= (double)LONG_MAX)
		node->ival = LONG_MAX;
	else if (value <= (double)LONG_MIN)
		node->ival = LONG_MIN;
	else
		node->ival = (long)value;
	return node;
}

/* ------------------------------------------------------------------ *
 * Attachment helpers
 * ------------------------------------------------------------------ */

static int link_as_sibling(sh_json *parent, sh_json *child)
{
	if (parent->head == NULL) {
		parent->head = child;
	} else {
		sh_json *last = parent->head;

		while (last->fwd != NULL)
			last = last->fwd;
		last->fwd = child;
		child->bck = last;
	}
	return 1;
}

static int object_attach(const sh_json *object, const char *name, sh_json *item)
{
	sh_json *obj = (sh_json *)object;
	char *key_copy;

	if (obj == NULL || name == NULL || item == NULL)
		return 0;
	key_copy = json_strdup(name);
	if (key_copy == NULL)
		return 0;
	item->name = key_copy;
	return link_as_sibling(obj, item);
}

sh_json *sh_json_attach(sh_json *object, const char *name, sh_json *item)
{
	if (object_attach(object, name, item))
		return item;
	return NULL;
}

int sh_json_attach_checked(sh_json *object, const char *name, sh_json *item)
{
	return object_attach(object, name, item);
}

int sh_json_push(sh_json *array, sh_json *item)
{
	if (array == NULL || array->kind != SH_JSON_SEQ || item == NULL)
		return 0;
	return link_as_sibling(array, item);
}

sh_json *sh_json_put_str(sh_json *object, const char *name, const char *value)
{
	sh_json *node;

	if (object == NULL || name == NULL || value == NULL)
		return NULL;
	node = sh_json_create_str(value);
	if (node == NULL)
		return NULL;
	if (!object_attach(object, name, node)) {
		sh_json_delete(node);
		return NULL;
	}
	return node;
}

int sh_json_put_str_checked(sh_json *object, const char *name, const char *value)
{
	return sh_json_put_str(object, name, value) != NULL;
}

sh_json *sh_json_put_num(sh_json *object, const char *name, double value)
{
	sh_json *node;

	if (object == NULL || name == NULL)
		return NULL;
	node = sh_json_create_num(value);
	if (node == NULL)
		return NULL;
	if (!object_attach(object, name, node)) {
		sh_json_delete(node);
		return NULL;
	}
	return node;
}

/* ------------------------------------------------------------------ *
 * Writer
 * ------------------------------------------------------------------ */

struct json_out {
	char *buffer;
	size_t length;   /* allocated capacity           */
	size_t offset;   /* bytes currently in use        */
	int failed;
};

static int out_ensure(struct json_out *out, size_t extra)
{
	size_t need;
	size_t next_cap;
	char *grown;

	if (out->failed)
		return 0;
	need = out->offset + extra + 1;
	if (need <= out->length)
		return 1;

	next_cap = (out->length == 0) ? 256U : out->length * 2U;
	if (next_cap < need)
		next_cap = need;
	grown = (char *)realloc(out->buffer, next_cap);
	if (grown == NULL) {
		out->failed = 1;
		return 0;
	}
	out->buffer = grown;
	out->length = next_cap;
	return 1;
}

static void out_byte(struct json_out *out, char ch)
{
	if (!out_ensure(out, 1))
		return;
	out->buffer[out->offset++] = ch;
}

static void out_text(struct json_out *out, const char *txt)
{
	size_t len = strlen(txt);

	if (!out_ensure(out, len))
		return;
	memcpy(out->buffer + out->offset, txt, len);
	out->offset += len;
}

static void out_repeat(struct json_out *out, char ch, int times)
{
	int i;

	for (i = 0; i < times; i++)
		out_byte(out, ch);
}

static void write_string_literal(struct json_out *out, const char *value)
{
	const char *walk;

	out_byte(out, '"');
	for (walk = value; *walk != '\0'; walk++) {
		switch (*walk) {
		case '\\':
			out_byte(out, '\\');
			out_byte(out, '\\');
			break;
		case '"':
			out_byte(out, '\\');
			out_byte(out, '"');
			break;
		case '\b':
			out_byte(out, '\\');
			out_byte(out, 'b');
			break;
		case '\f':
			out_byte(out, '\\');
			out_byte(out, 'f');
			break;
		case '\n':
			out_byte(out, '\\');
			out_byte(out, 'n');
			break;
		case '\r':
			out_byte(out, '\\');
			out_byte(out, 'r');
			break;
		case '\t':
			out_byte(out, '\\');
			out_byte(out, 't');
			break;
		default:
			out_byte(out, *walk);
			break;
		}
	}
	out_byte(out, '"');
}

static void write_number(struct json_out *out, const sh_json *node)
{
	char num_buf[64];

	if (fabs(node->dval - (double)node->ival) < DBL_EPSILON)
		snprintf(num_buf, sizeof(num_buf), "%ld", node->ival);
	else
		snprintf(num_buf, sizeof(num_buf), "%g", node->dval);
	out_text(out, num_buf);
}

static void write_node(struct json_out *out, const sh_json *node, int depth,
		       int pretty);

static void write_array(struct json_out *out, const sh_json *node, int depth,
			int pretty)
{
	const sh_json *child;

	out_byte(out, '[');
	child = node->head;
	while (child != NULL) {
		write_node(out, child, depth + 1, pretty);
		child = child->fwd;
		if (child != NULL) {
			out_byte(out, ',');
			if (pretty)
				out_byte(out, ' ');
		}
	}
	out_byte(out, ']');
}

static void write_object(struct json_out *out, const sh_json *node, int depth,
			 int pretty)
{
	const sh_json *child;

	out_byte(out, '{');
	if (pretty && node->head != NULL)
		out_byte(out, '\n');

	child = node->head;
	while (child != NULL) {
		if (pretty) {
			out_repeat(out, ' ', (depth + 1) * 2);
			if (child->name != NULL)
				write_string_literal(out, child->name);
			out_text(out, pretty ? ": " : ":");
		} else {
			if (child->name != NULL)
				write_string_literal(out, child->name);
			out_byte(out, ':');
		}
		write_node(out, child, depth + 1, pretty);

		child = child->fwd;
		if (child != NULL)
			out_byte(out, ',');
		if (pretty)
			out_byte(out, '\n');
	}

	if (pretty && node->head != NULL)
		out_repeat(out, ' ', depth * 2);
	out_byte(out, '}');
}

static void write_node(struct json_out *out, const sh_json *node, int depth,
		       int pretty)
{
	if (node == NULL) {
		out_text(out, "null");
		return;
	}

	switch (node->kind) {
	case SH_JSON_NIL:
		out_text(out, "null");
		break;
	case SH_JSON_NO:
		out_text(out, "false");
		break;
	case SH_JSON_YES:
		out_text(out, "true");
		break;
	case SH_JSON_NUM:
		write_number(out, node);
		break;
	case SH_JSON_BUF:
		if (node->str != NULL)
			write_string_literal(out, node->str);
		else
			out_text(out, "\"\"");
		break;
	case SH_JSON_SEQ:
		write_array(out, node, depth, pretty);
		break;
	case SH_JSON_MAP:
		write_object(out, node, depth, pretty);
		break;
	default:
		out_text(out, "null");
		break;
	}
}

char *sh_json_write(const sh_json *node)
{
	struct json_out out;

	memset(&out, 0, sizeof(out));
	write_node(&out, node, 0, 1);

	if (out.failed) {
		free(out.buffer);
		return NULL;
	}
	if (!out_ensure(&out, 1)) {
		free(out.buffer);
		return NULL;
	}
	out.buffer[out.offset] = '\0';
	return out.buffer;
}