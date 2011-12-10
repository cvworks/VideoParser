#define TRANSIENT_COUNT_MAX 10

#include "elevator.h"

unsigned int Elevator::id_generator = 1;

bool is_within_bounding_box(Pair p, Pair ul, Pair br);

Elevator::Elevator():id(id_generator), myOpen(false), transient_count(0), steady_state_count(0), myFeature(NULL)
{
    id_generator++;
    myFeature = new BlobFeature;
}

Elevator::~Elevator()
{
    delete myFeature;
}

void Elevator::set_open_state(BlobFeature* open_feature)
{
    bool is_open_new = (open_feature != NULL);      //if the open_feature is NULL, the elevator is closed

    if (is_open_new) memcpy(myFeature,open_feature, sizeof(BlobFeature));

    //compare the temporary open/close state with transient-corrected one
    if (myOpen == is_open_new)
    {
        transient_count = 0;
        if (steady_state_count < UINT_MAX) steady_state_count++;
    }
    else
    {
        if (transient_count < TRANSIENT_COUNT_MAX)
        {
            transient_count++;
        }

    }

    //change the open/close state if it is not transient
    if (transient_count >= TRANSIENT_COUNT_MAX)
    {
        //myFeature = open_feature;
        myOpen = is_open_new;
        steady_state_count = 0;
        transient_count = 0;
        is_opening = myOpen;
        is_closing = !myOpen;
    }
    else
    {
        is_opening = false;
        is_closing = false;
    }
}

int Elevator::get_distance_to_bounding_box(BlobFeature* f)
{
    //Return the shortest distance to the elevator's bounding box and f's
    BlobFeature* g = myFeature;
    Pair del;

    //Determine the distance along x
    int x1, x2;
    if(f->br.x < g->ul.x)
    {
        x1=f->br.x;
        x2=g->ul.x;
    }
    else if(f->ul.x > g->br.x)
    {
        x1 = g->br.x;
        x2 = f->ul.x;
    }
    else
    {
        x1 = x2 = (g->br.x < f->br.x) ? g->br.x : f->br.x;
    }
    del.x = x2-x1;

    //Determine the distance along x
    int y1, y2;
    if(f->br.y < g->ul.y)
    {
        y1=f->br.y;
        y2=g->ul.y;
    }
    else if(f->ul.y > g->br.y)
    {
        y1 = g->br.y;
        y2 = f->ul.y;
    }
    else
    {
        y1 = y2 = (g->br.y < f->br.y) ? g->br.y : f->br.y;
    }
    del.y = y2-y1;

    return (int)del.Norm();
}


//initializes values for the first frame
void Elevator::prepare_for_tracking()
{
    if (!segmentor) segmentor = new FSMBMBackgroundSegmentor(image->width, image->height);

    if (!luminance)
    {
        luminance = new ElementLuminance;
    }
}

