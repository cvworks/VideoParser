#pragma once

#include "../BlobTracker.h"
#include <vector>
#include <memory>
#include <GraphTheory/MaxFlow/FlowGraph.h>
#include <Tools/ImageUtils.h>
#include <Tools/CvMatView.h>

#include <MachineLearning/EM/ExpectationMaximizer.h>
#include <MachineLearning/EM/GaussianMixture.h>

typedef maxflow::FlowGraph<double,double,double> GraphType;

#define OBJECT_MODEL_BUFFER_WIDTH 250
#define OBJECT_MODEL_BUFFER_HEIGHT 300

struct Object;
struct BasicBlob;
struct Observation;
class CCExtractor;
class BackgroundSubtractor;

namespace vpl {

class MinCutBlobTracker : public BlobTracker
{
public:
	typedef std::shared_ptr<GraphType> GraphTypePtr;
	typedef std::shared_ptr<Object> ObjectPtr;
	typedef std::shared_ptr<BasicBlob> BasicBlobPtr;
	typedef std::shared_ptr<Observation> ObservationPtr;

protected:
	std::shared_ptr<const BackgroundSubtractor> m_pBackSub;

	struct Params {
		
	};

private:
	void SelectGoodCornersToTrack(int maxCorners, double qualityLevel, 
		double minDistance,	int blockSize = 3, bool useHarrisDetector = false, 
		double harrisK = 0.04);

    void getOpticalFlow();

    void getMotionModel(BasicBlobPtr blob, const cv::Mat & labelImage, std::vector<cv::Point2d> & flowPts);
    
	void getObservations();
        void getColourModel(ObservationPtr observation);

    void getObjectModels();
        void getColourModel(ObjectPtr object);

    void getObjectLabels();
        void buildGraph(ObjectPtr object);
            void setPixelNodeCapacities(ObjectPtr object);
            void setObservationNodeCapacities(ObjectPtr object);
            void setPixelEdges();
                void getSigma_t_squared();
                double getBinaryTermForPixelNodes(unsigned char *s, unsigned char *r);
            void setObservationEdges();
        void setObjectMask(ObjectPtr object);
        void associateObjectToObservations(ObjectPtr object);

    void getNewObjects();

    // Modules
    //BackgroundSegmentor backSeg;               //!< background segmentation
    std::shared_ptr<CCExtractor> m_ccExtractor;  //!< connected component extraction
    ExpectationMaximizer m_colourEM;             //!< expectation maximization for the colour models of objects
    ExpectationMaximizer m_motionEM;             //!< expectation maximization for the motion models of objects
    ExpectationMaximizer m_bgColourEM;           //!< expectation maximization for the colour model for the background

    // Images
    CvMatView m_currentImage, m_prevImage; // RGB
	CvMatView m_grayImage, m_prevGrayImage; // float
    cv::Mat m_objectLabelImage, m_flowLabelImage; // int
	CvMatView m_mask; // byte

	ByteImg GetBlobMask() const
	{
		return m_mask.ImgPtr();
	}

    // Variables
    int m_frameCount;
    int m_objectCounter;              // When new objects are initilized, their ID is set to objectCounter
    std::vector<ObjectPtr> m_objects;        // list of current objects
    std::vector<ObservationPtr> m_observations;              // list of observations (connected components)
    std::vector<ObservationPtr> m_consideredObservations;    // the observations considered for the object for which we are updating the labels
    std::vector<cv::Point2f> m_corners;        // list of corners obtained from SelectGoodCornersToTrack()
    std::vector<cv::Point2d> m_flowPts1,       // list of corresponding points in the previous image found by optical flow
                    m_flowPts2,       // list of corresponding points in the current image (equal to corners)
                    m_flowVectors;    // flowPts2 - flowPts1

    // Graph Properties
    GraphTypePtr m_graph;           // the graph
    cv::Rect m_graphRect;             // the graphs rectangle (its ROI) in the image
    int m_pixelNodeCount;         // number of pixel nodes = area of graphRect
    double m_sigma_t_squared;      // variance used in the binary term between neighbouring pixels

    // Background Properties
    GaussianMixture3D m_bgColourModel;
    GaussianMixture2D m_bgMotionModel;

    // Track and Cut Parameters
    unsigned int m_max_displacement;      // we assume objects will not move more than this amount between frames
    double m_lamda1, m_lamda2;
    //unsigned int m_blobColourClusters, m_blobMotionClusters, m_blobEMBufferSize,
    //             m_bgColourClusters, m_bgMotionClusters, m_bgEMBufferSize;

    // ***** For displaying results ***************************
    void drawObjects(cv::Mat outImg) const;
    void drawObservations(cv::Mat outImg) const;
    void drawOpticalFlow(cv::Mat outImg) const;
    void drawObjectLabel(ObjectPtr ptrObj, cv::Mat outImg) const;

    CvMatView m_outputImage; // rgb
    //cv::Mat m_observationImage;
    //cv::Mat m_objectImage;
    //cv::Mat m_opticalFlowImage;
    //cv::Mat m_labelOutputImage;
    // ********************************************************

public: // Required functions from parent VisSysComponent
	MinCutBlobTracker();

	virtual void ReadParamsFromUserArguments();

	virtual void Initialize(graph::node v);

	//virtual void Clear();

	virtual std::string ClassName() const
	{
		return "MinCutBlobTracker";
	}

	virtual StrArray Dependencies() const
	{
		StrArray deps;
		
		deps.push_back("ImageProcessor");
		deps.push_back("BackgroundSubtractor");

		return deps;
	}
	
	virtual void Run();

	virtual void GetDisplayInfo(const DisplayInfoIn& dii, 
		DisplayInfoOut& dio) const;

	/*virtual void GetParameterInfo(int i, DoubleArray* pMinVals, 
		DoubleArray* pMaxVals, DoubleArray* pSteps) const
	{
		InitArrays(1, pMinVals, pMaxVals, pSteps, 0, 1, 1);
	}*/
	
	virtual int NumOutputImages() const 
	{ 
		return 4; 
	}

	virtual std::string GetOutputImageLabel(int i) const
	{
		ASSERT(i >= 0 && i < 4);

		switch (i)
		{
		case 0: return "Observations";
		case 1: return "Objects";
		case 2: return "Optical Flow";
		case 3: return "Object Labels";
		}

		return "error";
	}
};

} // namespace vpl
