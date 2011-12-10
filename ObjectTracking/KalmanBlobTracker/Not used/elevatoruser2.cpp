#include "elevatoruser2.h"

ElevatorUser::ElevatorUser():myTrace(NULL), transient_count(0), steady_state_count(0), myPresence(false), myComing(false), myGoing(false),
        myBehaviour(0), myOrigin(0), myDestination(0)
{
}

void ElevatorUser::set_presence(bool is_present)
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

bool ElevatorUser::is_approaching(Pair location)
{
    return is_approaching(location, 0);
}

bool ElevatorUser::is_approaching(Pair location, int minimum_speed)
{
    Pair A = get_velocity();
    Pair B = location - get_last_feature()->centroid;
    float dp = (float)prDotProduct(A,B) / (float)prDotProduct(B,B);
    Pair C = B*dp;

    int speed = (float)C.Norm();
    int temp = ((float)prDotProduct(A,B));

    return (speed > minimum_speed && temp > 0);
}

bool ElevatorUser::is_leaving(Pair location)
{
    return is_leaving(location, 0);
}

bool ElevatorUser::is_leaving(Pair location, int minimum_speed)
{
    Pair A = get_velocity();
    Pair B = location - get_last_feature()->centroid;

    int speed = (int)A.Norm();
    int temp = prDotProduct(A,B);

    cout << "L direction = " << temp << endl;
    cout << "L speed = " << speed << endl;

    return (speed > minimum_speed && temp < 0);
}
