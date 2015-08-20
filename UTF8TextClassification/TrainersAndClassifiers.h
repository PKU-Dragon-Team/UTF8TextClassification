#ifndef TRAINER_CLASSIFIER
#define TRAINER_CLASSIFIER

#include "UStringVector.h"
#include "TextUtilities.h"

extern const Lf NB_CUTDOWN;
extern const Lf NB_THRESHOLD;
extern const Lf NB_CONVERGE;

/* Trainer which simply add the vector when the class meets and sub the vector when not
   朴素训练器 */
int naive_trainer(struct hash_vector * ap_hv[TYPE_COUNT + 1], const struct text_list * p_tl, Parser parser, Checker checker);

/* Tester using K-Nearest Neighbors algorithm
   使用KNN算法的分类测试器 */
int KNN_tester(FILE * out, struct text_list * p_tl, struct hash_vector * const statistic[TYPE_COUNT + 1], Parser parser, Checker checker);

/* Classifier using K-Nearest Neighbors algorithm
   使用KNN算法的分类器 */
int KNN_classifier(FILE * out, struct text_list * p_tl, struct hash_vector * const statistic[TYPE_COUNT + 1], Parser parser, Checker checker);

/* Tester using Naive Bayes algorithm
   使用朴素贝叶斯算法的分类测试器 */
int NB_tester(FILE * out, struct text_list * p_tl, struct hash_vector * const statistic[TYPE_COUNT + 1], Parser parser, Checker checker);

/* Classifier using Naive Bayes algorithm
   使用朴素贝叶斯算法的分类器 */
int NB_classifier(FILE * out, struct text_list * p_tl, struct hash_vector * const statistic[TYPE_COUNT + 1], Parser parser, Checker checker);
#endif