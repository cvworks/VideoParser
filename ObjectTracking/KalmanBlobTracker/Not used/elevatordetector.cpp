#define DOOR_CLOSE_RECENCY 200
#define EVENT_NULL 0
#define EVENT_ELEVATOR_ENTRY 1      // a user enters the image via an elevator  (comes out of an elevator)
#define EVENT_ELEVATOR_EXIT 2       // a user exits the image via an elevator  (goes into an elevator)
#define EVENT_SCENE_ENTRY 3         // a user enters the image from somewhere outside the camera view
#define EVENT_SCENE_EXIT 4          // a user exits the image to somewhere outside the camera view
#define EVENT_NO_ENTRY 5            // a user has failed to enter an open elevator
#define EVENT_LOITER 6              // a user is loitering by the elevators
#define EDGE_PROXIMITY_THRESHOLD 20
#define LOCATION_UNCERTAIN 0
#define LOCATION_SCENE 1
#define LOCATION_ELEVATOR 2
#define BEHAVIOUR_NORMAL 0
#define BEHAVIOUR_NO_ENTRY 1
#define BEHAVIOUR_LOITER 2
#define WAITING_THRESHOLD 1000

#include "elevatordetector.h"

ElevatorDetector::ElevatorDetector(EventTracker* tracker, QList<ElevatorAttributes*> elevators, IplImage* image)
    : n_users(0), n_before(0), n_after(0), open_state(false), first_door_is_opening(false), last_door_is_closing(false),
    detect(false),event_display_frame_count(1000),failed_entry(false), suspicious_users(false), steady_state_count(0),
    mySelectedEvent(NULL), myImage(NULL)
{
    cvInitFont(&font,CV_FONT_HERSHEY_COMPLEX, 0.8,0.8,0,1);
    myTracker = tracker;
    set_elevators(elevators);
    set_image_size(image);
}

void ElevatorDetector::set_elevators(QList<ElevatorAttributes*> elevators)
{
    //Remove all existing elevators
    for (QList<Elevator*>::iterator iter = myElevators.begin(); iter != myElevators.end(); iter++)
    {
        delete *iter;
    }
    myElevators.clear();

    //Add the new elevators
    ElevatorAttribute* el;
    for (QList<ElevatorAttribute*>::iterator iter = elevators.begin(); iter != elevators.end(); iter++)
    {
        Elevator* new_elevator = new Elevator;
        el = *iter;
        new_elevator->set_top(el->top);
        new_elevator->set_center(el->center);
        myElevators.push_back(new_elevator);
    }
}

void ElevatorDetector::set_image_size(IplImage* image)
{
    myImageSize.x = image->width;
    myImageSize.y = image->height;
}

void ElevatorDetector::next_frame(IplImage* image)
{
    myImage = image;

    reset_states();
    //Determine the open/close state of each elevator
    set_elevator_states();
    //Detect elevator users, 2nd version (people around the elevators)
    detect_elevator_users();

    //For debugging:
   // output_user_summary();

    //Get the list of users present in this frame
    get_present_users();

    //Determine if a door is opening
    set_open_state(a_door_is_open());

    elevator_exit = detect_elevator_exit();
    elevator_entry = detect_elevator_entry();
    scene_entry = detect_scene_entry();
    scene_exit = detect_scene_exit();

    suspicious_users = detect_suspicious_users();

    set_background_model();

     if(elevator_exit)
    {
        cout << "Elevator Exit" << endl;
        char c;
        cin >> c;
    }

     if(elevator_entry)
    {
        cout << "Elevator Entry" << endl;
        char c;
        cin >> c;
    }

    if(scene_entry)
    {
        cout << "Scene Entry" << endl;
        char c;
        cin >> c;
    }

    if(scene_exit)
    {
        cout << "Scene Exit" << endl;
        char c;
        cin >> c;
    }

}

void ElevatorDetector::set_background_model()
{
    for (QList<Elevator*>::iterator iter = myElevators.begin(); iter != myElevators.end(); iter++)
    {
        Elevator* el = *iter;
        if (el->is_open())
        {

        }
    }
}

void ElevatorDetector::reset_states()
{
    event_state = 0;
    first_door_is_opening = false;
    last_door_is_closing = false;
    elevator_exit = false;
    elevator_entry = false;
    scene_exit = false;
    scene_entry = false;
}

void ElevatorDetector::set_open_state(bool new_open_state)
{
    if (!open_state && new_open_state)        //a door is opening
    {
        open_state = true;
        steady_state_count = 0;
        first_door_is_opening = true;
        //Determine the number of targets waiting prior to a door opening
        n_before = n_users;

        //Get the waiting targets upon door opening
        users_on_open = present_users;
    }

    //Determine if a door is closing
    else if (open_state && !new_open_state)       //a door is closing
    {
        open_state = false;
        steady_state_count = 0;
        last_door_is_closing = true;
         //Determine the number of targets waiting after a door closes
        n_after = n_users;

        //Get the waiting targets upon door closing
        users_on_close = present_users;
    }
    steady_state_count++;
}

void ElevatorDetector::detect_elevator_users()
{
   // cout << "CP Start" << endl;
    myTraces = get_elevator_users();
   // cout << "Traces detected = " << myTraces.count() << endl;
    ElevatorUser* eu;

    //For each elevator user
    int k = 0;
    for (QList<ElevatorUser*>::iterator u_iter = myUsers.begin(); u_iter != myUsers.end(); u_iter++)
    {
        eu = (*u_iter);
        //Check if this user is currently being detected
        bool is_present =  myTraces.contains(eu->get_trace());
     //   cout << "Is Present = " << is_present << endl;
        eu->set_presence(is_present);
     //   cout << "CP" << endl;
        if (is_present) k++;
    }


    //For each trace
    k=0;
    for (QList<Trace*>::iterator t_iter = myTraces.begin(); t_iter != myTraces.end(); t_iter++)
    {
        //Determine if the target matches an elevator user
        bool is_matched = false;
        for (QList<ElevatorUser*>::iterator u_iter = myUsers.begin(); u_iter != myUsers.end(); u_iter++)
        {
            if (*t_iter == (*u_iter)->get_trace())
            {
                is_matched = true;
                break;
            }
        }

        if (!is_matched)        //the target is novel, so create a new elevator user
        {
            ElevatorUser* new_eu = new ElevatorUser;
            new_eu->set_trace(*t_iter);
            //eu->set_presence(true);
            myUsers.push_back(new_eu);
            k++;

        }
    }
   // cout << "Number of novel traces = " << k << endl;
   // cout << "Total number of users = " << myUsers.count() << endl;
    //char c;
    //cin >> c;
}

void ElevatorDetector::output_user_summary()
{
    cout << endl << endl;
    cout << "User Count = " << myUsers.count() << endl << endl;
    int k = 0;
    for (QList<ElevatorUser*>::iterator u_iter = myUsers.begin(); u_iter != myUsers.end(); u_iter++)
    {
        ElevatorUser* eu = (*u_iter);
        cout << "User " << k << endl;
        cout << "\tTrace Address  = " << eu->get_trace() << endl;
        cout << "\tTrace ID  = " << eu->get_trace()->get_id() << endl;
        cout << "\tPresence  = " << eu->is_present() << endl;
        cout << "\tTransient Count = " << (int)eu->transient_count << endl;
        cout << "\tSteady State Count = " << eu->steady_state_count << endl;
        k++;
    }
}

void ElevatorDetector::set_elevator_states()
{
    //Consider only relevant blob features
    myFeatures = get_relevant_features();


    //Determine if an elevator is open or closed
    if (myFeatures.count())      //if ther any blob features to check
    {
        CvMat* M = get_matches();
        resolve_conflicts(M);
        //QList<BlobFeature*> copy_of_myFeatures = myFeatures;

        //For each elevator
        for (int i=0; i<M->rows; i++)
        {
            //Check for matches
            //bool is_open = false;
            BlobFeature* open_feature = NULL;     //the feature which indicates the elevator is open
            for (int j=0; j< M->cols; j++)
            {
                if (cvmGet(M,i,j))      //if feature j matches elevator i
                {
                    //is_open = true;
                    open_feature = myFeatures[j];
                    break;
                }
            }

            //If there is at least 1 match, the elevator door is open
            //myElevators[i]->set_open_state(is_open);
            myElevators[i]->set_open_state(open_feature);

        }
        cvReleaseMat(&M);
    }
    else
    {
        //Each elevator must be closed because there are no targets
        for (QList<Elevator*>::iterator iter = myElevators.begin(); iter != myElevators.end(); iter++)
        {
            (*iter)->set_open_state(false);
        }
    }
}

QList<BlobFeature*> ElevatorDetector::get_relevant_features()
{
    QList<BlobFeature*> relevant_features = myTracker->get_features();

    QList<BlobFeature*> temp = relevant_features;

    //Remove features which are too small
    for (QList<BlobFeature*>::iterator iter = temp.begin(); iter != temp.end(); iter++)
    {
        if ((*iter)->height < 100) relevant_features.removeOne(*iter);
    }

    return relevant_features;
}

QList<Trace*> ElevatorDetector::get_elevator_users()
{

    QList<Trace*> temp;
    BlobFeature* f;

    //Begin with all traces
    QList<Trace*> relevant_traces = myTracker->get_active_traces();

    //Remove targets that are too small
    temp = relevant_traces;
    for (QList<Trace*>::iterator iter = temp.begin(); iter != temp.end(); iter++)
    {
        f = (*iter)->get_last_feature();
        if (f->size < 2000) relevant_traces.removeOne(*iter);
    }

    //Remove the trace that corresponds to the elevator
    temp = relevant_traces;
    for (QList<Trace*>::iterator iter = temp.begin(); iter != temp.end(); iter++)
    {
        f = (*iter)->get_last_feature();
        for (QList<Elevator*>::iterator el_iter = myElevators.begin(); el_iter != myElevators.end(); el_iter++)
        {
            BlobFeature* el_f = (*el_iter)->get_feature();
            if (blob_features_are_equal(f,el_f)) relevant_traces.removeOne(*iter);

        }

    }

    return relevant_traces;
}

void ElevatorDetector::get_relevant_traces()
{
    //Get the traces that are active, mobile, long enough

    //Active traces
    QList<Trace*> myTraces = myTracker->get_active_traces();

    cout << "myTraces.count = " << myTraces.count() << endl;

    for (QList<Trace*>::iterator it = myTraces.begin(); it != myTraces.end(); it++)
    {
        cout << "Address = " << *it << endl;
    }

    //Remove the traces that are immobile
    for (QList<Trace*>::iterator it = myTraces.begin(); it != myTraces.end(); it++)
    {

        if (!(*it)->is_mobile())
        {
            cout << "Remove" << *it << endl;
          //  myTraces.removeOne(*it);
        }
        else cout << "Accept" << *it << endl;
    }

    //Remove the traces that are too short
    for (QList<Trace*>::iterator it = myTraces.begin(); it != myTraces.end(); it++)
    {
        //BlobFeature* f = (*it)->get_last_feature();
       // if (f->height < 100) myTraces.removeOne(*it);
    }
}

void ElevatorDetector::resolve_conflicts(CvMat* M)
{
    //For each target
    for (int j=0; j<M->cols; j++)
    {
        //Determine if there is a conflict, i.e. if the target matches to more than one elevator
        int sum = 0;
        for (int i=0; i<M->rows; i++)
        {
            sum += cvmGet(M,i,j);
        }
        if (sum > 1)    //there is a conflict
        {
            resolve_conflict(M,j);
        }
    }
}

void ElevatorDetector::resolve_conflict(CvMat* M, int column)
{
    //Get index of the elevator the most closely matches the blob feature
    int min_row_index = minimum_cost_value_index(M, column);

    //Set all match values to zero that do not correspond to the closest match
    for (int row = 0; row < M->rows; row++)
    {
        if (row != min_row_index) cvmSet(M,row,column,0.0);
    }
}

int ElevatorDetector::minimum_cost_value_index(CvMat* M, int column)
{
    //Get the minimum distance between this blob feature and each elevator and return the corresponding elevator index
    int min_index = -1;
    float min_value = -1;

    //Set min_value to the first valid cost value
    int i=0;
    while ((min_value < 0) && (i < M->rows))
    {
        if (cvmGet(M,i,column))
        {
            min_value = cost_value(myElevators[i], myFeatures[column]);
        }
        i++;
    }

    //Compare this value to all other cost values
    for (int row = i ; row < M->rows; row++)
    {
        if (cvmGet(M,row,column))
        {
            float temp = cost_value(myElevators[row], myFeatures[column]);
            if (min_value > temp)
            {
                min_value = temp;
                min_index = row;
            }
        }
    }

    return min_index;
}

CvMat* ElevatorDetector::get_matches()
{
    CvMat* M = cvCreateMat(myElevators.size(), myFeatures.size(), CV_32FC1);

    int i = 0;
    for (QList<Elevator*>::iterator el_iter = myElevators.begin(); el_iter != myElevators.end(); el_iter++)
    {
        int j = 0;
        for (QList<BlobFeature*>::iterator f_iter = myFeatures.begin(); f_iter != myFeatures.end(); f_iter++)
        {
            if (feature_matches_elevator(*f_iter,*el_iter))
            {
                cvmSet(M, i, j, 1);
            }
            else cvmSet(M, i, j, 0);
            j++;
        }
        i++;
    }
    return M;
}

bool ElevatorDetector::feature_matches_elevator(BlobFeature* f, Elevator* el)
{
    return ( abs(f->ul.y - el->get_top()) < 20 );
}

void ElevatorDetector::detect_after_close(QList<ElevatorUser*> targets)
{
    QList<ElevatorUser*> matching_targets;
    //QList<Target*> copy_of_users_on_close = users_on_close;
    if (detect && door_closed_x_frames_ago(300))
    {
        detect = false;
        for (QList<ElevatorUser*>::iterator it = targets.begin(); it != targets.end(); it++)
        {
            if (users_on_close.contains(*it))
            {
                matching_targets.push_back(*it);
                //copy_of_users_on_close.removeOne(*it);
            }
        }
        if (matching_targets.count() > 0) event_state = EVENT_NO_ENTRY;
        else event_state = EVENT_NULL;
    }
    else event_state = EVENT_NULL;
}

bool ElevatorDetector::a_door_is_open()
{
    for (QList<Elevator*>::iterator iter = myElevators.begin(); iter != myElevators.end(); iter++)
    {
        if ((*iter)->is_open()) return true;
    }
    return false;
}

void ElevatorDetector::show_states(IplImage* image)
{
    QString text;
    Pair point;

    //Draw the current elevator traces
    for (QList<ElevatorUser*>::iterator u_iter = myUsers.begin(); u_iter != myUsers.end(); u_iter++)
    {
        if ((*u_iter)->is_present())
        {
            (*u_iter)->get_trace()->draw(image);
        }
    }

    //Display the open/close state for each elevator
    for (QList<Elevator*>::iterator iter = myElevators.begin(); iter != myElevators.end(); iter++)
    {
        if ((*iter)->is_open()) text = "Open";
        else text = "Closed";
        point = (*iter)->get_center();
        add_text(text.toAscii().constData(), &point, image);

        //cvRectangle(image, cvPoint((*iter)->get_ul().x,(*iter)->get_ul().y), cvPoint((*iter)->get_br().x,(*iter)->get_br().y), cvScalar(255,0,0), 1);
    }

    //Display the number of targets waiting in front of the elevators
    point.x = 120;
    point.y = 50;
    text = "Users = ";
    append_int(&text,n_users);
    add_text(text.toAscii().constData(), &point, image);

/*
    //Display the event state of the detector (people entering, exiting, or not-entering)
    switch (event_state)
    {
    case EVENT_ENTER :
    // people have entered an elevator
        event_text = "ENTRY";
        cout << "ENTRY" << endl;
        break;

    case EVENT_EXIT :
    // people have exited an elevator
         event_text = "EXIT";
        break;

    case EVENT_NO_ENTRY :
    // a person has not entered the elevator when it opened
        event_text = "NO ENTRY EVENT DETECTED";
        break;
    }
    if (event_state)       // An event has occured
    {
        cout << "RESET DISPLAY COUNT" << endl;
        event_display_frame_count = 0;
    }
    point.x = 500;
    point.y = 50;
    if (event_display_frame_count < 200)
    {
        add_text(event_text.toAscii().constData(), &point, image);
        event_display_frame_count++;
    }
*/

}



bool ElevatorDetector::door_closed_x_frames_ago(unsigned x)
{
    if (!open_state)
    {
        return (minimum_steady_state_count() == x);
    }
    return false;
}

bool ElevatorDetector::door_has_recently_closed()
{
    if (!open_state)
    {
        return (minimum_steady_state_count() < DOOR_CLOSE_RECENCY);
    }
    return false;
}

unsigned ElevatorDetector::minimum_steady_state_count()
{
    unsigned min_count = myElevators.first()->get_steady_state_count() ;
    for (QList<Elevator*>::iterator it = myElevators.begin()+1; it != myElevators.end(); it++)
    {
        int temp = (*it)->get_steady_state_count();
        if (min_count > temp) min_count = temp;
    }
    return min_count;
}

void ElevatorDetector::add_text(const char* text, Pair* point, IplImage* image)
{
    Pair text_placement;
    get_text_placement(&text_placement, point, text);
    cvPutText (image, text, cvPoint(text_placement.x,text_placement.y), &font,cvScalar(0,0,255));
}


void ElevatorDetector::get_text_placement(Pair* text_placement, Pair* point, const char* text)
{
    //Get the text size
    CvSize textSize;
    int ymin;
    cvGetTextSize(text ,&font, &textSize, &ymin);

    text_placement->x = point->x - 0.5*(int)textSize.width;
    text_placement->y = point->y;
}

bool ElevatorDetector::detect_elevator_entry()
{
    bool el_entry = false;

    //For each elevator
    for (QList<Elevator*>::iterator el_iter = myElevators.begin(); el_iter != myElevators.end(); el_iter++)
    {
        //For each elevator user
        for (QList<ElevatorUser*>::iterator u_iter = myUsers.begin(); u_iter != myUsers.end(); u_iter++)
        {
            if (user_is_entering_elevator(*el_iter, *u_iter))
            {
                el_entry = true;
                add_event(EVENT_ELEVATOR_ENTRY, *u_iter);
            }
        }
    }
    return el_entry;
}

bool ElevatorDetector::user_is_entering_elevator(Elevator* el, ElevatorUser* el_user)
{
    bool el_entry = false;
    if (el->is_open())          //the elevator is open
    {
        if (el_user->is_going())//get_possible_exit_id() == el->get_id())         //the user could have possible come from this elevator
        {
            BlobFeature* f = el_user->get_last_feature();
            if (el->get_distance_to_bounding_box(f) < 20)
            {
               // if (el_user->is_approaching(el->get_center(), 10))    //the user is walking away from the elevator
               // {
                    el_entry = true;
                    el_user->set_destination(LOCATION_ELEVATOR);
                //}
            }
        }
    }
    return el_entry;
}

bool ElevatorDetector::detect_elevator_exit()
{
    bool el_exit = false;

    //For each elevator
    for (QList<Elevator*>::iterator el_iter = myElevators.begin(); el_iter != myElevators.end(); el_iter++)
    {
        //For each elevator user
        for (QList<ElevatorUser*>::iterator u_iter = myUsers.begin(); u_iter != myUsers.end(); u_iter++)
        {
            if (user_is_exiting_elevator(*el_iter, *u_iter))
            {
                el_exit = true;
                add_event(EVENT_ELEVATOR_EXIT, *u_iter);
            }
        }
    }
    return el_exit;
}

bool ElevatorDetector::user_is_exiting_elevator(Elevator* el, ElevatorUser* el_user)
{
    bool el_exit = false;
    if (el->is_open())          //the elevator is open
    {
        if (el_user->is_coming())       //this is the first frame in which the user is detected
        {
            BlobFeature* f = el_user->get_first_feature();
            if (el->get_distance_to_bounding_box(f) < 20)       //the trace of the user first appeared near the elevator's bounding box
            {  
                if (el_user->is_leaving(el->get_center(), 0))      //the user is walking away from the elevator
                {
                    el_exit = true;
                    el_user->set_origin(LOCATION_ELEVATOR);
                }
            }
        }
    }
    return el_exit;
}

bool ElevatorDetector::detect_scene_entry()
{
    bool s_entry = false;

    //For each elevator user
    for (QList<ElevatorUser*>::iterator u_iter = myUsers.begin(); u_iter != myUsers.end(); u_iter++)
    {
        if (user_is_entering_scene(*u_iter))
        {
            s_entry = true;
            add_event(EVENT_SCENE_ENTRY, *u_iter);
        }
    }

    return s_entry;
}

bool ElevatorDetector::user_is_entering_scene(ElevatorUser* el_user)
{
    bool s_entry = false;

    if (el_user->is_coming())       //this is the first frame in which the user is detected
    {

        BlobFeature* f = el_user->get_first_feature();

        if (feature_is_near_edge_of_image(f))       //the user is near the edge of the image
        {
            if (el_user->is_approaching(image_center(), 10))
            {
                s_entry = true;           //the user is likely entering the scene from outside the camera view
                el_user->set_origin(LOCATION_SCENE);
            }
        }
    }
    return s_entry;
}

bool ElevatorDetector::detect_scene_exit()
{
    bool s_exit = false;

    //For each elevator user
    for (QList<ElevatorUser*>::iterator u_iter = myUsers.begin(); u_iter != myUsers.end(); u_iter++)
    {
        if (user_is_exiting_scene(*u_iter))
        {
            s_exit = true;
            add_event(EVENT_SCENE_EXIT, *u_iter);
        }
    }

    return s_exit;
}

bool ElevatorDetector::user_is_exiting_scene(ElevatorUser* el_user)
{
    bool s_exit = false;

    if (el_user->is_going())       //this is the first frame in which the user is detected
    {

        BlobFeature* f = el_user->get_last_feature();

        if (feature_is_near_edge_of_image(f))       //the user is near the edge of the image
        {
            if (el_user->is_approaching(get_edge_point(f), 10))
            {
                s_exit = true;           //the user is likely entering the scene from outside the camera view
                el_user->set_destination(LOCATION_SCENE);
            }
        }
    }
    return s_exit;
}

bool ElevatorDetector::detect_suspicious_users()
{
    failed_entry = detect_failed_entry();
    loitering = detect_loitering();
    suspicious_users = (failed_entry || loitering);

    if (failed_entry)
    {
        cout << "Failed entry." << endl;
        char c;
        cin >> c;
    }
    if (loitering)
    {
        cout << "Loitering." << endl;
        char c;
        cin >> c;
    }
}

bool ElevatorDetector::detect_failed_entry()
{
    bool f_entry = false;

    if (last_door_is_closing)
    {

        QList<ElevatorUser*> users_on_open_only;
        QList<ElevatorUser*> users_on_close_only;
        QList<ElevatorUser*> users_on_both;

        //Create the user lists.
        for (QList<ElevatorUser*>::iterator u_iter = users_on_open.begin(); u_iter != users_on_open.end(); u_iter++)
        {
            if (users_on_close.contains(*u_iter)) users_on_both.push_back(*u_iter);
            else users_on_open_only.push_back(*u_iter);
        }

        for (QList<ElevatorUser*>::iterator u_iter = users_on_close.begin(); u_iter != users_on_close.end(); u_iter++)
        {
            if (!users_on_open.contains(*u_iter)) users_on_open_only.push_back(*u_iter);
        }

        //For each user present when a door opened and when it closed
        for (QList<ElevatorUser*>::iterator u_iter = users_on_both.begin(); u_iter != users_on_both.end(); u_iter++)
        {
            f_entry = true;
            (*u_iter)->set_behaviour(BEHAVIOUR_NO_ENTRY);
            add_event(EVENT_NO_ENTRY, *u_iter);
        }

        //For each user present when a door opened, but not when it closed
        for (QList<ElevatorUser*>::iterator u_iter = users_on_open_only.begin(); u_iter != users_on_open_only.end(); u_iter++)
        {
            if ((*u_iter)->get_destination() == LOCATION_SCENE)     //the user has left the scene while a door was opened
            {
                f_entry = true;
                (*u_iter)->set_behaviour(BEHAVIOUR_NO_ENTRY);
                 add_event(EVENT_NO_ENTRY, *u_iter);
            }
        }

        //For each user present when a door closed, but not when it opened
        for (QList<ElevatorUser*>::iterator u_iter = users_on_close_only.begin(); u_iter != users_on_close_only.end(); u_iter++)
        {
            if ((*u_iter)->get_origin() == LOCATION_SCENE)     //the user has entered the scene while a door was opened
            {
                f_entry = true;
                (*u_iter)->set_behaviour(BEHAVIOUR_NO_ENTRY);
                 add_event(EVENT_NO_ENTRY, *u_iter);
            }
        }
    }
    return f_entry;
}


bool ElevatorDetector::detect_loitering()
{
    bool is_loitering = false;

    //For each present elevator user
    for (QList<ElevatorUser*>::iterator u_iter = present_users.begin(); u_iter != present_users.end(); u_iter++)
    {
        if ((*u_iter)->get_behaviour() != BEHAVIOUR_LOITER)
        {
            if ((*u_iter)->get_frame_count() > WAITING_THRESHOLD)
            {
                (*u_iter)->set_behaviour(BEHAVIOUR_LOITER);
                is_loitering = true;
                 add_event(EVENT_LOITER, *u_iter);
            }
        }
    }
    return is_loitering;
}

bool ElevatorDetector::feature_is_near_edge_of_image(BlobFeature* f)
{
    return (feature_is_near_bottom_edge(f) || (feature_is_near_left_edge(f) ||
                  (feature_is_near_right_edge(f) || feature_is_near_top_edge(f))));
}

Pair ElevatorDetector::get_edge_point(BlobFeature* f)
{
    Pair edge;

    bool top = feature_is_near_top_edge(f);
    bool bottom = feature_is_near_bottom_edge(f);
    bool right = feature_is_near_right_edge(f);
    bool left = feature_is_near_left_edge(f);

     //Set x component
    if (right)
    {
        edge.x = myImageSize.x;
    }
    else if (left)
    {
        edge.x = 0;
    }
    else
    {
        edge.x = f->centroid.x;
    }


    //Set y component
    if (bottom)
    {
        edge.y = myImageSize.y;
    }
    else if (top)
    {
        edge.y = 0;
    }
    else
    {
        edge.y = f->centroid.y;
    }
    return edge;
}

bool ElevatorDetector::feature_is_near_bottom_edge(BlobFeature* f)
{
    return ((myImageSize.y - f->br.y) < EDGE_PROXIMITY_THRESHOLD);
}

bool ElevatorDetector::feature_is_near_top_edge(BlobFeature* f)
{
    return (f->ul.y < EDGE_PROXIMITY_THRESHOLD);
}

bool ElevatorDetector::feature_is_near_left_edge(BlobFeature* f)
{
    return (f->ul.x < EDGE_PROXIMITY_THRESHOLD);
}

bool ElevatorDetector::feature_is_near_right_edge(BlobFeature* f)
{
    return ((myImageSize.x - f->br.x) < EDGE_PROXIMITY_THRESHOLD);
}

void ElevatorDetector::get_present_users()
{
    present_users.clear();
    for (QList<ElevatorUser*>::iterator u_iter = myUsers.begin(); u_iter != myUsers.end(); u_iter++)
    {
        if ((*u_iter)->is_present()) present_users.push_back(*u_iter);
    }
    n_users = present_users.count();
}

void ElevatorDetector::set_selected_event(int index)
{
    if (index < 0 || index > myEvents.count())
    {
        mySelectedEvent = NULL;
    }
    else
    {
        mySelectedEvent = myEvents[index];
    }
}

QString ElevatorDetector::get_event_description(int index)
{
    switch (index)
    {
        case EVENT_ELEVATOR_ENTRY:
            return "Elevator Entry";
            break;
        case EVENT_ELEVATOR_EXIT:
            return "Elevator Exit";
            break;
        case EVENT_SCENE_ENTRY:
            return "Scene Entry";
            break;
        case EVENT_SCENE_EXIT:
            return "Scene Exit";
            break;
        default:
            return "DEFAULT";
    }
}

void ElevatorDetector::add_event(int event_index, ElevatorUser* el_user)
{
    ElevatorEvent* new_event = new ElevatorEvent;
    new_event->id = event_index;
    new_event->frame = myTracker->get_frame_number();
    new_event->user = el_user;
    myEvents.push_back(new_event);
}

float cost_value(Elevator* elevator, BlobFeature* f)
{
    //Use the distance between the center of the elevator and the centroid of the blob feature
    return (float)(elevator->get_center() - f->centroid).Norm();
}
