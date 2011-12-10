/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>

#include <cstdlib>

#include "misc.h"
#include "filter.h"
//#include "segment-graph.h"

#include "FHImageSegmenter.h"
#include <Tools/cv.h>

#include <Tools/ImageUtils.h>
//#include <Tools/IplImageView.h>
#include <Tools/CvMatView.h>
#include <Tools/LinearAlgebra.h>
#include <Tools/UserArguments.h>
#include <Tools/UserEvents.h>

using namespace vpl;

extern UserArguments g_userArgs;

void FHImageSegmenter::ReadParamsFromUserArguments()
{
	ImageSegmenter::ReadParamsFromUserArguments();
}

void FHImageSegmenter::Initialize(graph::node v)
{
	ImageSegmenter::Initialize(v);

	// The params are constructed with valid default values
	std::list<Params> defVals(1), vals;

	g_userArgs.ReadArgs(Name(), "params", 
		"{sigma, scale, min} for each image segmentation. "
		"'sigma' controls the Gaussian smoothing. Larger 'scale' "
		"sets preference for larger regions. "
		"'min' is minimum region size.", defVals, &vals);

	Params minVal(0, 1, 2);

	g_userArgs.CheckMinValues(Name(), "params", minVal);

	const unsigned int n = vals.size();

	m_segImgs.resize(n);
	m_num_ccs.resize(n, 0);

	m_params.reserve(n);
	m_params.assign(vals.begin(), vals.end());

	// State that we want to serialize the parameters
	g_userArgs.AddSerializableFieldKey(Name());
}

void FHImageSegmenter::Segment(const RGBImg inputImg) 
{
	for (unsigned int i = 0; i < m_segImgs.size(); i++)
	{
		m_num_ccs[i] = 0;

		m_segImgs[i] = segment_image(inputImg, m_params[i].soomthSigma, 
			m_params[i].scalePref, m_params[i].minSize, &m_num_ccs[i]);
	}
}

/*
* Segment an image
*
* Returns a color image representing the segmentation.
*
* im: image to segment.
* sigma: to smooth the image.
* c: constant for treshold function.
* min_size: minimum component size (enforced by post-processing stage).
* num_ccs: number of connected components in the segmentation.
*/
IntImg FHImageSegmenter::segment_image(const RGBImg srcImg, float sigma, 
	float c, int min_size, int* num_ccs) 
{
	const int width = srcImg.ni();
	const int height = srcImg.nj();

	// Note that sigma == 0 in opencv means AUTO sigma
	// so, but we instead consider that sigma < 0 means auto sigma
	// and sigma == 0 means no smoothing
	RGBImg newImg(width, height);

	if (sigma != 0)
	{
		RGBImg rgbImg(srcImg);
		//vil_convert_cast(srcImg, rgbImg); // get rid of const

		CvMatView srcMat(rgbImg), tgtMat(newImg);
		
		cv::GaussianBlur(srcMat, tgtMat, cv::Size(5,5), (sigma < 0) ? 0 : sigma);
	}
	else
	{
		newImg.deep_copy(srcImg);
	}

	m_srcRGBImg = newImg;

	// Copy and normalize
	/*LabImg rgbImg(width, height);
	forall_cells2(rgbImg, srcImg)
	{
		it0->r = (float)(it1->r / 255.0);
		it0->g = (float)(it1->g / 255.0);
		it0->b = (float)(it1->b / 255.0);
	}
	LabImg newImg(width, height);
	

	// Note that sigma == 0 in opencv means AUTO sigma
	// so, use sigma < 0 to get the auto sigma
	if (sigma != 0)
	{
		IplImageView cvImg1(rgbImg);
		IplImageView cvImg2(newImg);

		cvCvtColor(cvImg1, cvImg2, CV_RGB2Lab);

		cvSmooth(cvImg2, cvImg2, CV_GAUSSIAN, 3, 0, (sigma < 0) ? 0 : sigma);
	}
	
	m_srcLabImg = newImg;

	*/

	///////////////////////

	m_gs.init(width * height, width * height * 4);

	int numEdges = CreateGraph(m_gs.edges(), m_srcRGBImg, sigma);

	m_gs.segment_graph(numEdges, c);

	m_gs.postProcessSmallComponents(numEdges, min_size);

	*num_ccs = m_gs.num_sets();

	IntImg outImg(width, height);
	std::vector<int> regionIds(width * height, -1);
	int comp, counter = 0;

	for (int y = 0; y < height; y++) 
	{
		for (int x = 0; x < width; x++) 
		{
			comp = m_gs.find(y * width + x);

			int& id = regionIds[comp];

			if (id == -1)
				id = counter++;

			outImg(x, y) = id;
		}
	}

	return outImg;
}

int FHImageSegmenter::CreateGraph(FHGraphSegmenter::Edges& edges, 
	RGBImg img, double sigma) const
{
	ASSERT(img.ni() * img.nj() <= edges.size())

	const int width = img.ni();
	const int height = img.nj();

	int num = 0;

	for (int y = 0; y < height; y++) 
	{
		for (int x = 0; x < width; x++) 
		{
			if (x < width-1) 
			{
				edges[num].a = y * width + x;
				edges[num].b = y * width + (x+1);
				edges[num].w = (float)ColorDiff(img(x, y), img(x+1, y));
				num++;
			}

			if (y < height-1) {
				edges[num].a = y * width + x;
				edges[num].b = (y+1) * width + x;
				edges[num].w = (float)ColorDiff(img(x, y), img(x, y+1));
				num++;
			}

			if ((x < width-1) && (y < height-1)) {
				edges[num].a = y * width + x;
				edges[num].b = (y+1) * width + (x+1);
				edges[num].w = (float)ColorDiff(img(x, y), img(x+1, y+1));
				num++;
			}

			if ((x < width-1) && (y > 0)) {
				edges[num].a = y * width + x;
				edges[num].b = (y-1) * width + (x+1);
				edges[num].w = (float)ColorDiff(img(x, y), img(x+1, y-1));
				num++;
			}
		}
	}

	return num;
}

/*!
	This function is called when the left mouse botton and
	the mouse pointer was on the component's view.

	@return zero if the even was not dealt with and 1 otherwise.
*/
bool FHImageSegmenter::OnGUIEvent(const UserEventInfo& uei)
{
	static LabColor prevCol(100,0,0);

	if (uei.id == EVENT_MOUSE_PUSH)
	{
		ASSERT(!uei.params.empty());

		unsigned segmentIdx = (unsigned) uei.params[0];

		// See if the click is within the image limits
		if (uei.coord.x >= m_srcRGBImg.ni() || uei.coord.y >= m_srcRGBImg.nj())
			return false;

		LabColor col = m_srcRGBImg((unsigned)uei.coord.x, 
			(unsigned)uei.coord.y);

		std::cout << "Current color: " << col << std::endl;
		std::cout << "Previous color: " << prevCol << std::endl;
		std::cout << "Diff: " << ColorDiff(col, prevCol) << "\n" << std::endl;

		prevCol = col;
	}

	return true;
}
