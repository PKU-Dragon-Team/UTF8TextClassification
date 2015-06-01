#include "UTF8TextClassification.h"

int init_text_list(struct text_list ** l) {
	return 0;
}

int resize_text_list(struct text_list * l, size_t len) {
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

			struct text * t = malloc(sizeof(struct text));
			if (t == NULL) {
				return -1;
			}
			init_ustring(&t->text, index, s, BUF_SIZE);
			parse_type(ts, t->types);

			if (i + 1 > l->len) {
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