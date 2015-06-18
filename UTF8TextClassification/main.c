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
	if (load_texts(in, tl) != 0) {
		return -1;
	}
	output_texts(out, tl);

	FILE * out2;
	fopen_s(&out2, "out2.txt", "w");
	if (out2 == NULL) {
		return -1;
	}

	get_char_analysis(tl, &uca);
	output_char_analysis(out2, &uca);

	FILE * out3[TYPE_COUNT + 1];
	for (int i = 0; i < TYPE_COUNT + 1; ++i) {
		char s[BUF_SIZE] = { 0 };
		sprintf_s(s, BUF_SIZE, "out3(%d).txt", i);
		fopen_s(&out3[i], s, "w");
		if (out3[i] == NULL) {
			return -1;
		}
	}

	struct hash_vector * statistic[TYPE_COUNT + 1] = { 0 };

	for (int i = 0; i < TYPE_COUNT + 1; ++i) {
		init_hash_vector(&statistic[i]);
	}

	for (llu i = 0; i < tl->len; ++i) {
		struct hash_vector * temp = malloc(sizeof(struct hash_vector));
		struct ustring_parse_list * p_list;
		init_hash_vector(&temp);
		init_uspl(&p_list);
		commonParser(p_list, tl->list[i].us, checker);
		append_hash_vector(temp, tl->list[i].us, p_list);

		add_hash_vector(statistic[TYPE_COUNT], temp);
		for (int j = 0; j < TYPE_COUNT; ++j) {
			if (tl->list[i].types[j] == 1) {
				add_hash_vector(statistic[j], temp);
			}
		}
		clear_hash_vector(&temp);
	}

	for (int i = 0; i < TYPE_COUNT + 1; ++i) {
		output_hash_vector(out3[i], statistic[i]);
	}

	fclose(in);
	fclose(out);
	fclose(out2);

	for (int i = 0; i < TYPE_COUNT + 1; ++i) {
		fclose(out3[i]);
	}
	return 0;
}