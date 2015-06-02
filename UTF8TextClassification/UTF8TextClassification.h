#ifndef UTF8TEXT
#define UTF8TEXT

#include "ucharLib.h"

#define TYPE_COUNT 4
#define BUF_SIZE BUFSIZ

struct text {
	struct ustring * text;
	int8_t types[TYPE_COUNT];
};

struct text_list {
	struct text * list;
	size_t len;
};

int init_text(struct text ** t, struct ustring * us, int8_t types[TYPE_COUNT]);
int clear_text(struct text ** t);

int init_text_list(struct text_list ** l, struct text * list, size_t len);
int resize_text_list(struct text_list * l, size_t len);
int clear_text_list(struct text_list ** l);

int load_texts(struct text_list * l, FILE * in);
int parse_type(uchar * ts, int8_t types[TYPE_COUNT]);
int output_texts(const struct text_list * l, FILE * out);
#endif
