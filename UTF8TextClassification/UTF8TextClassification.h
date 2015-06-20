#ifndef UTF8TEXT
#define UTF8TEXT

#include "ucharLib.h"
#include <ctype.h>

#define TYPE_COUNT 4
#define BUF_SIZE 10000
#define BASE_HASH_LEN 65535
#define MAX_UNICODE 130000
#define HASH_SEED 0

typedef bool(*Checker)(const uchar uc[]);

struct text {
	struct ustring * us;
	int8_t types[TYPE_COUNT];
};

struct text_list {
	struct text * list;
	llu len;
};

struct uchar_analysis {
	llu total_count;
	lld uchar_list[MAX_UNICODE];
};

struct ustring_analysis {
	struct ustring * us;
	lld count;
	struct ustring_analysis * next;
};

struct hash_vector {
	llu total_count;
	struct ustring_analysis ** usa_list;
	llu hashlen;
	llu count;
};

struct ustring_parse_list {
	llu * start;
	llu * end;
	llu len;
};

static bool insert_usa_list(struct ustring_analysis * ap_usa[], struct ustring_analysis * p_usa, llu hashcode);
static bool is_blank(const uchar uc[]);
static void free_if_not_null(void * p);

int init_text(struct text ** pp_text, struct ustring * us, int8_t types[TYPE_COUNT]);
int clear_text(struct text ** pp_text);

int init_text_list(struct text_list ** pp_tlist, const struct text a_text[], llu len);
int resize_text_list(struct text_list * p_tlist, llu len);
int clear_text_list(struct text_list ** pp_tlist);

int load_texts(FILE * in, struct text_list * p_tlist);
int parse_type(const uchar a_type[], int8_t types[TYPE_COUNT]);
int output_texts(FILE * out, const struct text_list * p_tlist);

int get_char_analysis(const struct text_list * cp_tlist, struct uchar_analysis * uca);
int output_char_analysis(FILE * out, const struct uchar_analysis * uca);

int init_hash_vector(struct hash_vector ** pp_hv);
int rehash_hash_vector(struct hash_vector * p_hv, llu hashlen);

// Parse cp_us with pf and use the result to build the p_hv
int append_hash_vector(struct hash_vector * p_hv, const struct ustring * cp_us, const struct ustring_parse_list * p_uspl);

// Insert the us, count, next as a struct ustring_analysis into p_hv
static int insert_hash_vector(struct hash_vector * p_hv, const struct ustring * us, lld count, struct ustring_analysis * next);

int add_hash_vector(struct hash_vector * p_hv1, const struct hash_vector * p_hv2);
int sub_hash_vector(struct hash_vector * p_hv1, const struct hash_vector * p_hv2);
lld product_hash_vector(const struct hash_vector * p_hv1, const struct hash_vector * p_hv2);
llu len2_hash_vector(const struct hash_vector * p_hv);

int clear_hash_vector(struct hash_vector ** pp_hv);

int commonParser(struct ustring_parse_list * p, const struct ustring * cp_us, const Checker cf);
int ucharParser(struct ustring_parse_list * p, const struct ustring * cp_us, const Checker cf);

int init_uspl(struct ustring_parse_list ** pp_uspl);
int clear_uspl(struct ustring_parse_list ** pp_uspl);

void output_hash_vector(FILE * out, const struct hash_vector * p_hv);

int save_vector(FILE * out, const struct hash_vector * p_hv);
int load_vector(FILE * in, struct hash_vector * p_hv);

#endif
