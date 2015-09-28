#include "TrainersAndClassifiers.h"
#define OUTPUT_TEST     \
{\
    Lf precision = (Lf)truePositive / ((Lf)truePositive + (Lf)falsePositive);\
    Lf recall = (Lf)truePositive / (Lf)PTotal;\
    printf("\n"\
    "%llu Total\n"\
    "    Positive Total: %llu\n"\
    "        True Positive: %llu\n"\
    "        ----rate: %Lf%%\n"\
    "        False Negative: %llu\n"\
    "        ----rate: %Lf%%\n"\
    "    Negative Total: %llu\n"\
    "        True Negative: %llu\n"\
    "        ----rate: %Lf%%\n"\
    "        False Positive: %llu\n"\
    "        ----rate: %Lf%%\n"\
    "   Precision: %Lf%%\n"\
    "   Recall: %Lf%%\n"\
    "   F1-measure: %Lf%%\n",\
    p_tl->len, PTotal, truePositive, (Lf)truePositive / (Lf)PTotal * 100, \
    falseNegative, (Lf)falseNegative / (Lf)PTotal * 100, \
    NTotal, trueNegative, (Lf)trueNegative / (Lf)NTotal * 100, \
    falsePositive, (Lf)falsePositive / (Lf)NTotal * 100, \
    precision * 100, recall * 100, 2 * precision*recall / (precision + recall) * 100);\
}\
for (type_t i = 0; i < TYPE_COUNT; ++i) {\
    Lf precision = (Lf)TP[i] / ((Lf)TP[i] + (Lf)FP[i]);\
    Lf recall = (Lf)TP[i] / (Lf)PT[i];\
    printf("\n"\
        "    Type %d:\n"\
        "    Positive Total: %llu\n"\
        "        True Positive: %llu\n"\
        "        ----rate: %Lf%%\n"\
        "        False Negative: %llu\n"\
        "        ----rate: %Lf%%\n"\
        "    Negative Total: %llu\n"\
        "        True Negative: %llu\n"\
        "        ----rate: %Lf%%\n"\
        "        False Positive: %llu\n"\
        "        ----rate: %Lf%%\n"\
        "   Precision: %Lf%%\n"\
        "   Recall: %Lf%%\n"\
        "   F1-measure: %Lf%%\n",\
        i + 1, PT[i], TP[i], (Lf)TP[i] / (Lf)PT[i] * 100, \
        FN[i], (Lf)FN[i] / (Lf)PT[i] * 100, \
        NT[i], TN[i], (Lf)TN[i] / (Lf)NT[i] * 100, \
        FP[i], (Lf)FP[i] / (Lf)NT[i] * 100, \
        precision * 100, recall * 100, 2 * precision * recall / (precision + recall) * 100);\
}

static Lf get_possibility(const struct hash_vector * text, const struct hash_vector * statistic) {
    const struct hash_vector * p1 = text;
    const struct hash_vector * p2 = statistic;
    Lf possibility = 1;

    for (llu i = 0; i < p1->hashlen; ++i) {
        if (p1->usa_list[i] != NULL) {
            struct ustring_analysis * p = p1->usa_list[i];
            struct ustring_analysis * q;
            if (p1->hashlen == p2->hashlen) {
                q = p2->usa_list[i];
            }
            else {
                q = p2->usa_list[hash_ustring(p->us, HASH_SEED, p2->hashlen)];
            }

            while (p != NULL) {
                while (q != NULL) {
                    if (compare_ustring(p->us, q->us) == 0) {
                        possibility *= p->count * q->count * NB_CONVERGE;
                    }
                    q = q->next;
                }
                possibility /= NB_CONVERGE;
                p = p->next;
            }
        }
    }
    return possibility;
}

int naive_trainer(struct hash_vector * ap_hv[TYPE_COUNT + 1], const struct text_list * p_tl, Parser parser, Checker checker) {
    if (ap_hv == NULL || p_tl == NULL) {
        return -1;
    }

    for (type_t i = 0; i < TYPE_COUNT; ++i) {
        if (ap_hv[i] == NULL) {
            return -1;
        }
    }

    for (llu i = 0; i < p_tl->len; ++i) {
        struct hash_vector * temp;
        struct ustring_parse_list * p_list;
        init_hash_vector(&temp);
        init_uspl(&p_list);
        parser(p_list, p_tl->list[i].us, checker);
        append_hash_vector(temp, p_tl->list[i].us, p_list);

        add_hash_vector(ap_hv[TYPE_COUNT], temp);
        for (type_t j = 0; j < TYPE_COUNT; ++j) {
            if (p_tl->list[i].types[j] == 1) {
                add_hash_vector(ap_hv[j], temp);
            }
            else if (p_tl->list[i].types[j] == 0) {
                sub_hash_vector(ap_hv[j], temp);
            }
        }
        clear_hash_vector(&temp);
    }

    for (type_t i = 0; i < TYPE_COUNT + 1; ++i) {
        low_cut_hash_vector(ap_hv[i], 1);
    }
    return 0;
}

int KNN_tester(FILE * out, struct text_list * p_tl, struct hash_vector * const statistic[TYPE_COUNT + 1], Parser parser, Checker checker) {
    llu truePositive = 0;
    llu TP[TYPE_COUNT] = { 0 };
    llu trueNegative = 0;
    llu TN[TYPE_COUNT] = { 0 };
    llu falsePositive = 0;
    llu FP[TYPE_COUNT] = { 0 };
    llu falseNegative = 0;
    llu FN[TYPE_COUNT] = { 0 };
    llu PTotal = 0;
    llu PT[TYPE_COUNT] = { 0 };
    llu NTotal = 0;
    llu NT[TYPE_COUNT] = { 0 };
    for (llu i = 0; i < p_tl->len; ++i) {
        struct hash_vector * temp;
        struct ustring_parse_list * p_list;
        Lf cos[TYPE_COUNT] = { 0 };

        init_hash_vector(&temp);
        init_uspl(&p_list);
        parser(p_list, p_tl->list[i].us, checker);
        append_hash_vector(temp, p_tl->list[i].us, p_list);

        for (type_t j = 0; j < TYPE_COUNT; ++j) {
            cos[j] = cos_hash_vector(statistic[j], temp);
            fprintf(out, "%Lf%s", cos[j], (j == TYPE_COUNT - 1) ? "" : "\t");
        }
        fprintf(out, "%s", (i == p_tl->len - 1) ? "" : "\n");

        int8_t test[TYPE_COUNT] = { 0 };
        {
            for (type_t j = 0; j < TYPE_COUNT; ++j) {
                if (cos[j] > 0) {
                    test[j] = 1;
                }
            }
        }
        bool pflag = false;
        for (type_t j = 0; j < TYPE_COUNT; ++j) {
            if (p_tl->list[i].types[j] == 1) {
                pflag = true;
                ++PT[j];
            }
            else
            {
                ++NT[j];
            }
        }
        if (pflag) {
            ++PTotal;
        }
        else {
            ++NTotal;
        }
        {
            bool cflag = false;
            for (type_t j = 0; j < TYPE_COUNT; ++j) {
                bool btemp = p_tl->list[i].types[j] == 1;
                if (test[j] == 1) {
                    cflag = true;
                    if (btemp) {
                        ++TP[j];
                    }
                    else {
                        ++FP[j];
                    }
                }
                else
                {
                    if (btemp) {
                        ++FN[j];
                    }
                    else
                    {
                        ++TN[j];
                    }
                }
            }
            if (pflag && cflag) {
                ++truePositive;
            }
            if (!pflag && !cflag) {
                ++trueNegative;
            }
            if (!pflag && cflag) {
                ++falsePositive;
            }
            if (pflag && !cflag) {
                ++falseNegative;
            }
        }
        clear_hash_vector(&temp);
    }
    OUTPUT_TEST
    return 0;
}

int KNN_classifier(FILE * out, struct text_list * p_tl, struct hash_vector * const statistic[TYPE_COUNT + 1], Parser parser, Checker checker) {
    for (llu i = 0; i < p_tl->len; ++i) {
        struct hash_vector * temp;
        struct ustring_parse_list * p_list;
        Lf cos[TYPE_COUNT] = { 0 };

        init_hash_vector(&temp);
        init_uspl(&p_list);
        parser(p_list, p_tl->list[i].us, checker);
        append_hash_vector(temp, p_tl->list[i].us, p_list);

        for (type_t j = 0; j < TYPE_COUNT; ++j) {
            cos[j] = cos_hash_vector(statistic[j], temp);
        }

        int8_t a_class[TYPE_COUNT] = { 0 };
        {
            for (type_t j = 0; j < TYPE_COUNT; ++j) {
                if (cos[j] > 0) {
                    a_class[j] = 1;
                }
            }
        }

        for (type_t j = 0; j < TYPE_COUNT; ++j) {
            fprintf(out, "%d", a_class[j]);
        }
        fprintf(out, "\n");
        clear_hash_vector(&temp);
    }
    return 0;
}

int NB_tester(FILE * out, struct text_list * p_tl, struct hash_vector * const statistic[TYPE_COUNT + 1], Parser parser, Checker checker)
{
    llu truePositive = 0;
    llu TP[TYPE_COUNT] = { 0 };
    llu trueNegative = 0;
    llu TN[TYPE_COUNT] = { 0 };
    llu falsePositive = 0;
    llu FP[TYPE_COUNT] = { 0 };
    llu falseNegative = 0;
    llu FN[TYPE_COUNT] = { 0 };
    llu PTotal = 0;
    llu PT[TYPE_COUNT] = { 0 };
    llu NTotal = 0;
    llu NT[TYPE_COUNT] = { 0 };
    for (llu i = 0; i < p_tl->len; ++i) {
        struct hash_vector * temp;
        struct ustring_parse_list * p_list;
        Lf cos[TYPE_COUNT] = { 0 };
        Lf possibility[TYPE_COUNT] = { 0 };

        init_hash_vector(&temp);
        init_uspl(&p_list);
        parser(p_list, p_tl->list[i].us, checker);
        append_hash_vector(temp, p_tl->list[i].us, p_list);

        for (type_t j = 0; j < TYPE_COUNT; ++j) {
            cos[j] = cos_hash_vector(statistic[j], temp);
            possibility[j] = get_possibility(temp, statistic[j]) / (Lf)TYPE_COUNT;
            fprintf(out, "%Lf %Lf%s", cos[j], possibility[j], (j == TYPE_COUNT - 1) ? "" : "\t");
        }
        fprintf(out, "%s", (i == p_tl->len - 1) ? "" : "\n");

        int8_t test[TYPE_COUNT] = { 0 };
        {
            for (type_t j = 0; j < TYPE_COUNT; ++j) {
                if (cos[j] > NB_CUTDOWN || possibility[j] > NB_THRESHOLD) {
                    test[j] = 1;
                }
            }
        }
        bool pflag = false;
        for (type_t j = 0; j < TYPE_COUNT; ++j) {
            if (p_tl->list[i].types[j] == 1) {
                pflag = true;
                ++PT[j];
            }
            else
            {
                ++NT[j];
            }
        }
        if (pflag) {
            ++PTotal;
        }
        else {
            ++NTotal;
        }
        {
            bool cflag = false;
            for (type_t j = 0; j < TYPE_COUNT; ++j) {
                bool btemp = p_tl->list[i].types[j] == 1;
                if (test[j] == 1) {
                    cflag = true;
                    if (btemp) {
                        ++TP[j];
                    }
                    else {
                        ++FP[j];
                    }
                }
                else
                {
                    if (btemp) {
                        ++FN[j];
                    }
                    else
                    {
                        ++TN[j];
                    }
                }
            }
            if (pflag && cflag) {
                ++truePositive;
            }
            if (!pflag && !cflag) {
                ++trueNegative;
            }
            if (!pflag && cflag) {
                ++falsePositive;
            }
            if (pflag && !cflag) {
                ++falseNegative;
            }
        }
        clear_hash_vector(&temp);
    }
    OUTPUT_TEST
    return 0;
}

int NB_classifier(FILE * out, struct text_list * p_tl, struct hash_vector * const statistic[TYPE_COUNT + 1], Parser parser, Checker checker)
{
    for (llu i = 0; i < p_tl->len; ++i) {
        struct hash_vector * temp;
        struct ustring_parse_list * p_list;
        Lf cos[TYPE_COUNT] = { 0 };
        Lf possibility[TYPE_COUNT] = { 0 };

        init_hash_vector(&temp);
        init_uspl(&p_list);
        parser(p_list, p_tl->list[i].us, checker);
        append_hash_vector(temp, p_tl->list[i].us, p_list);

        for (type_t j = 0; j < TYPE_COUNT; ++j) {
            cos[j] = cos_hash_vector(statistic[j], temp);
            possibility[j] = get_possibility(temp, statistic[j]) / (Lf)TYPE_COUNT;
        }

        int8_t a_class[TYPE_COUNT] = { 0 };
        {
            for (type_t j = 0; j < TYPE_COUNT; ++j) {
                if (cos[j] > NB_CUTDOWN || possibility[j] > NB_THRESHOLD) {
                    a_class[j] = 1;
                }
            }
        }

        for (type_t j = 0; j < TYPE_COUNT; ++j) {
            fprintf(out, "%d", a_class[j]);
        }
        fprintf(out, "\n");
        clear_hash_vector(&temp);
    }
    return 0;
}

#undef OUTPUT_TEST