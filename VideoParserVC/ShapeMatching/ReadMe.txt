// Erase any previous good descriptor info
	m_goodDInfo.clear();

	// Find the info of the current good descriptors
	for (auto it = m_bagOfDescriptors.begin(); it != m_bagOfDescriptors.end(); ++it)
	{
		// Compute mean and stddev of matching distances found for this "good" descriptor
		(*it)->ComputeStats(m_trainImages.size());

		DBG_PRINT1((*it)->reliability)

		if ((*it)->reliability >= m_params.minReliability)
			m_goodDInfo.push_back(*it);		
	}

struct DescriptorInfo
	{
		unsigned trainId;
		int row;
		double reliability;
		std::list<double> distances;
		double meanDist;
		double avgDistError;
		double maxDist;

		DescriptorInfo(unsigned tid, int r, const double& rel,
			const double& first_dist) : distances(1, first_dist)
		{
			trainId = tid;
			row = r;
			reliability = rel;
			meanDist = 0;
			avgDistError = 0;
			maxDist = 0;
		}

		DescriptorInfo(const DescriptorInfo& rhs)
		{
			operator=(rhs);
		}

		void operator=(const DescriptorInfo& rhs)
		{
			trainId     = rhs.trainId;
			row         = rhs.row;
			reliability = rhs.reliability;
			distances   = rhs.distances;

			meanDist     = rhs.meanDist;
			avgDistError = rhs.avgDistError;
			maxDist      = rhs.maxDist;
		}

		void ComputeStats()
		{
			meanDist = 0;
			avgDistError = 0;
			maxDist = 0;

			for (auto it = distances.begin(); it != distances.end(); ++it)
			{
				meanDist += *it;

				if (maxDist < *it)
					maxDist = *it;
			}

			meanDist /= distances.size();

			for (auto it = distances.begin(); it != distances.end(); ++it)
				avgDistError += (*it - meanDist) * (*it - meanDist);

			avgDistError = sqrt(avgDistError / distances.size());

			// If there is no error, add some to make sure there is room for noise
			// ie, regularize the measure?
			if (avgDistError < 0.1 * meanDist)
				avgDistError = 0.1 * meanDist;

		}
	};

void DescriptorMatcher::FindPreviousMatches(const unsigned id1, const unsigned id2,
	std::vector<bool>& matched1)
{
	const cv::Mat& m1 = m_trainDescriptors[id1];
	const cv::Mat& m2 = m_trainDescriptors[id2];

	MatToBagMap& queryMap = m_mbMapArray[id1];
	MatToBagMap& trainMap = m_mbMapArray[id2];

	const Keypoints& k1 = m_trainKeypoints[id1];
	const Keypoints& k2 = m_trainKeypoints[id2];

	std::vector<cv::DMatch> matches1to2;
	
	cv::Mat mask = ConstrainPossibleMatches(k1, k2);

	// Match the current and previous descriptors
	m_matcher->match(m1, m2, matches1to2, mask);

	std::vector<bool> matched2(m2.rows, false);

	// Note: the match is one-to-many. It seems that it simply finds
	// the closest match for each query point in the training set
	for(size_t i = 0; i < matches1to2.size(); i++ )
	{
		const cv::DMatch& match = matches1to2[i];
		
		matched1[match.queryIdx] = true;
		matched2[match.trainIdx] = true;

		// Get the info of the training descriptor (might be null)
		DescriptorInfoPtr& ptrTrainDI = trainMap[match.trainIdx];

		if (ptrTrainDI)
		{
			// If this is not the first match found for id2 in the current match set...
			if (matched2[match.trainIdx])
			{
				// ...update the distance we last appended if it is now smaller
				if (match.distance < ptrTrainDI->distances.back())
					ptrTrainDI->distances.back() = match.distance;
			}
			else // it is the first match found in THIS match set 'matches1to2'
			{
				ptrTrainDI->distances.push_back(match.distance);
			}
		}
		else // it is the first match found across ALL match sets, create an info object for it
		{
			// Always insert with reliability 1 (ie, for the past detection)
			ptrTrainDI.reset(new DescriptorInfo(id2, match.trainIdx, 1, match.distance));
			m_bagOfDescriptors.push_back(ptrTrainDI);
		}

		// Link the info that we associated to the training descriptor
		// with the info of the query descriptor
		queryMap[match.queryIdx] = ptrTrainDI;
	}
}

void DescriptorMatcher::AddNewDescriptorsToBag()
{
	ASSERT(!m_trainImages.empty());
	ASSERT(m_mbMapArray.size() + 1 == m_trainDescriptors.size());

	m_mbMapArray.push_back(MatToBagMap());

	if (m_trainDescriptors.size() < 2)
		return; // there is nothing else to do

	const unsigned id1 = m_trainDescriptors.size() - 1;
	const unsigned id2 = m_trainDescriptors.size() - 2;

	const cv::Mat& m1 = m_trainDescriptors[id1];
	const cv::Mat& m2 = m_trainDescriptors[id2];

	MatToBagMap& queryMap = m_mbMapArray[id1];
	MatToBagMap& trainMap = m_mbMapArray[id2];

	const Keypoints& k1 = m_trainKeypoints[id1];
	const Keypoints& k2 = m_trainKeypoints[id2];

	std::vector<bool> matchedTrain(m2.rows, false);
	std::vector<cv::DMatch> matches1to2;
	
	cv::Mat mask = ConstrainPossibleMatches(k1, k2);

	// Match the current and previous descriptors
	m_matcher->match(m1, m2, matches1to2, mask);

	// Note: the match is one-to-many. It seems that it simply finds
	// the closest match for each query point in the training set
	for(size_t i = 0; i < matches1to2.size(); i++ )
	{
		const cv::DMatch& match = matches1to2[i];
			
		// Ignore if the train descriptor was already matched
		if (matchedTrain[match.trainIdx])
			continue;
		
		matchedTrain[match.trainIdx] = true;

		// Get the info of the training descriptor (might be null)
		DescriptorInfoPtr& ptrTrainDI = trainMap[match.trainIdx];

		if (ptrTrainDI)
		{
			ptrTrainDI->distances.push_back(match.distance);
		}
		else // there is no info object, create one
		{
			// Always insert with reliability 1 (ie, for the past detection)
			ptrTrainDI.reset(new DescriptorInfo(id2, match.trainIdx, 1, match.distance));
			m_bagOfDescriptors.push_back(ptrTrainDI);
		}
		
		// Make sure to increasy the reliability only by one in case
		// of multiple matches
		ASSERT(ptrTrainDI->reliability < m_trainDescriptors.size());

		// Increase the reliability of the descriptor
		ptrTrainDI->reliability++;

		// Link the info that we associated to the training descriptor
		// with the info of the query descriptor
		queryMap[match.queryIdx] = ptrTrainDI;
	}
}

cv::Mat desc = m_goodDescriptors.row(i);

double n = 0;

			for (int j = 0; j < desc.cols; j++)
				n += desc.at<double>(0, j) * desc.at<double>(0, j);

			oss << sqrt(n); 

cv::Mat sqDesc;


			cv::pow(m_goodDescriptors.row(i), 2.0, sqDesc);

			double n = std::sqrt(cv::sum(sqDesc)[0]);

if (!boxes.empty())
	{
		ROISequence ra;

		for (auto it = boxes.begin(); it != boxes.end(); ++it)
			ra.push_back(*it);

		//std::copy(boxes.begin(), boxes.end(), ra.begin());

		SetROISequence(ra);
	}

int radius;

double avgSz = 0;

		for (auto it = pts.begin(); it != pts.end(); ++it)
			avgSz += it->size;

		avgSz /= pts.size();

		radius = (int) avgSz + 1;



class GridFeatureDetector : public FeatureDetector
{
	unsigned m_size;

public:
    GridFeatureDetector(unsigned sz)
	{
		m_size = sz;
	}

	virtual void read( const cv::FileNode& fn ) { }
	virtual void write( cv::FileStorage& fs ) const { }

protected:
	virtual void detectImpl( const cv::Mat& image, vector<cv::KeyPoint>& keypoints, 
		const cv::Mat& mask=cv::Mat() ) const
	{
		const int r = ROUND_NUM(m_size / 2.0);

		for (int i = r; i < image.rows; i += m_size)
			for (int j = r; j < image.cols; j += m_size)
				keypoints.push_back(cv::KeyPoint((float)i, (float)j, (float)m_size, 0));
	}

    //cv::SURF surf;
};

	class GridFeatureDetector : public FeatureDetector
{
public:
    GridFeatureDetector( double hessianThreshold=400., int octaves=3, int octaveLayers=4 )
		//: surf(hessianThreshold, octaves, octaveLayers)
	{
	}

	virtual void read( const cv::FileNode& fn ) { }
	virtual void write( cv::FileStorage& fs ) const { }

protected:
	virtual void detectImpl( const cv::Mat& image, vector<cv::KeyPoint>& keypoints, 
		const cv::Mat& mask=cv::Mat() ) const
	{

	}

    //cv::SURF surf;
};
			
				
				dist = cv::norm(k1[match.queryIdx].pt - k2[match.trainIdx].pt);

		// Ignore if it isn't a good match or if the train descriptor
		// was already matched
		if (dist > m_params.maxFeatureJitter || matchedTrain[match.trainIdx])
			continue;
		else
			matchedTrain[match.trainIdx] = true;
		
		for (auto it = m_goodDInfo.begin(); it != m_goodDInfo.end(); ++it, ++i)
			goodPts[i] = m_trainKeypoints[(*it)->trainId][(*it)->row];

if (!m_matches.empty())
		{
			const std::vector<cv::DMatch>& m = m_matches;

			std::vector<char> matchesMask(m.size(), 0);
			DescriptorInfoPtr ptrDI;

			for (size_t i = 0; i < m_matches.size(); i++)
			{
				ptrDI = m_goodDInfo[m_matches[i].trainIdx];

				matchesMask[i] = (m_matches[i].distance < ptrDI->meanDist 
					+ 2 * ptrDI->avgDistError);
			}

			Keypoints goodPts(m_goodDInfo.size());
			unsigned i = 0;

			for (auto it = m_goodDInfo.begin(); it != m_goodDInfo.end(); ++it, ++i)
				goodPts[i] = m_trainKeypoints[(*it)->trainId][(*it)->row];
		
			cv::drawMatches(queryMat, m_queryKeypoints, trainMat, goodPts, m, outImg, 
				cv::Scalar(255, 0, 0), cv::Scalar(0, 255, 0), matchesMask);
		}
		else
		{
			cv::drawMatches(queryMat, Keypoints(), trainMat, Keypoints(), 
				std::vector<cv::DMatch>(), outImg);
		}

		//DBG_PRINT1(m.size())

		/*unsigned minIdx = (unsigned) std::ceil(alpha * m.size());
		unsigned maxIdx = minIdx + (unsigned) std::ceil(0.05 * m.size());

		// Set matches between min and max to one
		for (unsigned i = minIdx; i < maxIdx && i < m.size(); i++)
			matchesMask[i] = 1;*/

void DescriptorMatcher::GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const
{
	unsigned param0 = (unsigned) dii.params[0];

	if (param0 >= m_trainImages.size())
		return;

	RGBImg trainImg;

	trainImg.deep_copy(m_trainImages[param0]);

	CvMatView trainMat(trainImg);

	const Keypoints& trainPts = m_trainKeypoints[param0];

	if (dii.outputIdx == 0)
	{
		cv::drawKeypoints(trainMat, trainPts, trainMat, cv::Scalar::all(-1), 
			cv::DrawMatchesFlags::DRAW_OVER_OUTIMG);

		dio.imageType = RGB_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(trainImg);
	}
	else if (dii.outputIdx == 1)
	{
		double alpha = dii.params[1];

		const std::vector<cv::DMatch>& m = m_matches;

		std::vector<char> matchesMask(m.size(), 0);

		unsigned minIdx = (unsigned) std::ceil(alpha * m.size());
		unsigned maxIdx = minIdx + (unsigned) std::ceil(0.05 * m.size());

		for (unsigned i = minIdx; i < maxIdx && i < m.size(); i++)
			matchesMask[i] = 1;

		RGBImg queryImg;

		queryImg.deep_copy(m_queryImage);

		CvMatView queryMat(queryImg);
		cv::Mat outImg;
		
		cv::drawMatches(queryMat, m_queryKeypoints, trainMat, trainPts, m, outImg, 
			cv::Scalar(255, 0, 0), cv::Scalar(0, 255, 0), matchesMask);

		cv::cvtColor(outImg, outImg, CV_RGB2BGR);

		cv::imshow("Matches", outImg);
	}
	else if (dii.outputIdx == 2 && !m_mbMapArray.empty())
	{
		RGBImg img;

		img.deep_copy(m_trainImages.back());

		CvMatView mat(img);
		DescriptorInfoPtr ptrDI;
		const double minRel = m_trainImages.size() * m_params.minReliabilityRatio;

		for (auto it = m_bagOfDescriptors.begin(); it != m_bagOfDescriptors.end(); ++it)
		{
			ptrDI = *it;

			if (ptrDI && ptrDI->reliability >= minRel)
			{
				const cv::KeyPoint& kpt = m_trainKeypoints[ptrDI->trainId][ptrDI->row];
			
				cv::circle(mat, kpt.pt, 4, cv::Scalar(255, 0,0));
			}
		}

		/*const MatToBagMap& mbMap = m_mbMapArray.back();

		for (auto mapIt = mbMap.begin(); mapIt != mbMap.end(); ++mapIt)
		{
			minRel = m_mbMapArray.size() / m_params.minReliabilityRatio;

			if (mapIt->second && mapIt->second->reliability >= minRel)
			{
				const cv::KeyPoint& kpt = 
					m_trainKeypoints[mapIt->second->trainId][mapIt->second->row];
			
				cv::circle(mat, kpt.pt, 4, cv::Scalar(255, 0,0));
			}
		}*/

		dio.imageType = RGB_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(img);
	}
}

if (i == 0)
		{
			//InitArrays(1, pMinVals, pMaxVals, pSteps, 0, 
			//	m_trainImages.empty() ? 0 : m_trainImages.size() - 1, 1);
		}
		else if (i == 1)
		{
			InitArrays(2, pMinVals, pMaxVals, pSteps, 0, 
				m_trainImages.empty() ? 0 : m_trainImages.size() - 1, 1);

			pMaxVals->at(1) = 0.95;
			pSteps->at(1) = 0.05;
		}
		else
		{
			VisSysComponent::GetParameterInfo(i, pMinVals, pMaxVals, pSteps);
		}

/*m_matches.resize(m_trainDescriptors.size());

		m_queryImage = m_pImgProcessor->GetRGBImage();
		m_pFeatureDetector->GetKeypoints(m_queryKeypoints);

		CvMatView mat(m_queryImage);

		m_extractor->compute(mat, m_queryKeypoints, m_queryDescriptors);

		for (unsigned i = 0; i < m_trainDescriptors.size(); i++)
		{
			m_matches[i].clear();
			m_matcher->match(m_queryDescriptors, m_trainDescriptors[i], m_matches[i]);
		}*/

void DescriptorMatcher::SelectGoodDescriptors()
{
	const double minRel = m_trainImages.size() * m_params.minReliabilityRatio;

	for (auto it = m_bagOfDescriptors.begin(); it != m_bagOfDescriptors.end(); )
	{
		if (!(*it) && (*it)->reliability < minRel)
			it = m_bagOfDescriptors.erase(it);
		else
			++it;
	}

	m_goodDescriptors.resize(m_bagOfDescriptors.size(), 
		m_trainDescriptors.front().cols);

	int i = 0;

	for (auto it = m_bagOfDescriptors.begin(); it != m_bagOfDescriptors.end(); ++it, ++i)
	{
		m_goodDescriptors.row(i) = m_trainDescriptors[(*it)->trainId].row((*it)->row);
	}
}

/*int i1 = matches1to2[i].queryIdx;
			int i2 = matches1to2[i].trainIdx;

			if( matchesMask.empty() || matchesMask[i] )
			{
				const KeyPoint &kp1 = keypoints1[i1], &kp2 = keypoints2[i2];
				_drawMatch( outImg, outImg1, outImg2, kp1, kp2, matchColor, flags );
			}*/

/*if (m_trainDescriptors.size() == 1)
	{
		DescriptorInfoPtr p;

		for (int i = 0; i < m1.rows; i++)
		{
			p.reset(new DescriptorInfo(id1, i, 1));
			m_bagOfDescriptors.push_back(p);
			queryMap[i] = p;
		}
	}
	else*/

m_trainImages.push_back(m_pImgProcessor->GetRGBImage());
		
		CvMatView mat(m_trainImages.back());

		m_trainKeypoints.push_back(Keypoints());
		m_trainDescriptors.push_back(cv::Mat());

		m_extractor->compute(mat, m_trainKeypoints.back(), m_trainDescriptors.back());

	// Fill the upper triangle and copy it to low triangle
	if (pIndexMap) // using index remapping
	{
		*pIndexMap = RandomArrayPermutation(N);

		
	}
	else // NOT using index remapping
	{
		for (unsigned i = 0; i < N; i++)
		{
			for (unsigned j = i + 1; j < N; j++)
			{
				m(i, j) = Match(mh[i].spg, mh[j].spg);
				m(j, i) = m(i, j);
			}
		}
	}

========================================================================
    STATIC LIBRARY : ShapeMatching Project Overview
========================================================================

AppWizard has created this ShapeMatching library project for you.

No source files were created as part of your project.


ShapeMatching.vcxproj
    This is the main project file for VC++ projects generated using an Application Wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

ShapeMatching.vcxproj.filters
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
