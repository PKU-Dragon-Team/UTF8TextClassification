#include "UTF8TextClassification.h"
#include <locale.h>

static bool checker(const uchar uc[]) {
    return *uc == '\0' || *uc == '\n' || *uc == '\r' || *uc == '\t' || *uc == ' ' || *uc == '.' || *uc == ',';
}

static const char * SHORT_USAGE = "UTF8TextClassification [-h] [-t file_name] [-T file_name] [-l file_name] [-s file_name]\n";

static const char * USAGE =
"UTF8TextClassification [option1] [value1] [option2] [value2] ...\n"
"[-/]h : display usage guide.\n"
"[-/]t file_name : set the file for training.\n"
"[-/]T file_name : set the file for testing.\n"
"[-/]c file_name : set the file for classification.\n"
"[-/]l file_name : set the vector file for loading.\n"
"\tIf both -t and -l exist, incrementally training will be done.\n"
"[-/]s file_name : set the vector file for saving.\n";

int main(int argc, char * argv[]) {
    setlocale(LC_ALL, "en_US.UTF-8");

    // Deal with console command
    if (argc < 2) {
        printf(SHORT_USAGE);
        return 1;
    }

    struct hash_vector * statistic[TYPE_COUNT + 1] = { 0 };
    struct text_list * tl_train = NULL;
    struct text_list * tl_test = NULL;
    struct text_list * tl_class = NULL;
    FILE * input_file = NULL;
    FILE * output_file = NULL;
    FILE * bin_in = NULL;
    FILE * bin_out = NULL;

    bool isLoaded = false;
    bool isToTest = false;
    bool isToClass = false;

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

        case 't':
            fopen_s(&input_file, argv[++i], "r");
            if (input_file == NULL) {
                printf("Cannot open training file \"%s\"\n", argv[i]);
                return 1;
            }
            init_text_list(&tl_train, NULL, 0);
            if (load_texts(input_file, tl_train) != 0) {
                printf("Training file loading fail\n");
                return 1;
            }
            fclose(input_file);
            isLoaded = true;
            break;

        case 'T':
            fopen_s(&input_file, argv[++i], "r");
            if (input_file == NULL) {
                printf("Cannot open testing file \"%s\"\n", argv[i]);
                return 1;
            }
            init_text_list(&tl_test, NULL, 0);
            if (load_texts(input_file, tl_test) != 0) {
                printf("Testing file loading fail\n");
                return 1;
            }
            fclose(input_file);
            isToTest = true;
            break;

        case 'c':
            fopen_s(&input_file, argv[++i], "r");
            if (input_file == NULL) {
                printf("Cannot open testing file \"%s\"\n", argv[i]);
                return 1;
            }
            init_text_list(&tl_class, NULL, 0);
            if (load_texts(input_file, tl_class) != 0) {
                printf("Testing file loading fail\n");
                return 1;
            }
            fclose(input_file);
            isToClass = true;
            break;

        case 'l':
            fopen_s(&bin_in, "argv[++i]", "rb");
            if (bin_in == NULL) {
                printf("Cannot open vector file \"%s\"\n", argv[i]);
                return 1;
            }
            if (load_vectors(bin_in, statistic) != 0) {
                printf("Vector file loading fail\n");
                return 1;
            }
            fclose(bin_in);
            isLoaded = true;
            break;

        case 's':
            fopen_s(&bin_out, "argv[++i]", "rb");
            if (bin_out == NULL) {
                printf("Cannot open vector file \"%s\"\n", argv[i]);
                return 1;
            }
            break;

        default:
            printf("Illegal parameter\"%s\"\n", s);
            return 1;
            break;
        }
        ++i;
    }
    if (!isLoaded) {
        printf("There shall be either a train file or a vector file\n");
        return 1;
    }

    naive_trainer(statistic, tl_train, commonParser, checker);

    if (bin_out != NULL) {
        save_vectors(bin_out, statistic);
        fclose(bin_out);
    }

    fopen_s(&output_file, "out.txt", "w");
    if (output_file == NULL) {
        return -1;
    }

    if (isToTest) {
        KNN_tester(output_file, tl_test, statistic, commonParser, checker);
    }

    // TODO: the classifier

    fclose(output_file);

    return 0;
}