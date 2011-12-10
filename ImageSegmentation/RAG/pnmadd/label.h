/*
 * pnmadd/label.h
 *
 * Sven Wachsmuth, 7.10.2004
 */

#ifndef PNM_LABEL_H
#define PNM_LABEL_H

#ifdef __cplusplus
extern "C" {
#endif

int *pnm_label_createLookup(int *labelMax, int n_labels, const int *labels);
int pnm_label_lookup(int labelMax, const int *labelFlags, int label);

#ifdef __cplusplus
}
#endif

#endif
