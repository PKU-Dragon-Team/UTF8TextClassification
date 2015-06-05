#include "UTF8TextClassification.h"

int init_text(struct text ** t, struct ustring * us, int8_t types[TYPE_COUNT]) {
	if (t == NULL) {
		return -1;
	}

	*t = malloc(sizeof(struct text));
	if (*t == NULL) {
		return -1;
	}

	(*t)->text = us;

	for (int i = 0; i < TYPE_COUNT; ++i) {
		(*t)->types[i] = types[i];
	}
	return 0;
}

int clear_text(struct text ** t) {
	if (t == NULL) {
		return -1;
	}

	if (*t != NULL) {
		if ((*t)->text != NULL) {
			clear_ustring(&(*t)->text);
		}
		*t = NULL;
	}
	return 0;
}

int init_text_list(struct text_list ** l, struct text * list, size_t len) {
	if (l == NULL) {
		return -1;
	}

	*l = malloc(sizeof(struct text_list));
	if (*l == NULL) {
		return -1;
	}

	if (list == NULL) {
		(*l)->list = NULL;
		(*l)->len = 0;
	}
	else {
		struct text * p = calloc(len, sizeof(struct text));
		if (p == NULL) {
			return -1;
		}
		memcpy(p, list, len);
		(*l)->list = p;
		(*l)->len = len;
	}
	return 0;
}

int resize_text_list(struct text_list * l, size_t len) {
	if (l == NULL) {
		return -1;
	}

	struct text * p = realloc(l->list, len * sizeof(struct text));
	if (p == NULL) {
		return -1;
	}
	else {
		l->list = p;
		l->len = len;
	}
	return 0;
}

int clear_text_list(struct text_list ** l) {
	if (l == NULL) {
		return -1;
	}
	for (size_t i = 0; i < (*l)->len; ++i) {
		clear_ustring(&(*l)->list[i].text);
	}
	free((*l)->list);
	free(*l);
	*l = NULL;
	return 0;
}

int load_texts(FILE * in, struct text_list * l) {
	if (l == NULL || in == NULL) {
		return -1;
	}
	else {
		uchar s[BUF_SIZE] = { 0 };
		size_t i = 0;

		while (fgets(s, BUF_SIZE, in)) {	// the first line
			fgets(s, BUF_SIZE, in);		// the second line
			struct ustring * us = NULL;
			init_ustring(&us, index, s, BUF_SIZE);

			fgets(s, BUF_SIZE, in);		// the third line
			int8_t types[TYPE_COUNT] = { 0 };
			parse_type(s, types);

			struct text * t = NULL;
			init_text(&t, us, types);

			if (i + 1 > l->len) {	// Dynamic Expand
				if (resize_text_list(l, l->len * 2 + 1) != 0) {
					return -1;
				}
			}
			l->list[i] = *t;
			++i;
		}
		l->len = i;
	}
	return 0;
}

int parse_type(uchar * ts, int8_t types[TYPE_COUNT]) {
	if (ts == NULL) {
		return -1;
	}

	for (int i = 0; i < TYPE_COUNT; ++i) {
		types[i] = -1;
	}

	size_t n = strlen(ts);
	int ti = 0;
	for (int i = 0; i < n; ++i) {
		if (ti < TYPE_COUNT && ts[i] != '*' && ts[i] != '(' && ts[i] != ')' && ts[i] != '\n') {
			types[ti] = ts[i] - '0';
			++ti;
		}
	}

	return 0;
}

int output_texts(FILE * out, const struct text_list * l) {
	if (l == NULL || out == NULL) {
		return -1;
	}
	for (size_t i = 0; i < l->len; ++i) {
		fprintf_s(out, "***%d\n%s***(", i + 1, l->list[i].text->string, l->list[i].text->string_len);
		for (int j = 0; j < TYPE_COUNT; ++j) {
			if (l->list[i].types[j] != -1) {
				fprintf_s(out, "%d", l->list[i].types[j]);
			}
		}
		fprintf_s(out, ")%s", (i == l->len - 1) ? "" : "\n");
	}
	return 0;
}

void get_char_analysis(const struct text_list * l, struct uchar_analysis * uca) {
	for (size_t i = 0; i < l->len; ++i) {
		for (size_t j = 0; j < l->list[i].text->string_len; ++j) {
			++uca->uchar_list[l->list[i].text->string[j]];
			++uca->total_count;
		}
	}
}

void output_char_analysis(FILE * out, const struct uchar_analysis * uca) {
	fprintf_s(out, "Total Characters: %d\n", uca->total_count);
	for (int i = 0; i < 130000; ++i) {
		if (uca->uchar_list[i] != 0) {
			fprintf_s(out, "0x%02X\t%5d\n", i, uca->uchar_list[i]);
		}
	}
}