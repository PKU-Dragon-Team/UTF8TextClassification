#include "UTF8TextClassification.h"
#include <locale.h>

struct uchar_analysis uca = { 0 };

int main(int arc, char * argv[]) {
	setlocale(LC_ALL, "en_US.UTF-8");

	FILE * in;
	FILE * out;
	fopen_s(&in, "in.txt", "r");
	fopen_s(&out, "out.txt", "w");

	struct text_list * tl = NULL;
	init_text_list(&tl, NULL, 0);
	load_texts(in, tl);
	output_texts(out, tl);

	FILE * out2;
	fopen_s(&out2, "out2.txt", "w");
	get_char_analysis(tl, &uca);
	output_char_analysis(out2, &uca);

	fclose(in);
	fclose(out);
	fclose(out2);
	return 0;
}