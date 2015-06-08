#include "UTF8TextClassification.h"

static void insert_usa_list(struct ustring_analysis * ap_usa[], struct ustring_analysis * p_usa, size_t hashcode) {
	if (ap_usa[hashcode] == NULL) {
		ap_usa[hashcode] = p_usa;
	}
	else {
		struct ustring_analysis * p = ap_usa[hashcode];
		while (p->next != NULL) {
			p = p->next;
		}
		p->next = p_usa;
	}
}

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

			if (i >= p_tlist->len) {	// Dynamic Expand
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
	for (int i = 0; i < MAX_UNICODE; ++i) {
		if (uca->uchar_list[i] != 0) {
			fprintf_s(out, "0x%02X\t%5d\n", i, uca->uchar_list[i]);
		}
	}
	return 0;
}

int init_hash_vector(struct hash_vector ** pp_hv) {
	if (pp_hv == NULL) {
		return -1;
	}

	*pp_hv = malloc(sizeof(struct hash_vector));
	if (*pp_hv == NULL) {
		return -1;
	}

	(*pp_hv)->usa_list = calloc(BASE_HASH_LEN, sizeof(struct ustring_analysis *));
	if ((*pp_hv)->usa_list == NULL) {
		return -1;
	}
	(*pp_hv)->hashlen = BASE_HASH_LEN;
	(*pp_hv)->total_count = 0;
	(*pp_hv)->count = 0;

	return 0;
}

int rehash_hash_vector(struct hash_vector * p_hv, size_t hashlen) {
	if (p_hv == NULL) {
		return -1;
	}

	struct ustring_analysis ** temp = calloc(hashlen, sizeof(struct ustring_analysis *));
	if (temp == NULL) {
		return -1;
	}

	for (size_t i = 0; i < p_hv->hashlen; ++i) {
		struct ustring_analysis * p = p_hv->usa_list[i];
		while (p != NULL) {
			insert_usa_list(temp, p, hash_ustring(p->us, HASH_SEED, hashlen));
			struct ustring_analysis * q = p->next;
			p->next = NULL;
			p = q;
		}
	}
	free(p_hv->usa_list);
	p_hv->usa_list = temp;
	p_hv->hashlen = hashlen;
	return 0;
}

int append_hash_vector(struct hash_vector * p_hv, const struct ustring * cp_us, parser f) {
	if (p_hv == NULL || cp_us == NULL || f == NULL) {
		return -1;
	}

	p_uspl parse_list = f(cp_us);

	for (size_t i = 0; i + 1 < parse_list->len; ++i) {
		// check if hashmap is overload
		if (p_hv->count * 2 > p_hv->hashlen) {
			rehash_hash_vector(p_hv, p_hv->hashlen * 2 + 1);
		}

		struct ustring * temp = malloc(sizeof(struct ustring));
		init_ustring(&temp, index, NULL, 0);
		slice_ustring(cp_us, temp, parse_list->parse_list[i], parse_list->parse_list[i + 1]);

		size_t hashcode = hash_ustring(temp, HASH_SEED, p_hv->hashlen);
		// search for existence
		struct ustring_analysis * p = p_hv->usa_list[hashcode];
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
			p_hv->usa_list[hashcode] = p_ua;
			++p_hv->count;
		}
		++p_hv->total_count;
	}
	return 0;
}

int add_hash_vector(struct hash_vector * p_hv1, const struct hash_vector * p_hv2) {
	if (p_hv1 == NULL || p_hv2 == NULL) {
		return -1;
	}

	return 0;
}

int sub_hash_vector(struct hash_vector * p_hv1, const struct hash_vector * p_hv2) {
	if (p_hv1 == NULL || p_hv2 == NULL) {
		return -1;
	}
	return 0;
}

long long product_hash_vector(const struct hash_vector * p_hv1, const struct hash_vector * p_hv2) {
	long long product = 0;

	for (size_t i = 0; i < p_hv1->hashlen; ++i) {
		struct ustring_analysis * p = p_hv1->usa_list[i];
		struct ustring_analysis * q;
		if (p_hv1->hashlen == p_hv2->hashlen) {
			q = p_hv2->usa_list[i];
		}
		else {
			q = p_hv2->usa_list[hash_ustring(p->us, HASH_SEED, p_hv2->hashlen)];
		}

		while (p != NULL) {
			while (q != NULL) {
				if (compare_ustring(p->us, q->us) == 0) {
					product += p->count * q->count;
				}
				q = q->next;
			}
			p = p->next;
		}
	}
	return product;
}

unsigned long long len2_hash_vector(const struct hash_vector * p_hv) {
	long long len2 = 0;

	for (size_t i = 0; i < p_hv->hashlen; ++i) {
		struct ustring_analysis * p = p_hv->usa_list[i];

	}
	return len2;
}

int clear_hash_vector(struct hash_vector ** pp_hv) {
	if (pp_hv == NULL || *pp_hv == NULL || (*pp_hv)->usa_list == NULL) {
		return -1;
	}

	for (size_t i = 0; i < (*pp_hv)->hashlen; ++i) {
		struct ustring_analysis * p = (*pp_hv)->usa_list[i];
		while (p != NULL) {
			clear_ustring(&p->us);
			p = p->next;
		}
	}
	free((*pp_hv)->usa_list);
	free(*pp_hv);
	return 0;
}

p_uspl blankParser(const struct ustring * cp_us) {
	return NULL;
}

p_uspl ucharParser(const struct ustring * cp_us) {
	return NULL;
}
