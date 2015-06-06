#ifndef UTF8TEXT
#define UTF8TEXT

#include "ucharLib.h"

#define TYPE_COUNT 4
#define BUF_SIZE 10000
#define BASE_HASH_LEN 65535

struct text {
	struct ustring * us;
	struct hashmap_ustring_analysis * usa;
	int8_t types[TYPE_COUNT];
};

struct text_list {
	struct text * list;
	size_t len;
};

struct uchar_analysis {
	size_t total_count;
	long long uchar_list[130000];
};

struct ustring_analysis {
	struct ustring * us;
	size_t hashcode;
	long long count;
};

struct hashmap_ustring_analysis {
	long long total_count;
	struct ustring_analysis ** usa_list;
	size_t hashlen;
};

int init_text(struct text ** t, struct ustring * us, int8_t types[TYPE_COUNT]);
int clear_text(struct text ** t);

int init_text_list(struct text_list ** l, const struct text * list, size_t len);
int resize_text_list(struct text_list * l, size_t len);
int clear_text_list(struct text_list ** l);

int load_texts(FILE * in, struct text_list * l);
int parse_type(uchar * ts, int8_t types[TYPE_COUNT]);
int output_texts(FILE * out, const struct text_list * l);

int get_char_analysis(const struct text_list * l, struct uchar_analysis * uca);
int output_char_analysis(FILE * out, const struct uchar_analysis * uca);

int init_hashmap_ustring_analysis(struct hashmap_ustring_analysis ** husa);
int append_hashmap_ustring_analysis(struct hashmap_ustring_analysis * husa, const struct ustring * us);
int clear_hashmap_ustring_analysis(struct hashmap_ustring_analysis ** husa);

#endif
