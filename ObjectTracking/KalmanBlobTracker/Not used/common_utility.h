#ifndef COMMON_UTILITY_H
#define COMMON_UTILITY_H

#include "cv.h"
#include "highgui.h"
#include "pair.h"
#include <QString>
#include <QList>

#define PLCMNT_NONE 0
#define PLCMNT_CENTER 1

void append_int(QString* text, int num);
void add_text(IplImage* image, const char* text, Pair* point, CvFont* font, CvScalar color, int placement);
CvPoint center_placement(Pair* point, const char* text, CvFont* font);

template <class T>
QList<T> list_intersection(QList<QList<T>*>* lists);

void trace_flag_add();
void target_flag_add();
void trace_flag_zero();
void target_flag_zero();
int trace_flag_get();
int target_flag_get();


#endif // COMMON_UTILITY_H
