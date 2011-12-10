#ifndef ELEVATORUSER_H
#define ELEVATORUSER_H

#include "trace.h"

class ElevatorUser
{
public:
    ElevatorUser();
    inline bool is_present();
    void set_presence(bool is_present);
    inline bool is_coming();
    inline bool is_going();
    inline bool is_coming_from_elevator();
    inline bool is_going_to_elevator();
    inline BlobFeature* get_first_feature();
    inline BlobFeature* get_last_feature();
    inline Trace* get_trace();
    inline void set_trace(Trace* tr);
    inline Pair get_velocity();
    bool is_approaching(Pair location);
    bool is_approaching(Pair location, int minimum_speed);
    bool is_leaving(Pair location);
    bool is_leaving(Pair location, int minimum_speed);
    inline void set_behaviour(int behaviour);
    inline int get_behaviour();
    inline void set_origin(int origin);
    inline int get_origin();
    inline void set_destination(int destination);
    inline int get_destination();
    inline unsigned get_frame_count();          //the number of frames the user has been present
    inline unsigned get_first_frame();
    inline unsigned get_last_frame();

    char transient_count;
    int steady_state_count;

private:
    //char transient_count;
    //int steady_state_count;
    bool myPresence;
    bool myComing;
    bool myGoing;
    int myBehaviour;
    int myOrigin;
    int myDestination;
    Trace* myTrace;
};

inline bool ElevatorUser::is_present() {return myPresence;}
inline bool ElevatorUser::is_coming() {return myComing;}
inline bool ElevatorUser::is_going() {return myGoing;}
inline Trace* ElevatorUser::get_trace() {return myTrace;}
inline BlobFeature* ElevatorUser::get_first_feature(){return myTrace->get_first_feature();}
inline void ElevatorUser::set_trace(Trace* tr) {myTrace = tr;}
inline BlobFeature* ElevatorUser::get_last_feature() {return myTrace->get_last_feature();}
inline Pair ElevatorUser::get_velocity() {return myTrace->get_velocity();}
inline void ElevatorUser::set_origin(int origin) {myOrigin = origin;}
inline int ElevatorUser::get_origin() {return myOrigin;}
inline void ElevatorUser::set_destination(int destination){myDestination = destination;}
inline int ElevatorUser::get_destination() {return myDestination;}
inline unsigned ElevatorUser::get_frame_count() {return myTrace->get_length();}
inline void ElevatorUser::set_behaviour(int behaviour) {myBehaviour = behaviour;}
inline int ElevatorUser::get_behaviour(){return myBehaviour;}
inline unsigned ElevatorUser::get_first_frame() {return myTrace->get_initial_frame();}
inline unsigned ElevatorUser::get_last_frame(){return myTrace->get_last_frame();}



#endif // ELEVATORUSER_H
