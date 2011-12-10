#ifndef ELEVATORDETECTOR_H
#define ELEVATORDETECTOR_H

#include <QList.h>
#include "elevator.h"
#include "eventtracker.h"
#include "trace.h"
#include "target.h"
#include "main.h"
#include <QString>
#include "elevatoruser2.h"
#include "pair.h"

typedef struct {
   unsigned id;
   unsigned frame;
   ElevatorUser* user;
} ElevatorEvent;


class ElevatorDetector
{
public:
    ElevatorDetector(EventTracker* tracker, QList<ElevatorAttributes*> elevators, IplImage* image);
    void set_elevators(QList<ElevatorAttribute*> elevators);
    void set_image_size(IplImage* image);
    inline QList<Elevator*> get_elevators();
    inline int get_elevator_count();
    void next_frame(IplImage* image);
    void show_states(IplImage* image);
    inline bool get_first_door_is_opening();
    inline bool get_last_door_is_closing();
    inline QList<ElevatorEvent*> get_events();
    void set_selected_event(int index);
    inline ElevatorEvent* get_selected_event();
    QString get_event_description(int index);



private:
    void get_text_placement(Pair* text_placement, Pair* point, const char* text);
    void add_text(const char* text, Pair* point, IplImage* image);
    bool a_door_is_open();
    unsigned minimum_steady_state_count();
    bool door_has_recently_closed();
    bool door_closed_x_frames_ago(unsigned x);
    void detect_after_close(QList<ElevatorUser*> targets);
    void set_elevator_states();
    void set_open_state(bool new_open_state);
    CvMat* get_matches();
    void resolve_conflicts(CvMat* M);
    void resolve_conflict(CvMat* M, int column);
    int minimum_cost_value_index(CvMat* M, int column);
    void get_relevant_traces();
    QList<BlobFeature*> get_relevant_features();
    bool feature_matches_elevator(BlobFeature* f, Elevator* el);
    bool detect_elevator_exit();
    bool detect_elevator_entry();
    bool user_is_entering_elevator(Elevator* el, ElevatorUser* el_user);
    void detect_elevator_users();
    QList<Trace*> get_elevator_users();
    bool user_is_exiting_elevator(Elevator* el, ElevatorUser* el_user);
    void reset_states();
    bool detect_scene_entry();
    bool feature_is_near_edge_of_image(BlobFeature* f);
    bool user_is_approaching_elevator(Elevator* el, ElevatorUser* el_user);
    bool user_is_entering_scene(ElevatorUser* el_user);
    bool ElevatorDetector::detect_scene_exit();
    bool user_is_exiting_scene(ElevatorUser* el_user);
    inline Pair image_center();
    void get_present_users();
    bool feature_is_near_bottom_edge(BlobFeature* f);
    bool feature_is_near_top_edge(BlobFeature* f);
    bool feature_is_near_left_edge(BlobFeature* f);
    bool feature_is_near_right_edge(BlobFeature* f);
    Pair get_edge_point(BlobFeature* f);
    bool detect_suspicious_users();
    bool detect_failed_entry();
    bool detect_loitering();
    void add_event(int event_index, ElevatorUser* el_user);
    void set_background_model();

    void output_user_summary();

    int event_display_frame_count;
    int steady_state_count;         //the number of frames the doors have been open or closed.
    QString event_text;
    QList<Elevator*> myElevators;
    CvFont font;
    bool open_state;
    int event_state;
    bool first_door_is_opening;
    bool last_door_is_closing;
    bool detect;
    QList<ElevatorUser*> users_on_open;      //the id's of the active targets present when a door is opened.
    QList<ElevatorUser*> users_on_close;     //the id's of the active targets present when a door is closed.
    int n_users;      //the number of targets waiting at the elevators
    int n_before;       //the number of targets waiting at the elevators just before a door opens
    int n_after;        //the number of targets waiting at the elevators just after a door opens
    QList<BlobFeature*> myFeatures;
    QList<Trace*> myTraces;
    QList<ElevatorUser*> myUsers;
    QList<ElevatorUser*> present_users;
    bool elevator_exit;
    bool elevator_entry;
    bool scene_exit;
    bool scene_entry;
    bool failed_entry;
    bool loitering;
    bool suspicious_users;
    Pair myImageSize;
    QList<ElevatorEvent*> myEvents;
    EventTracker* myTracker;
    ElevatorEvent* mySelectedEvent;
    IplImage* myImage;



};

inline QList<Elevator*> ElevatorDetector::get_elevators() {return myElevators;}
inline int ElevatorDetector::get_elevator_count() {return myElevators.count();}
inline bool ElevatorDetector::get_first_door_is_opening(){return first_door_is_opening;}
inline bool ElevatorDetector::get_last_door_is_closing(){return last_door_is_closing;}
inline QList<ElevatorEvent*> ElevatorDetector::get_events() {return myEvents;}
inline ElevatorEvent* ElevatorDetector::get_selected_event() {return mySelectedEvent;}

float cost_value(Elevator* elevator, BlobFeature* f);
inline Pair ElevatorDetector::image_center() {return myImageSize*0.5;}

#endif // ELEVATORDETECTOR_H
