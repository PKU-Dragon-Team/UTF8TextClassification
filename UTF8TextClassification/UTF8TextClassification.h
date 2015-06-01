#include "ucharLib.h"

#define TYPE_COUNT 4
#define BUF_SIZE BUFSIZ

struct text {
	struct ustring text;
	int8_t types[TYPE_COUNT];
};

struct text_list {
	struct text * list;
	size_t len;
};

int init_text_list(struct text_list ** l);
int resize_text_list(struct text_list * l, size_t len);
int clear_text_list(struct text_list ** l);

int load_texts(const struct text_list * l, FILE * in);
int parse_type(uchar * ts, int8_t types[TYPE_COUNT]);
int output_texts(const struct text_list * l, FILE * out);