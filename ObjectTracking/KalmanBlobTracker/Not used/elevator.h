#ifndef ELEVATOR_H
#define ELEVATOR_H

#include <QList>
#include "target.h"
#include "pair.h"
#include "FSMBMBackgroundSegmentor.h"

class Elevator
{
public:
    Elevator();
    ~Elevator();
    inline void set_top(int top);
    inline int get_top();
    inline void set_center(Pair center);
    inline Pair get_center();
    inline unsigned get_steady_state_count();
    inline bool is_open();
    void set_open_state(BlobFeature* open_feature);
    inline unsigned get_id();
    inline bool get_is_opening();
    inline bool get_is_closing();
    int get_distance_to_bounding_box(BlobFeature* f);
    inline BlobFeature* get_feature();

    static unsigned int id_generator;

private:
    void prepare_for_tracking();


    unsigned id;
    bool myOpen;
    int myTop;
    Pair myCenter;
    char transient_count;
    unsigned steady_state_count;
    bool is_opening;
    bool is_closing;
    BlobFeature* myFeature;     //The feature that indicates the elevator is open
    FSMBMBackgroundSegmentor* segmentor;
    IplImage* image;
    ElementLuminance* luminance;


};

inline void Elevator::set_top(int top) {myTop = top;}
inline int Elevator::get_top() {return myTop;}
inline void Elevator::set_center(Pair center) {myCenter = center;}
inline Pair Elevator::get_center() {return myCenter;}
inline unsigned Elevator::get_id() {return id;}
inline bool Elevator::is_open(){return (myOpen);}
inline unsigned Elevator::get_steady_state_count() {return steady_state_count;}
inline bool Elevator::get_is_opening() {return is_opening;}
inline bool Elevator::get_is_closing() {return is_closing;}
inline BlobFeature* Elevator::get_feature() {return myFeature;}

#endif // ELEVATOR_H
