#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define STATE_STARTED_TRACKING 0
#define STATE_STOPPED_TRACKING  1
#define STATE_NOT_STARTED  2
#define STATE_PLAY_CLIP 3
#define STATE_PAUSE_CLIP 4
#define STATE_STOP_CLIP 5


#include <QtGui/QMainWindow>
#include "videoloader.h"
#include "trace.h"
#include "target.h"
#include "elevator.h"
#include "elevatordetector.h"
#include "imagebuffer.h"
#include "main.h"


class EventTracker;

namespace Ui
{
    class MainWindowClass;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void addTrace(Trace* tr);
    void addTarget(Target* tr);
    void removeTrace(int tr);
    void markTrace(int traceId);
    void closeEvent(QCloseEvent*);
    void enable_start(bool en);
    void enable_option(bool en);
    void enable_exit(bool en);


private slots:

     void on_event_table_cellClicked(int row, int column);
     void on_clip_clicked();
     void on_trace_table_cellClicked(int row, int column);
     void on_target_table_cellClicked(int row, int column);
     void on_output_nodes_clicked();
     void on_draw_clicked();
     void on_reset_clicked();
     void on_start_clicked();
     void on_exit_clicked();
     void on_option_clicked();

private:
    Ui::MainWindowClass *ui;
    EventTracker* tracker;
    void update_gui();
    int tracker_state;
    int clip_state;
    VideoLoader video_loader;
    void start_tracking();
    void draw_traces(IplImage* img);
    void draw_targets(IplImage* img);
    QList<Trace*> traces;
    QList<Target*> targets;
    void update_trace_list();
    void update_target_list();
    Trace* selected_trace;
    QString MainWindow::get_trace_id_list(Target* T);
    inline bool next_frame_is_valid(IplImage* image);
    void play_clip(const char* window_name);

    Target* selected_target;

    //Recording
    CvVideoWriter* writer;
    ImageBuffer* image_buffer;
    void perpare_for_recording(IplImage* image);
    void prepare_image_buffer();
    void MainWindow::ReleaseVideoWriter();
    void MainWindow::ReleaseVideoBuffer();
    unsigned long extra_frame_countdown;
    bool recording;
    inline void record_activity(IplImage* image);
    QString get_record_filename();
    int videofile_index;

    //Elevator Detection
    ElevatorDetector* elevator_detector;
    void prepare_elevator_detector(IplImage* image);
    inline void detect_elevator_events(IplImage* image);
    void addEvent(ElevatorEvent* event);
    void update_event_list();


};

#endif // MAINWINDOW_H
