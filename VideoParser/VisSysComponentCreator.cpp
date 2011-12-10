/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "VisSysComponentCreator.h"
#include <Tools/UserArguments.h>
#include <Tools/Num2StrConverter.h>
#include <Tools/Exceptions.h>

#include <MachineLearning/BoostMap/BoostMapEmbedder.h>
#include <MachineLearning/BehaviorAnalyzer.h>
#include <VideoSegmentation/AffineMotionEstimator/AffineMotionEstimator.h>
#include <VideoSegmentation/CvBackgroundSubtractor/CvFGDBackgroundSubtractor.h>
#include <VideoSegmentation/CvBackgroundSubtractor/CvGaussBackgroundSubtractor.h>
#include <VideoSegmentation/FSMBackgroundSubtractor/FSMBackgroundSubtractor.h>
#include <VideoSegmentation/AdaptiveBackgroundSubtractor.h>
#include <ImageSegmentation/FHSegmenter/FHImageSegmenter.h>
#include <ImageSegmentation/CvPyrImageSegmenter.h>
#include <ImageSegmentation/RegionAnalyzer.h>
#include <ShapeParsing/ShapeParser.h>
#include <ShapeMatching/ShapeMatcher.h>
#include <MachineLearning/ObjectLearner.h>
#include <ObjectRecognition/ObjectRecognizer.h>
#include <ObjectRecognition/CvPeopleDetector.h>
#include "ResultsAnalyzer.h"
#include <Tools/Tuple.h>

// Headers with cv.h in them
#include <ImageSegmentation/BlobFinder.h>
#include <ObjectTracking/KalmanBlobTracker/KalmanBlobTracker.h>
#include <ObjectTracking/MinCutBlobTracker/MinCutBlobTracker.h>
#include <ImageSegmentation/FeatureDetector.h>
#include <VideoSegmentation/BackgroundFeatureSubtractor.h>
#include <VideoSegmentation/FeatureGridSubtractor.h>

using namespace vpl;

extern UserArguments g_userArgs;

char* VisSysComponentCreator::s_defaultNoneArgument = "None";


/*! 
	Populates the given map with all valid component labels.
	It does nothing if the map is NOT empty.
*/
void VisSysComponentCreator::InitVisSysComponentLabels()
{
	if (!m_compLblMap.empty())
		return; // already initialized

	m_compLblMap["ImageProcessor"].push_back("ImageProcessor");

	m_compLblMap["ImageSegmenter"] = Tokenize("None FHImageSegmenter CvPyrImageSegmenter");

	m_compLblMap["FeatureDetector"] = Tokenize("None FeatureDetector");

	//m_compLblMap["DescriptorMatcher"] = Tokenize("None DescriptorMatcher");

	m_compLblMap["RegionAnalyzer"] = Tokenize("None RegionAnalyzer");

	m_compLblMap["BlobFinder"] = Tokenize("None BlobFinder");

	m_compLblMap["ShapeParser"] = Tokenize("None ShapeParser");

	m_compLblMap["MotionEstimator"] = Tokenize("None AffineMotionEstimator");

	m_compLblMap["BackgroundSubtractor"] = Tokenize("None BackgroundFeatureSubtractor "
		"FeatureGridSubtractor HybridFeatureGridSubtractor "
		"AdaptiveBackgroundSubtractor FSMBackgroundSubtractor "
		"CvFGDBackgroundSubtractor CvGaussBackgroundSubtractor");

	m_compLblMap["ObjectLearner"] = Tokenize("None ObjectLearner");

	m_compLblMap["BehaviorAnalyzer"] = Tokenize("None BehaviorAnalyzer");

	m_compLblMap["ObjectRecognizer"] = Tokenize("None ObjectRecognizer");

	m_compLblMap["PeopleDetector"] = Tokenize("None CvPeopleDetector");

	m_compLblMap["ResultsAnalyzer"] = Tokenize("None ResultsAnalyzer");

	m_compLblMap["FeatureEmbedder"] = Tokenize("None BoostMapEmbedder");

	m_compLblMap["ShapeMatcher"] = Tokenize("None ShapeMatcher");

	m_compLblMap["BlobTracker"] = Tokenize("None KalmanBlobTracker MinCutBlobTracker");
}

/*!
	Create an object the requested vision system component class 'name'.

	Note: 'None' is a valid class name which yields a null pointer.
*/
VisSysComponent* VisSysComponentCreator::NewVisSysComponent(
	const std::string& name) const
{
	if (name == "None")
		return NULL;

	// Create the appropriate object
	VisSysComponent* pComp;

	// Create a simple macro to ensure that the indices
	// of the components correspond to the actual class names
#define ADD_IF(C) if (name == #C) pComp = new C
	
	ADD_IF(ImageProcessor);
	else ADD_IF(FHImageSegmenter);
	else ADD_IF(CvPyrImageSegmenter);
	else ADD_IF(FeatureDetector);
	else ADD_IF(BackgroundFeatureSubtractor);
	else ADD_IF(FeatureGridSubtractor);
	else ADD_IF(HybridFeatureGridSubtractor);
	else ADD_IF(RegionAnalyzer);
	else ADD_IF(BlobFinder);
	else ADD_IF(ShapeParser);
	else ADD_IF(ObjectLearner);
	else ADD_IF(BehaviorAnalyzer);
	else ADD_IF(ObjectRecognizer);
	else ADD_IF(CvPeopleDetector);
	else ADD_IF(AffineMotionEstimator);
	else ADD_IF(CvFGDBackgroundSubtractor);
	else ADD_IF(CvGaussBackgroundSubtractor);
	else ADD_IF(FSMBackgroundSubtractor);
	else ADD_IF(AdaptiveBackgroundSubtractor);
	else ADD_IF(ResultsAnalyzer);
	else ADD_IF(BoostMapEmbedder);
	else ADD_IF(ShapeMatcher);
	else ADD_IF(KalmanBlobTracker);
	else ADD_IF(MinCutBlobTracker);
	else THROW_BASIC_EXCEPTION("Unknown vision system component class");

#undef ADD_IF

	return pComp;
}

/*!
	compLblIt is an iterator over the map of component names to
	array of possible instantiations.
*/
std::string VisSysComponentCreator::ReadUserSelection(
	const_iterator compLblIt, const std::string& task) const
{
	std::string filedName = "Components";
	std::string propName = compLblIt->first;
	
	bool readList = false;

	if (g_userArgs.HasField(filedName, propName))
	{
		std::string val = g_userArgs.GetStrValue(filedName, propName);

		readList = (!val.empty() && val.front() == '(');
	}

	StrArray validNames = compLblIt->second;
	const char* helpStr = "Derived component name";

	// If it's not a list, then it is a single enum value
	if (!readList)
	{
		int selection;

		// Read enumeration of possible values
		g_userArgs.ReadArg(filedName, propName, validNames, 
			helpStr, 0, &selection); // default val is 0th option

		return validNames[selection];
	}

	// Read a list of tuples (A0,B0) (A1,B1)...(An,Bn)
	typedef Tuple<std::string, 2, '(', ')'> Params;
	std::list<Params> opts;
	std::vector<StrArray> enums(2);

	enums[0] = m_taskLbls;
	enums[1] = validNames;

	// Read and validate list of tuples
	bool valid = g_userArgs.ReadEnumTupleArgs(filedName, propName, enums,
		helpStr, opts, &opts);

	if (valid)
	{
		ASSERT(contains(m_taskLbls, task));

		// Look for the value pair that corresponds to the requested task
		for (auto it = opts.begin(); it != opts.end(); ++it)
			if (it->at(0) == task)
				return it->at(1);
	}

	return s_defaultNoneArgument;
}

void VisSysComponentCreator::InitVisSysTaskLabels()
{
	if (!m_taskLbls.empty())
		return; // already initialized

	m_taskLbls.push_back("Recognition");
	m_taskLbls.push_back("Learning");
	m_taskLbls.push_back("Offline_learning");
	m_taskLbls.push_back("Offline_testing");
}

