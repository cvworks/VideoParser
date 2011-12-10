#define IMAGE_LOADER_VAR "image_loader="
#define VIDEO_FILENAME_VAR "video_filename="
#define IMAGE_FILE_PATTERN_VAR "image_file="
#define LOADING_DIR_VAR "loading_dir="
#define MODEL_UPDATE_VAR "model_update="
#define COLOR_OPTION_VAR "color_option="
#define RECORD_ACTIVITY_VAR "record_activity="
#define RECORD_FILENAME_VAR "record_filename="
#define RECORD_DIR_VAR "record_directory="
#define BUFFER_SIZE_VAR "buffer_size="
#define ELEVATOR_EVENT_VAR "elevator_event_detection="
#define ELEVATOR_COUNT_VAR "elevator_count="
#define ELEVATOR_ATTRIBUTES_VAR "elevator_attributes="
#define ELEVATOR_TOP_VAR "elevator_top="
#define ELEVATOR_CENTER_X_VAR "elevator_center_x="
#define ELEVATOR_CENTER_Y_VAR "elevator_center_y="


#include <QtGui/QApplication>
#include "mainwindow.h"
#include <QDir>
#include <QTextStream>
#include <QFileDialog>
#include "main.h"
#include <QIcon>
#include <QMessageBox>
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"
#include <math.h>


ConfigStruct *config;


//saving new user configurations, returns false in case of I/O errors
bool save_options()
{
    QFile file(CONFIG_FILENAME);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out << IMAGE_LOADER_VAR << config->imageloader << "\n";
        out << VIDEO_FILENAME_VAR << config->videofilename << "\n";
        out << IMAGE_FILE_PATTERN_VAR << config->imagefilepattern << "\n";
        out << LOADING_DIR_VAR << config->loadingdir << "\n";
        out << MODEL_UPDATE_VAR << config->always_update << "\n";
        out << COLOR_OPTION_VAR << config->grayscale_segmentation << "\n";
        out << RECORD_ACTIVITY_VAR << config->record_activity << "\n";
        out << RECORD_FILENAME_VAR << config->record_filename << "\n";
        out << RECORD_DIR_VAR << config->record_dir << "\n";
        out << BUFFER_SIZE_VAR << config->buffer_size << "\n";
        out << ELEVATOR_EVENT_VAR << config->detect_elevator_events << "\n";
        out << ELEVATOR_COUNT_VAR << config->elevator_count << "\n";
        //write elevator attributes
        out << ELEVATOR_ATTRIBUTES_VAR << "\n";
        for (QList<ElevatorAttribute*>::iterator iter = config->elevators.begin(); iter != config->elevators.end(); iter++)
        {
            ElevatorAttribute* el = *iter;
            out << ELEVATOR_TOP_VAR << el->top << "\n";
            out << ELEVATOR_CENTER_X_VAR << el->center.x << "\n";
            out << ELEVATOR_CENTER_Y_VAR << el->center.y << "\n";
        }

        file.close();
        return true;
    }
    return false;
}

//loading all the configurations before running the app
void load_options()
{

     config->imageloader = true;
     config->videofilename = "";
     config->imagefilepattern = "img####.jpg";
     config->loadingdir = "/home";

     QFile file(CONFIG_FILENAME);
     if (file.open(QIODevice::ReadOnly | QIODevice::Text))
     {
          QTextStream in(&file);
          while (!in.atEnd())
          {
              QString oneline = in.readLine();

              if (oneline.indexOf(IMAGE_LOADER_VAR) == 0)
              {

                  oneline.remove(QString(IMAGE_LOADER_VAR));

                  if (oneline == "1")
                      config->imageloader=true;
                  else
                      config->imageloader=false;
              }
              else if (oneline.indexOf(VIDEO_FILENAME_VAR) == 0)
              {
                  oneline.remove(QString(VIDEO_FILENAME_VAR));
                  config->videofilename = oneline;
              }
              else if (oneline.indexOf(IMAGE_FILE_PATTERN_VAR) == 0)
              {
                  oneline.remove(IMAGE_FILE_PATTERN_VAR);
                  config->imagefilepattern = oneline;
              }
              else if (oneline.indexOf(LOADING_DIR_VAR) == 0)
              {
                  oneline.remove(LOADING_DIR_VAR);
                  config->loadingdir = oneline;
              }
              else if (oneline.indexOf(MODEL_UPDATE_VAR) == 0)
              {
                  oneline.remove(MODEL_UPDATE_VAR);
                  if (oneline == "1")
                      config->always_update=true;
                  else
                      config->always_update=false;
              }
              else if (oneline.indexOf(COLOR_OPTION_VAR) == 0)
              {
                  oneline.remove(COLOR_OPTION_VAR);
                  if (oneline == "1")
                      config->grayscale_segmentation=true;
                  else
                      config->grayscale_segmentation=false;
              }
              else if (oneline.indexOf(RECORD_ACTIVITY_VAR) == 0)
              {
                  oneline.remove(RECORD_ACTIVITY_VAR);
                  if (oneline == "1")
                      config->record_activity=true;
                  else
                      config->record_activity=false;
              }
              else if (oneline.indexOf(RECORD_FILENAME_VAR) == 0)
              {
                  oneline.remove(QString(RECORD_FILENAME_VAR));
                  config->record_filename = oneline;
              }
              else if (oneline.indexOf(RECORD_DIR_VAR) == 0)
              {
                  oneline.remove(QString(RECORD_DIR_VAR));
                  config->record_dir = oneline;
              }
              else if (oneline.indexOf(BUFFER_SIZE_VAR) == 0)
              {
                  oneline.remove(QString(BUFFER_SIZE_VAR));
                  bool ok;
                  config->buffer_size = oneline.toUInt(&ok,10);
              }
              else if (oneline.indexOf(ELEVATOR_EVENT_VAR) == 0)
              {
                  oneline.remove(QString(ELEVATOR_EVENT_VAR));
                  if (oneline == "1")
                      config->detect_elevator_events=true;
                  else
                      config->detect_elevator_events=false;
              }
              else if (oneline.indexOf(ELEVATOR_COUNT_VAR) == 0)
              {
                  oneline.remove(QString(ELEVATOR_COUNT_VAR));
                  bool ok;
                  config->elevator_count = oneline.toInt(&ok,10);
              }
              else if (oneline.indexOf(ELEVATOR_ATTRIBUTES_VAR) == 0)
              {
                  //oneline.remove(QString(ELEVATOR_ATTRIBUTES_VAR));
                  for (int i=0; i<config->elevator_count; i++)
                  {
                      ElevatorAttribute* new_elevator = new ElevatorAttribute;

                      oneline = in.readLine();
                      if (oneline.indexOf(ELEVATOR_TOP_VAR) == 0)
                      {
                          oneline.remove(QString(ELEVATOR_TOP_VAR));
                          bool ok;
                          new_elevator->top = oneline.toInt(&ok,10);
                      }

                      oneline = in.readLine();
                      if (oneline.indexOf(ELEVATOR_CENTER_X_VAR) == 0)
                      {
                          oneline.remove(QString(ELEVATOR_CENTER_X_VAR));
                          bool ok;
                          new_elevator->center.x = oneline.toInt(&ok,10);
                      }

                      oneline = in.readLine();
                      if (oneline.indexOf(ELEVATOR_CENTER_Y_VAR) == 0)
                      {
                          oneline.remove(QString(ELEVATOR_CENTER_Y_VAR));
                          bool ok;
                          new_elevator->center.y = oneline.toInt(&ok,10);
                      }

                      config->elevators.push_back(new_elevator);
                  }
              }
          }

          file.close();

     }



}

int main(int argc, char *argv[])
{
    config = new ConfigStruct;
    load_options(); // load all the options before start
    QApplication a(argc, argv);
    QIcon app_icon("tracking_icon.jpg");

    a.setWindowIcon(app_icon);

    MainWindow w;
    w.show();
    int retVal = a.exec();
    delete config;
    return retVal;
}
