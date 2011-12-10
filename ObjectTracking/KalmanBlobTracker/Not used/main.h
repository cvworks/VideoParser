#ifndef MAIN_H
#define MAIN_H

#define CONFIG_FILENAME "tracker_config"


#include <QString>
#include "pair.h"
#include <QList>



typedef struct ElevatorAttribute
{
    int top;
    Pair center;
} ElevatorAttributes;

typedef struct ConfigStruct{
    bool imageloader;               //loading images | loading videos
    QString videofilename;          //video filename
    QString imagefilepattern;       //image file patterns
    QString loadingdir;             //input directory
    bool always_update;             //updating scheme for the model ("always", "static")
    bool grayscale_segmentation;        //type of segementation, color or grayscale ("color", "grayscale")
    bool record_activity;           //true if scenes with motion are to be recorded to a .avi file
    QString record_filename;        //the recording filename
    QString record_dir;             //the recording directory
    unsigned buffer_size;           //the size of the image buffer
    bool detect_elevator_events;    //true if elevator events are to be detected
    int elevator_count;
    QList<ElevatorAttributes*> elevators;
} ConfigStruct;

extern ConfigStruct *config;

void load_options();
bool save_options();
int main(int argc, char *argv[]);

#endif // MAIN_H
