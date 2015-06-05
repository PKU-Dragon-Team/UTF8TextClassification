#ifndef UTF8TEXT
#define UTF8TEXT

#include "ucharLib.h"

#define TYPE_COUNT 4
#define BUF_SIZE 10000

struct text {
	struct ustring * text;
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
	size_t count;
};

struct hashmap_ustring_analysis {
	size_t total_count;
	struct ustring_analysis * usa_list;
	size_t len;
};

int init_text(struct text ** t, struct ustring * us, int8_t types[TYPE_COUNT]);
int clear_text(struct text ** t);

int init_text_list(struct text_list ** l, struct text * list, size_t len);
int resize_text_list(struct text_list * l, size_t len);
int clear_text_list(struct text_list ** l);

int load_texts(FILE * in, struct text_list * l);
int parse_type(uchar * ts, int8_t types[TYPE_COUNT]);
int output_texts(FILE * out, const struct text_list * l);

void get_char_analysis(const struct text_list * l, struct uchar_analysis * uca);
void output_char_analysis(FILE * out, const struct uchar_analysis * uca);
#endif
