/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "Trace.h"
#include <Tools/BasicUtils.h>
#include <Tools/CvMatView.h>

using namespace vpl;

void smooth_trace_pts(std::vector<cv::Point2f>& pts);

int Trace::s_traceCount = 0;

Trace::Trace(fnum_t first_frame, unsigned unique_identifier)
{
	s_traceCount++;
	
	m_id = unique_identifier;
	
	m_start_frame = first_frame;
	m_is_matched = false;
	m_displacement = 0;

#ifdef SMOOTH_TRACES_ONLINE
	m_expected_feature.reset(new Blob(0));
	m_kalman_filter_is_initialized = false;
#endif

	m_color = cvScalar(rand() % 255, rand() % 255, rand() % 255);
	m_x_range.min = INT_MAX;
	m_x_range.max = 0;
	m_y_range.min = INT_MAX;
	m_y_range.max = 0;

	m_dbID = -1;
	m_numFeaturesSaved = 0;
}

bool Trace::is_blob_close(BlobPtr blob)
{
	XYCoord A, B;
	bool boxes_overlap(XYCoord A, XYCoord B, XYCoord a, XYCoord b);

	get_matching_region(A, B);
	
	//cout << "Matching region for trace " << m_id << 
	//" = [" << A.x << "," << A.y << "] [" << B.x << "," << B.y << "]" << endl;

	return boxes_overlap(A, B, blob->ul(), blob->br());
}

/*************************************************************************************************
* By Jeff Lalonde
* Date: 2009/05/12
* Description: Returns the uppler-left corner (A) and bottom-right corner (B) of the rectangular
* area in which we will consider components for matching with this trace.
*************************************************************************************************/
void Trace::get_matching_region(XYCoord &A, XYCoord &B)
{
	XYCoord d;
	int dialate_factor = 0;

	d.x = dialate_factor;
	d.y = dialate_factor;

	A = expected_feature().ul() - d;
	B = expected_feature().br() + d;
}

/*************************************************************************************************
* By Jeff Lalonde
* Date: 2009/05/12
* Description: Returns true if the boxes given by the pairs A,B,a,b overlap.  A and B are the
* upper-left and bottom-right corners of box 1.  a and b are the upper-left and bottom-right
* corners of box 2.
*************************************************************************************************/
bool boxes_overlap(XYCoord A, XYCoord B, XYCoord a, XYCoord b)
{
	return ((A.x < b.x) && (B.x > a.x) && (A.y < b.y) && (B.y > a.y));
}

Trace::~Trace()
{
	s_traceCount--;

#ifdef SMOOTH_TRACES_ONLINE
	if (get_length() > KALMAN_START_LENGTH)      // if the filters have been created
	{
		cvReleaseKalman(&m_filtFeature.centroid.x);
		cvReleaseKalman(&m_filtFeature.centroid.y);
		cvReleaseKalman(&m_filtFeature.size);
		cvReleaseKalman(&m_filtFeature.height);
		cvReleaseKalman(&m_filtFeature.width);

		for (int i=0; i<Blob::HISTO_SIZE; i++) 
			cvReleaseKalman(&m_filtFeature.color_histogram[i]);
	}
#endif
	// DIEGO TRACKING
	//for (auto it = m_nodes.begin(); it != m_nodes.end(); it++)
	//	delete *it;

	//for (auto it = m_filtered_nodes.begin(); it != m_filtered_nodes.end(); it++)
	//	delete *it;
}

void Trace::SaveChangesToDatabase(vpl::BlobTrackerDBManager& dbm, int target_id)
{
	ASSERT(dbm.hasDB());

	if (m_dbID < 0)
	{
		m_dbID = dbm.createTrace(m_id, start_time(), get_first_frame(), target_id);

		if (m_dbID < 0)
		{
			ShowError("Cannot save target to database");
			return;
		}
	}

	for (; m_numFeaturesSaved < m_nodes.size(); m_numFeaturesSaved++)
	{
		BlobPtr f = m_nodes[m_numFeaturesSaved];

		dbm.createTraceNode((int)f->centroid().x, (int)f->centroid().y, f->height(), f->width(), 
			f->timestamp(), m_numFeaturesSaved, m_dbID);
	}
}

void Trace::add_path_node(BlobPtr feature)
{
	if (!m_nodes.empty())
		m_displacement += (m_nodes.back()->centroid() - feature->centroid()).Norm();

	/*BlobPtr measured_point(new Blob(*feature));

	m_nodes.push_back(measured_point);*/

	m_nodes.push_back(feature);

#ifdef SMOOTH_TRACES_ONLINE
	if (get_length() == KALMAN_START_LENGTH)
		Initialize_Kalman_Filter();
#else
	if (m_x_range.min > feature->centroid().x) 
		m_x_range.min = feature->centroid().x;

	if (m_x_range.max < feature->centroid().x) 
		m_x_range.max = feature->centroid().x;

	if (m_y_range.min > feature->centroid().y) 
		m_y_range.min = feature->centroid().y;

	if (m_y_range.max < feature->centroid().y) 
		m_y_range.max = feature->centroid().y;
#endif
}

void Trace::set_color(CvScalar new_color)
{
	m_color = new_color;
}

bool Trace::is_mobile() const
{
	int del_x = m_x_range.max - m_x_range.min;
	int del_y = m_y_range.max - m_y_range.min;
	int del_max;

	if (del_x > del_y)
	{
		del_max = del_x;
	}
	else
	{
		del_max = del_y;
	}
		
	return (del_max > DISPLACEMENT_THRESHOLD);
}

BlobPtr Trace::get_first_feature()
{
	return m_nodes.front();
}

//Functions involving traces and blob features

double vpl::cost_value(const Blob& f, const Blob& g)
{
	float relative_cost(int a, int b);
	float correl(const Blob::ColorHistogram& x, const Blob::ColorHistogram& y, int n);

	float size_ratio = relative_cost(f.size(), g.size());
	float width_ratio = relative_cost(f.width(), g.width());
	float height_ratio = relative_cost(f.height(), g.height());
	float color_ratio = 1.0f - correl(f.color_histogram(), g.color_histogram(), Blob::HISTO_SIZE);

	float cv = size_ratio + width_ratio + height_ratio + color_ratio;

	return (double)cv;
}

float relative_cost(int a, int b)
{
	if (a>b) return (float)(a-b)/float(a);
	else return (float)(b-a)/float(b);
}

float correl(const Blob::ColorHistogram& x, const Blob::ColorHistogram& y, int n)
{
	float mean(const Blob::ColorHistogram& a, int n);

	float xm = mean(x,n);
	float ym = mean(y,n);
	float Sxx, Syy, Sxy;
	float xt, yt;
	Sxx = Syy = Sxy = 0.0;
	for (int i=0; i<n; i++)
	{
		xt = x[i]-xm;
		yt = y[i]-ym;

		Sxx += xt*xt;
		Syy += yt*yt;
		Sxy += xt*yt;
	}
	return Sxy/sqrt(Sxx*Syy);

}

float mean(const Blob::ColorHistogram& a, int n)
{
	int sum = 0;
	for (int i=0; i<n; i++)
	{
		sum += a[i];
	}
	return (float)sum/(float)n;
}

bool blob_features_are_equal(BlobPtr f, BlobPtr g)
{
	return (f && g &&(f->ul() == g->ul()) &&(f->br() == g->br()));
}

bool f_contains_g(BlobPtr f, BlobPtr g)
{
	return (f && g && (f->ul().x < g->ul().x) && 
		(f->br().x > g->br().x) && 
		(f->ul().y < g->ul().y) && 
		(f->br().y > g->br().y));
}

void Trace::draw(RGBImg image, SMOOTHING_METHOD smoothMethod)
{
#ifdef SMOOTH_TRACES_ONLINE
	//draw the trace line using the filtered centroid
	CvPoint pre = cvPoint((*(m_filtered_nodes.begin()))->centroid().x,
		(*(m_filtered_nodes.begin()))->centroid().y);

	CvPoint current;

	CvMatView mat(image);

	unsigned k=0;

	for (auto iter = m_filtered_nodes.begin()+1; k<m_filtered_nodes.size()-1; iter+=1)
	{

		current = cvPoint((*iter)->centroid().x,(*iter)->centroid().y);
		cv::line(mat, pre, current, m_color, 2, 1, 0);
		pre = current;
		k+=1;
	}

	current = cvPoint(m_filtered_nodes.back()->centroid().x,
		m_filtered_nodes.back()->centroid().y);

	cv::line(mat, pre, current, m_color, 2, 1, 0);
#else
	CvMatView mat(image);

	if (m_nodes.size() > 4 && smoothMethod == OFFLINE_POLYLINE)
	{
		std::vector<cv::Point2f> pts(m_nodes.size());

		for (unsigned i = 0; i < m_nodes.size(); i++)
		{
			pts[i].x = (float)m_nodes[i]->centroid().x;
			pts[i].y = (float)m_nodes[i]->centroid().y;
		}

		std::vector<cv::Point2f> smPts;

		cv::approxPolyDP(cv::Mat(pts), smPts, 25, false);

		for (unsigned i = 1; i < smPts.size(); i++)
			cv::line(mat, smPts[i - 1], smPts[i], m_color, 2, 1, 0);
	}
	else if (smoothMethod == OFFLINE_BEZIER)
	{
		std::vector<cv::Point2f> pts(m_nodes.size());

		for (unsigned i = 0; i < m_nodes.size(); i++)
		{
			pts[i].x = (float)m_nodes[i]->centroid().x;
			pts[i].y = (float)m_nodes[i]->centroid().y;
		}

		for (int i = 0; i < 5; i++)
			smooth_trace_pts(pts);

		for (unsigned i = 1; i < pts.size(); i++)
			cv::line(mat, pts[i - 1], pts[i], m_color, 2, 1, 0);
	}
	else
	{
		//draw the trace line using the filtered centroid
		CvPoint pre = cvPoint((*(m_nodes.begin()))->centroid().x,
			(*(m_nodes.begin()))->centroid().y);

		CvPoint current;

		unsigned k=0;

		for (auto iter = m_nodes.begin()+1; k < m_nodes.size()-1; iter+=1)
		{
			current = cvPoint((*iter)->centroid().x,(*iter)->centroid().y);
			cv::line(mat, pre, current, m_color, 2, 1, 0);
			pre = current;
			k += 1;
		}

		current = cvPoint(m_nodes.back()->centroid().x,
			m_nodes.back()->centroid().y);

		cv::line(mat, pre, current, m_color, 2, 1, 0);
	}
#endif
}

// Here we average the points. Diego: Bug fixed. it was "eating" one point.
void smooth_trace_pts(std::vector<cv::Point2f>& pts)
{
	if (pts.size() < 3)
		return;

	std::vector<cv::Point2f> tpts;
	cv::Point2f currPt;

	tpts.reserve(pts.size());

	tpts.push_back(pts.front());

	auto it0 = pts.begin();
	auto it1 = ++pts.begin();
	auto it2 = ++++pts.begin();

	for (; it2 != pts.end(); ++it0, ++it1, ++it2)
	{
		currPt.x = (it0->x + it1->x + it2->x) / 3.0f;
		currPt.y = (it0->y + it1->y + it2->y) / 3.0f;

		tpts.push_back(currPt);
	}

	tpts.push_back(pts.back());

	pts = tpts;
}

#ifdef SMOOTH_TRACES_ONLINE
void Trace::Evolve()
{
	m_filtFeature.centroid.Evolve();
}

void Evolve(CvKalman* KF, CvMat*z)
{
	// Evolves the Kalman states using the predicted state as the measurement
	cvKalmanPredict(KF, NULL);
	cvmSet(z,0,0,cvmGet(KF->state_pre,0,0));
	cvKalmanCorrect(KF, z);
}

Blob Trace::get_kalman_state_post()
{
	Blob f(get_last_feature()->timestamp());

	f.m_area = (int)cvmGet(m_filtFeature.size->state_post, 0, 0);
	f.m_bbox.width = (int)cvmGet(m_filtFeature.width->state_post, 0, 0);
	f.m_bbox.height = (int)cvmGet(m_filtFeature.height->state_post, 0, 0);
	
	for (int i = 0; i < Blob::HISTO_SIZE; i++) 
		f.m_color_histogram[i] = (int)cvmGet(m_filtFeature.color_histogram[i]->state_post, 0, 0);

	f.m_centroid = m_filtFeature.centroid.State(0);

	return f;
}

Blob Trace::get_kalman_state_pre()
{
	Blob f(get_last_feature()->timestamp());

	f.m_centroid.x = (int)cvmGet(m_filtFeature.centroid.x->state_pre, 0, 0);
	f.m_centroid.y = (int)cvmGet(m_filtFeature.centroid.y->state_pre, 0, 0);
	f.m_area = (int)cvmGet(m_filtFeature.size->state_pre, 0, 0);
	f.m_bbox.width = (int)cvmGet(m_filtFeature.width->state_pre, 0, 0);
	f.m_bbox.height = (int)cvmGet(m_filtFeature.height->state_pre, 0, 0);

	for (int i = 0; i < Blob::HISTO_SIZE; i++) 
		f.m_color_histogram[i] = (int)cvmGet(m_filtFeature.color_histogram[i]->state_pre,0,0);

	return f;
}

void Trace::output_nodes(ostream& ostr)
{
	fnum_t fr_n = m_start_frame;
	ostr << "Summary of the m_nodes for trace #" << m_id << "\n===================================================\n";
	ostr << "frame#   cent_x  cent_y width   height    COLOR HISTORGRAM\n";
	ostr << "------   ------  ------ -----   ------    -----------------------------------------------------------------------------------------------------------------------------------------\n";
	for (std::vector<BlobPtr>::iterator iter = m_nodes.begin();
		iter != m_nodes.end(); iter++)
	{
		ostr << fr_n << "\t ";
		ostr << (*iter)->centroid().x << "\t ";
		ostr << (*iter)->centroid().y << "\t";
		ostr << (*iter)->width() << "\t  ";
		ostr << (*iter)->height() << "\t  ";

		for (int i=0;i<Blob::HISTO_SIZE;i++)
			ostr << (*iter)->m_color_histogram[i] << "\t" ;

		fr_n++;


	}
}

void Trace::Predict_Kalman_Filter()
{
	//Centroid
	cvKalmanPredict(m_filtFeature.centroid.x, NULL);
	cvKalmanPredict(m_filtFeature.centroid.y, NULL);

	//Area
	cvKalmanPredict(m_filtFeature.size, NULL);

	//Width
	cvKalmanPredict(m_filtFeature.width, NULL);

	//Height
	cvKalmanPredict(m_filtFeature.height, NULL);

	//Color histogram
	for (int i = 0; i < Blob::HISTO_SIZE; i++)
	{
		cvKalmanPredict(m_filtFeature.color_histogram[i], NULL);
	}
}

/*!
	Adjusts the stochastic model state on the basis of the given 
	measurement of the model state.

	m is usually equal to get_last_feature().

	@see Perform_Kalman_Filtering()
*/
void Trace::Correct_Kalman_Filter(BlobPtr m)
{
	CvMat* z = cvCreateMat(1,1,CV_32FC1);

	//Centroid
	cvmSet(z,0,0,m->centroid().x);
	cvKalmanCorrect(m_filtFeature.centroid.x, z);

	cvmSet(z,0,0,m->centroid().y);
	cvKalmanCorrect(m_filtFeature.centroid.y, z);

	//Size
	cvmSet(z,0,0,m->size());
	cvKalmanCorrect(m_filtFeature.size, z);

	//Width
	cvmSet(z,0,0,m->width());
	cvKalmanCorrect(m_filtFeature.width, z);

	//Height
	cvmSet(z,0,0,m->height());
	cvKalmanCorrect(m_filtFeature.height, z);

	//Color histogram
	for (int i=0; i<Blob::HISTO_SIZE; i++)
	{
		cvmSet(z,0,0,m->m_color_histogram[i]);
		cvKalmanCorrect(m_filtFeature.color_histogram[i], z);
	}

	cvReleaseMat(&z);
}

/*!
	This routine performs the kalman filtering for the trace.
	It is also used to store the predicted states as the traces 'm_expected_feature'
	and to store the filtered features in the vector 'm_filtered_nodes'.
*/
void Trace::Perform_Kalman_Filtering()
{
	BlobPtr f(new Blob(0));

	if (m_kalman_filter_is_initialized)
	{
		// Predict the kalman filter states
		Predict_Kalman_Filter();

		//correct the kalman filter states using the latest blob features
		Correct_Kalman_Filter(get_last_feature());

		//set the expected features based on the predicted kalman filter states
		m_expected_feature = get_kalman_state_pre();

		//set the filtered feature to the corrected kalman state
		*f = get_kalman_state_post();
	}
	else
	{
		//set the expected features based on the previous feature
		m_expected_feature = *get_last_feature();

		//set the filtered feature to the measured feature until the kalman filter starts running
		*f = *get_last_feature();
	}

	//store the corrected filtered states
	m_filtered_nodes.push_back(f);

	//Get the range of the trace
	if (m_x_range.min > f->centroid().x) m_x_range.min = f->centroid().x;
	if (m_x_range.max < f->centroid().x) m_x_range.max = f->centroid().x;
	if (m_y_range.min > f->centroid().y) m_y_range.min = f->centroid().y;
	if (m_y_range.max < f->centroid().y) m_y_range.max = f->centroid().y;

	//cout << "Trace " << m_id << " x min = " << x.min << "\tx max = " << x.max << endl;

	//In addition set the object bounding box.  This must be done because these values are not directly filtered
	m_expected_feature.m_bbox.x = (int)(m_expected_feature.centroid().x - 0.5 * m_expected_feature.width());
	m_expected_feature.m_bbox.y = (int)(m_expected_feature.centroid().y - 0.5 * m_expected_feature.height());
	//m_expected_feature.br.x = (int)(m_expected_feature.centroid().x + 0.5 * m_expected_feature.width());
	//m_expected_feature.br.y = (int)(m_expected_feature.centroid().y + 0.5 * m_expected_feature.height());

}

void Trace::Initialize_Kalman_Filter()
{
	XYCoord position;
	XYCoord velocity;
	double s;

	ASSERT(!m_nodes.empty());

	BlobPtr last_feature = m_nodes.back();

	//Allocate the Kalman filter memory
	m_filtFeature.centroid.x = cvCreateKalman(2,1,0);
	m_filtFeature.centroid.y = cvCreateKalman(2,1,0);
	m_filtFeature.size = cvCreateKalman(1,1,0);
	m_filtFeature.height = cvCreateKalman(1,1,0);
	m_filtFeature.width = cvCreateKalman(1,1,0);

	for (int i = 0; i < Blob::HISTO_SIZE; i++)
	{
		m_filtFeature.color_histogram[i] = cvCreateKalman(1,1,0);
	}

	//CENTROID

	position = last_feature->centroid();

	ASSERT(m_nodes.size() >= 2);

	velocity = (position - m_nodes[m_nodes.size()-2]->centroid()) / TIME_STEP;

	m_filtFeature.centroid.Initialize(TIME_STEP, position, velocity);

	//AREA

	//Initialize the parameters
	cvSetIdentity(m_filtFeature.size->transition_matrix, cvRealScalar(1));
	cvSetIdentity(m_filtFeature.size->measurement_matrix, cvRealScalar(1));
	cvSetIdentity(m_filtFeature.size->process_noise_cov, cvRealScalar(1E-3));
	cvSetIdentity(m_filtFeature.size->measurement_noise_cov, cvRealScalar(1e-1));

	//Set the initial conditions
	s = last_feature->size();
	cvSetIdentity(m_filtFeature.size->state_post, cvRealScalar(s));
	cvSetIdentity(m_filtFeature.size->error_cov_post, cvRealScalar(1));

	//HEIGHT

	//Initialize the parameters
	cvSetIdentity(m_filtFeature.height->transition_matrix, cvRealScalar(1));
	cvSetIdentity(m_filtFeature.height->measurement_matrix, cvRealScalar(1));
	cvSetIdentity(m_filtFeature.height->process_noise_cov, cvRealScalar(1E-2));
	cvSetIdentity(m_filtFeature.height->measurement_noise_cov, cvRealScalar(1E-1));

	//Set the initial conditions
	s = last_feature->height();
	cvSetIdentity(m_filtFeature.height->state_post, cvRealScalar(s));
	cvSetIdentity(m_filtFeature.height->error_cov_post, cvRealScalar(1));

	//WIDTH

	//Initialize the parameters
	cvSetIdentity(m_filtFeature.width->transition_matrix, cvRealScalar(1));
	cvSetIdentity(m_filtFeature.width->measurement_matrix, cvRealScalar(1));
	cvSetIdentity(m_filtFeature.width->process_noise_cov, cvRealScalar(1E-2));
	cvSetIdentity(m_filtFeature.width->measurement_noise_cov, cvRealScalar(1E-1));

	//Set the initial conditions
	s = last_feature->width();
	cvSetIdentity(m_filtFeature.width->state_post, cvRealScalar(s));
	cvSetIdentity(m_filtFeature.width->error_cov_post, cvRealScalar(1));


	//COLOR HISTOGRAM

	for (int i = 0; i < Blob::HISTO_SIZE; i++)
	{
		//Initialize the parameters
		cvSetIdentity(m_filtFeature.color_histogram[i]->transition_matrix, cvRealScalar(1));
		cvSetIdentity(m_filtFeature.color_histogram[i]->measurement_matrix, cvRealScalar(1));
		cvSetIdentity(m_filtFeature.color_histogram[i]->process_noise_cov, cvRealScalar(1E-3));
		cvSetIdentity(m_filtFeature.color_histogram[i]->measurement_noise_cov, cvRealScalar(1E-1));

		//Set the initial conditions
		s = last_feature->m_color_histogram[i];
		cvSetIdentity(m_filtFeature.color_histogram[i]->state_post, cvRealScalar(s));
		cvSetIdentity(m_filtFeature.color_histogram[i]->error_cov_post, cvRealScalar(1));
	}

	m_kalman_filter_is_initialized = true;
}
#endif

/*void Display_Histograms(int a[], int b[])
{
	//Determine the maximum value
	int max_value = 0;

	for (int i=0; i<Blob::HISTO_SIZE; i++)
	{
		if (a[i] > max_value) max_value = a[i];
		if (b[i] > max_value) max_value = b[i];
	}

	//Scale the window according to the maximum value
	int scale_res = 100;      //number of pixels we scale by
	int scale = scale_res * (1 + (max_value)/scale_res);

	int window_height = 200;
	int window_width = 400;
	int bar_width = 20;
	int bar_space = 10;

	float pixel_ratio = float(window_height)/float(scale);

	//Diego: cvNamedWindow("Histogram");
	IplImage* graph = cvCreateImage(cvSize(window_width, window_height),8,3);

	cvZero(graph);

	//Draw the histograms
	int x_pos = bar_space;
	int ul_x, ul_y, br_x, br_y;

	for (int i=0; i<Blob::HISTO_SIZE; i++)
	{
		ul_x = x_pos;
		br_x = ul_x + bar_width;


		// Determine which value is largest
		if (a[i] > b[i])
		{
			// Draw the combined bar
			br_y = window_height;
			ul_y = window_height - int(b[i] * pixel_ratio);
			cvRectangle(graph,cvPoint(ul_x,ul_y), cvPoint(br_x,br_y),cvScalar(0,255,0),CV_FILLED);

			// Draw the bar for a
			br_y = ul_y - 1;
			ul_y = window_height - int(a[i] * pixel_ratio);
			cvRectangle(graph,cvPoint(ul_x,ul_y), cvPoint(br_x,br_y),cvScalar(255,0,0),CV_FILLED);

		}
		else
		{
			// Draw the combined bar
			br_y = window_height;
			ul_y = window_height - int(a[i] * pixel_ratio);
			cvRectangle(graph,cvPoint(ul_x,ul_y), cvPoint(br_x,br_y),cvScalar(255,0,0),CV_FILLED);

			// Draw the bar for b
			br_y = ul_y - 1;
			ul_y = window_height - int(b[i] * pixel_ratio);
			cvRectangle(graph,cvPoint(ul_x,ul_y), cvPoint(br_x,br_y),cvScalar(0,255,0),CV_FILLED);
		}


		x_pos += bar_space + bar_width;
	}

	cvReleaseImage(&graph);
}
*/
