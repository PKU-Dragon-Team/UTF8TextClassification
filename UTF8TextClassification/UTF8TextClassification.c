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

	for (int i = 0; i < 4; ++i) {
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

	if (list = NULL) {
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
	return 0;
}

int load_texts(struct text_list * l, FILE * in) {
	if (l == NULL || in == NULL) {
		return -1;
	}
	else {
		uchar s[BUF_SIZE] = { 0 };
		uchar ts[TYPE_COUNT] = { 0 };
		size_t i = 0;

		while (fscanf_s(in, "***%*d") != EOF) {
			fgets(s, BUF_SIZE, in);
			fscanf_s(in, "***(%s)", ts, TYPE_COUNT);

			struct ustring * us = NULL;
			init_ustring(&us, index, s, BUF_SIZE);

			int8_t types[TYPE_COUNT] = { 0 };
			parse_type(ts, types);

			struct text * t = NULL;
			init_text(&t, us, types);

			if (i + 1 > l->len) {	// Dynamic Expand
				if (resize_text_list(l, l->len * 2) != 0) {
					return -1;
				}
			}
			l->list[i] = *t;
		}
	}
	return 0;
}

int parse_type(uchar * ts, int8_t types[TYPE_COUNT]) {
	if (ts == NULL) {
		return -1;
	}
	return 0;
}

int output_texts(const struct text_list * l, FILE * out) {
	return 0;
}