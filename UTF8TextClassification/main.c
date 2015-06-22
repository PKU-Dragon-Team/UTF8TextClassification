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
	if (load_texts(in, tl) != 0) {
		return -1;
	}
	output_texts(out, tl);
	fclose(in);
	fclose(out);

	FILE * out2;
	fopen_s(&out2, "out2.txt", "w");
	if (out2 == NULL) {
		return -1;
	}

	get_char_analysis(tl, &uca);
	output_char_analysis(out2, &uca);
	fclose(out2);

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
		struct hash_vector * temp;
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
			else if (tl->list[i].types[j] == 0) {
				sub_hash_vector(statistic[j], temp);
			}
		}
		clear_hash_vector(&temp);
	}

	for (int i = 0; i < TYPE_COUNT + 1; ++i) {
		low_cut_hash_vector(statistic[i], 1);
		output_hash_vector(out3[i], statistic[i]);
	}
	for (int i = 0; i < TYPE_COUNT + 1; ++i) {
		fclose(out3[i]);
	}

	FILE * out4;
	FILE * out5;
	fopen_s(&out4, "out4.txt", "w");
	fopen_s(&out5, "out5.txt", "w");
	if (out4 == NULL || out5 == NULL) {
		return -1;
	}

	llu correct = 0;
	for (llu i = 0; i < tl->len; ++i) {
		struct hash_vector * temp;
		struct ustring_parse_list * p_list;
		Lf cos[TYPE_COUNT] = { 0 };

		init_hash_vector(&temp);
		init_uspl(&p_list);
		commonParser(p_list, tl->list[i].us, checker);
		append_hash_vector(temp, tl->list[i].us, p_list);

		for (int j = 0; j < TYPE_COUNT; ++j) {
			cos[j] = cos_hash_vector(statistic[j], temp);
			fprintf_s(out4, "%Lf%s", cos[j], (j == TYPE_COUNT - 1) ? "" : "\t");
		}
		fprintf_s(out4, "%s", (i == tl->len - 1) ? "" : "\n");

		int8_t test[TYPE_COUNT] = { 0 };
		{
			bool flag = true;
			for (int j = 0; j < TYPE_COUNT - 1; ++j) {
				if (cos[j] > 0) {
					test[j] = 1;
					flag = false;
				}
			}
			if (flag) {
				test[TYPE_COUNT - 1] = 1;
			}
		}
		{
			bool flag = true;
			for (int j = 0; j < TYPE_COUNT; ++j) {
				if (test[j] != tl->list[i].types[j]) {
					flag = false;
				}
			}
			if (flag) {
				++correct;
			}
		}

		clear_hash_vector(&temp);
	}
	fprintf_s(out5, "%llu\n%llu\n%Lf", correct, tl->len, (Lf)correct / (Lf)tl->len);

	// now with AVX2, fast float and cut down at 1, the time cost down to 30s/10000texts
	fclose(out4);
	fclose(out5);

	return 0;
}