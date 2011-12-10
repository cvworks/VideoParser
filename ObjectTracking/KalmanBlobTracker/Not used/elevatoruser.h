#ifndef ELEVATORUSER2_H
#define ELEVATORUSER2_H

#include "trace.h"

class TempUser
{
public:
    TempUser();
    inline bool is_present();
    void set_presence(bool is_present);
    inline bool is_coming();
    inline bool is_going();
    inline bool is_coming_from_elevator();
    inline bool is_going_to_elevator();
    inline void set_possible_exit_id(unsigned elevator_id);
    inline unsigned get_possible_exit_id();
    inline BlobFeature* get_first_feature();
    inline BlobFeature* get_last_feature();
    inline Trace* get_trace();
    inline void set_trace(Trace* tr);

private:
    char transient_count;
    int steady_state_count;
    bool myPresence;
    bool myComing;
    bool myGoing;
    bool myComing_from_elevator;
    bool myGoing_to_elevator;
    char myPossible_exit_id;
    Trace* myTrace;
};

inline bool TempUser::is_present() {return myPresence;}
inline bool TempUser::is_coming() {return myComing;}
inline bool TempUser::is_going() {return myGoing;}
inline bool TempUser::is_coming_from_elevator(){return myComing_from_elevator;}
inline bool TempUser::is_going_to_elevator(){return myGoing_to_elevator;}
inline void TempUser::set_possible_exit_id(unsigned exit_id) {myPossible_exit_id = exit_id;}
inline unsigned TempUser::get_possible_exit_id() {return myPossible_exit_id;}
inline Trace* TempUser::get_trace() {return myTrace;}
inline BlobFeature* TempUser::get_first_feature(){return myTrace->get_first_feature();}
inline void TempUser::set_trace(Trace* tr) {myTrace = tr;}
inline BlobFeature* TempUser::get_last_feature() {return myTrace->get_last_feature();}



#endif // ELEVATORUSER2_H
