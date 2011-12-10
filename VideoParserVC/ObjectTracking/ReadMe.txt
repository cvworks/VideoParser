			int expectedNodeId = 0;

		if (expectedNodeId++ != atoi(row[0]))
		{
			showDBError("Invalid node id found");
			return false;
		}

    #include <Tools/PiecewiseApprox/PolyLineTLSApprox.h>

	POINTS data(m_nodes.size());

		for (unsigned i = 0; i < m_nodes.size(); i++)
		{
			data[i].x = m_nodes[i]->centroid().x;
			data[i].y = m_nodes[i]->centroid().y;
		}
		
		PolyLineTLSApprox poly(data.Size(), 5);
		
		poly.Fit(data);
		
		for (int j = 0; j < poly.m_knots.GetSize(); j++)
		{
			auto seg = poly.m_knots[j].seg;

			cv::Point2f p0((float)seg.p0.x, (float)seg.p0.y);
			cv::Point2f p1((float)seg.p1.x, (float)seg.p1.y);

			cv::line(mat, p0, p1, m_color, 2, 1, 0);
		}

	CubicBezierParams par();

				POINTS curve = par.Rasterize(pts.size() * 4);

				for (int i = 1; i < curve.Size(); i++)
				{
					cv::Point2f p0((float)curve[i - 1].x, (float)curve[i - 1].y);
					cv::Point2f p1((float)curve[i].x, (float)curve[i].y);

					cv::line(mat, p0, p1, m_color, 2, 1, 0);
				}

	std::vector<cv::Point2f> pts(m_nodes.size());
		//std::vector<cv::Point2f> widths(m_nodes.size());
		//std::vector<cv::Point2f> heights(m_nodes.size());

		for (unsigned i = 0; i < m_nodes.size(); i++)
		{
			pts[i].x = (float)m_nodes[i]->centroid().x;
			pts[i].y = (float)m_nodes[i]->centroid().y;

			//widths[i].x = (float)i;
			//widths[i].y = (float)m_nodes[i]->width();

			//heights[i].x = (float)i;
			//heights[i].y = (float)m_nodes[i]->height();
		}

		std::vector<cv::Point2f> smPts, smWidths, smHeights;

		cv::approxPolyDP(cv::Mat(pts), smPts, 50, false);
		//cv::approxPolyDP(cv::Mat(widths), smWidths, 20, false);
		//cv::approxPolyDP(cv::Mat(heights), smHeights, 20, false);

		for (unsigned i = 1; i < smPts.size(); i++)
			cv::line(mat, smPts[i - 1], smPts[i], m_color, 2, 1, 0);
	
	// dealing with transient noise
    void remove_transient_matches(Matrix& M);
    bool exists_non_transient_matches(int i, const Matrix& M) const;
    void remove_transient_matches(int i, Matrix& M);
	void fix_separated_components(Matrix& M, const Matrix& C);

//! returns true if there is any transient matches with component #i
bool KalmanBlobTracker::exists_non_transient_matches(int i, const Matrix& M) const
{
	int j = 0;
	
	for (auto iter=m_traces.begin(); iter != m_traces.end(); iter++)
	{
		if (M(j, i) && (*iter)->get_length() > 1) 
			return true;

		j++;
	}

	return false;
}

//removes all the transient matches of component #i in the matrix M (distance matrix)
void KalmanBlobTracker::remove_transient_matches(int i, Matrix& M)
{
	int j = 0;

	for (auto iter=m_traces.begin(); iter != m_traces.end(); ++iter)
	{
		if (M(j, i) && (*iter)->get_length() < 2) 
			M(j, i) = 0;

		j++;
	}
}

//removes the transient matches(if there is any non-transient ones). helps with removing noise
void KalmanBlobTracker::remove_transient_matches(Matrix& M)
{
	for (int i = 0; i < (int)m_features_for_matching.size(); i++)
	{
		if (exists_non_transient_matches(i, M)) 
			remove_transient_matches(i, M);
	}
}

//if a component is broken into two or more components, we take care of it here
void KalmanBlobTracker::fix_separated_components(Matrix& M, const Matrix& C)
{
	for(int i = 0; i < (int)m_traces.size();i++)
	{
		std::vector<BlobPtr> matched_components = get_matched_components(i, M);

		if (matched_components.size() > 1)
		{
			BlobPtr merged_feature = merge_components(matched_components);

			double min_cost = get_minumum_cost_of_trace(i, C, M);

			if (cost_value(m_traces[i]->get_expected_feature(), *merged_feature) < min_cost)
				replace_first_match(i, merged_feature, M);
			else
				remove_non_minimal(M, C, i, min_cost);
		}
	}
}


// See if we have to save a new trace
	if (!m_ptrLastTraceSaved || m_ptrLastTraceSaved != tr)
	{
		m_ptrLastTraceSaved = tr;

		m_lastSavedTraceID = dbm.createTrace(tr->get_length(), tr->start_time());
	}

	// See if we have to save a new feature
	if (!m_ptrLastFeatureSaved || m_ptrLastFeatureSaved != f)
	{
		m_ptrLastFeatureSaved = f;
		
		dbm.createPathLocation(f->centroid.x, f->centroid.y, f->height, f->width, 
			tr->get_length() - 1, m_lastSavedTraceID);
	}

//! Removes the 1-to-N relation "period-to-traces" and the associated traces
void BlobTrackerDBManager::removePtot(int pid)
{
    // Get all the associate tid(s) to this pid
    m_pSQLDatabase->Execute("select tid from ptot where pid=%d", pid);

    SQLQueryResult res = m_pSQLDatabase->StoreQueryResult();
	SQLDataRow row;

    // Remove points for ptot
    m_pSQLDatabase->Execute("delete from ptot where pid=%d", pid);

    // Remove all the traces associated with this pid
    while (row = m_pSQLDatabase->FetchDataRow(res))
        removeTrace(atoi(row[0]));
}

//! Create a relation between a period and a trace
bool BlobTrackerDBManager::createPeriodToTrace(int pid, int tid)
{
    // Create the table row and return 0 upon success
    return m_pSQLDatabase->Execute("insert into ptot(pid, tid) values(%d, %d)", 
		pid, tid);
}

	// The 'm_history' contains all m_traces that have ended already
	/*for (auto iter = m_history.begin(); iter != m_history.end(); iter++)
	{
		tr = *iter;
		tr->draw(img);
		tr->get_matching_region(A,B);
		BlobPtr b = tr->get_last_feature();
		p1 = cvPoint(b->ul.x,b->ul.y);
		p2 = cvPoint(b->br.x, b->br.y);
		//cvRectangle(img, p1, p2, tr->get_color(), 1, 1, 0);
	}*/

//! adds new frames to each trace according to M matrix
void KalmanBlobTracker::assign_traces_to_components(CvMat* M)
{
	unsigned j;

	for (unsigned i = 0; i < m_traces.size(); i++)
	{
		if (get_matched_component(M, i, &j))
		{
			m_traces[i]->add_path_node(m_features_for_matching[j]);
			
			m_features_for_matching[j].reset();
		}
		else
		{
			// Move unmatched non-transient traces to the history
			// if it's not too short. Otherwise, just delete it.
			if (m_traces[i]->get_length() > TRANSIENT_TIME)
				m_history.push_back(m_traces[i]); 

			m_traces[i].reset();
		}
	}

	//remove all the nulls from vector m_traces
	std::vector<TracePtr> temp;

	for (auto iter = m_traces.begin(); iter != m_traces.end(); iter++)
		if (*iter) 
			temp.push_back(*iter);

	m_traces = temp;
}

void OutputMatrix(CvMat* M)
{
	cout << endl;

	for (int i=0; i<M->rows; i++)
	{
		for (int j=0; j<M->cols; j++)
		{
			cout << cvmGet(M,i,j) << "\t";
		}
		cout << endl;
	}

	cout << endl;
}

if (dii.outputIdx == 1 && (int)dii.params.front() > 0)
	{
	}
	else
	{
		unsigned mobileTargets = 0;
		unsigned recentTargets = 0;
		unsigned trackedTargets = 0;
		unsigned recentMobileTargets = 0;
		unsigned trackedMobileTargets = 0;
		TargetPtr tgt;

		for (auto it = m_targets.begin(); it != m_targets.end(); ++it)
		{
			tgt = *it;

			if (tgt->is_mobile()) 
			{
				mobileTargets++;

				if (target_is_being_tracked(tgt))
					trackedMobileTargets++;

				if (target_is_recent(tgt))
					recentMobileTargets++;
			}

			if (target_is_being_tracked(tgt))
				trackedTargets++;

			if (target_is_recent(tgt))
				recentTargets++;
		}

		std::ostringstream oss;

		oss << "Traces: " << m_traces.size() 
			<< ", targets: " << m_targets.size()
			<< ", unmatched: " << get_unmatched_traces().size()
			<< ", \nmobile: " << mobileTargets << ", tracked: " << trackedTargets
			<< ", recent: " << recentTargets << ", tracked-mobile: " << trackedMobileTargets
			<< ", recent-mobile: " << recentMobileTargets
			<< ".";

		dio.message = oss.str();
	}

Rect imgRect(0, 0, m_currentImage.cols, m_currentImage.rows);
	Rect clipRect;

	Rect predictedRect = m_graphRect;

	predictedRect.x -= ROUND_NUM(object->meanFlow.x);
	predictedRect.y -= ROUND_NUM(object->meanFlow.y);

	bool doSomething = rectIntersect(predictedRect, imgRect, clipRect);

	int yMin = clipRect.y + ROUND_NUM(object->meanFlow.y);
	int yMax = clipRect.height + yMin;

	int xMin = clipRect.x + ROUND_NUM(object->meanFlow.x);
	int xMax = clipRect.width + xMin;

	int objectID =  object->id;

	int objectID =  object->id,
		yMin = m_graphRect.y,
		yMax = yMin + m_graphRect.height;

	/*int xMin = m_graphRect.x - ROUND_NUM(object->meanFlow.x),
		xMax = xMin + m_graphRect.width;
*/

void operator=(const Blob& rhs)
	{
		memcpy(this, &rhs, sizeof(Blob));
	}

#include <VideoSegmentation/FSMBMBackgroundSegmentor/FSMBMBackgroundSegmentor.h>
#include <VideoSegmentation/FSMBMBackgroundSegmentor/FSMBMColorBackgroundSegmentor.h>

//IplImageView rgbView(m_pImgProcessor->GetRGBImage(), false); //swapRB=false


	// See if the image has been static for a long time
	if (m_static_frame_count >= m_params.staticSceneThreshold)
	{
		//image is static, update the background
		getSegmentedImageAndUpdateModel(gray_image, binary_image, 25, luminance);
	}

			double getFSMBMAdaptationRate();
		void setFSMBMAdaptationRate(double adptationRate);
		int getFSMBMNegativeLearningStep();
		void setFSMBMNegativeLearningStep(int negativeLearningStep);
		int getFSMBMPositiveLearningStep();
		void setFSMBMPositiveLearningStep(int positiveLearningStep);
		int getFSMBMMaxConfidenceLevel();
		void setFSMBMMaxConfidenceLevel(int maxConfidenceLevel);
		IplImage* getFSMBMBackgroundModel();


		void FSMBMBackgroundSegmentor::UpdateModel(IplImage * grayImage,int *currentMode,int *previousMode)
{
	UpdateModel(grayImage, getDifferencePixelValueThreshold( grayImage,currentMode,previousMode));
}

void FSMBMBackgroundSegmentor::UpdateModel(IplImage * grayImage, int threshold)
{
	int ls;
	IplImageIterator<float>bmIterator(m_backgroundModel);
	IplImageIterator<signed int>cliIterator(m_modelConfidence);
	IplImageIterator<unsigned char> it(grayImage);

	while(!(it))
	{
		if( ((*it-*bmIterator)>threshold) ||
			((*bmIterator-*it)>threshold) )
		{
			ls = -m_params.negativeLearningStep;
		} else {
			ls = m_params.positiveLearningStep;
		}
		//Update Model
		if(*cliIterator+ls <= 0)
		{
			*cliIterator = m_params.positiveLearningStep - m_params.negativeLearningStep;
			*bmIterator = *it;
		} 
		else 
		{
			if(*cliIterator+ls>fsmbmMaxConfidenceLevel)
				*cliIterator = fsmbmMaxConfidenceLevel;
			else
				*cliIterator = *cliIterator + ls;
			if(ls > 0)
				*bmIterator = (float)((1.0-m_params.backgroundModelAdaptationRate) * 
				              (double)(*bmIterator) + (m_params.backgroundModelAdaptationRate * 
							  (double)(*it)));
		}
		it++;
		bmIterator++;
		cliIterator++;
	}
}

long FSMBMBackgroundSegmentor::getSegmentedImage(IplImage * grayImage,
	IplImage *resultImage, int threshold)
{
	long whitePixels=0;
	IplImageIterator<unsigned char>result(resultImage);
	IplImageIterator<float>bmIterator(m_backgroundModel);
	IplImageIterator<unsigned char>  it(grayImage);
	while(!(it))
	{
		if( ((*it-*bmIterator)>threshold) ||
			((*bmIterator-*it)>threshold) )
		{
			*result=255;
			whitePixels++;
		} else {
			*result=0;
		}
		it++;
		bmIterator++;
		result++;
	}
	return whitePixels;
}
int FSMBMBackgroundSegmentor::getDifferencePixelValueThreshold(IplImage * grayImage, int *currentMode,int *previousMode)
{
	int differencePixelValueThreshold;
	CvScalar mean, sdev;
	cvAvgSdv(grayImage,&mean,&sdev,NULL);

	if(sdev.val[0]<30)
	{
		if(mean.val[0]<=50)
		{
			differencePixelValueThreshold=10;// uniform dark
			*currentMode=NIGHT_MODE;
		}
		else if(mean.val[0]<85 && mean.val[0]>50)
		{
			differencePixelValueThreshold=15;// uniform gray
			*currentMode=NIGHT_MODE;
		}
		else if(mean.val[0]>100)
		{
			differencePixelValueThreshold=20;// uniform bright
			*currentMode=DAY_MODE;
		}
		else
		{
			*currentMode=*previousMode;
			differencePixelValueThreshold=prevDifferencePixelValueThreshold;
		}
	}
	else if (sdev.val[0]>40)
	{
		if(mean.val[0]<=55)
		{
			differencePixelValueThreshold=15;
			*currentMode=NIGHT_MODE;
		}
		else if(mean.val[0]>65 && mean.val[0]<90)
		{
			differencePixelValueThreshold=20;
			*currentMode=DAY_MODE;
		}
		else if(mean.val[0]>90)
		{
			differencePixelValueThreshold=25;
			*currentMode=DAY_MODE;
		}
		else
		{
			*currentMode=*previousMode;
			differencePixelValueThreshold=prevDifferencePixelValueThreshold;
		}
	}
	else
	{
		*currentMode=*previousMode;
		differencePixelValueThreshold=prevDifferencePixelValueThreshold;
	}

	prevDifferencePixelValueThreshold=differencePixelValueThreshold;

	return differencePixelValueThreshold;
}
========================================================================
    STATIC LIBRARY : ObjectTracking Project Overview
========================================================================

AppWizard has created this ObjectTracking library project for you.

No source files were created as part of your project.


ObjectTracking.vcxproj
    This is the main project file for VC++ projects generated using an Application Wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

ObjectTracking.vcxproj.filters
    This is the filters file for VC++ projects generated using an Application Wizard. 
    It contains information about the association between the files in your project 
    and the filters. This association is used in the IDE to show grouping of files with
    similar extensions under a specific node (for e.g. ".cpp" files are associated with the
    "Source Files" filter).

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" comments to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////
