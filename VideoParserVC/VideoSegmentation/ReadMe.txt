PixelwiseFrameBuffer(unsigned ni, unsigned nj, unsigned n_reserve = 0) 
		: ParentClass(ni, nj)
	{
		//if (n_reserve > 0)
		//	reserve(n_reserve);
	}

	void set_size(unsigned ni, unsigned nj, unsigned n_reserve = 0)
	{
		ParentClass::resize(ni, nj);

		//if (n_reserve > 0)
		//	reserve(n_reserve);

		// Make sure that all buffers are cleared
		clear_pixel_buffers();
	}

	/*void reserve(unsigned n_reserve)
	{
		for (unsigned i = 0; i < ni(); i++)
			for (unsigned j = 0; j < nj(); j++)
				get(i, j).reserve(n_reserve);
	}*/

	//! Clears all pixel buffers
	void clear_pixel_buffers()
	{
		for (unsigned i = 0; i < ni(); i++)
			for (unsigned j = 0; j < nj(); j++)
				get(i, j).clear();
	}

		/*for (size_t i = 0; i < m_goodKeypoints.size(); i++)
		{
			std::ostringstream oss;	
			oss.precision(2);

			double n = std::sqrt(cv::sum(m_goodDescriptors.row(i))[0]); //0th channel

			oss << n; 

			DrawCenteredText(trainMat, oss.str(), m_goodKeypoints[i].pt, 
				cv::FONT_HERSHEY_COMPLEX, 0.25, cv::Scalar(255, 0, 0));
		}

		dio.imageType = RGB_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(trainImg);*/

	/*struct MatchStats
	{
		unsigned numGoodFeaturesMatched;
		double matchedGoodFeaturesRatio;
		double unmatchedQueryFeaturesRatio;
		bool changeDetected;

		void clear()
		{
			numGoodFeaturesMatched = 0;
			matchedGoodFeaturesRatio = 0;
			unmatchedQueryFeaturesRatio = 0;
			changeDetected = false;
		}
	};*/

	MatchStats m_matchStats;

		if (m_matchStats.changeDetected && !m_roiSeq.empty())
		{
			dio.imageType = RGB_IMAGE;
			dio.imagePtr = ConvertToBaseImgPtr(m_queryImage);
			// Draw does the rest...
		}
		else
		{
			dio.imageType = RGB_IMAGE;
			dio.imagePtr = ConvertToBaseImgPtr(trainImg);
		}

/*DBG_PRINT1(m_goodDescriptors)

			double n = 0;

			for (int j = 0; j < m_goodDescriptors.cols; j++)
				n += m_goodDescriptors.at<float>(i, j) * m_goodDescriptors.at<float>(i, j);*/


			/*DBG_PRINT1(m_goodDescriptors.type());

			cv::Mat desc(1, m_goodDescriptors.cols, CV_32F);

			cv::pow(m_goodDescriptors.row(i), 2.0, desc);*/


		//cv::drawKeypoints(trainMat, m_goodKeypoints, trainMat, cv::Scalar(0, 255, 0), 
		//	cv::DrawMatchesFlags::DRAW_OVER_OUTIMG);

		//for (auto it = m_goodKeypoints.begin(); it != m_goodKeypoints.end(); ++it)

/*for (auto it = m_bagOfDescriptors.begin(); it != m_bagOfDescriptors.end(); ++it)
		{
			if (*it && (*it)->reliability >= m_params.minReliability)
			{
				const cv::KeyPoint& kpt = m_trainKeypoints[(*it)->trainId][(*it)->row];
			
				cv::circle(trainMat, kpt.pt, 4, cv::Scalar(255, 0,0));
			}
		}*/

	/*
void FSMBMBackgroundSegmentor::processImage(IplImage* grayImage,
	bool updateModel)
{
	int ls;

	m_stats.Clear();

	IplImageView resultImage(m_foreground);
	IplImageView backgroundModel(m_backgroundModel);
	IplImageView confidenceLevelImage(m_modelConfidence);

	IplImageIterator<unsigned char> maskIt(resultImage);
	IplImageIterator<float> bmIt(backgroundModel);
	IplImageIterator<int> confIt(confidenceLevelImage);
	IplImageIterator<float> pixelIt(grayImage);

	// Iterate over each pixel
	while (!(pixelIt))
	{
		if( ((*pixelIt - *bmIt) > m_params.pixelDiffThreshold) ||
			((*bmIt - *pixelIt) > m_params.pixelDiffThreshold) )
		{
			*maskIt = 255;
			m_stats.numberWhitePixels++;
			ls = -m_params.negativeLearningStep;
		} else {
			*maskIt = 0;
			ls = m_params.positiveLearningStep;
		}

		if((*pixelIt - *bmIt) > m_params.pixelDiffThreshold)
			m_stats.numberPixelsAboveThreshold++;
		else if((*bmIt - *pixelIt) > m_params.pixelDiffThreshold)
			m_stats.numberPixelsBelowThreshold++;
		else
			m_stats.numberPixelsInThreshold++;

		// Update the Confidence Level Image and optionally, the background model
		if(*confIt + ls <= 0)
		{
			*confIt = m_params.positiveLearningStep - m_params.negativeLearningStep;

			if (updateModel)
				*bmIt = *pixelIt;
		} 
		else 
		{
			if(*confIt + ls > m_params.maxConfidenceLevel)
				*confIt = m_params.maxConfidenceLevel;
			else
				*confIt = *confIt + ls;

			if(updateModel && ls > 0)
			{
				*bmIt = (float)((1.0 - m_params.backgroundModelAdaptationRate) * 
					(double)(*bmIt) + (m_params.backgroundModelAdaptationRate * (double)(*pixelIt)));
			}
		}

		maskIt++;
		pixelIt++;
		bmIt++;
		confIt++;
	}
}
*/
	
	/*while (!pixelIt)
	{
		ASSERT(!maskIt && !bmIt && !confIt);

		*maskIt = (unsigned char) *pixelIt;

		maskIt++;
		pixelIt++;
		bmIt++;
		confIt++;
	}*/

			/*double getAdaptationRate()
		{
			return m_params.backgroundModelAdaptationRate;
		}

		void setAdaptationRate(double adptationRate)
		{
			m_params.backgroundModelAdaptationRate = adptationRate;
		}

		int getNegativeLearningStep()
		{
			return m_params.negativeLearningStep;
		}

		void setNegativeLearningStep(int negativeLearningStep)
		{
			m_params.negativeLearningStep = negativeLearningStep;
		}

		int getPositiveLearningStep()
		{
			return m_params.positiveLearningStep;
		}

		void setPositiveLearningStep(int positiveLearningStep)
		{
			m_params.positiveLearningStep = positiveLearningStep;
		}

		int getMaxConfidenceLevel()
		{
			return m_params.maxConfidenceLevel;
		}

		void setMaxConfidenceLevel(int maxConfidenceLevel)
		{
			m_params.maxConfidenceLevel = maxConfidenceLevel;
		}*/

		/*IplImage* getBackgroundModel()
		{
			return m_backgroundModel;
		}

		IplImage* getConfidenceLevelImage() 
		{
			return m_modelConfidence;
		}*/

	//m_nFrame = 0;

WARNING2(m_nFrame != num, "Frame numbers differ", m_nFrame, num);

/////////////////////////////////////////////////////////////////////////////
std::list<ZTString> m_labels; //!< copy of std::string labels used

	const char* AddLabel(const char* lbl)
	{
		m_labels.push_back(ZTString(lbl));

		return m_labels.back().c_str();
	}

	const char* AddLabel(const std::string& lbl)
	{
		return AddLabel(lbl.c_str());
	}

/////////////////////////////////////////////////////////////////////////////
void UserArgumentsQuickView::AddBoolWidget(WidgetParamList::iterator it)
{
	bool val;

	try {
		val = g_userArgs.GetBoolValue(it->FieldKey(), it->PropKey());
	}
	catch (BasicException e)
	{
		e.Print();
		return;
	}

	it->x = Fl_Group::x();
	it->y = Fl_Group::y();
	it->dx = 25;
	it->dy = 25;

	if (it != m_widgetParams.begin())
	{
		WidgetParamList::iterator it0 = it;

		--it0;

		it->x = it0->x + it0->dx + 4;

		/*Fl_Widget* lastButton = ChildWidget(NumWidgets() - 1);

		int dx, dy;
		
		lastButton->measure_label(dx, dy);

		x = lastButton->x() + lastButton->w() + dx;	*/
	}

	Fl_Button* p = new Fl_Check_Button(it->x, it->y, it->dx, it->dy, 
		it->label.c_str());

	p->value(val);
	p->callback(ChildWidgetCallback);

	add(p);

	// Add the width of the label
	int dx_label, dy_label;
		
	p->measure_label(dx_label, dy_label);

	it->dx += dx_label;
}

/*void UserArgumentsQuickView::AddBoolArgument(const std::string& fieldKey, 
	const std::string& propKey, const char* szDisplayLabel)
{
	bool val;

	try {
		val = g_userArgs.GetBoolValue(fieldKey, propKey);
	}
	catch (BasicException e)
	{
		e.Print();
		return;
	}

	int x = Fl_Group::x();
	int y = Fl_Group::y();
	int w = 25;
	int h = 25;

	if (NumWidgets() > 0)
	{
		Fl_Widget* lastButton = ChildWidget(NumWidgets() - 1);

		int dx, dy;
		
		lastButton->measure_label(dx, dy);

		x = lastButton->x() + lastButton->w() + dx;			
	}

	Fl_Button* p = new Fl_Check_Button(x, y, w, h, szDisplayLabel);

	int dx, dy;
		
	p->measure_label(dx, dy);

	p->value(val);
	p->callback(ChildWidgetCallback);

	add(p);
}

void UserArgumentsQuickView::AddStringArgument(const std::string& fieldKey, 
	const std::string& propKey, const char* szDisplayLabel)
{
	std::string val;

	try {
		val = g_userArgs.GetStrValue(fieldKey, propKey);
	}
	catch (BasicException e)
	{
		e.Print();
		return;
	}

	int x = Fl_Group::x();
	int y = Fl_Group::y();
	int w = 25;
	int h = 25;

	if (NumWidgets() > 0)
	{
		Fl_Widget* lastButton = ChildWidget(NumWidgets() - 1);

		int dx, dy;
		
		lastButton->measure_label(dx, dy);

		x = lastButton->x() + lastButton->w() + dx;			
	}

	Fl_Output* p = new Fl_Output(x, y, w, h, szDisplayLabel);

	p->value(val.c_str());
	p->callback(ChildWidgetCallback);

	add(p);
}*/

/////////////////////////////////////////////////////////////////////////////
