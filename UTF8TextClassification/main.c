#include "UTF8TextClassification.h"
#include <locale.h>

int main(int arc, char * argv[]) {
	setlocale(LC_ALL, "en_US.UTF-8");

	FILE * in;
	FILE * out;
	fopen_s(&in, "in.txt", "r");
	fopen_s(&out, "out.txt", "w");

	struct text_list * tl = NULL;
	init_text_list(&tl, NULL, 0);
	load_texts(tl, in);
	output_texts(tl, out);


	fclose(in);
	fclose(out);
	return 0;
}