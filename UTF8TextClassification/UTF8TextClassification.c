#include "UTF8TextClassification.h"

static bool insert_usa_list(struct ustring_analysis * ap_usa[], struct ustring_analysis * p_usa, llu hashcode) {
	if (ap_usa[hashcode] == NULL) {
		ap_usa[hashcode] = p_usa;
		return true;
	}
	else {
		struct ustring_analysis * p = ap_usa[hashcode];

		if (compare_ustring(p->us, p_usa->us) == 0) {
			p->count += p_usa->count;
		}
		else {
			bool flag = true;
			while (p->next != NULL) {
				if (compare_ustring(p->next->us, p_usa->us) == 0) {
					p->next->count += p_usa->count;
					flag = false;
					break;
				}
				p = p->next;
			}
			if (flag) {
				if (compare_ustring(p->us, p_usa->us) == 0) {
					p->count += p_usa->count;
				}
				else {
					p->next = p_usa;
					return true;
				}
			}
		}
		return false;
	}
	return true;
}

static bool is_blank(const uchar uc[]) {
	return *uc == '\n' || *uc == '\r' || *uc == '\t' || *uc == ' ';
}

static void free_if_not_null(void * p) {
	if (p != NULL) {
		free(p);
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
		free(*pp_text);
		*pp_text = NULL;
	}
	return 0;
}

int init_text_list(struct text_list ** pp_tlist, const struct text a_text[], llu len) {
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

int resize_text_list(struct text_list * p_tlist, llu len) {
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
	for (llu i = 0; i < (*pp_tlist)->len; ++i) {
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
		enum {
			init,
			first,
			text,
			type,
			error
		} state = init;

		uchar s[BUF_SIZE] = { 0 };
		int8_t types[TYPE_COUNT] = { 0 };
		struct ustring * us = NULL;
		llu i = 0;
		llu line = 0;
		bool flag = true;

		while (flag) {
			if (fgets(s, BUF_SIZE, in) == NULL) {
				flag = false;
			}
			switch (state) {
			case init:
				if (strnlen(s, BUF_SIZE) < 4 || !isdigit(s[3])) {
					state = error;
				}
				else {
					state = first;
				}
				break;
			case first:
				if (s[0] == '*') {
					state = error;
				}
				else {
					init_ustring(&us, index, s, BUF_SIZE);
					state = text;
				}
				break;
			case text:
				if (s[0] == '*') {
					if (strnlen(s, BUF_SIZE) < 5 || s[3] != '(') {
						state = error;
					}
					else {
						parse_type(s, types);
						state = type;
					}
				}
				else {
					struct ustring * temp = NULL;
					init_ustring(&temp, index, s, BUF_SIZE);
					cat_ustring(us, temp);
					clear_ustring(&temp);
				}
				break;
			case type:
				if (s[0] != '*') {
					state = error;
				}
				else {
					struct text * t = NULL;
					init_text(&t, us, types);
					if (i >= p_tlist->len) {	// Dynamic Expand
						if (resize_text_list(p_tlist, p_tlist->len * 2 + 1) != 0) {
							return -1;
						}
					}
					p_tlist->list[i] = *t;
					++i;
					state = first;
				}
				break;
			case error:
				printf("Load error at line %llu, %llu texts have been read.\n", line, i);
				return -1;
				break;
			default:
				break;
			}
			++line;
		}
		resize_text_list(p_tlist, i);
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

	llu n = strlen(a_type);
	int ti = 0;
	for (llu i = 0; i < n; ++i) {
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

	for (llu i = 0; i < p_tlist->len; ++i) {
		fprintf_s(out, "***%llu\n%s***(", i + 1, p_tlist->list[i].us->string);
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

	for (llu i = 0; i < cp_tlist->len; ++i) {
		for (llu j = 0; j < cp_tlist->list[i].us->string_len; ++j) {
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

	fprintf_s(out, "Total Characters: %llu\n", uca->total_count);
	for (int i = 0; i < MAX_UNICODE; ++i) {
		if (uca->uchar_list[i] != 0) {
			fprintf_s(out, "0x%X\t%lld\n", i, uca->uchar_list[i]);
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

int rehash_hash_vector(struct hash_vector * p_hv, llu hashlen) {
	if (p_hv == NULL) {
		return -1;
	}

	struct ustring_analysis ** temp = calloc(hashlen, sizeof(struct ustring_analysis *));
	if (temp == NULL) {
		return -1;
	}

	for (llu i = 0; i < p_hv->hashlen; ++i) {
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

int append_hash_vector(struct hash_vector * p_hv, const struct ustring * cp_us, const struct ustring_parse_list * p_uspl) {
	if (p_hv == NULL || cp_us == NULL || p_uspl == NULL) {
		return -1;
	}

	for (llu i = 0; i + 1 <= p_uspl->len; ++i) {
		// check if hash-map is overload
		if (p_hv->count * 2 > p_hv->hashlen) {
			rehash_hash_vector(p_hv, p_hv->hashlen * 2 + 1);
		}

		struct ustring * temp = malloc(sizeof(struct ustring));
		init_ustring(&temp, index, NULL, 0);
		slice_ustring(temp, cp_us, p_uspl->start[i], p_uspl->end[i]);
		insert_hash_vector(p_hv, temp, 1, NULL);
	}
	return 0;
}

int insert_hash_vector(struct hash_vector * p_hv, const struct ustring * us, lld count, struct ustring_analysis * next) {
	if (p_hv == NULL) {
		return -1;
	}

	struct ustring_analysis * p_ua = malloc(sizeof(struct ustring_analysis));
	if (p_ua == NULL) {
		return -1;
	}
	struct ustring * temp;
	init_ustring(&temp, index, NULL, 0);
	clone_ustring(temp, us);

	p_ua->us = temp;
	p_ua->count = count;
	p_ua->next = next;

	if (insert_usa_list(p_hv->usa_list, p_ua, hash_ustring(p_ua->us, HASH_SEED, p_hv->hashlen))) {
		++p_hv->count;
	}
	else {
		clear_ustring(&temp);
	}
	p_hv->total_count += llabs(count);

	// check if hash-map is overload
	if (p_hv->count * 2 > p_hv->hashlen) {
		rehash_hash_vector(p_hv, p_hv->hashlen * 2 + 1);
	}

	return 0;
}

int add_hash_vector(struct hash_vector * p_hv1, const struct hash_vector * p_hv2) {
	if (p_hv1 == NULL || p_hv2 == NULL) {
		return -1;
	}

	for (llu i = 0; i < p_hv2->hashlen; ++i) {
		struct ustring_analysis * p = p_hv2->usa_list[i];

		while (p != NULL) {
			insert_hash_vector(p_hv1, p->us, p->count, NULL);
			p = p->next;
		}
	}
	return 0;
}

int sub_hash_vector(struct hash_vector * p_hv1, const struct hash_vector * p_hv2) {
	if (p_hv1 == NULL || p_hv2 == NULL) {
		return -1;
	}

	for (llu i = 0; i < p_hv2->hashlen; ++i) {
		struct ustring_analysis * p = p_hv2->usa_list[i];

		while (p != NULL) {
			insert_hash_vector(p_hv1, p->us, -p->count, NULL);
			p = p->next;
		}
	}
	return 0;
}

int low_cut_hash_vector(struct hash_vector * p_hv, lld min_count) {
	if (p_hv == NULL) {
		return -1;
	}
	for (llu i = 0; i < p_hv->hashlen; ++i) {
		if (p_hv->usa_list[i] != NULL) {
			struct ustring_analysis ** pp = &p_hv->usa_list[i];
			while (*pp != NULL) {
				struct ustring_analysis * p_next = (*pp)->next;
				if ((*pp)->count < min_count) {
					clear_ustring(&(*pp)->us);
					free(*pp);
					*pp = p_next;
				}
				else {
					pp = &(*pp)->next;
				}
				*pp = p_next;
			}
		}
	}
	return 0;
}

lld product_hash_vector(const struct hash_vector * p_hv1, const struct hash_vector * p_hv2) {
	const struct hash_vector * p1;
	const struct hash_vector * p2;
	lld product = 0;

	if (p_hv1->count > p_hv2->count) {
		p1 = p_hv2;
		p2 = p_hv1;
	}
	else {
		p1 = p_hv1;
		p2 = p_hv2;
	}

	for (llu i = 0; i < p1->hashlen; ++i) {
		if (p1->usa_list[i] != NULL) {
			struct ustring_analysis * p = p1->usa_list[i];
			struct ustring_analysis * q;
			if (p1->hashlen == p2->hashlen) {
				q = p2->usa_list[i];
			}
			else {
				q = p2->usa_list[hash_ustring(p->us, HASH_SEED, p2->hashlen)];
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
	}
	return product;
}

llu len2_hash_vector(const struct hash_vector * p_hv) {
	llu len2 = 0;

	for (llu i = 0; i < p_hv->hashlen; ++i) {
		struct ustring_analysis * p = p_hv->usa_list[i];
		while (p != NULL) {
			len2 += p->count * p->count;
			p = p->next;
		}
	}
	return len2;
}

Lf cos_hash_vector(const struct hash_vector * p_hv1, const struct hash_vector * p_hv2) {
	return (Lf)product_hash_vector(p_hv1, p_hv2) / sqrtl((Lf)len2_hash_vector(p_hv1) * (Lf)len2_hash_vector(p_hv2));
}


int clear_hash_vector(struct hash_vector ** pp_hv) {
	if (pp_hv == NULL || *pp_hv == NULL || (*pp_hv)->usa_list == NULL) {
		return -1;
	}

	for (llu i = 0; i < (*pp_hv)->hashlen; ++i) {
		struct ustring_analysis * p = (*pp_hv)->usa_list[i];
		while (p != NULL) {
			clear_ustring(&p->us);
			p = p->next;
		}
	}
	free((*pp_hv)->usa_list);
	free(*pp_hv);
	*pp_hv = NULL;
	return 0;
}

int commonParser(struct ustring_parse_list * p, const struct ustring * cp_us, const Checker cf) {
	if (p == NULL || cp_us == NULL) {
		return -1;
	}
	Checker func;
	if (cf == NULL) {
		func = is_blank;
	}
	else {
		func = cf;
	}

	p->start = calloc(cp_us->index_len + 1, sizeof(llu));
	p->end = calloc(cp_us->index_len + 1, sizeof(llu));
	if (p->start == NULL || p->end == NULL) {
		return -1;
	}

	llu j = 0;
	bool inword = false;
	for (llu i = 0; i <= cp_us->index_len; ++i) {
		if (func(&cp_us->string[cp_us->index[i]])) {
			if (inword) {
				p->end[j] = i;
				++j;
				inword = false;
			}
		}
		else {
			if (!inword) {
				p->start[j] = i;
				inword = true;
			}
		}
	}
	{
		llu * temp = realloc(p->start, j * sizeof(llu));
		if (temp == NULL) {
			return -1;
		}
		p->start = temp;
	}
	{
		llu * temp = realloc(p->end, j * sizeof(llu));
		if (temp == NULL) {
			return -1;
		}
		p->end = temp;
	}
	p->len = j;

	return 0;
}

int ucharParser(struct ustring_parse_list * p, const struct ustring * cp_us, const Checker cf) {
	if (p == NULL || cp_us == NULL || cf == NULL) {
		return -1;
	}
	p->start = calloc(cp_us->index_len + 1, sizeof(llu));
	p->end = calloc(cp_us->index_len + 1, sizeof(llu));
	if (p->start == NULL || p->end == NULL) {
		return -1;
	}

	llu j = 0;
	for (llu i = 0; i < cp_us->index_len; ++i) {
		if (cf(&cp_us->string[cp_us->index[i]])) {
			continue;
		}
		p->start[j] = i;
		p->end[j] = i + 1;
		++j;
	}
	p->len = j;
	return 0;
}

int init_uspl(struct ustring_parse_list ** pp_uspl) {
	if (pp_uspl == NULL) {
		return -1;
	}
	*pp_uspl = malloc(sizeof(struct ustring_parse_list));
	if (*pp_uspl == NULL) {
		return -1;
	}
	(*pp_uspl)->len = 0;
	(*pp_uspl)->start = NULL;
	(*pp_uspl)->end = NULL;
	return 0;
}

int clear_uspl(struct ustring_parse_list ** pp_uspl) {
	if (pp_uspl == NULL) {
		return -1;
	}
	if (*pp_uspl != NULL) {
		free_if_not_null((*pp_uspl)->start);
		free_if_not_null((*pp_uspl)->end);
		free(*pp_uspl);
		*pp_uspl = NULL;
	}
	return 0;
}

void output_hash_vector(FILE * out, const struct hash_vector * p_hv) {
	fprintf_s(out, "count\t%llu\ntotal_count\t%llu\nhashlen\t%llu\n", p_hv->count, p_hv->total_count, p_hv->hashlen);
	for (llu i = 0; i < p_hv->hashlen; ++i) {
		struct ustring_analysis * p = p_hv->usa_list[i];
		while (p != NULL) {
			fprintf_s(out, "%s\t%lld\n", p->us->string, p->count);
			p = p->next;
		}
	}
}

int save_vector(FILE * out, const struct hash_vector * p_hv) {
	if (out == NULL) {
		return -1;
	}

	fwrite(&p_hv->total_count, sizeof(llu), 1, out);
	fwrite(&p_hv->hashlen, sizeof(llu), 1, out);
	fwrite(&p_hv->count, sizeof(llu), 1, out);

	// traverse the hashmap
	for (llu i = 0; i < p_hv->hashlen; ++i) {
		struct ustring_analysis * p = p_hv->usa_list[i];
		while (p != NULL) {
			fwrite(&p->count, sizeof(lld), 1, out);
			fwrite(&p->us->string_len, sizeof(llu), 1, out);
			fwrite(p->us->string, sizeof(uchar), p->us->string_len, out);
			p = p->next;
		}
	}
	return 0;
}

int load_vector(FILE * in, struct hash_vector * p_hv) {
	if (in == NULL) {
		return -1;
	}

	fread(&p_hv->total_count, sizeof(llu), 1, in);
	fread(&p_hv->hashlen, sizeof(llu), 1, in);
	fread(&p_hv->count, sizeof(llu), 1, in);
	p_hv->usa_list = calloc(p_hv->hashlen, sizeof(struct ustring_analysis *));
	if (p_hv->usa_list == NULL) {
		return -1;
	}

	for (llu i = 0; i < p_hv->count; ++i) {
		struct ustring_analysis * p = malloc(sizeof(struct ustring_analysis));
		if (p == NULL) {
			return -1;
		}
		fread(&p->count, sizeof(lld), 1, in);

		llu string_len;
		fread(&string_len, sizeof(llu), 1, in);
		uchar *s = calloc(string_len + 1, sizeof(uchar));
		if (s == NULL) {
			return -1;
		}
		fread(s, sizeof(uchar), string_len, in);
		s[string_len] = '\0';
		struct ustring *us = NULL;
		init_ustring(&us, index, s, string_len);
		p->us = us;

		insert_usa_list(p_hv->usa_list, p, hash_ustring(us, 0, p_hv->hashlen));
	}
	return 0;
}