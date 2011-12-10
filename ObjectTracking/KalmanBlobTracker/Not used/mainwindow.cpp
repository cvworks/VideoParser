#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qmessagebox.h"
#include "qevent.h"
#include "eventtracker.h"
#include <QDir>
#include <QTextStream>
#include <QFileDialog>
#include <QHeaderView>
#include "optiondialog.h"
#include "ImageIterator.h"
#include "common_utility.h"
#include <stdio.h>
#include <time.h>

bool image_is_blank(IplImage* image);
int frame_count;

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::MainWindowClass), tracker_state(STATE_NOT_STARTED), clip_state(STATE_STOP_CLIP), writer(NULL),
	selected_trace(NULL), selected_target(NULL), elevator_detector(NULL), extra_frame_countdown(0),
	image_buffer(NULL), recording(false), videofile_index(0)

{
	ui->setupUi(this);
	tracker = new EventTracker();
	QStringList lables;
	lables << tr("ID#") << tr("Name") << tr("Initial Frame") << tr("Last Frame") << tr("Length");
	ui->trace_table->setHorizontalHeaderLabels(lables);
	ui->trace_table->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
	ui->trace_table->setShowGrid(true);

	QStringList target_lables;
	target_lables << tr("ID#") << tr("Trace IDs") << tr("Initial Frame") << tr("Last Frame");
	ui->target_table->setHorizontalHeaderLabels(target_lables);
	ui->target_table->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
	ui->target_table->setShowGrid(true);

	QStringList event_lables;
	event_lables << tr("Event") << tr("Frame");
	ui->event_table->setHorizontalHeaderLabels(event_lables);
	ui->event_table->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
	ui->event_table->setShowGrid(true);

	frame_count = 0;

	update_gui();

}

MainWindow::~MainWindow()
{
	delete tracker;
	delete ui;
}

//start button handler
void MainWindow::on_start_clicked()
{
	switch(tracker_state)
	{
	case STATE_NOT_STARTED:
		//start the tracker...
		tracker_state = STATE_STARTED_TRACKING;
		update_gui();
		video_loader.reset();
		//if (config->detect_elevator_events) prepare_elevator_detector();
		if (config->record_activity) prepare_image_buffer();
		start_tracking();
		break;
	case STATE_STARTED_TRACKING:
		//pause the tracker...
		update_trace_list();
		//Update the target table
		update_target_list();
		//Update the elevator list table
		update_event_list();

		tracker_state = STATE_STOPPED_TRACKING;
		update_gui();
		break;
	case STATE_STOPPED_TRACKING:
		//resume the tracker...

		//Update the trace table
		update_trace_list();
		//Update the target table
		update_target_list();
		//Update the elevator list table
		update_event_list();

		tracker_state = STATE_STARTED_TRACKING;
		update_gui();
		start_tracking();
		break;
	}
}

//start the tracker
void MainWindow::start_tracking()
{
	cvNamedWindow("binary");
	cvNamedWindow("vid");
	IplImage* image=NULL;
	IplImage* binary_image = NULL;

	//int k = 0;      //skipped frame count
	clock_t start_time = clock();

	while(tracker_state == STATE_STARTED_TRACKING && 
		((image = video_loader.next_frame())!= NULL))
	{
		tracker->next_image(image);
		binary_image = tracker->get_binary_model();
		draw_targets(image);

		//Record when there is substantial motion
		if (config->record_activity) {
			cout << "RECORDING" << endl;
			record_activity(image);
		}

		cvShowImage("binary", binary_image);
		cvShowImage("vid", image);
		cvWaitKey(1);
	}

	clock_t elapsed_time = clock() - start_time;

	cout << "clock ticks = " << elapsed_time << endl;
	cout << "seconds = " << (float)elapsed_time/(float)CLOCKS_PER_SEC << endl;


	if (tracker_state == STATE_STARTED_TRACKING && !image)
	{
		//video is over, terminate tracking.
		tracker->end_tracking();
		ReleaseVideoWriter();
		ReleaseVideoBuffer();
		tracker_state = STATE_NOT_STARTED;

		//Update the trace table
		update_trace_list();
		//Update the target table
		update_target_list();
		//Update the elevator list table
		update_event_list();

		update_gui();
	}

}

inline void MainWindow::detect_elevator_events(IplImage* image)
{
	prepare_elevator_detector(image);
	elevator_detector->next_frame(image);
	elevator_detector->show_states(image);
}

inline void MainWindow::record_activity(IplImage* image)
{
	//perpare_for_recording(image);

	//Record when there is substantial motion
	if (tracker->motion_is_detected())
	{
		if (!recording)     //if not previously recording, write the images that are in the buffer
		{
			recording = true;
			perpare_for_recording(image);
			image_buffer->write(writer);
			extra_frame_countdown = config->buffer_size;
		}

		//Save this frame to an avi file
		cout << "Motion Detected" << endl;
		cvWriteFrame(writer,image);
	}
	else
	{
		if (extra_frame_countdown > 0)
		{
			cout << "extra frame countdown = " << extra_frame_countdown << endl;
			cvWriteFrame(writer,image);
			extra_frame_countdown--;
			if (extra_frame_countdown == 0)
			{
				recording = false;
				//stop_record_frame = video_loader.get_frame_number();     //this is the last recorded frame number
			}
		}
		else
		{
			image_buffer->next_image(image);
		}
	}
}

inline bool MainWindow::next_frame_is_valid(IplImage* image)
{
	image = NULL;
	image = video_loader.next_frame();
	return ((image != NULL) && (image_is_blank(image)));
}


void MainWindow::ReleaseVideoWriter()
{
	if (writer)
	{
		cvReleaseVideoWriter(&writer);
		writer = NULL;
	}
}

void MainWindow::ReleaseVideoBuffer()
{
	//Release the buffer
	if (image_buffer)
	{
		image_buffer->clear();
		image_buffer = NULL;
	}
}

void MainWindow::perpare_for_recording(IplImage* image)
{
	ReleaseVideoWriter();

	int fps     = 10;  // or 30
	int frameW  = image->width;
	int frameH  = image->height;
	QString filename = get_record_filename();

	writer = cvCreateVideoWriter(filename.toAscii().constData(),CV_FOURCC('X', 'V', 'I', 'D'),fps,cvSize(frameW,frameH),1);
}

QString MainWindow::get_record_filename()
{
	char num[200];
	videofile_index++;
	sprintf(num, "%d", videofile_index);
	QString file_suffix = "_";
	file_suffix.append(num);
	QString filename = config->record_filename;
	int p = filename.lastIndexOf(".");
	filename.insert(p, file_suffix);
	return filename;
}

//add a trace to the list of traces
void MainWindow::addTrace(Trace* tr)
{
	int row = ui->trace_table->rowCount();
	ui->trace_table->insertRow(row);
	QString num;

	num.setNum(tr->get_id());
	ui->trace_table->setItem(row, 0, new QTableWidgetItem(num, QTableWidgetItem::UserType));
	num.setNum(tr->get_initial_frame());
	ui->trace_table->setItem(row, 2, new QTableWidgetItem(num, QTableWidgetItem::UserType));
	// num.setNum(tr->get_initial_frame()+tr->get_length());
	num.setNum(tr->get_last_frame());
	ui->trace_table->setItem(row, 3, new QTableWidgetItem(num, QTableWidgetItem::UserType));
	ui->trace_table->setItem(row, 1, new QTableWidgetItem("default", QTableWidgetItem::UserType));
	num.setNum(tr->get_length());
	ui->trace_table->setItem(row, 4, new QTableWidgetItem(num, QTableWidgetItem::UserType));


	ui->trace_table->verticalHeader()->resizeSection(row, 18);

}

//remove a trace from the GUI list
void MainWindow::removeTrace(int tr)
{
}

//mark a trace on the GUI list
void MainWindow::markTrace(int traceId)
{
}


void MainWindow::update_trace_list()
{
	QList<Trace*> history = tracker->get_history();
	list<Trace*> new_elements;


	for (QList<Trace*>::iterator iter = history.begin();
		iter != history.end(); iter++)
		new_elements.push_back(*iter);

	for (QList<Trace*>::iterator iter = traces.begin();
		iter != traces.end(); iter++)
		new_elements.remove(*iter);

	traces = history;

	for (list<Trace*>::iterator iter = new_elements.begin();
		iter != new_elements.end();
		iter++)
		addTrace(*iter);

}

void MainWindow::on_exit_clicked()
{
	QMessageBox msgBox;
	msgBox.setText("Would you like to exit the application?");
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::No);
	if (msgBox.exec()==QMessageBox::Yes) {
		ReleaseVideoWriter();
		ReleaseVideoBuffer();
		exit(0);
	}
}



void MainWindow::on_option_clicked()
{
	OptionDialog dlg;
	if (dlg.exec())
	{
		*config = dlg.get_configurations();
		if (!save_options())
			QMessageBox::warning(this, "I/O Error ... ",
			"Please note that the changes could not be save\non the disk, as a result, the next time that the\napplication is loaded, all the current configurations\n will get reset. The new configuration will last by the\n end of the current session", "Ok", NULL, NULL
			,0,0);

	}
}

void MainWindow::enable_exit(bool en){ ui->exit->setEnabled(en); }
void MainWindow::enable_start(bool en){ui->start->setEnabled(en); }
void MainWindow::enable_option(bool en){ ui->option->setEnabled(en); }



void MainWindow::closeEvent(QCloseEvent* event)
{

	on_exit_clicked();
	event->ignore();
}

void MainWindow::draw_traces(IplImage* img)
{
	tracker->draw_traces(img);
}

void MainWindow::draw_targets(IplImage* img)
{
	tracker->draw_targets(img);
}

//update the GUI according to the state machine
void MainWindow::update_gui()
{

	switch(tracker_state)
	{
	case STATE_STARTED_TRACKING:
		enable_start(true);
		ui->start->setText("Pause");
		enable_exit(false);
		enable_option(false);
		ui->reset->setEnabled(false);
		ui->draw->setEnabled(false);
		ui->output_nodes->setEnabled(false);
		ui->clip->setEnabled(false);
		break;
	case STATE_STOPPED_TRACKING:
		enable_start(true);
		ui->start->setText("Resume");
		enable_exit(true);
		enable_option(false);
		ui->reset->setEnabled(true);
		ui->draw->setEnabled(selected_trace || selected_target);
		ui->output_nodes->setEnabled(selected_trace);
		break;
	case STATE_NOT_STARTED:
		enable_start(true);
		ui->start->setText("Start");
		enable_exit(true);
		enable_option(true);
		ui->reset->setEnabled(false);
		ui->draw->setEnabled(selected_trace || selected_target);
		ui->output_nodes->setEnabled(selected_trace);
		break;

	}

	switch(clip_state)
	{
	case STATE_PLAY_CLIP:
		ui->clip->setEnabled(true);
		ui->clip->setText("Pause Clip");
		break;
	case STATE_PAUSE_CLIP:
		ui->clip->setEnabled(true);
		ui->clip->setText("Resume Clip");
		break;
	case STATE_STOP_CLIP:
		ui->clip->setEnabled(true);
		ui->clip->setText("Play Clip");
		break;

	}
	bool temp = (elevator_detector && elevator_detector->get_selected_event());
	ui->clip->setEnabled(temp);


}

void MainWindow::on_reset_clicked()
{
	while (ui->trace_table->rowCount())
		ui->trace_table->removeRow(0);

	while (ui->target_table->rowCount())
		ui->target_table->removeRow(0);

	traces.clear();
	targets.clear();
	tracker->reset();
	video_loader.reset();

	tracker_state = STATE_NOT_STARTED;

	target_flag_zero();
	trace_flag_zero();

	frame_count = 0;

	update_gui();
}

void MainWindow::on_draw_clicked()
{
	//Trace* selected_trace = get_selected_trace();
	if (selected_trace)
	{
		IplImage* temp = video_loader.get_image_at(selected_trace->get_initial_frame());

		selected_trace->draw(temp);
		cvShowImage("binary", temp);
	}
	else
	{
		// see if a target is selected
		// Target* selected_target = get_selected_target();
		if (selected_target)
		{
			IplImage* temp = video_loader.get_image_at(selected_target->get_initial_frame());

			selected_target->draw(temp);
			cvShowImage("binary", temp);
		}
	}
}

void MainWindow::on_output_nodes_clicked()
{
	if (selected_trace) selected_trace->output_nodes(cout);
}


//add a target to the target list
void MainWindow::addTarget(Target* T)
{
	int row = ui->target_table->rowCount();
	ui->target_table->insertRow(row);
	QString num;

	num.setNum(T->get_id());
	ui->target_table->setItem(row, 0, new QTableWidgetItem(num, QTableWidgetItem::UserType));
	num.setNum(T->get_initial_frame());
	ui->target_table->setItem(row, 2, new QTableWidgetItem(num, QTableWidgetItem::UserType));
	num.setNum(T->get_last_frame());
	ui->target_table->setItem(row, 3, new QTableWidgetItem(num, QTableWidgetItem::UserType));
	//char* temp = get_trace_id_list(T);
	num = get_trace_id_list(T);

	ui->target_table->setItem(row, 1, new QTableWidgetItem(num, QTableWidgetItem::UserType));
	// ui->target_table->setItem(row, 1, new QTableWidgetItem("default", QTableWidgetItem::UserType));
	//num.setNum(tr->get_length());
	// num.setNum((T->get_last_trace())->get_id());
	//ui->target_table->setItem(row, 4, new QTableWidgetItem(num, QTableWidgetItem::UserType));


	ui->target_table->verticalHeader()->resizeSection(row, 18);

	//delete[] temp;

}

void MainWindow::addEvent(ElevatorEvent* event)
{
	int row = ui->event_table->rowCount();
	ui->event_table->insertRow(row);

	//Set the event description
	QString temp = elevator_detector->get_event_description(event->id);
	ui->event_table->setItem(row, 0, new QTableWidgetItem(temp, QTableWidgetItem::UserType));

	//Set the frame number of the event
	temp.setNum(event->frame);
	ui->event_table->setItem(row, 1, new QTableWidgetItem(temp, QTableWidgetItem::UserType));
}

QString MainWindow::get_trace_id_list(Target* T)
{
	int n = T->get_number_of_traces()-1;
	QString id;
	QString id_list;

	for (int i=0; i<n; i++)
	{
		id.setNum(T->get_trace(i)->get_id());
		id_list.append(id);
		id_list.append(", ");
	}
	id.setNum(T->get_trace(n)->get_id());
	id_list.append(id);

	return id_list;
}

void MainWindow::update_target_list()
{

	//Remove all the rows in the table
	int row = ui->target_table->rowCount();
	for (int i=row; i>=0; i--)
	{
		ui->target_table->removeRow(i);
	}

	//Add the updated targets
	QList<Target*> tracker_targets = tracker->get_targets();

	int imobile_target_count = 0;
	int non_transient_trace_count = 0;
	for (QList<Target*>::iterator iter = tracker_targets.begin(); iter != tracker_targets.end(); iter++)
	{
		if((*iter)->is_mobile()) addTarget(*iter);
		else
		{
			imobile_target_count++;
			non_transient_trace_count += (*iter)->get_number_of_traces();
		}
	}

	targets = tracker_targets;

	cout << "imobile_target_count = " << imobile_target_count << endl;
	cout << "non_transient_trace_count = " << non_transient_trace_count << endl;


}

void MainWindow::update_event_list()
{
	if (elevator_detector)
	{
		//Remove all the rows in the table
		int row = ui->event_table->rowCount();
		for (int i=row; i>=0; i--)
		{
			ui->event_table->removeRow(i);
		}

		//Add the updated events
		QList<ElevatorEvent*> elevator_events = elevator_detector->get_events();

		for (QList<ElevatorEvent*>::iterator iter = elevator_events.begin(); iter != elevator_events.end(); iter++)
			addEvent(*iter);

	}
}


void MainWindow::on_target_table_cellClicked(int row, int column)
{

	QTableWidgetItem* first_cell = ui->target_table->item(row,0);
	unsigned id = atoi((const char*) first_cell->text().toLatin1());

	selected_target = NULL;
	selected_trace = NULL;

	QList<Target*> targets = tracker->get_targets();
	for (QList<Target*>::iterator iter = targets.begin(); iter != targets.end(); iter++)
	{
		if ((*iter)->get_id() == id)
		{
			selected_target = *iter;
			break;
		}
	}
	update_gui();
	ui->target_table->selectRow(row);
}


void MainWindow::on_trace_table_cellClicked(int row, int column)
{
	QTableWidgetItem* first_cell = ui->trace_table->item(row,0);
	int id = atoi((const char*) first_cell->text().toLatin1());

	selected_trace = NULL;
	selected_target = NULL;

	QList<Trace*> traces = tracker->get_history();
	for (QList<Trace*>::iterator iter = traces.begin(); iter != traces.end(); iter++)
	{
		if ((*iter)->get_id() == id)
		{
			selected_trace = *iter;
			break;
		}
	}
	update_gui();
	ui->trace_table->selectRow(row);
}

void MainWindow::prepare_elevator_detector(IplImage* image)
{
	if(!elevator_detector)
	{
		elevator_detector = new ElevatorDetector(tracker, config->elevators, image);
		// elevator_detector->set_elevators(config->elevators);
		//elevator_detector->set_image_size(image);
	}
}

void MainWindow::prepare_image_buffer()
{
	if(!image_buffer) { image_buffer = new ImageBuffer; }
	image_buffer->set_size(config->buffer_size);
}

bool image_is_blank(IplImage* image)
{
	long int sum=0;
	long int pixels=0;
	IplImageIterator<unsigned char>  it(image);         //image has three channels

	unsigned char* data;

	while (!it)
	{
		data= &it; // get pointer to current pixel
		sum += data[0]+data[1]+data[2];
		it+= 300; // next pixel
		pixels++;
	}
	// cout << "Pixels = " << pixels << endl;
	if (pixels > 0 )
	{
		float average_pixel = ((float)sum) / ((float)pixels);
		//cout << "Average Pixel = " << average_pixel << endl;
		//char c;
		// cin >> c;
		return (average_pixel < 10);
	}
	else
	{
		return false;
	}

}

void MainWindow::play_clip(const char* window_name)
{
	cvNamedWindow(window_name);
	IplImage* image=NULL;

	while(clip_state == STATE_PLAY_CLIP && ((image = video_loader.next_frame())!= NULL))
	{
		cvShowImage(window_name, image);
		cvWaitKey(5);
	}

	if (clip_state == STATE_PLAY_CLIP && !image)
	{
		//video is over.
		clip_state = STATE_STOP_CLIP;
		cvDestroyWindow(window_name);
		update_gui();
	}

}
void MainWindow::on_clip_clicked()
{


	ElevatorEvent* el_event = elevator_detector->get_selected_event();
	int start_frame = el_event->user->get_first_frame() - 50;
	int end_frame = el_event->user->get_last_frame() + 50;

	QString window_name = elevator_detector->get_event_description(el_event->id);

	switch(clip_state)
	{
	case STATE_STOP_CLIP:
		//start the clip...
		clip_state = STATE_PLAY_CLIP;
		update_gui();
		video_loader.reset(start_frame, end_frame);
		play_clip(window_name.toAscii().constData());
		break;
	case STATE_PLAY_CLIP:
		//pause the clip...
		clip_state = STATE_PAUSE_CLIP;
		update_gui();
		break;
	case STATE_PAUSE_CLIP:
		//resume the clip...
		clip_state = STATE_PLAY_CLIP;
		update_gui();
		play_clip(window_name.toAscii().constData());
		break;
	}
}

void MainWindow::on_event_table_cellClicked(int row, int column)
{
	QTableWidgetItem* first_cell = ui->event_table->item(row,0);

	selected_target = NULL;
	selected_trace = NULL;
	elevator_detector->set_selected_event(row);

	update_gui();
	ui->event_table->selectRow(row);
}
