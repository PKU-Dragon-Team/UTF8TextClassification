#ifndef TRAINER_CLASSIFIER
#define TRAINER_CLASSIFIER

#include "UStringVector.h"
#include "TextUtilities.h"

extern const Lf NB_CUTDOWN;
extern const Lf NB_THRESHOLD;
extern const Lf NB_CONVERGE;

/* Trainer which simply add the vector when the class meets and sub the vector when not
   ����ѵ���� */
int naive_trainer(struct hash_vector * ap_hv[TYPE_COUNT + 1], const struct text_list * p_tl, Parser parser, Checker checker);

/* Tester using K-Nearest Neighbors algorithm
   ʹ��KNN�㷨�ķ�������� */
int KNN_tester(FILE * out, struct text_list * p_tl, struct hash_vector * const statistic[TYPE_COUNT + 1], Parser parser, Checker checker);

/* Classifier using K-Nearest Neighbors algorithm
   ʹ��KNN�㷨�ķ����� */
int KNN_classifier(FILE * out, struct text_list * p_tl, struct hash_vector * const statistic[TYPE_COUNT + 1], Parser parser, Checker checker);

/* Tester using Naive Bayes algorithm
   ʹ�����ر�Ҷ˹�㷨�ķ�������� */
int NB_tester(FILE * out, struct text_list * p_tl, struct hash_vector * const statistic[TYPE_COUNT + 1], Parser parser, Checker checker);

/* Classifier using Naive Bayes algorithm
   ʹ�����ر�Ҷ˹�㷨�ķ����� */
int NB_classifier(FILE * out, struct text_list * p_tl, struct hash_vector * const statistic[TYPE_COUNT + 1], Parser parser, Checker checker);
#endif