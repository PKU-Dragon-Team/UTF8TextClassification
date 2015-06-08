#ifndef UTF8TEXT
#define UTF8TEXT

#include "ucharLib.h"

#define TYPE_COUNT 4
#define BUF_SIZE 10000
#define BASE_HASH_LEN 65535
#define MAX_UNICODE 130000
#define HASH_SEED 0

typedef struct ustring_parse_list * p_uspl;
typedef p_uspl(*parser)(const struct ustring * cp_us);

struct text {
	struct ustring * us;
	struct hash_vector * usa;
	int8_t types[TYPE_COUNT];
};

struct text_list {
	struct text * list;
	size_t len;
};

struct uchar_analysis {
	size_t total_count;
	long long uchar_list[MAX_UNICODE];
};

struct ustring_analysis {
	struct ustring * us;
	long long count;
	struct ustring_analysis * next;
};

struct hash_vector {
	unsigned long long total_count;
	struct ustring_analysis ** usa_list;
	size_t hashlen;
	size_t count;
};

struct ustring_parse_list {
	size_t * parse_list;
	size_t len;
};

// Incert p_usa to the hashcode position of ap_usa
static void insert_usa_list(struct ustring_analysis * ap_usa[], struct ustring_analysis * p_usa, size_t hashcode);

int init_text(struct text ** pp_text, struct ustring * us, int8_t types[TYPE_COUNT]);
int clear_text(struct text ** pp_text);

int init_text_list(struct text_list ** pp_tlist, const struct text a_text[], size_t len);
int resize_text_list(struct text_list * p_tlist, size_t len);
int clear_text_list(struct text_list ** pp_tlist);

int load_texts(FILE * in, struct text_list * p_tlist);
int parse_type(const uchar a_type[], int8_t types[TYPE_COUNT]);
int output_texts(FILE * out, const struct text_list * p_tlist);

int get_char_analysis(const struct text_list * cp_tlist, struct uchar_analysis * uca);
int output_char_analysis(FILE * out, const struct uchar_analysis * uca);

int init_hash_vector(struct hash_vector ** pp_hv);
int rehash_hash_vector(struct hash_vector * p_hv, size_t hashlen);
int append_hash_vector(struct hash_vector * p_hv, const struct ustring * cp_us, parser f);
int add_hash_vector(struct hash_vector * p_hv1, const struct hash_vector * p_hv2);
int sub_hash_vector(struct hash_vector * p_hv1, const struct hash_vector * p_hv2);
long long product_hash_vector(const struct hash_vector * p_hv1, const struct hash_vector * p_hv2);
unsigned long long len2_hash_vector(const struct hash_vector * p_hv);
int clear_hash_vector(struct hash_vector ** pp_hv);

p_uspl blankParser(const struct ustring * cp_us);
p_uspl ucharParser(const struct ustring * cp_us);

#endif
