/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include <VideoParser/ImageProcessor.h>
#include <VideoSegmentation/BackgroundSubtractor.h>
#include "MinCutBlobTracker.h"
#include "CCExtractor.h"
#include "Observation.h"
#include "CommonUtilities.h"
#include <Tools/BasicUtils.h>
#include <Tools/ColorMatrix.h>
#include <Tools/cv.h>

//#include <MachineLearning/EM/ExpectationMaximizer.h>
//#include <MachineLearning/EM/GaussianMixture.h>

#define TC_DEFAULT_LAMDA1 1
#define TC_DEFAULT_LAMDA2 1
#define TC_DEFAULT_MAX_DISPLACEMENT 30
#define TC_DEFAULT_BLOB_COLOUR_CLUSTERS 3
#define TC_DEFAULT_BLOB_COLOUR_EM_BUFFER 50000
#define TC_DEFAULT_BLOB_MOTION_CLUSTERS 1 // must always be 1
#define TC_DEFAULT_BLOB_MOTION_EM_BUFFER 500
#define TC_DEFAULT_BG_COLOUR_CLUSTERS 3
#define TC_DEFAULT_BG_COLOUR_EM_BUFFER 500000
#define TC_DEFAULT_BG_MOTION_COVARIANCE 0.25

using namespace vpl;

//! Global random number generator
cv::RNG g_rng;

int rectIntersectionArea(cv::Rect & r1, cv::Rect & r2);

template<typename T> struct greaterThanPtr
{
	bool operator()(const T* a, const T* b) const { return *a > *b; }
};

inline void clip_coord(int* val, int minVal, int maxVal)
{
	if (*val >= maxVal)
		*val = maxVal - 1;

	if (*val < minVal)
		*val = minVal;
}

bool rectIntersect(const Rect & A, const Rect & B, Rect & C)
{
    int xMax, xMin, yMax, yMin;
    xMin = (A.x > B.x) ? A.x : B.x;
    yMin = (A.y > B.y) ? A.y : B.y;
    xMax = (A.x + A.width < B.x + B.width) ? A.x + A.width : B.x + B.width;
    yMax = (A.y + A.height < B.y + B.height) ? A.y + A.height : B.y + B.height;
    int w = xMax - xMin, h = yMax - yMin;
    C.x = xMin;
    C.y = yMin;
    C.width = w;
    C.height = h;
    return (w > 0 && h > 0);
}

///////////////////////////////////////////////////////
// begin required functions from parent VisSysComponent
MinCutBlobTracker::MinCutBlobTracker() : m_ccExtractor(new CCExtractor)
{
	// nothing else to do
}

void MinCutBlobTracker::ReadParamsFromUserArguments()
{
	BlobTracker::ReadParamsFromUserArguments();
}

void MinCutBlobTracker::Initialize(graph::node v)
{
	BlobTracker::Initialize(v);

	m_pImgProcessor = FindParentComponent(ImageProcessor);
	m_pBackSub = FindParentComponent(BackgroundSubtractor);

	m_objectCounter = 0;
	m_frameCount = -1;

	m_lamda1 = TC_DEFAULT_LAMDA1;
	m_lamda2 = TC_DEFAULT_LAMDA2;
	m_max_displacement = TC_DEFAULT_MAX_DISPLACEMENT;
	
	m_colourEM.initialize(TC_DEFAULT_BLOB_COLOUR_CLUSTERS, 3,
		TC_DEFAULT_BLOB_COLOUR_EM_BUFFER);

	m_motionEM.initialize(TC_DEFAULT_BLOB_MOTION_CLUSTERS, 2,
		TC_DEFAULT_BLOB_MOTION_EM_BUFFER);

	m_bgColourEM.initialize(TC_DEFAULT_BG_COLOUR_CLUSTERS, 3,
		TC_DEFAULT_BG_COLOUR_EM_BUFFER);
	
	// Set the background motion model assuming static background
	cv::Mat means = cv::Mat::zeros(1, 2, CV_64FC1);
	cv::Mat cov = cv::Mat::zeros(2, 2, CV_64FC1);

	double* c = (double*)cov.data;
	c[0] = c[3] = TC_DEFAULT_BG_MOTION_COVARIANCE;

	std::vector<cv::Mat> covs;

	covs.push_back(cov);

	cv::Mat weights = cv::Mat::ones(1, 1, CV_64FC1);

	m_bgMotionModel.initialize(means, covs, weights);
	
#ifdef _DEBUG
	setBreakOnError(true);
#endif
}

void MinCutBlobTracker::Run()
{
	if (!m_pImgProcessor)
	{
		ShowMissingDependencyError(ImageProcessor);
		return;
	}

	if (!m_pBackSub)
	{
		ShowMissingDependencyError(BackgroundSubtractor);
		return;
	}
	
	m_frameCount++;

	//DBG_PRINT1(m_frameCount)

	// Copy the current frames as previous frame (might be nulls if m_frameCount == 0)
	m_prevImage = m_currentImage;
	m_prevGrayImage = m_grayImage;

	// Get the new frame
	m_currentImage = m_pImgProcessor->GetRGBImage();
	m_grayImage = m_pImgProcessor->GetGreyImage();

	m_objectLabelImage.create(m_currentImage.size(), CV_32SC1);

	m_flowLabelImage.create(m_currentImage.size(), CV_32SC1);

	m_mask = m_pBackSub->Foreground();

	// @todo currently the code needs an image to draw on
	m_outputImage.deep_copy(m_pImgProcessor->GetRGBImage());

	if (m_frameCount == 0)
	{	
		m_flowLabelImage.setTo(Scalar(-1));

		return; // needs at least two frames to start tracking
	}
	else
	{
		CvMatView binImg(m_mask);

		m_ccExtractor->process(binImg);

		SelectGoodCornersToTrack(1000, 0.01, 10);

		// Compute the optical flow
		getOpticalFlow();

		// Get the m_observations
		getObservations();

		// Get the object models
		getObjectModels();

		// Get the binary labelling (foreground or background) of each 
		// pixel w.r.t. each object
		getObjectLabels();

		// Set the Object Labelling Image
		getNewObjects();
	}
}

void MinCutBlobTracker::GetDisplayInfo(const DisplayInfoIn& dii, 
	DisplayInfoOut& dio) const
{
	RGBImg img;
	
	if (dii.outputIdx >= 0 && dii.outputIdx < 2)
		img.deep_copy(m_currentImage.ImgPtr());
	else
		img.deep_copy(m_outputImage.ImgPtr());

	CvMatView mat(img);

	if (dii.outputIdx == 0)
		drawObservations(mat);
	else if (dii.outputIdx == 1)
		drawObjects(mat);
	else if (dii.outputIdx == 2)	
		drawOpticalFlow(mat);

	dio.imageType = RGB_IMAGE;
	dio.imagePtr = ConvertToBaseImgPtr(img);

	std::ostringstream oss;

	oss << "Observations " << m_observations.size() 
		<< ", objects " << m_objects.size()
		<< ".";

	dio.message = oss.str();
}
///////////////////////////////////////////////////////
// end required functions from parent VisSysComponent

void MinCutBlobTracker::getObjectLabels()
{
	for (auto ob = m_objects.begin(), obEnd = m_objects.end(); ob != obEnd; ob++)
	{
		// 1. Build the graph (draws on m_outputImage)
		buildGraph(*ob);

		// 2. Perform min-cut max-flow optimization
		m_graph->maxflow();

		// 3. Set the object's mask based on the optimization
		setObjectMask(*ob);

		// 4. Associate the object to the new m_observations (draws on m_outputImage)
		associateObjectToObservations(*ob);

		// 5. Draw labels on m_outputImage
		drawObjectLabel(*ob, m_outputImage);
	}
}

void MinCutBlobTracker::setObjectMask(ObjectPtr object)
{
	// 1. Get the foreground label bounding box

	// 1.1 Get the minimum y
	int nodeID = 0;
	int yMin = -1;

	for (int y=0; y<m_graphRect.height; y++)
	{
		for (int x=0; x<m_graphRect.width; x++, nodeID++)
		{
			if (m_graph->what_segment(nodeID) == GraphType::SINK)
			{
				yMin = y;
				y = m_graphRect.height;
				break;
			}
		}
	}

	ASSERT(yMin >= 0);

	// 1.2 Get the minimum x
	int xMin = -1;

	for (int x=0; x<m_graphRect.width; x++)
	{
		nodeID = x;
		for (int y=0; y<m_graphRect.height; y++, nodeID+=m_graphRect.width)
		{
			if (m_graph->what_segment(nodeID) == GraphType::SINK)
			{
				xMin = x;
				x = m_graphRect.width;
				break;
			}
		}
	}

	ASSERT(xMin >= 0);

	// 1.3 Get the maximum y
	int yMax = -1;

	for (int y=m_graphRect.height-1; y>=0; y--)
	{
		nodeID = m_graphRect.width*y;
		for (int x=0; x<m_graphRect.width; x++, nodeID++)
		{
			if (m_graph->what_segment(nodeID) == GraphType::SINK)
			{
				yMax = y + 1;
				y = -1;
				break;
			}
		}
	}

	ASSERT(yMax >= 0);

	// 1.4 Get the maximum x
	int xMax = -1;

	for (int x=m_graphRect.width-1; x>=0; x--)
	{
		nodeID = x;
		for (int y=0; y<m_graphRect.height; y++, nodeID+=m_graphRect.width)
		{
			if (m_graph->what_segment(nodeID) == GraphType::SINK)
			{
				xMax = x + 1;
				x = -1;
				break;
			}
		}
	}

	ASSERT(xMax >= 0);

	// 2. Erase the previous object mask
	object->mask(cv::Rect(0,0,object->rect.width, object->rect.height)).setTo(Scalar(0));

	// 3. Set the new object bounding box
	object->rect.x = m_graphRect.x + xMin;
	object->rect.y = m_graphRect.y + yMin;
	object->rect.width = xMax - xMin;
	object->rect.height = yMax - yMin;

	// 4. Set the new mask of the object
	int w = xMax - xMin;
	for (int y=yMin; y<yMax; y++)
	{
		nodeID = m_graphRect.width * y + xMin;
		for (unsigned char *m = object->mask.ptr(y-yMin), *mEnd = m + w; m!=mEnd; m++, nodeID++)
		{
			if (m_graph->what_segment(nodeID) == GraphType::SINK)
			{
				*m = 255;
			}
			else
			{
				*m = 0;
			}
		}
	}
}

void MinCutBlobTracker::associateObjectToObservations(ObjectPtr object)
{
	int nodeID = m_pixelNodeCount;
	ObservationPtr observation;

	for (auto obs = m_consideredObservations.begin(), 
		obsEnd = m_consideredObservations.end(); obs != obsEnd; obs++, nodeID++)
	{
		observation = *obs;

		if (m_graph->what_segment(nodeID) == GraphType::SINK)
		{
			observation->associatedObjects.push_back(object);

			// display the association
			line(m_outputImage, observation->cc->centroid, 
				cv::Point(object->rect.x, object->rect.y),Scalar(255,0,0),2);
		}
	}
}

void MinCutBlobTracker::buildGraph(ObjectPtr object)
{
	// 1. Define the object's search window.
	//    It is the object's bounding box, enlarged on each edge by m_max_displacement, and shifted to the predicted location.
	double xMin = object->rect.x + object->meanFlow.x - m_max_displacement;
	double yMin = object->rect.y + object->meanFlow.y - m_max_displacement;
	double xMax = xMin + object->rect.width + 2 * m_max_displacement;
	double yMax = yMin + object->rect.height + 2 * m_max_displacement;

	// Set the initial values for the m_graph bounding box
	cv::Rect searchWindow((int)xMin, (int)yMin, (int)(xMax-xMin), (int)(yMax-yMin));


	// 3. Get the set of m_observations we will consider for this object
	//    These m_observations must intersect the object's search window.

	int observationEdgeCount = 0;
	m_consideredObservations.clear();
	cv::Rect observationRect;
	int xMaxCandidate, yMaxCandidate;

	for(auto ob = m_observations.begin(), obEnd = m_observations.end(); ob != obEnd; ob++)
	{
		observationRect = (*ob)->cc->rect;
		observationRect.x -= (int)m_max_displacement;
		observationRect.y -= (int)m_max_displacement;
		observationRect.width += 2*(int)m_max_displacement;
		observationRect.height += 2*(int)m_max_displacement;

		if (rectIntersectionArea(observationRect, searchWindow) > 0)
		{
			// Add this observation to the list of considered m_observations
			m_consideredObservations.push_back(*ob);

			// Add the number of observation edges required for this observation
			observationEdgeCount += (*ob)->cc->size;

			// Extend the search window if it the observation goes beyond its boundary
			if (observationRect.x < xMin) xMin = observationRect.x;
			if (observationRect.y < yMin) yMin = observationRect.y;
			xMaxCandidate = observationRect.x + observationRect.width;
			yMaxCandidate = observationRect.y + observationRect.height;
			if (xMaxCandidate > xMax) xMax = xMaxCandidate;
			if (yMaxCandidate > yMax) yMax = yMaxCandidate;
		}
	}

	// 2.  Assert m_graphRect is within the image boundaries
	if (xMin < 0) xMin = 0;
	if (yMin < 0) yMin = 0;
	if (xMax > m_currentImage.cols) xMax = m_currentImage.cols;
	if (yMax > m_currentImage.rows) yMax = m_currentImage.rows;


	// Set the final values for the m_graph bounding box
	m_graphRect.x = (int)xMin;
	m_graphRect.y = (int)yMin;
	m_graphRect.width = int(xMax-xMin);
	m_graphRect.height = int(yMax-yMin);

	cv::rectangle(m_outputImage, m_graphRect, Scalar(255,0,0));

	// Compute the GMM for the backroung in this region
	CvMatView colourBackgroundModel(m_pBackSub->Background());

	m_bgColourEM.fit(colourBackgroundModel(m_graphRect),5);

	ASSERT(m_bgColourEM.getMeans().rows > 0 && m_bgColourEM.getMeans().cols > 0);

	m_bgColourModel.initialize(m_bgColourEM.getMeans(), m_bgColourEM.getCovs(), 
		m_bgColourEM.getWeights());

	m_pixelNodeCount = m_graphRect.width*m_graphRect.height;
	int pixelEdgeCount = 2*m_pixelNodeCount - m_graphRect.height - m_graphRect.width;

	int nodeCount = m_pixelNodeCount + m_consideredObservations.size();
	int edgeCount = pixelEdgeCount + observationEdgeCount;

	m_graph.reset(new GraphType(nodeCount, edgeCount));
	m_graph->add_node(nodeCount);

	setPixelNodeCapacities(object);
	setPixelEdges();
	setObservationNodeCapacities(object);
	setObservationEdges();
}

void MinCutBlobTracker::setPixelNodeCapacities(ObjectPtr object)
{
    // Iterate over the object's predicted region
    // (x,y) are in input image coordinates
    int objectID =  object->id,
            xMin = m_graphRect.x,
            yMin = m_graphRect.y,
            xMax = xMin + m_graphRect.width,
            yMax = yMin + m_graphRect.height;

    double objectProb, backgroundProb;
    double pixel[3];
    int nodeID;

    int _xMin = xMin - ROUND_NUM(object->meanFlow.x),
        _yMin = yMin - ROUND_NUM(object->meanFlow.y);

    for(int y=yMin, _y=_yMin; y<yMax; y++, _y++)
    {
        // Get the starting node ID for this row of pixels
        nodeID = m_graphRect.width*(y - m_graphRect.y) + xMin - m_graphRect.x;

        if (_y >= 0 && _y < m_objectLabelImage.rows)    // if this row, offset by the object's mean optical flow,
                                                        // lies within the object label image
        {
            // Point to the current image in the predicted region
            unsigned char* i = m_currentImage.ptr<unsigned char>(y) + 3*xMin;
            // Point to the optical flow label image in the predicted region
            int* f = m_flowLabelImage.ptr<int>(y) + xMin;

            for(int x=xMin, _x=_xMin; x<xMax; x++, _x++, nodeID++, f++, i+=3)
            {
                if (_x >= 0 && _x < m_objectLabelImage.cols &&
					m_objectLabelImage.at<int>(_y, _x) == objectID) // if this pixel is in the predicted region...
                    //*(m_objectLabelImage.ptr<int>(_y) + _x) == objectID)     // if this pixel is in the predicted region...
                {
                    // Convert the pixel to double values
                    pixel[0]=(double)i[0];
					pixel[1]=(double)i[1];
					pixel[2]=(double)i[2];

                    // Get the probability the pixel belongs to the object's colour GMM
                    objectProb = object->colourModel.prob(pixel);
                    // Get the probability the pixel belongs to the background's colour GMM
                    backgroundProb = m_bgColourModel.prob(pixel);
                    if (*f >= 0)    // If this pixel has an optical flow vector associated with it...
                    {
                        objectProb *= object->motionModel.prob(m_flowVectors[*f]);
                        backgroundProb *= m_bgMotionModel.prob(m_flowVectors[*f]);
                    }
					
                    m_graph->add_tweights(nodeID, -log(objectProb), -log(backgroundProb));
                }
                else
                {
                    // this is setting the weights to -log(0.49) and -log(0.51)
                    m_graph->add_tweights(nodeID, 0.71335, 0.673345);
                }
            }
        }
        else
        {
            // This row does not lie within the previous object label image,
            // so set the entire row to the default setting
            for(int x=xMin; x<xMax; x++, nodeID++)
            {
                m_graph->add_tweights(nodeID, 0.71335, 0.673345);
            }
        }
    }
}

//! Look me up Diego
/*void MinCutBlobTracker::setPixelNodeCapacities(ObjectPtr object)
{
	// Iterate over the object's predicted region
	// (x,y) are in input image coordinates
	int objectID =  object->id,
            xMin = m_graphRect.x,
            yMin = m_graphRect.y,
            xMax = xMin + m_graphRect.width,
            yMax = yMin + m_graphRect.height;

	double objectProb, backgroundProb;
	double pixel[3];
	int nodeID, row, col;

	unsigned counter = 0;

	for(int y=yMin; y<yMax; y++)
	{
		row = y - ROUND_NUM(object->meanFlow.y);
		col = xMin - int(object->meanFlow.x);

		clip_coord(&row, 0, m_currentImage.rows);
		clip_coord(&col, 0, m_currentImage.cols);

		// cv::Point to the object label image
		int* l = m_objectLabelImage.ptr<int>(row) + col;

		// cv::Point to the current image in the predicted region
		unsigned char* i = m_currentImage.ptr<unsigned char>(y) + 3*xMin;

		// cv::Point to the optical flow label image in the predicted region
		int* f = m_flowLabelImage.ptr<int>(y) + xMin;
		// Get the starting node ID for this row of pixels
		nodeID = m_graphRect.width*(y - m_graphRect.y) + xMin - m_graphRect.x;


		for(int x=xMin; x<xMax; x++, nodeID++, l++, f++, i+=3)
		{
			if (*l == objectID)     // if this pixel is in the predicted region...
			{
				counter++;
				// Convert the pixel to double values
				pixel[0]=(double)i[0], pixel[1]=(double)i[1], pixel[2]=(double)i[2];

				// Get the probability the pixel belongs to the object's colour GMM
				objectProb = object->colourModel.prob(pixel);

				// Get the probability the pixel belongs to the background's colour GMM
				backgroundProb = m_bgColourModel.prob(pixel);

				if (*f >= 0)    // If this pixel has an optical flow std::vector associated with it...
				{
					objectProb *= object->motionModel.prob(m_flowVectors[*f]);
					backgroundProb *= m_bgMotionModel.prob(m_flowVectors[*f]);
				}

				m_graph->add_tweights(nodeID, -log(objectProb), -log(backgroundProb));
			}
			else
			{
				// this is setting the weights to -log(0.49) and -log(0.51)
				m_graph->add_tweights(nodeID, 0.71335, 0.673345);
			}
		}
	}

	ASSERT(counter > 0);
}*/

void MinCutBlobTracker::setPixelEdges()
{
	// Get the variance used in the pixel node binary term
	getSigma_t_squared();

	int yMin = m_graphRect.y,
		yMax = yMin + m_graphRect.height - 1,     // second last row
		xMin = m_graphRect.x,
		xMax = xMin + m_graphRect.width - 1;      //  second last column

	int id1=0;
	int id2=m_graphRect.width;
	double B;

	// For all rows except the last...
	for (int y=yMin; y<yMax; y++)
	{
		// cv::Point to the current image
		unsigned char *i = m_currentImage.ptr<unsigned char>(y)   + 3*xMin;
		// cv::Point to the current image, one row below
		unsigned char *d = m_currentImage.ptr<unsigned char>(y+1) + 3*xMin;

		// For all columns except the last...
		for (int x=xMin; x<xMax; x++, id1++, id2++, i+=3, d+=3)
		{
			// Set the binary term for this pixel and its neighbour below
			B = getBinaryTermForPixelNodes(i,d);
			m_graph->add_edge(id1, id2, B, B);

			// Set the binary term for this pixel and its neighbour to the right
			B = getBinaryTermForPixelNodes(i,i+3);
			m_graph->add_edge(id1, id1+1, B, B);
		}

		// For the last column
		// Only set the binary term for the pixel and its neighbour below
		B = getBinaryTermForPixelNodes(i,d);
		m_graph->add_edge(id1,id2,B,B);
		id1++;
		id2++;
	}

	// For the last row
	// For all columns except the last
	unsigned char *i = m_currentImage.ptr<unsigned char>(yMax) + 3*xMin;
	for (int x=xMin; x<xMax; x++, id1++, i+=3)
	{
		// Set the binary term for this pixel and its neighbour to the right
		B = getBinaryTermForPixelNodes(i,i+3);
		m_graph->add_edge(id1, id1+1, B, B);
	}
}

void MinCutBlobTracker::getSigma_t_squared()
{
	unsigned char *i, *d;
	int d1,d2,d3,linesum;
	double sum = 0;

	int yMin = m_graphRect.y,
		yMax = yMin + m_graphRect.height - 1,     // second last row
		xMin = m_graphRect.x,
		xMax = xMin + m_graphRect.width - 1;      //  second last column

	// For all rows except the last
	for (int y=yMin; y<yMax; y++)
	{
		i = m_currentImage.ptr<unsigned char>(y)   + 3*xMin;
		d = m_currentImage.ptr<unsigned char>(y+1) + 3*xMin;
		linesum=0;

		// For all columns except the last
		for (int x=xMin; x<xMax; x++, i+=3, d+=3)
		{
			d1 = i[0] - d[0];
			d2 = i[1] - d[1];
			d3 = i[2] - d[2];

			d1*=d1, d2*=d2, d3*=d3;
			linesum += d1 + d2 + d3;

			d1 = i[0] - d[3];
			d2 = i[1] - d[4];
			d3 = i[2] - d[5];

			d1*=d1, d2*=d2, d3*=d3;
			linesum += d1 + d2 + d3;
		}
		sum += (double)linesum;
	}
	m_sigma_t_squared = 4 * (sum/(double)m_graphRect.area());
	//m_sigma_t_squared *= m_sigma_t_squared;
}

double MinCutBlobTracker::getBinaryTermForPixelNodes(unsigned char *s, unsigned char *r)
{
	double d1, d2, d3, t;
	d1 = s[0]-r[0];
	d2 = s[1]-r[1];
	d3 = s[2]-r[2];
	d1*=d1, d2*=d2, d3*=d3;
	t = d1 + d2 + d3;

	return m_lamda1 * exp(-t/m_sigma_t_squared);
}

void MinCutBlobTracker::setObservationNodeCapacities(ObjectPtr object)
{
	int nodeID = m_pixelNodeCount;
	ObservationPtr observation;
	double KL_fg=1, KL_bg=0;
	for(std::vector<ObservationPtr>::iterator ob = m_consideredObservations.begin(), obEnd = m_consideredObservations.end(); ob != obEnd; ob++, nodeID++)
	{
		observation = *ob;
		//KL_fg = DIV_KL(observation, object);
		//KL_bg = DIV_KL(observation, bgColourModel, bgMotionModel);
		m_graph->add_tweights(nodeID, KL_fg, KL_bg);
	}
}

//float DIV_KL(ObservationPtr observation, ObjectPtr object)
//{
////    KL = sum_i (obj(i) log (obj(i) / obs(i)) )
////    where obj and obs are the GMM of the object and the observation :
////    obj(i) = N( i; \mu,\Sigma).

//    float sum = 0.f;

//}

void MinCutBlobTracker::setObservationEdges()
{
	ObservationPtr observation;
	ConnectedComponentPtr cc;
	unsigned char *i;
	int *label, *flow;

	double pixel[3];
	double B, prob=0.;
	cv::Mat labelImage = m_ccExtractor->getLabelImage();

	int obsID = m_pixelNodeCount;
	int pixelID;
	for (auto ob = m_consideredObservations.begin(), obEnd = m_consideredObservations.end(); ob != obEnd; ob++, obsID++)
	{
		observation = *ob;
		cc = observation->cc;
		int ccID = cc->id;
		int xMin = cc->rect.x, xMax = xMin + cc->rect.width;

		// Scan over the connected component
		for (int y = cc->rect.y, yMax = y + cc->rect.height; y < yMax; y++)
		{
			label = labelImage.ptr<int>(y) + xMin;
			flow = m_flowLabelImage.ptr<int>(y) + xMin;
			i = m_currentImage.ptr<unsigned char>(y) + 3*xMin;
			pixelID = m_graphRect.width * (y-m_graphRect.y) + xMin - m_graphRect.x;

			for(int x=xMin; x<xMax; x++, pixelID++, label++, flow++, i+=3)
			{
				if ( *label == ccID )
				{
					// Add this observation edge
					pixel[0] = i[0], pixel[1] = i[1], pixel[2] = i[2];
					prob = observation->colourModel.prob(pixel);
					if (*flow >= 0)
					{
						ASSERT(*flow < (int)m_flowVectors.size());
						prob *= observation->motionModel.prob(m_flowVectors[*flow]);
					}
					B = m_lamda2 * prob;
					ASSERT(obsID < m_graph->get_node_num());
					ASSERT(pixelID < m_graph->get_node_num());
					m_graph->add_edge(obsID, pixelID, B, B);
				}
			}
		}
	}
}


void MinCutBlobTracker::getNewObjects()
{
	// ****  For now, add a new object for every observation, at every frame *******************
	m_objectLabelImage.setTo(Scalar(-1));
	
	m_objects.clear();
	// ******************************************************************************************

	ConnectedComponentPtr cc;
	ObjectPtr object;
	ObservationPtr observation;

	cv::Mat observationLabelImage = m_ccExtractor->getLabelImage();
	cv::Mat observationLabelROI, objectLabelROI, objectMaskROI;

	int *s, *sEnd, *d;
	Scalar randColour;
	int obsIdx = 0;

	for (auto ob = m_observations.begin(), obEnd = m_observations.end(); ob != obEnd; ob++, obsIdx++)
	{
		observation = *ob;
		cc = observation->cc;
		
		/*randColour[0] = g_rng.uniform(0,255);
		randColour[1] = g_rng.uniform(0,255);
		randColour[2] = g_rng.uniform(0,255);*/

		RGBColor color = ColorMatrix(obsIdx, ColorMatrix::PASTELS);
		randColour[0] = color.r;
		randColour[1] = color.g;
		randColour[2] = color.b;

		object.reset(new Object(m_objectCounter, randColour));

		// Copy Observation properties to the object
		object->rect = cc->rect;
		object->size = cc->size;

		observation->colourModel.copyTo(object->colourModel);

		//qDebug() << "obs" << observation->motionModel.getMeans().rows;
		observation->motionModel.copyTo(object->motionModel);
		object->meanFlow = observation->meanFlow;

		// Define the ROIs
		observationLabelROI = observationLabelImage(cc->rect);
		objectLabelROI = m_objectLabelImage(cc->rect);
		objectMaskROI = object->mask(cv::Rect(0, 0, cc->rect.width, cc->rect.height));

		// Add the object to the object label image and set the object's mask
		int observationID = observation->cc->id;

		for(int y=0, yMax = cc->rect.height; y<yMax; y++)
		{
			s = observationLabelROI.ptr<int>(y), sEnd = s + observationLabelROI.cols;
			d = objectLabelROI.ptr<int>(y);
			unsigned char* m = objectMaskROI.ptr(y);
			for(; s != sEnd; s++, d++, m++)
			{
				if (*s == observationID)
				{
					*d = object->id;
					*m = 255;
				}
			}
		}

		// Add the object to the list of m_objects
		m_objects.push_back(object);
		m_objectCounter++;
	}

	//showObjects();
}

void MinCutBlobTracker::getOpticalFlow()
{
	m_flowPts1.clear();
	m_flowPts2.clear();
	m_flowVectors.clear();
	m_flowLabelImage.create(m_currentImage.size(), CV_32SC1);
	m_flowLabelImage.setTo(Scalar(-1));

	std::vector<cv::Point2f> prevPts;        // value must be float to work with calcOpticalFlowPyrLK
	std::vector<unsigned char> status;
	std::vector<float> err;              // value must be float to work with calcOpticalFlowPyrLK

	calcOpticalFlowPyrLK(m_grayImage, m_prevGrayImage, m_corners, prevPts, status, err);

	// Filter out bad flow vectors
	cv::Point2d p1, p2, d;
	double max_r = m_max_displacement*m_max_displacement;

	int *fl, id=0;
	auto s = status.begin();
	for(auto c = m_corners.begin(), cEnd = m_corners.end(), p = prevPts.begin(); c != cEnd; c++, p++, s++)
	{
		if (*s)
		{
			p1.x = (*p).x;
			p1.y = (*p).y;
			p2.x = (*c).x;
			p2.y = (double)(*c).y;
			d = p2 - p1;

			//            ASSERT(x >= 0 && x < m_grayImage.cols && y >= 0 && y < m_grayImage.rows);
			//            if (x >= 0 && x < m_grayImage.cols && y >= 0 && y < m_grayImage.rows)
			//            {
			if (d.x*d.x + d.y*d.y < max_r)
			{
				m_flowPts1.push_back(p1);
				m_flowPts2.push_back(p2);
				m_flowVectors.push_back(d);
				fl = m_flowLabelImage.ptr<int>((int)(*c).y) + (int)(*c).x;
				*fl = id;
				id++;
			}
			//            }
		}
	}

	//showOpticalFlow();
}

void MinCutBlobTracker::getObservations()
{
	// Clear the old m_observations
	m_observations.clear();

	// Create the new m_observations
	auto connectedComponents = m_ccExtractor->getConnectedComponents();
	
	for (auto cc = connectedComponents.begin(), ccEnd = connectedComponents.end(); cc != ccEnd; cc++)
	{
		m_observations.push_back(ObservationPtr(new Observation(*cc)));
	}

	// For each observation, get the colour and motion model
	ObservationPtr observation;

	for (auto ob = m_observations.begin(), obEnd = m_observations.end(); ob != obEnd; ob++)
	{
		//qDebug()<< "OBS id = " << count;
		observation = *ob;

		// Get the observation's colour Gaussian Mixture Model
		getColourModel(observation);

		// Get the observation's motion Gaussian Mixture Model
		getMotionModel(observation, m_ccExtractor->getLabelImage(), m_flowPts2);
	}

	// Output Observations
	//showObservations();
}

void MinCutBlobTracker::drawObjectLabel(ObjectPtr ptrObj, cv::Mat outImg) const
{
	int nodeID = 0;

	for (int y = 0; y < m_graphRect.height; y++)
	{
		for (unsigned char *i = outImg.ptr<unsigned char>(y+m_graphRect.y) + 3*m_graphRect.x, 
			*iEnd =  i + 3*m_graphRect.width; i<iEnd; nodeID++, i+=3)
		{
			if (m_graph->what_segment(nodeID) == GraphType::SINK)
			{
				i[0] = (unsigned char) ptrObj->colour[0];
				i[1] = (unsigned char) ptrObj->colour[1];
				i[2] = (unsigned char) ptrObj->colour[2];
			}
		}
	}
}

void MinCutBlobTracker::drawObservations(cv::Mat outImg) const
{
	cv::Mat labelImage = m_ccExtractor->getLabelImage();
	
	//diego outImg.create(labelImage.size(), CV_8UC3);
	outImg.setTo(Scalar(0,0,0));
	
	cv::Mat imageROI, labelROI, observationROI;
	
	ObservationPtr observation;
	cv::Point2d p2;
	cv::Rect rect;
	
	for (auto ob = m_observations.begin(), obEnd = m_observations.end(); ob != obEnd; ob++)
	{
		observation = *ob;
		rect = observation->cc->rect;

		//if (rect.x + rect.width >= m_currentImage.rows)
		//	rect.width = m_currentImage.rows - rect.x;
		
		// Cluster the m_observations pixels based on the Gaussian Mixture Model
		imageROI = m_currentImage(rect);
		labelROI = labelImage(rect);
		observationROI = outImg(rect);
		
		observation->colourModel.showClusters(imageROI, observationROI, 
			labelROI, observation->cc->id);
		
		// Draw the mean optical flow std::vector
		
		p2.x = observation->cc->centroid.x + observation->meanFlow.x;
		p2.y = observation->cc->centroid.y + observation->meanFlow.y;
		
		line(outImg, observation->cc->centroid, p2, Scalar(0,0,255), 2);
	}

	//imshow("Observations", m_observationImage);
}

void MinCutBlobTracker::drawObjects(cv::Mat outImg) const
{
	m_currentImage.copyTo(outImg);

	cv::Mat imageROI, labelROI, objectROI;

	ObjectPtr object;
	cv::Point2d p1, p2;
	cv::Rect rect;

	//qDebug() << "Object #  = "<< m_objects.size();

	for (auto ob = m_objects.begin(), obEnd = m_objects.end(); ob != obEnd; ob++)
	{
		object = *ob;
		rect = object->rect;

		// Cluster the m_observations pixels based on the Gaussian Mixture Model
		imageROI = m_currentImage(rect);
		labelROI = m_objectLabelImage(rect);
		objectROI = outImg(rect);

		object->colourModel.showClusters(imageROI, objectROI, labelROI, object->id);

		// Draw the mean optical flow std::vector
		p1.x = rect.x + rect.width/2;
		p1.y = rect.y + rect.height/2;
		p2.x = p1.x + object->meanFlow.x;
		p2.y = p1.y + object->meanFlow.y;

		line(outImg, p1, p2, Scalar(0,0,255), 2);
	}

	//imshow("Objects", m_objectImage);
}

void MinCutBlobTracker::drawOpticalFlow(cv::Mat outImg) const
{
	m_currentImage.copyTo(outImg);

	//Draw All detected m_corners in blue
	for(auto c = m_corners.begin(), cEnd = m_corners.end(); c != cEnd; c++)
	{
		line(outImg, *c, *c, Scalar(255,0,0), 3);
	}

	// Draw All flow vectors in red
	for(auto p1 = m_flowPts1.begin(), p1End = m_flowPts1.end(), p2 = m_flowPts2.begin(); p1 != p1End; p1++, p2++)
	{
		line(outImg, *p1, *p2, Scalar(0,0,255), 3);
	}

	//    //Draw the flow vectors in yellow
	//    for(int y=0; y<m_flowLabelImage.rows; y++)
	//    {
	//        for(int *f = m_flowLabelImage.ptr<int>(y), *fEnd = f + m_flowLabelImage.cols; f!=fEnd; f++)
	//        {
	//            if (*f >= 0)
	//            {
	//                line(m_opticalFlowImage, m_flowPts1[*f], m_flowPts2[*f], Scalar(0,255,255),3);
	//            }
	//        }
	//    }

	//DIEGO imshow("Optical Flow", m_opticalFlowImage);
}

void MinCutBlobTracker::getObjectModels()
{
	ObjectPtr object;

	for (std::vector<ObjectPtr>::iterator ob = m_objects.begin(), obEnd = m_objects.end(); ob != obEnd; ob++)
	{
		object = *ob;

		getColourModel(object);

		//getMotionModel(object);
		getMotionModel(object, m_objectLabelImage, m_flowPts1);
	}
}

void MinCutBlobTracker::getColourModel(ObjectPtr object)
{
	cv::Mat labelROI = m_objectLabelImage(object->rect);
	cv::Mat imageROI = m_prevImage(object->rect);

	// Get the object's colour Gaussian Mixture Model
	m_colourEM.fit(imageROI, labelROI, object->id);

	ASSERT(m_colourEM.getMeans().rows > 0 && m_colourEM.getMeans().cols > 0);

	object->colourModel.initialize(m_colourEM.getMeans(), m_colourEM.getCovs(), m_colourEM.getWeights());
}

void MinCutBlobTracker::getColourModel(ObservationPtr observation)
{
	ConnectedComponentPtr cc = observation->cc;
	cv::Mat labelROI = m_ccExtractor->getLabelImage()(cc->rect);
	cv::Mat imageROI = m_currentImage(cc->rect);

	// Get the observation's colour Gaussian Mixture Model
	m_colourEM.fit(imageROI, labelROI, cc->id);

	ASSERT(m_colourEM.getMeans().rows > 0 && m_colourEM.getMeans().cols > 0);
	observation->colourModel.initialize(m_colourEM.getMeans(), m_colourEM.getCovs(), m_colourEM.getWeights());
}

void MinCutBlobTracker::getMotionModel(BasicBlobPtr blob, const cv::Mat & labelImage, std::vector<cv::Point2d> & flowPts)
{
	// Get the object's motion Gaussian Mixture Model
	int id, blobID = blob->ID();
	std::vector<cv::Point2d> flows;
	cv::Point2d meanFlow;
	int x,y;

	for (auto p = flowPts.begin(), pEnd = flowPts.end(), v = m_flowVectors.begin(); p != pEnd; p++, v++)
	{
		x = (int)(*p).x;
		y = (int)(*p).y;
		if (x >= 0 && x < labelImage.cols && y >= 0 && y < labelImage.rows)
		{
			id = labelImage.ptr<int>(y)[x];
			if (id == blobID)
			{
				flows.push_back(*v);
				meanFlow.x += (*v).x;
				meanFlow.y += (*v).y;
			}
		}
	}

	unsigned flowCount = flows.size();

	if (flowCount == 0)
	{
		cv::Mat means = cv::Mat::zeros(1,2,CV_64FC1);
		cv::Mat cov = cv::Mat::zeros(2,2,CV_64FC1);
		double* c = (double*)cov.data;
		c[0] = c[3] = 10;
		std::vector<cv::Mat> covs;
		covs.push_back(cov);
		cv::Mat weights = cv::Mat::ones(1,1,CV_64FC1);
		blob->motionModel.initialize(means, covs, weights);
	}
	else if (flowCount == 1)
	{
		cv::Mat means(1,2,CV_64FC1);
		double* m = (double*)means.data;
		m[0] = flows.front().x;
		m[1] = flows.front().y;
		cv::Mat cov = cv::Mat::zeros(2,2,CV_64FC1);
		double* c = (double*)cov.data;
		c[0] = c[3] = 2;
		std::vector<cv::Mat> covs;
		covs.push_back(cov);
		cv::Mat weights = cv::Mat::ones(1,1,CV_64FC1);
		blob->motionModel.initialize(means, covs, weights);
	}
	else
	{
		meanFlow.x /= (double)flowCount;
		meanFlow.y /= (double)flowCount;
		m_motionEM.fit(flows);
		blob->motionModel.initialize(m_motionEM.getMeans(), m_motionEM.getCovs(), m_motionEM.getWeights());
	}

	blob->meanFlow = meanFlow;
}


int rectIntersectionArea(cv::Rect & r1, cv::Rect & r2)
{
	int x1,x2,y1,y2,dx,dy;

	x1 = max(r1.x, r2.x);
	x2 = min(r1.x + r1.width, r2.x + r2.width);

	dx = x2-x1;
	if (dx <= 0) return 0;

	y1 = max(r1.y, r2.y);
	y2 = min(r1.y + r1.height, r2.y + r2.height);

	dy = y2-y1;
	if (dy <= 0) return 0;

	return dx*dy;
}

void MinCutBlobTracker::SelectGoodCornersToTrack(
	int maxCorners, double qualityLevel, double minDistance,
	int blockSize, bool useHarrisDetector, double harrisK)
{
	ASSERT( qualityLevel > 0 && minDistance >= 0 && maxCorners >= 0 );

	ASSERT( !m_mask.data || m_mask.size() == m_grayImage.size() );

	CvMatView image(m_grayImage);
	cv::Mat eig, tmp;

	if( useHarrisDetector )
		cornerHarris( image, eig, blockSize, 3, harrisK );
	else
		cornerMinEigenVal( image, eig, blockSize, 3 );

	threshold( eig, eig, qualityLevel, 0, THRESH_TOZERO );
	dilate( eig, tmp, cv::Mat());

	Size imgsize = image.size();

	std::vector<const float*> tmpCorners;

	// collect list of pointers to features - put them into temporary image
	for( int y = 1; y < imgsize.height - 1; y++ )
	{
		const float* eig_data = (const float*)eig.ptr(y);
		const float* tmp_data = (const float*)tmp.ptr(y);

		const uchar* mask_data = m_mask.data ? m_mask.ptr(y) : 0;

		for( int x = 1; x < imgsize.width - 1; x++ )
		{
			float val = eig_data[x];

			if( val != 0 && val == tmp_data[x] && (!mask_data || mask_data[x]) )
				tmpCorners.push_back(eig_data + x);
		}
	}

	cv::sort( tmpCorners, greaterThanPtr<float>() );
	
	size_t i, j, total = tmpCorners.size(), ncorners = 0;

	m_corners.clear();

	if(minDistance >= 1)
	{
		// Partition the image into larger grids
		int w = image.cols;
		int h = image.rows;

		const int cell_size = cvRound(minDistance);
		const int grid_width = (w + cell_size - 1) / cell_size;
		const int grid_height = (h + cell_size - 1) / cell_size;

		std::vector<std::vector<cv::Point2f> > grid(grid_width*grid_height);

		minDistance *= minDistance;

		for( i = 0; i < total; i++ )
		{
			int ofs = (int)((const uchar*)tmpCorners[i] - eig.data);
			int y = (int)(ofs / eig.step);
			int x = (int)((ofs - y*eig.step)/sizeof(float));

			int x_cell = x / cell_size;
			int y_cell = y / cell_size;

			int x1 = x_cell - 1;
			int y1 = y_cell - 1;
			int x2 = x_cell + 1;
			int y2 = y_cell + 1;

			// boundary check
			x1 = MAX(0, x1);
			y1 = MAX(0, y1);
			x2 = MIN(grid_width-1, x2);
			y2 = MIN(grid_height-1, y2);

			bool good = true;

			for( int yy = y1; yy <= y2 && good; yy++ )
			{
				for( int xx = x1; xx <= x2 && good; xx++ )
				{
					std::vector <cv::Point2f> &m = grid[yy*grid_width + xx];

					if( m.size() )
					{
						for(j = 0; j < m.size(); j++)
						{
							float dx = x - m[j].x;
							float dy = y - m[j].y;

							if( dx * dx + dy * dy < minDistance )
							{
								good = false;
								break;
							}
						}
					}
				}
			}

			if (good)
			{
				// printf("%d: %d %d -> %d %d, %d, %d -- %d %d %d %d, %d %d, c=%d\n",
				//    i,x, y, x_cell, y_cell, (int)minDistance, cell_size,x1,y1,x2,y2, grid_width,grid_height,c);
				grid[y_cell*grid_width + x_cell].push_back(cv::Point2f((float)x, (float)y));

				m_corners.push_back(cv::Point2f((float)x, (float)y));
				++ncorners;

				if( maxCorners > 0 && (int)ncorners == maxCorners )
					break;
			}
		}
	}
	else
	{
		for( i = 0; i < total; i++ )
		{
			int ofs = (int)((const uchar*)tmpCorners[i] - eig.data);
			int y = (int)(ofs / eig.step);
			int x = (int)((ofs - y*eig.step)/sizeof(float));

			m_corners.push_back(cv::Point2f((float)x, (float)y));
			++ncorners;
			if( maxCorners > 0 && (int)ncorners == maxCorners )
				break;
		}
	}
}
