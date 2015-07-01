#include "UTF8TextClassification.h"
#include <locale.h>

struct uchar_analysis uca = { 0 };

static bool checker(const uchar uc[]) {
    return *uc == '\0' || *uc == '\n' || *uc == '\r' || *uc == '\t' || *uc == ' ' || *uc == '.' || *uc == ',';
}

static const char * SHORT_USAGE = "UTF8TextClassification [-h] [-t file_name] [-T file_name] [-l file_name] [-s file_name]\n";

static const char * USAGE =
"UTF8TextClassification [option1] [value1] [option2] [value2] ...\n"
"[-/]h, --help : display usage guide.\n"
"[-/]t, --train file_name : set the file for training.\n"
"[-/]T, --test file_name : set the file for testing.\n"
"[-/]l, --load file_name : set the vector file for loading.\n"
"\tIf both -t and -l exist, will do incrementally training.\n"
"[-/]s, --save file_name : set the vector file for saving.\n";

int main(int argc, char * argv[]) {
    setlocale(LC_ALL, "en_US.UTF-8");

    if (argc < 2) {
        printf(SHORT_USAGE);
        return 0;
    }

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
    for (type_t i = 0; i < TYPE_COUNT + 1; ++i) {
        char s[BUF_SIZE] = { 0 };
        sprintf_s(s, BUF_SIZE, "out3(%d).txt", i);
        fopen_s(&out3[i], s, "w");
        if (out3[i] == NULL) {
            return -1;
        }
    }

    struct hash_vector * statistic[TYPE_COUNT + 1] = { 0 };
    train_vector(statistic, tl, checker);

    for (type_t i = 0; i < TYPE_COUNT + 1; ++i) {
        output_hash_vector(out3[i], statistic[i]);
    }
    for (type_t i = 0; i < TYPE_COUNT + 1; ++i) {
        fclose(out3[i]);
    }

    FILE * bin_out;
    fopen_s(&bin_out, "vector.bin", "wb");
    save_vectors(bin_out, statistic);
    fclose(bin_out);

    FILE * bin_in;
    fopen_s(&bin_in, "vector.bin", "rb");
    load_vectors(bin_in, statistic);
    fclose(bin_in);

    // TODO: split the trainer and the tester
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

        for (type_t j = 0; j < TYPE_COUNT; ++j) {
            cos[j] = cos_hash_vector(statistic[j], temp);
            fprintf_s(out4, "%Lf%s", cos[j], (j == TYPE_COUNT - 1) ? "" : "\t");
        }
        fprintf_s(out4, "%s", (i == tl->len - 1) ? "" : "\n");

        int8_t test[TYPE_COUNT] = { 0 };
        {
            bool flag = true;
            for (type_t j = 0; j < TYPE_COUNT - 1; ++j) {
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
            for (type_t j = 0; j < TYPE_COUNT; ++j) {
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