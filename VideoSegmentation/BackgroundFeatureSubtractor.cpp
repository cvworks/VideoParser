/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "BackgroundFeatureSubtractor.h"
#include <VideoParser/ImageProcessor.h>
#include <ImageSegmentation/FeatureDetector.h>
#include <Tools/CvMatView.h>
#include <Tools/UserArguments.h>
#include <Tools/NamedColor.h>
#include <VideoParserGUI/DrawingUtils.h>

#include <Tools/CvUtils.h>

using namespace vpl;

extern UserArguments g_userArgs;

void BackgroundFeatureSubtractor::AddSubordinateComponents()
{
	AddSubordinateComponent(VSCPtr(new FeatureDetector));
}

/*!
	Reads the parameters provided by the user. 

	It is called by BackgroundSubtractor::Initialize(), which
	is in turn calledwhen the vis component is created.
	
	It is NOT an static function because the user is allowed
	to changed the parameters of a vision system component
	several times during the execution of a program. 
*/
void BackgroundFeatureSubtractor::ReadParamsFromUserArguments()
{
	BackgroundSubtractor::ReadParamsFromUserArguments();

	g_userArgs.ReadArg(Name(), "matcherType", Tokenize("simple medium complex"),
		"A descriptor matcher type supported by OpenCV", 
		0, &m_params.matcherTypeOption);

	g_userArgs.ReadArg(Name(), "maxNumMatchesPerFeature",
		"Value of K in the knn search for feature matches", 
		10, &m_params.maxNumMatchesPerFeature);

	g_userArgs.ReadArg(Name(), "minReliability",
		"Ratio of training frames in which a good feature must be present", 
		0.5, &m_params.minReliability);

	g_userArgs.ReadArg(Name(), "maxFeatureJitter",
		"Maximum displacement of any 'good' feature between last detected and current", 
		4.0, &m_params.maxFeatureJitter);

	g_userArgs.ReadBoolArg(Name(), "matchWithMaxDistance",
		"If 'yes', match threshod is (max_dist * coeff), otherwise it's (mean + stdev * coeff)", 
		true, &m_params.matchWithMaxDistance);

	g_userArgs.ReadArg(Name(), "goodMatchStdDevCoeff",
		"Coefficient used to determine good matches: dist <= mean_dist + stdev_dist * coeff", 
		2.5, &m_params.goodMatchStdDevCoeff);

	g_userArgs.ReadArg(Name(), "goodMatchMaxDistCoeff",
		"Coefficient used to determine good matches: dist <= max_dist * coeff", 
		1.1, &m_params.goodMatchMaxDistCoeff);

	g_userArgs.ReadArg(Name(), "unmatchFeatureMaskRadius",
		"Feature disk radius used to create the mask of unmatched features", 
		8, &m_params.unmatchFeatureMaskRadius);

	g_userArgs.ReadBoolArg(Name(), "saveUnmatchedKeypoints",
		"Whether to save the unmatched keypoints or not", 
		true, &m_params.saveUnmatchedKeypoints);

	g_userArgs.ReadArg(Name(), "outputFilename",
		"Name of the file where the results are saved", 
		std::string(), &m_params.outputFilename);

	g_userArgs.ReadArg(Name(), "changeDetectionThreshold",
		"Minimum ratio of matched background features before change is detected", 
		0.85, &m_params.changeDetectionThreshold);
}

/*!
	This function is called by void VSCGraph::Reset()
	before a new video is processed.
*/
void BackgroundFeatureSubtractor::Clear()
{
	BackgroundSubtractor::Clear();

	m_bagOfDescriptors.clear();
	m_mbMapArray.clear();

	m_goodDInfo.clear();

	m_trainKeypoints.clear();
	m_trainDescriptors.clear();
	m_trainImages.clear();

	m_bestMatches.clear();
	m_bestMatchesMask.clear();
	m_goodFeaturesMatched.clear();
	m_queryFeaturesMatched.clear();

	m_queryKeypoints.clear();
	m_queryImage.clear();

	m_roiSeq.clear();
}

/*!
	Now the following matcher types are supported: 

	"BruteForce" (it uses L2), "BruteForce-L1",
	"BruteForce-Hamming", "BruteForce-HammingLUT", "FlannBased".

	You can not match SURF descriptors by Hamming or HammingLUT distances. 
	These distances are used for matching the bit strings stored as uchar vectors only, 
	but SURF descriptor is a float vector (is not binary descriptor). 

	SURF descriptors can be matched by L2 or L1 distances. Hamming or HammingLUT distances 
	are used to match eg. BRIEF descriptors.
*/
void BackgroundFeatureSubtractor::Initialize(graph::node v)
{
	BackgroundSubtractor::Initialize(v);

	m_pImgProcessor = FindParentComponent(ImageProcessor);
	m_pFeatureDetector = FindSubordinateComponent(FeatureDetector);

	if (!m_pFeatureDetector)
		return;

	ASSERT(m_params.matcherTypeOption >= 0 && m_params.matcherTypeOption <= 2);

	m_params.extractorType = m_pFeatureDetector->GetDetectorType();

	if (m_params.extractorType == "FAST")
	{
		m_params.extractorType = "BRIEF"; // the name is different from the detector

		switch (m_params.matcherTypeOption)
		{
			case 0: m_params.matcherType = "BruteForce-Hamming"; break;
			case 1: m_params.matcherType = "BruteForce-HammingLUT"; break;
			case 2: m_params.matcherType = "BruteForce-HammingLUT"; break;
		}
	}
	else if (m_params.extractorType == "SURF")
	{
		switch (m_params.matcherTypeOption)
		{
			case 0: m_params.matcherType = "BruteForce-L1"; break;
			case 1: m_params.matcherType = "BruteForce"; break;
			case 2: m_params.matcherType = "FlannBased"; break;
		}
	}
	else if (m_params.extractorType == "SIFT")
	{
		switch (m_params.matcherTypeOption)
		{
			case 0: m_params.matcherType = "BruteForce"; break;
			case 1: m_params.matcherType = "FlannBased"; break;
			case 2: m_params.matcherType = "FlannBased"; break;
		}
	}
	else if (m_params.extractorType == "GridTrivial")
	{
		m_params.extractorType = "SURF";

		switch (m_params.matcherTypeOption)
		{
			case 0: m_params.matcherType = "BruteForce-L1"; break;
			case 1: m_params.matcherType = "BruteForce"; break;
			case 2: m_params.matcherType = "FlannBased"; break;
		}
	}
	else
	{
		ShowError("No descriptor extractor can be used "
			"with the current feature detector ");

		// The parent component isn't valid
		m_pFeatureDetector = NULL;
	}

	m_extractor = cv::DescriptorExtractor::create(m_params.extractorType);
	m_matcher = cv::DescriptorMatcher::create(m_params.matcherType);

	std::list<UserCommandInfo> cmds;

	GetSwitchCommands(cmds);

	RegisterUserSwitchCommands(4, cmds);
}

void BackgroundFeatureSubtractor::Run()
{
	if (!m_pImgProcessor)
	{
		ShowMissingDependencyError(m_pImgProcessor);
		return;
	}

	if (!m_pFeatureDetector)
	{
		ShowMissingDependencyError(FeatureDetector);
		return;
	}

	if (!m_extractor)
	{
		ShowError("There is no descriptor extractor");
		return;
	}

	ShowStatus("Matching descriptors...");

	// Copy the current roi sequence
	m_roiSeq = GetInputImageInfo().roiSequence;

	// Shallow copy the current image
	m_queryImage = m_pImgProcessor->GetRGBImage();

	// Get the keypoints of the i'th ROI
	m_pFeatureDetector->GetKeypoints(m_queryKeypoints, 0);

	// Get a collection of descriptors as a cv::Mat, where each row 
	// is one keypoint descriptor
	CvMatView mat(m_queryImage);

	m_extractor->compute(mat, m_queryKeypoints, m_queryDescriptors);

	// Call the base class Run() to process the background / foreground frames
	BackgroundSubtractor::Run();
}

void BackgroundFeatureSubtractor::InitializeBackgroundModel(
	unsigned width, unsigned height)
{
}

void BackgroundFeatureSubtractor::ProcessBackgroundFrame(
	RGBImg rgbImg, FloatImg greyImg, fnum_t frameIndex)
{
	m_trainImages.push_back(m_queryImage);
	m_trainKeypoints.push_back(m_queryKeypoints);

	// Note, a clone of the decriptors is needed because the matrix
	// might be reused in next iteration
	m_trainDescriptors.push_back(m_queryDescriptors.clone());	

	AddNewDescriptorsToBag();
}

/*!
	Once we've seen all the train images, select the keypoints and 
	descriptors that we want to keep.
*/
void BackgroundFeatureSubtractor::FinalizeBackgroundModel()
{
	typedef std::pair<double, DescriptorInfoPtr> RelPtrPair;
	std::vector<RelPtrPair> allGood;
	DescriptorInfoPtr ptrDI;

	allGood.reserve(m_bagOfDescriptors.size());

	// Find the info of the current good descriptors
	for (auto it = m_bagOfDescriptors.begin(); it != m_bagOfDescriptors.end(); ++it)
	{
		ptrDI = *it;

		// Compute mean and stddev of matching distances found for this "good" descriptor
		ptrDI->ComputeStats(m_trainImages.size());

		if (ptrDI->reliability >= m_params.minReliability)
			allGood.push_back(std::make_pair(ptrDI->reliability, ptrDI));		
	}

	// Sort features based on their reliability
	std::sort(allGood.begin(), allGood.end(), std::greater<RelPtrPair>());

	// Find average number of features in each image
	double avgNumFeat = 0;

	for (auto it = m_trainKeypoints.begin(); it != m_trainKeypoints.end(); ++it)
		avgNumFeat += it->size();

	avgNumFeat /= m_trainKeypoints.size();

	// Erase any previous good descriptor info
	m_goodDInfo.clear();

	// Find the info of the current good descriptors by copying
	// AT MOST the average number of features found on the training image
	for (size_t i = 0; i <= avgNumFeat && i < allGood.size(); i++)
	{
		//DBG_PRINT1(allGood[i].second->reliability)
		m_goodDInfo.push_back(allGood[i].second);	
	}

	// Create a vector to store the good keypoints
	m_goodKeypoints.resize(m_goodDInfo.size());

	// Create an matrix to store the good descriptors
	m_goodDescriptors.create(m_goodDInfo.size(), m_trainDescriptors.front().cols, 
		m_trainDescriptors.front().type());

	int i = 0;

	// Creates a matrix of good descriptors
	for (auto it = m_goodDInfo.begin(); it != m_goodDInfo.end(); ++it, ++i)
	{
		ptrDI = *it;

		m_trainDescriptors[ptrDI->trainId].row(ptrDI->row).copyTo(m_goodDescriptors.row(i));

		m_goodKeypoints[i] = m_trainKeypoints[ptrDI->trainId][ptrDI->row];
	}
}

void BackgroundFeatureSubtractor::FindForeground(
	RGBImg rgbImg, FloatImg greyImg)
{
	cv::Mat mask = ConstrainPossibleMatches(m_queryKeypoints, m_goodKeypoints);

	std::vector<std::vector<cv::DMatch>> knnMatches;

	m_matcher->knnMatch(m_queryDescriptors, m_goodDescriptors, knnMatches, 
		m_params.maxNumMatchesPerFeature, mask);

	// Resize mask and init all elements
	m_goodFeaturesMatched.assign(m_goodDescriptors.rows, false);
	m_queryFeaturesMatched.assign(m_queryDescriptors.rows, false);

	// Resize mask and init all elements
	m_bestMatchesMask.assign(knnMatches.size(), 0);

	// Make sure that the best matches array has the right size
	m_bestMatches.resize(knnMatches.size());

	for (size_t i = 0; i < knnMatches.size(); i++)
	{
		const std::vector<cv::DMatch>& matches = knnMatches[i];

		for (size_t k = 0; k < matches.size(); k++)
		{
			const cv::DMatch& dm = matches[k];

			if (!m_goodFeaturesMatched[dm.trainIdx] &&
				IsGoodMatch(*m_goodDInfo[dm.trainIdx], dm.distance))
			{
				m_goodFeaturesMatched[dm.trainIdx] = true;
				m_queryFeaturesMatched[dm.queryIdx] = true;

				m_bestMatchesMask[i] = 1;
				m_bestMatches[i] = dm;

				break;
			}
		}
	}
}

void BackgroundFeatureSubtractor::ComputeForegroundStats(RGBImg rgbImage, FloatImg grayImage)
{
	m_stats.numGoodFeaturesMatched = 0;

	for (size_t i = 0; i < m_goodFeaturesMatched.size(); i++)
		if (m_goodFeaturesMatched[i])
			m_stats.numGoodFeaturesMatched++;

	m_stats.matchedGoodFeaturesRatio = (m_goodKeypoints.empty()) ? 1 :	
		double(m_stats.numGoodFeaturesMatched) / m_goodKeypoints.size();

	m_stats.unmatchedQueryFeaturesRatio = (m_goodKeypoints.empty()) ? 0 :	
		double(m_queryKeypoints.size() - m_stats.numGoodFeaturesMatched) 
		/ m_queryKeypoints.size();

	m_stats.changeDetected = !m_goodKeypoints.empty() && 
		m_stats.matchedGoodFeaturesRatio <= m_params.changeDetectionThreshold;
}

void BackgroundFeatureSubtractor::FinalizeForegroundProcessing()
{
	std::vector<cv::KeyPoint> pts;

	pts.reserve(m_goodFeaturesMatched.size() + m_queryFeaturesMatched.size());

	m_foreground.set_size(m_queryImage.ni(), m_queryImage.nj());
	m_foreground.fill(0);

	CvMatView mask(m_foreground);

	for (size_t i = 0; i < m_goodFeaturesMatched.size(); i++)
		if (!m_goodFeaturesMatched[i])
			pts.push_back(m_goodKeypoints[i]);

	size_t numUnmatchedTrain = pts.size();

	for (size_t i = 0; i < m_queryFeaturesMatched.size(); i++)
		if (!m_queryFeaturesMatched[i])
			pts.push_back(m_queryKeypoints[i]);

	size_t numUnmatchedQuery = pts.size() - numUnmatchedTrain;

	// Create mask
	cv::Size sz;

	if (m_pFeatureDetector->GetDetectorType() == "GridTrivial")
	{
		cv::Size2f cell = m_pFeatureDetector->CellSize();

		sz.width = int(cell.width * 1.1);
		sz.height = int(cell.height * 1.1);
	}
	else
	{
		sz = cv::Size(m_params.unmatchFeatureMaskRadius, m_params.unmatchFeatureMaskRadius);
	}

	// Fill mask values
	size_t begin = (m_params.maskUnmatchedTrainFeatures) ? 0 : numUnmatchedTrain;
	size_t end  = (m_params.maskUnmatchedQueryFeatures) ? pts.size() : numUnmatchedTrain;

	for (size_t i = begin; i < end; i++)
		cv::ellipse(mask, pts[i].pt, sz, 0, 0, 360, cv::Scalar(255), -1);

	if (m_params.saveUnmatchedKeypoints && m_stats.changeDetected)
	{
		if (!m_outputFile.is_open())
		{
			// Do not modify the param filename because 'empty' means
			// use the *current* video file name
			std::string fname;

			if (m_params.outputFilename.empty())
			{
				// Remove the "::" chars of a possible subordinate component's name
				fname = replace_str(Name(), "::", "_") + "_" + VideoFilename() 
					+ "_features.txt";
			}
			else
			{
				fname = m_params.outputFilename;
			}

			m_outputFile.open(fname, std::ios::out | std::ios::trunc);

			if (!m_outputFile.is_open())
			{
				ShowOpenFileError2(fname, strerror(errno));
				m_params.saveUnmatchedKeypoints = false;
				return;
			}
		}

		m_outputFile << FrameNumber() << " " << numUnmatchedTrain << " " 
			<< numUnmatchedQuery << " ";

		for (auto it = pts.begin(); it != pts.end(); ++it)
			m_outputFile << it->pt << " ";

		m_outputFile << std::endl;
	}
}

bool BackgroundFeatureSubtractor::IsGoodMatch(const DescriptorInfo& di, 
	const double& distance) const
{
	if (m_params.matchWithMaxDistance)
		return distance <= (di.maxDist * m_params.goodMatchMaxDistCoeff);
	else
	{
		bool res = distance <= (di.meanDist + di.avgDistError * m_params.goodMatchStdDevCoeff);
		
		//if (!res)
		//	DBG_PRINT5(distance, di.meanDist, di.avgDistError, m_params.goodMatchStdDevCoeff, res)
		
			
		return res;
	}
}

/*!
	Uses known matches and 'm_params.maxFeatureJitter' to constrain the possible matches.
*/
cv::Mat BackgroundFeatureSubtractor::ConstrainPossibleMatches(
	const Keypoints& queryPts, const Keypoints& trainPts, 
	const std::vector<bool>& matchedQueryPts,
	const std::vector<bool>& matchedTrainPts) const
{
	cv::Mat mask(queryPts.size(), trainPts.size(), cv::DataType<uchar>::type);
	const double D = m_params.maxFeatureJitter * m_params.maxFeatureJitter;
	bool unmatched;

	for (int i = 0; i < mask.rows; i++)
	{
		for (int j = 0; j < mask.cols; j++)
		{
			unmatched = (matchedQueryPts.empty() || !matchedQueryPts[i]) && 
				(matchedTrainPts.empty() || !matchedTrainPts[j]);

			mask.at<uchar>(i, j) = (unmatched &&
				squaredDistance(queryPts[i], trainPts[j]) <= D);
		}
	}

	return mask;
}

/*!
	@return the number of matches found.
*/
unsigned BackgroundFeatureSubtractor::FindPreviousMatches(
	const unsigned id1, const unsigned id2,	std::vector<bool>& matched1)
{
	const cv::Mat& m1 = m_trainDescriptors[id1];
	const cv::Mat& m2 = m_trainDescriptors[id2];

	MatToBagMap& queryMap = m_mbMapArray[id1];
	MatToBagMap& trainMap = m_mbMapArray[id2];

	const Keypoints& k1 = m_trainKeypoints[id1];
	const Keypoints& k2 = m_trainKeypoints[id2];

	std::vector<cv::DMatch> matches1to2;
	
	cv::Mat mask = ConstrainPossibleMatches(k1, k2, matched1);

	// Match the current and previous descriptors
	m_matcher->match(m1, m2, matches1to2, mask);

	std::vector<bool> matched2(m2.rows, false);
	unsigned numMatchesFound = 0;

	// Note: the match is one-to-many. It seems that it simply finds
	// the closest match for each query point in the training set
	for (size_t i = 0; i < matches1to2.size(); i++)
	{
		const cv::DMatch& match = matches1to2[i];

		ASSERT(!matched1[match.queryIdx]);
				
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
		else
		{
			ptrTrainDI.reset(new DescriptorInfo(id2, match.trainIdx, match.distance));
			m_bagOfDescriptors.push_back(ptrTrainDI);
		}

		queryMap[match.queryIdx] = ptrTrainDI;
		matched1[match.queryIdx] = true;
		matched2[match.trainIdx] = true;
		numMatchesFound++;
	}

	return numMatchesFound;
}

void BackgroundFeatureSubtractor::AddNewDescriptorsToBag()
{
	ASSERT(!m_trainImages.empty());
	ASSERT(m_mbMapArray.size() + 1 == m_trainDescriptors.size());

	m_mbMapArray.push_back(MatToBagMap());

	if (m_trainDescriptors.size() < 2)
		return; // there is nothing else to do

	const unsigned id1 = m_trainDescriptors.size() - 1;
	std::vector<bool> matched1(m_trainDescriptors[id1].rows, false);
	unsigned numMatchesFound = 0;

	// Go back in time until a match is found for each feature
	for (unsigned i = 1; i <= id1 && numMatchesFound < matched1.size(); i++)
		numMatchesFound += FindPreviousMatches(id1, id1 - i, matched1);
}

void BackgroundFeatureSubtractor::Draw(const DisplayInfoIn& dii) const
{
	if (dii.outputIdx == 0)
	{
		if (m_stats.changeDetected && !m_roiSeq.empty())
		{
			DrawSelection(m_roiSeq.front().top_left(), m_roiSeq.front().bottom_right(),
				RGBColor(237, 28, 36));
		}
	}
}

/*!	
	Returns the basic information specifying the output of this component.
	It must provide an image, its type, and a text message. All of this parameters 
	are optional. For example, if there is no output image, the image type
	can be set to VOID_IMAGE.
*/
void BackgroundFeatureSubtractor::GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const
{
	if (m_trainImages.empty())
		return;

	unsigned roiIdx = (unsigned) dii.params[0];

	std::ostringstream oss;

	oss << "Good features matched: " << m_stats.matchedGoodFeaturesRatio * 100 << "% ("
			<< m_stats.numGoodFeaturesMatched << " out of " << m_goodKeypoints.size() << ")\n"
			<< "Unmatched query features: " << m_stats.unmatchedQueryFeaturesRatio * 100 << "% ("
			<< m_queryKeypoints.size() - m_stats.numGoodFeaturesMatched << " out of " 
			<< m_queryKeypoints.size() << ")";

	dio.message = oss.str();

	if (dii.outputIdx == CHANGE_DETECTION)
	{
		// Just show the current image and let Draw() do the rest...
		dio.imageType = RGB_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(m_queryImage);
	}
	else if (dii.outputIdx == BACKGROUND_FEATURES)
	{
		RGBImg trainImg;

		trainImg.deep_copy(m_trainImages.back());

		CvMatView trainMat(trainImg);

		for (auto it = m_goodDInfo.begin(); it != m_goodDInfo.end(); ++it)
		{
			const cv::KeyPoint& kpt = m_trainKeypoints[(*it)->trainId][(*it)->row];
			
			cv::circle(trainMat, kpt.pt, 4, cv::Scalar(255, 0,0));
		}

		dio.imageType = RGB_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(trainImg);
	}
	else if (dii.outputIdx == FEATURE_CORRESPONDENCES)
	{
		RGBImg trainImg;

		trainImg.deep_copy(m_trainImages.back());

		CvMatView trainMat(trainImg);

		RGBImg queryImg;
		cv::Mat outImg;

		queryImg.deep_copy(m_queryImage);
		CvMatView queryMat(queryImg);
		
		cv::drawMatches(queryMat, m_queryKeypoints, 
			trainMat, m_goodKeypoints, m_bestMatches, outImg, 
			cv::Scalar(255, 0, 0), cv::Scalar(0, 255, 0), m_bestMatchesMask);

		cv::cvtColor(outImg, outImg, CV_RGB2BGR);

		cv::imshow("Matches", outImg);

		dio.imageType = RGB_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(trainImg);
	}
	else if (dii.outputIdx == FOREGROUND_FEATURES)
	{
		RGBImg queryImg;

		queryImg.deep_copy(m_queryImage);
		CvMatView queryMat(queryImg);

		for (auto it = m_queryKeypoints.begin(); it != m_queryKeypoints.end(); ++it)	
			cv::circle(queryMat, (*it).pt, 4, cv::Scalar(255, 0,0));

		dio.imageType = RGB_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(queryImg);
	}
	else if (dii.outputIdx == UNMATCHED_MASK)
	{
		dio.imageType = BYTE_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(m_foreground);
	}
	else if (dii.outputIdx == IMAGE_MASK_BLEND)
	{
		RGBImg blendImg;

		blendImg.deep_copy(m_queryImage);

		for (unsigned i = 0; i < m_foreground.ni(); i++)
			for (unsigned j = 0; j < m_foreground.nj(); j++)
				if (!m_foreground(i, j))
					blendImg(i, j) = RGBColor(255, 255, 255);

		dio.imageType = RGB_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(blendImg);
	}
}

//! [static] Update drawing parameters
void BackgroundFeatureSubtractor::GetSwitchCommands(std::list<UserCommandInfo>& cmds)
{
	cmds.push_back(UserCommandInfo("unmatched query", "mask unmatched query features", 'q', 
		&m_params.maskUnmatchedQueryFeatures));

	cmds.push_back(UserCommandInfo("unmatched train", "mask unmatched training features", 't', 
		&m_params.maskUnmatchedTrainFeatures));
}


