#include "UTF8TextClassification.h"
#include <locale.h>

struct uchar_analysis uca = { 0 };

bool checker(const uchar uc[]) {
	return *uc == '\0' || *uc == '\n' || *uc == '\r' || *uc == '\t' || *uc == ' ' || *uc == '.' || *uc == ',';
}

int main(int arc, char * argv[]) {
	setlocale(LC_ALL, "en_US.UTF-8");

	FILE * in;
	FILE * out;
	fopen_s(&in, "in.txt", "r");
	fopen_s(&out, "out.txt", "w");
	if (in == NULL || out == NULL) {
		return -1;
	}

	struct text_list * tl = NULL;
	init_text_list(&tl, NULL, 0);
	// TODO: the input file is too big, and the memory usage is too high, should use incremental method

	load_texts(in, tl);
	output_texts(out, tl);

	FILE * out2;
	fopen_s(&out2, "out2.txt", "w");
	if (out2 == NULL) {
		return -1;
	}

	get_char_analysis(tl, &uca);
	output_char_analysis(out2, &uca);

	FILE * out3;
	fopen_s(&out3, "out3.txt", "w");
	if (out3 == NULL) {
		return -1;
	}

	for (size_t i = 0; i < tl->len; ++i) {
		struct hash_vector * temp = malloc(sizeof(struct hash_vector));
		init_hash_vector(&temp);
		append_hash_vector(temp, tl->list[i].us, ucharParser, checker);
		tl->list[i].usa = temp;
		output_hash_vector(out3, temp);
	}

	for (size_t i = 0; i < tl->len; ++i) {
		clear_hash_vector(&tl->list[i].usa);
	}

	fclose(in);
	fclose(out);
	fclose(out2);
	fclose(out3);
	return 0;
}