#include "UTF8TextClassification.h"
#include <locale.h>

static bool checker(const uchar uc[]) {
    return *uc == '\0' || *uc == '\n' || *uc == '\r' || *uc == '\t' || *uc == ' ' || *uc == '.' || *uc == ',';
}

static const char * SHORT_USAGE = "Usage: classifier [-h] [-t file_name] [-T file_name] [-c file_name] [-l file_name] [-s file_name] [-o file_name]\n";

static const char * USAGE =
"classifier [option1] [value1] [option2] [value2] ...\n"
"[-/]h : display usage guide.\n"
"[-/]t file_name : set the file for training.\n"
"[-/]T file_name : set the file for testing.\n"
"[-/]c file_name : set the file for classification.\n"
"[-/]l file_name : set the vector file for loading.\n"
"\tIf both -t and -l exist, incrementally training will be done.\n"
"[-/]s file_name : set the vector file for saving.\n"
"\tIf both -l and -s exist, make sure that the -l is before -s.\n"
"[-/]o file_name : set the file for output."
"\tIf not given, the stdout will be used.";

// The function to parse a type string
int parse_type(const uchar a_type[], int8_t types[TYPE_COUNT]) {
    if (a_type == NULL) {
        return -1;
    }

    for (type_t i = 0; i < TYPE_COUNT; ++i) {
        types[i] = -1;
    }

    llu n = strlen(a_type);
    type_t ti = 0;
    for (llu i = 0; i < n; ++i) {
        if (ti < TYPE_COUNT && a_type[i] != '*' && a_type[i] != '(' && a_type[i] != ')' && a_type[i] != '\n') {
            types[ti] = a_type[i] - '0';
            ++ti;
        }
    }
    return 0;
}

// The function to load text from input stream and store it in text_list
int load_texts(FILE * input, struct text_list * p_tlist) {
    if (p_tlist == NULL || input == NULL) {
        return -1;
    }
    else {
        enum {
            init,
            first,
            text,
            type,
            error
        } state = init;

        uchar s[BUF_SIZE] = { 0 };
        int8_t types[TYPE_COUNT] = { 0 };
        struct ustring * us = NULL;
        llu i = 0;
        llu line = 0;
        bool flag = true;

        while (flag) {
            if (fgets(s, BUF_SIZE, input) == NULL) {
                flag = false;
            }
            switch (state) {
            case init:
                if (strnlen(s, BUF_SIZE) < 4 || !isdigit(s[3])) {
                    state = error;
                }
                else {
                    state = first;
                }
                break;
            case first:
                if (s[0] == '*') {
                    state = error;
                }
                else {
                    init_ustring(&us, index, s, BUF_SIZE);
                    state = text;
                }
                break;
            case text:
                if (s[0] == '*') {
                    if (strnlen(s, BUF_SIZE) < 5 || s[3] != '(') {
                        state = error;
                    }
                    else {
                        parse_type(s, types);
                        state = type;
                    }
                }
                else {
                    struct ustring * temp = NULL;
                    init_ustring(&temp, index, s, BUF_SIZE);
                    cat_ustring(us, temp);
                    clear_ustring(&temp);
                }
                break;
            case type:
                if (s[0] != '*') {
                    state = error;
                }
                else {
                    struct text * t = NULL;
                    init_text(&t, us, types);
                    if (i >= p_tlist->len) {	// Dynamic Expand
                        if (resize_text_list(p_tlist, p_tlist->len * 2 + 1) != 0) {
                            return -1;
                        }
                    }
                    p_tlist->list[i] = *t;
                    ++i;
                    state = first;
                }
                break;
            case error:
                printf("Load error at line %llu, %llu texts have been read.\n", line, i);
                return -1;
                break;
            default:
                break;
            }
            ++line;
        }
        resize_text_list(p_tlist, i);
    }
    return 0;
}

// The function to output a text_list with the same format
int output_texts(FILE * out, const struct text_list * p_tlist) {
    if (p_tlist == NULL || out == NULL) {
        return -1;
    }

    for (llu i = 0; i < p_tlist->len; ++i) {
        fprintf(out, "***%llu\n%s***(", i + 1, p_tlist->list[i].us->string);
        for (type_t j = 0; j < TYPE_COUNT; ++j) {
            if (p_tlist->list[i].types[j] != -1) {
                fprintf(out, "%d", p_tlist->list[i].types[j]);
            }
        }
        fprintf(out, ")%s", (i == p_tlist->len - 1) ? "" : "\n");
    }
    return 0;
}

int main(int argc, char * argv[]) {
    setlocale(LC_ALL, "en_US.UTF-8");

    // Deal with console command
    if (argc < 2) {
        printf(SHORT_USAGE);
        return 1;
    }

    struct hash_vector * statistic[TYPE_COUNT + 1] = { 0 };
    for (type_t i = 0; i < TYPE_COUNT + 1; ++i) {
        init_hash_vector(&statistic[i]);
    }

    struct text_list * tl_train = NULL;
    struct text_list * tl_test = NULL;
    struct text_list * tl_class = NULL;
    FILE * input_stream = NULL;
    FILE * output_stream = NULL;
    FILE * bin_in = NULL;
    FILE * bin_out = NULL;

    bool isTrainingFileGiven = false;
    bool isTestFileGiven = false;
    bool isClassFileGiven = false;
    bool isOutputFileGiven = false;
    bool isVectorFileGiven = false;
    bool isBinaryOutputGiven = false;

    int i = 1;
    while (i < argc) {
        char * s = argv[i];
        if (s[0] != '-' && s[0] != '/') {
            printf(SHORT_USAGE);
            return 1;
        }

        switch (s[1]) {
        case 'h':
            printf(USAGE);
            return 0;
            break;

        case 't':
            input_stream = fopen(argv[++i], "r");
            if (input_stream == NULL) {
                printf("Cannot open training file \"%s\".\n", argv[i]);
                return 1;
            }
            init_text_list(&tl_train, NULL, 0);
            if (load_texts(input_stream, tl_train) != 0) {
                printf("Training file \"%s\" loading fail.\n", argv[i]);
                return 1;
            }
            else {
                printf("Training file \"%s\" loaded.\n", argv[i]);
            }
            fclose(input_stream);
            isTrainingFileGiven = true;
            break;

        case 'T':
            input_stream = fopen(argv[++i], "r");
            if (input_stream == NULL) {
                printf("Cannot open testing file \"%s\".\n", argv[i]);
                return 1;
            }
            init_text_list(&tl_test, NULL, 0);
            if (load_texts(input_stream, tl_test) != 0) {
                printf("Testing file \"%s\" loading fail.\n", argv[i]);
                return 1;
            }
            else {
                printf("Testing file \"%s\" loaded.\n", argv[i]);
            }
            fclose(input_stream);
            isTestFileGiven = true;
            break;

        case 'c':
            input_stream = fopen(argv[++i], "r");
            if (input_stream == NULL) {
                printf("Cannot open class file \"%s\".\n", argv[i]);
                return 1;
            }
            init_text_list(&tl_class, NULL, 0);
            if (load_texts(input_stream, tl_class) != 0) {
                printf("Class file \"%s\" loading fail.\n", argv[i]);
                return 1;
            }
            else {
                printf("Class file \"%s\" loaded.\n", argv[i]);
            }
            fclose(input_stream);
            isClassFileGiven = true;
            break;

        case 'l':
            bin_in = fopen(argv[++i], "rb");
            if (bin_in == NULL) {
                printf("Cannot open vector file \"%s\".\n", argv[i]);
                return 1;
            }
            if (load_vectors(bin_in, statistic) != 0) {
                printf("Vector file \"%s\" loading fail.\n", argv[i]);
                return 1;
            }
            else {
                printf("Vector file \"%s\" loaded.\n", argv[i]);
            }
            fclose(bin_in);
            isVectorFileGiven = true;
            break;

        case 's':
            bin_out = fopen(argv[++i], "wb");
            if (bin_out == NULL) {
                printf("Cannot open vector file \"%s\".\n", argv[i]);
                return 1;
            }
            isBinaryOutputGiven = true;
            break;

        case 'o':
            output_stream = fopen(argv[++i], "w");
            if (output_stream == NULL) {
                printf("Cannot open output file \"%s\".\n", argv[i]);
                return 1;
            }
            isOutputFileGiven = true;
            break;

        default:
            printf("Illegal parameter\"%s\"\n", s);
            return 1;
            break;
        }
        ++i;
    }
    if (!isVectorFileGiven && !isTrainingFileGiven) {
        printf("There shall be either a train file or a vector file\n");
        return 1;
    }

    if (isTrainingFileGiven) {
        printf("Training...\n");
        naive_trainer(statistic, tl_train, commonParser, checker);
        printf("Training finished.\n");
    }

    if (isBinaryOutputGiven) {
        printf("Saving vectors...\n");
        save_vectors(bin_out, statistic);
        printf("Saving vectors finished.\n");
        fclose(bin_out);
    }

    if (!isOutputFileGiven) {
        output_stream = stdout;
    }

    if (isTestFileGiven) {
        printf("Testing...\n");
        KNN_tester(output_stream, tl_test, statistic, commonParser, checker);
        printf("Testing finished.\n");
    }

    if (isClassFileGiven) {
        printf("Classing...\n");
        KNN_classifier(output_stream, tl_class, statistic, commonParser, checker);
        printf("Classification finished.\n");
    }

    if (isOutputFileGiven) {
        fclose(output_stream);
    }

    return 0;
}