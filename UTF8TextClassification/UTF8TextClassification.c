#include "UTF8TextClassification.h"

int init_text(struct text ** pp_text, struct ustring * us, int8_t types[TYPE_COUNT]) {
	if (pp_text == NULL) {
		return -1;
	}

	*pp_text = malloc(sizeof(struct text));
	if (*pp_text == NULL) {
		return -1;
	}

	(*pp_text)->us = us;

	for (int i = 0; i < TYPE_COUNT; ++i) {
		(*pp_text)->types[i] = types[i];
	}
	return 0;
}

int clear_text(struct text ** pp_text) {
	if (pp_text == NULL) {
		return -1;
	}

	if (*pp_text != NULL) {
		if ((*pp_text)->us != NULL) {
			clear_ustring(&(*pp_text)->us);
		}
		*pp_text = NULL;
	}
	return 0;
}

int init_text_list(struct text_list ** pp_tlist, const struct text a_text[], size_t len) {
	if (pp_tlist == NULL) {
		return -1;
	}

	*pp_tlist = malloc(sizeof(struct text_list));
	if (*pp_tlist == NULL) {
		return -1;
	}

	if (a_text == NULL) {
		(*pp_tlist)->list = NULL;
		(*pp_tlist)->len = 0;
	}
	else {
		struct text * p = calloc(len, sizeof(struct text));
		if (p == NULL) {
			return -1;
		}
		memcpy(p, a_text, len);
		(*pp_tlist)->list = p;
		(*pp_tlist)->len = len;
	}
	return 0;
}

int resize_text_list(struct text_list * p_tlist, size_t len) {
	if (p_tlist == NULL) {
		return -1;
	}

	struct text * p = realloc(p_tlist->list, len * sizeof(struct text));
	if (p == NULL) {
		return -1;
	}
	else {
		p_tlist->list = p;
		p_tlist->len = len;
	}
	return 0;
}

int clear_text_list(struct text_list ** pp_tlist) {
	if (pp_tlist == NULL) {
		return -1;
	}
	for (size_t i = 0; i < (*pp_tlist)->len; ++i) {
		clear_ustring(&(*pp_tlist)->list[i].us);
	}
	free((*pp_tlist)->list);
	free(*pp_tlist);
	*pp_tlist = NULL;

	return 0;
}

int load_texts(FILE * in, struct text_list * p_tlist) {
	if (p_tlist == NULL || in == NULL) {
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

			if (i + 1 > p_tlist->len) {	// Dynamic Expand
				if (resize_text_list(p_tlist, p_tlist->len * 2 + 1) != 0) {
					return -1;
				}
			}
			p_tlist->list[i] = *t;
			++i;
		}
		p_tlist->len = i;
	}
	return 0;
}

int parse_type(const uchar a_type[], int8_t types[TYPE_COUNT]) {
	if (a_type == NULL) {
		return -1;
	}

	for (int i = 0; i < TYPE_COUNT; ++i) {
		types[i] = -1;
	}

	size_t n = strlen(a_type);
	int ti = 0;
	for (int i = 0; i < n; ++i) {
		if (ti < TYPE_COUNT && a_type[i] != '*' && a_type[i] != '(' && a_type[i] != ')' && a_type[i] != '\n') {
			types[ti] = a_type[i] - '0';
			++ti;
		}
	}
	return 0;
}

int output_texts(FILE * out, const struct text_list * p_tlist) {
	if (p_tlist == NULL || out == NULL) {
		return -1;
	}

	for (size_t i = 0; i < p_tlist->len; ++i) {
		fprintf_s(out, "***%d\n%s***(", i + 1, p_tlist->list[i].us->string, p_tlist->list[i].us->string_len);
		for (int j = 0; j < TYPE_COUNT; ++j) {
			if (p_tlist->list[i].types[j] != -1) {
				fprintf_s(out, "%d", p_tlist->list[i].types[j]);
			}
		}
		fprintf_s(out, ")%s", (i == p_tlist->len - 1) ? "" : "\n");
	}
	return 0;
}

int get_char_analysis(const struct text_list * cp_tlist, struct uchar_analysis * uca) {
	if (cp_tlist == NULL || uca == NULL) {
		return -1;
	}

	for (size_t i = 0; i < cp_tlist->len; ++i) {
		for (size_t j = 0; j < cp_tlist->list[i].us->string_len; ++j) {
			++uca->uchar_list[cp_tlist->list[i].us->string[j]];
			++uca->total_count;
		}
	}
	return 0;
}

int output_char_analysis(FILE * out, const struct uchar_analysis * uca) {
	if (out == NULL || uca == NULL) {
		return -1;
	}

	fprintf_s(out, "Total Characters: %d\n", uca->total_count);
	for (int i = 0; i < 130000; ++i) {
		if (uca->uchar_list[i] != 0) {
			fprintf_s(out, "0x%02X\t%5d\n", i, uca->uchar_list[i]);
		}
	}
	return 0;
}

int init_hashmap_ustring_analysis(struct hashmap_ustring_analysis ** pp_husa) {
	if (pp_husa == NULL) {
		return -1;
	}

	*pp_husa = malloc(sizeof(struct hashmap_ustring_analysis));
	if (*pp_husa == NULL) {
		return -1;
	}

	(*pp_husa)->usa_list = calloc(BASE_HASH_LEN, sizeof(struct ustring_analysis *));
	if ((*pp_husa)->usa_list == NULL) {
		return -1;
	}
	(*pp_husa)->hashlen = BASE_HASH_LEN;
	(*pp_husa)->total_count = 0;
	(*pp_husa)->count = 0;

	return 0;
}

int rehash_hashmap_ustring_analysis(struct hashmap_ustring_analysis * p_husa, size_t hashlen) {
	if (p_husa == NULL) {
		return -1;
	}
	return 0;
}

int append_hashmap_ustring_analysis(struct hashmap_ustring_analysis * p_husa, const struct ustring * cp_us, parser f) {
	if (p_husa == NULL || cp_us == NULL || f == NULL) {
		return -1;
	}

	p_uspl parse_list = f(cp_us);

	for (size_t i = 0; i < parse_list->len - 1; ++i) {
		// check if hashmap is overload
		if (p_husa->count * 2 > p_husa->hashlen) {
			rehash_hashmap_ustring_analysis(p_husa, p_husa->hashlen * 2 + 1);
		}

		struct ustring * temp = malloc(sizeof(struct ustring));
		init_ustring(&temp, index, NULL, 0);
		slice_ustring(cp_us, temp, parse_list->parse_list[i], parse_list->parse_list[i + 1]);

		size_t hashcode = hash_ustring(temp, 0, p_husa->hashlen);
		// search for existence
		struct ustring_analysis * p = p_husa->usa_list[hashcode];
		if (p != NULL) {
			while (p->next != NULL) {
				if (compare_ustring(p->next->us, temp) == 0) {
					++p->next->count;
					break;
				}
				p = p->next;
			}
			if (p->next == NULL) {
				// while check failed: check p
				if (compare_ustring(p->us, temp) == 0) {
					++p->count;
				}
				else {
					// add to the end of link list
					struct ustring_analysis * p_ua = malloc(sizeof(struct ustring_analysis));
					p_ua->us = temp;
					p_ua->count = 1;
					p_ua->next = NULL;
					p->next = p_ua;
				}
			}
		}
		else {
			// add to the hashmap
			struct ustring_analysis * p_ua = malloc(sizeof(struct ustring_analysis));
			p_ua->us = temp;
			p_ua->count = 1;
			p_ua->next = NULL;
			p_husa->usa_list[hashcode] = p_ua;
			++p_husa->count;
		}
		++p_husa->total_count;
	}
	return 0;
}

int clear_hashmap_ustring_analysis(struct hashmap_ustring_analysis ** pp_husa) {
	if (pp_husa == NULL || *pp_husa == NULL || (*pp_husa)->usa_list == NULL) {
		return -1;
	}

	for (size_t i = 0; i < (*pp_husa)->hashlen; ++i) {
		struct ustring_analysis * p = (*pp_husa)->usa_list[i];
		while (p != NULL) {
			clear_ustring(&p->us);
			p = p->next;
		}
	}
	free((*pp_husa)->usa_list);
	free(*pp_husa);
	return 0;
}

p_uspl blankParser(const struct ustring * cp_us) {
	return NULL;
}

p_uspl ucharParser(const struct ustring * cp_us) {
	return NULL;
}
