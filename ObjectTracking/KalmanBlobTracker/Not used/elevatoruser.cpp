#include "elevatoruser2.h"

TempUser::TempUser():myTrace(NULL), transient_count(0), steady_state_count(0), myPresence(false), myPossible_exit_id(0)
{
}

void TempUser::set_presence(bool is_present)
{
    if (myPresence == is_present)
    {
        transient_count = 0;
        if (steady_state_count < UINT_MAX) steady_state_count++;
    }
    else
    {
        //if (transient_count < TRANSIENT_COUNT_MAX) transient_count++;
        if (transient_count < 10) transient_count++;
    }

    //change the state if it is not transient
    //if (transient_count >= TRANSIENT_COUNT_MAX)
    if (transient_count >= 10)
    {
        myPresence = is_present;
        steady_state_count = 0;
        transient_count = 0;
        myComing = myPresence;
        myGoing = !myPresence;
    }
    else
    {
        myComing = false;
        myGoing = false;
    }
}
