#include "common_utility.h"
int trace_flag;
int target_flag;


void trace_flag_add(){trace_flag++;}
void target_flag_add(){target_flag++;}
void trace_flag_zero(){trace_flag = 0;}
void target_flag_zero(){target_flag = 0;}
int trace_flag_get(){return trace_flag;}
int target_flag_get(){return target_flag;}

void append_int(QString* text, int num)
{
    QString return_string;
    char array[10] = {0};
    sprintf(array, "%d", num);
    text->append(array);
}

void add_text(IplImage* image, const char* text, Pair* point, CvFont* font, CvScalar color, int placement)
{
    CvPoint text_placement;

    //Get text placement
    switch (placement)
    {
    case PLCMNT_CENTER :
    // Center the text
        text_placement = center_placement(point, text, font);
        break;

    default :
        // No special placement.
        text_placement = cvPoint(point->x,point->y);
    }

    cvPutText (image, text, text_placement, font, color);
}

CvPoint center_placement(Pair* point, const char* text, CvFont* font)
{
    CvPoint text_placement;

    //Get the text size
    CvSize textSize;
    int ymin;
    cvGetTextSize(text ,font, &textSize, &ymin);

    text_placement.x = (double)point->x - (double)0.5*(int)textSize.width;
    text_placement.y = (double)point->y + (double)0.5*(int)textSize.height;

    return text_placement;
}

template <class T>
QList<T> list_intersection(QList<QList<T>*>* lists)
{
    //Create a list consisting of the elements each list has in common

    //Begin with the first list
    QList<T> common_list = *(lists->first());
    QList<T>* temp_list;

    //eleminate from the common list the elements that are not in each additional list
    //for (QList<T>::iterator list_iter = lists->begin(); list_iter != lists->end(); list_iter++)
    for (int i=0; i<lists->count(); i++)
    {
        temp_list = lists[i];
        //For each element in the common list
        //for (QList<T>::iterator T_iter = common_list.begin(); T_iter != common_list.end(); T_iter++)
        for (int j=0; j<common_list.count(); j++)
        {
            //if the element does not appear in the other list, remove it from the common list
            if (!temp_list->contains(common_list[j]))
            {
                common_list.removeAt(j);
                j--;
            }
        }
    }

    return common_list;
}
