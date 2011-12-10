/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "BackgroundSubtractor.h"
#include <Tools/STLUtils.h>
#include <Tools/cv.h>

namespace vpl {

// Declare prototypes of expected parent components
class ImageProcessor;
class FeatureDetector;

/*!
	@brief Wrapper for a generic object recognition algorithm
*/
class BackgroundFeatureSubtractor : public BackgroundSubtractor
{
	enum {CHANGE_DETECTION, FOREGROUND_FEATURES, BACKGROUND_FEATURES,
		FEATURE_CORRESPONDENCES, UNMATCHED_MASK, IMAGE_MASK_BLEND};

	struct Params {
		std::string extractorType;
		int matcherTypeOption;
		std::string matcherType;
		double minReliability;
		double maxFeatureJitter;
		bool matchWithMaxDistance;
		double goodMatchStdDevCoeff;
		double goodMatchMaxDistCoeff;
		int unmatchFeatureMaskRadius;
		bool saveUnmatchedKeypoints;
		std::string outputFilename;
		int maxNumMatchesPerFeature;
		double changeDetectionThreshold;

		// GUI params
		bool maskUnmatchedQueryFeatures;
		bool maskUnmatchedTrainFeatures;

		Params()
		{
			maskUnmatchedQueryFeatures = false;
			maskUnmatchedTrainFeatures = true;
		}
	};

	struct DescriptorInfo
	{
		unsigned trainId;
		int row;

		std::list<double> distances;

		double meanDist;
		double avgDistError;
		double maxDist;
		double reliability;

		DescriptorInfo(unsigned tid, int r, const double& first_dist) 
			: distances(1, first_dist)
		{
			trainId = tid;
			row = r;

			meanDist = 0;
			avgDistError = 0;
			maxDist = 0;
			reliability = 0;
		}

		DescriptorInfo(const DescriptorInfo& rhs)
		{
			operator=(rhs);
		}

		void operator=(const DescriptorInfo& rhs)
		{
			trainId     = rhs.trainId;
			row         = rhs.row;
			distances   = rhs.distances;

			meanDist     = rhs.meanDist;
			avgDistError = rhs.avgDistError;
			maxDist      = rhs.maxDist;
			reliability  = rhs.reliability;
		}

		void ComputeStats(unsigned numTrainImages)
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
			if (avgDistError < 0.05 * meanDist)
				avgDistError = 0.05 * meanDist;

			reliability = distances.size() / double(numTrainImages);
		}
	};

	typedef std::shared_ptr<DescriptorInfo> DescriptorInfoPtr;
	typedef std::map<int, DescriptorInfoPtr> MatToBagMap;

protected:
	typedef std::vector<cv::KeyPoint> Keypoints;
	typedef cv::Ptr<cv::DescriptorExtractor> Extractor;
	typedef cv::Ptr<cv::DescriptorMatcher> Matcher;

	//Parent components
	std::shared_ptr<const ImageProcessor> m_pImgProcessor;
	std::shared_ptr<const FeatureDetector> m_pFeatureDetector;

	Params m_params;

	Extractor m_extractor;
	Matcher m_matcher;

	std::list<DescriptorInfoPtr> m_bagOfDescriptors;
	std::vector<MatToBagMap> m_mbMapArray;

	std::vector<DescriptorInfoPtr> m_goodDInfo;
	cv::Mat m_goodDescriptors;
	Keypoints m_goodKeypoints;

	std::vector<Keypoints> m_trainKeypoints;
	std::vector<cv::Mat> m_trainDescriptors;
	std::vector<RGBImg> m_trainImages;

	//std::vector<std::vector<cv::DMatch>> m_bestMatches;
	std::vector<cv::DMatch> m_bestMatches;
	std::vector<char> m_bestMatchesMask;
	std::vector<bool> m_goodFeaturesMatched;
	std::vector<bool> m_queryFeaturesMatched;

	Keypoints m_queryKeypoints;
	cv::Mat m_queryDescriptors;
	RGBImg m_queryImage;
	ByteImg m_unmatchedMask;

	ROISequence m_roiSeq;

	std::fstream m_outputFile;

	unsigned FindPreviousMatches(const unsigned id1, const unsigned id2,
		std::vector<bool>& matched1);

	void AddNewDescriptorsToBag();

	cv::Mat ConstrainPossibleMatches(
		const Keypoints& queryPts, const Keypoints& trainPts, 
		const std::vector<bool>& matchedQueryPts = std::vector<bool>(),
		const std::vector<bool>& matchedTrainPts = std::vector<bool>()) const;

	bool IsGoodMatch(const DescriptorInfo& di, const double& distance) const;

	// Pure virtual functions in the parent class that must be defined
	virtual void InitializeBackgroundModel(unsigned width, unsigned height);
	virtual void ProcessBackgroundFrame(RGBImg rgbImg, FloatImg greyImg, fnum_t frameIndex);
	virtual void FindForeground(RGBImg rgbImg, FloatImg greyImg);
	virtual void FinalizeBackgroundModel();
	virtual void ComputeForegroundStats(RGBImg rgbImage, FloatImg grayImage);
	virtual void FinalizeForegroundProcessing();

public:	
	BackgroundFeatureSubtractor() 
		: BackgroundSubtractor(BackgroundStats::FEATURE_BASED)
	{
	}

	double squaredDistance(const cv::KeyPoint& kpt1, 
		const cv::KeyPoint& kpt2) const
	{ 
		double dx = kpt1.pt.x - kpt2.pt.x;
		double dy = kpt1.pt.y - kpt2.pt.y;

		return (dx * dx + dy * dy); 
	}

	virtual void ReadParamsFromUserArguments();

	virtual void AddSubordinateComponents();

	virtual void Initialize(graph::node v);

	virtual void Clear();

	virtual std::string ClassName() const
	{
		return "BackgroundFeatureSubtractor";
	}

	virtual StrArray Dependencies() const
	{
		StrArray deps;

		deps.push_back("ImageProcessor");
		deps.push_back("FeatureDetector");

		return deps;
	}
	
	virtual void Run();

	virtual void Draw(const DisplayInfoIn& dii) const;

	virtual void GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const;

	virtual void GetParameterInfo(int i, DoubleArray* pMinVals, 
		DoubleArray* pMaxVals, DoubleArray* pSteps) const
	{
		InitArrays(1, pMinVals, pMaxVals, pSteps, 0, 
			m_roiSeq.size(), 1);
	}
	
	virtual int NumOutputImages() const 
	{ 
		return 6; 
	}

	virtual std::string GetOutputImageLabel(int i) const
	{
		switch (i)
		{
			case CHANGE_DETECTION: return "Change detection";
			case FOREGROUND_FEATURES: return "Foreground features";
			case BACKGROUND_FEATURES: return "Background features";
			case FEATURE_CORRESPONDENCES: return "Feature correspondences";
			case UNMATCHED_MASK: return "Unmatched mask";
			case IMAGE_MASK_BLEND: return "Blend";
		}

		return "error";
	}

	virtual void GetSwitchCommands(std::list<UserCommandInfo>& cmds);
};

} // namespace vpl

