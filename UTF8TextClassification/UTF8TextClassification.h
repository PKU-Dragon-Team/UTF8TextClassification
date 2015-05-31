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

void load_texts(const struct text_list * l, FILE * in);
void output_texts(const struct text_list * l, FILE * out);