/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include <map>
#include "SkeletalGraph.h"
#include "skeleton.h"  // AFMM skeleton headers
#include <vil/vil_load.h>
#include <Tools/ImageUtils.h>
#include <Tools/UserArguments.h>

using namespace vpl;
using namespace sg;

extern UserArguments g_userArgs;

/*!
	@brief Returns a pointer to a FIELD<float> object with a copy of img.

	The returned pointer must be deleted by the last user of the FIELD.
*/
FIELD<float>* CopyImageToField(FloatImg img)
{
	FIELD<float>* pField = new FIELD<float>(img.ni(), img.nj());
	float* const p = pField->data();
	
	for (unsigned int y = 0; y < img.nj(); y++)
		for (unsigned int x = 0; x < img.ni(); x++)
			*(p + y * img.ni() + x) = img(x, y);

	return pField;
}

/*!
	Reads parameters from global user's arguments
*/
void SkeletalGraphParams::ReadFromUserArguments()
{
	// SkeletalGraph
	g_userArgs.ReadArg("SkeletalGraph", "skeletonAlgorithm", 
		Tokenize("FluxSkeleton,AFMMSkeleton"),
		"Skeletonization algorithm", 0, &nSkelAlgorithm);

	// FluxSkeleton
	DDSGraph::ReadParamsFromUserArguments();

	// AFMMSkeleton
	g_userArgs.ReadArg("AFMMSkeleton", "tau", 
		"Minimum distance from branch endpoint to boundary", 20.0, &afmmTau);

	g_userArgs.ReadArg("AFMMSkeleton", "simplifyExternal", Tokenize("no yes"),
		"Perform external branch simplification", 1, &nSimplifyExternal);

	g_userArgs.ReadArg("AFMMSkeleton", "simplifyInternal", Tokenize("no yes"),
		"Perform internal branch simplification", 0, &nSimplifyInternal);

	g_userArgs.ReadArg("AFMMSkeleton", "bndryWeight", 
		"Boundary reconstruction error weight", 500, &nBndryRecErrorWeight);

	g_userArgs.ReadArg("AFMMSkeleton", "strucWeight", 
		"Structural reconstruction error weight", 500, &nStrucRecErrorWeight);
}

/*!
	@brief Removes the DDSGraph and the BoundaryPointMapping objects
	associated with the skeletal graph.
*/
SkeletalGraph::~SkeletalGraph()
{ 
	delete m_pDDSGraph; 
}

/*!
	@brief Creates a skeleton graph using either a FluxSkeleton algorithm or
	a Augmmented Fast Marching Method algorithm.
*/
bool SkeletalGraph::Create(const ImageInfo& imgInfo, const SkeletalGraphParams& params, 
                           ShapeDims* pDims /*= NULL*/)
{
	delete m_pDDSGraph; // delete any existing graph (if there is one)

	if (params.nSkelAlgorithm == SkeletalGraphParams::FluxAlgorithm)
	{
		if (imgInfo.pField != NULL)
			ShowError("FluxSkeletons can only be computed from an image file");

		return CreateUsingFluxSkeleton(imgInfo.strFileName.c_str(), pDims);
	}
	else if (params.nSkelAlgorithm == SkeletalGraphParams::AFMMAlgorithm)
	{
		return CreateUsingAFMMSkeleton(imgInfo, params, pDims);
	}
	else
	{
		m_pDDSGraph = NULL;
		ShowError("Unknown skeleton algorithm");
		return false;
	}
}

/*!
	@brief Computes the discrete divergence skeleton of a closed curve in a ppm file.
*/
bool SkeletalGraph::CreateUsingFluxSkeleton(const DiscreteXYArray& coords)
{
	std::vector<Point> contour(coords.Size());

	// Copy the points
	for (unsigned int i = 0; i < contour.size(); i++)
		contour[coords.Size() - i - 1].set(coords.xa[i], coords.ya[i]);

	// The shape contour is being made here and
	// it's deleted by the DivergenceSkeletonMaker
	ShapeBoundary* pShape = sg::ShapeMaker::getShape(contour);

	// Use the shape contour to create a skeleton
	m_pDDSGraph = DDSGraph::createDDSGraph(pShape);

	return m_pDDSGraph != NULL;
}

/*!
	@brief Computes the discrete divergence skeleton of a closed curve in a ppm file.
*/
bool SkeletalGraph::CreateUsingFluxSkeleton(const char* szPPMFileName, ShapeDims* pDims)
{
	using namespace sg;
	
	ShowStatus1("Reading", szPPMFileName);
	
	FloatImg image = ConvertToGreyImage(vil_load(szPPMFileName));

	if (image.size() == 0)
	{
		ShowError1("Can't open image file", szPPMFileName);
		return NULL;
	}
	
	sg::ShapeMaker sm(image.ni(), image.nj());

	unsigned int x, y;
	
	// update the shape
	for (y = 0; y < image.nj(); y++)
		for (x = 0; x < image.ni(); x++)
			sm(x,y) = (image(x, y) != 0);
	
	ShowStatus("Computing flux skeleton...");

	// The shape contour is being made here
	ShapeBoundary* pShape = sm.getShape(); // it's deleted by DivergenceSkeletonMaker

	if (pDims)
	{
		pShape->getBounds(&pDims->xmin, &pDims->xmax, 
			&pDims->ymin, &pDims->ymax);
	}

	// Use the shape contour to create a skeleton
	m_pDDSGraph = DDSGraph::createDDSGraph(pShape);

	return m_pDDSGraph != NULL;
}

/*!
	@brief Computes a AFMM Star skeleton from an image in memory
*/
bool SkeletalGraph::CreateUsingAFMMSkeleton(const ImageInfo& imgInfo, const SkeletalGraphParams& params, 
                                       ShapeDims* pDims /*= NULL*/)
{
	SkelCompParams afmmParams; // all params are initialized to valid values

	afmmParams.fThresSimp1 = 0; // deprecated threshold
	afmmParams.fThresSimp2 = (float)params.afmmTau;
	afmmParams.nRecErrorWeightBnd = params.nBndryRecErrorWeight;
	afmmParams.nRecErrorWeightStr = params.nStrucRecErrorWeight;

	// Instead of passing a NULL pField and setting afmmParams.pInputfile = imgInfo.strFileName, 
	// we read the image here so that we can preprocess it and also deal with more image formats. 
	if (imgInfo.pField == NULL)
	{
		FloatImg img = ConvertToGreyImage(vil_load(imgInfo.strFileName.c_str()));

		if (img.size() == 0)
		{
			ShowError1("Can't open image file", imgInfo.strFileName);
			return false;
		}

		// Removes "white holes" and "black satellites" from the image
		//RemoveNonMaxComponents(img);

		afmmParams.pInputField = CopyImageToField(img);
	}
	else
		afmmParams.pInputField = imgInfo.pField;
	
	AFMMSkeleton* pAFMMSGraph = AFMMSkeleton::MakeSkeleton(afmmParams);

	if(pAFMMSGraph)
	{
		// Simplify external branches first
		if(params.nSimplifyExternal == 1)
			pAFMMSGraph->SimplifyExternal();
		
		if(params.nSimplifyInternal == 1)
			pAFMMSGraph->SimplifyInternal();

		if (pDims)
		{
			pAFMMSGraph->GetDimensions(&pDims->xmin, &pDims->xmax, 
				&pDims->ymin, &pDims->ymax);
		}
	}

	m_pDDSGraph = pAFMMSGraph;

	return m_pDDSGraph != NULL;
}

/*!
	Approximates each skeletal branch with a piecewise linear function.
*/
void SkeletalGraph::ApproxSkeletonWithLines(double dMinError, int nMaxSegments)
{
	DDSEdgeVect::iterator edgeIt;
	int i, n;
	
	ASSERT(m_pDDSGraph);
	
	DDSEdgeVect& edges = m_pDDSGraph->getEdges();
	
	m_lineSegments.Resize(edges.size());

	for(edgeIt = edges.begin(), n = 0; edgeIt != edges.end(); edgeIt++, n++)
	{
		FluxPointArray& flux_points = (*edgeIt)->getFluxPoints();
		
		POINTS data(flux_points.size());
		
		for (i = 0; i < data.GetSize(); i++)
			data[i].Set(flux_points[i].p.x, flux_points[i].p.y);
		
		PolyLineTLSApprox poly(data.GetSize() / dMinError, nMaxSegments);
		
		poly.Fit(data);
		
		// Copy all the line segments
		m_lineSegments[n].Resize(poly.m_knots.GetSize());
		
		for (int i = 0; i < poly.m_knots.GetSize(); i++)
			m_lineSegments[n][i] = poly.m_knots[i].seg;
	}
}

/*!
	@brief Approximates each skeleton branch with a piecewise cubic Bezier curve.
	It also sets the tangent at each skeleton point.
*/
void SkeletalGraph::ApproxSkeletonWithCubicBeziers(double dMinError, int nMaxSegments)
{
	DDSEdgeVect::iterator edgeIt;
	int i, n, nSize;
	Point pt;
	
	ASSERT(m_pDDSGraph);
	
	DDSEdgeVect& edges = m_pDDSGraph->getEdges();
	
	m_bezierSegments.Resize(edges.size());

	for(edgeIt = edges.begin(), n = 0; edgeIt != edges.end(); edgeIt++, n++)
	{
		FluxPointArray& fpl = (*edgeIt)->getFluxPoints();
		BoundaryInfoArray& bil = (*edgeIt)->getBoundaryInfoArray();
		nSize = fpl.size();
		
		POINTS data(nSize);
		
		for (i = 0; i < nSize; i++)
			data[i].Set(fpl[i].p.x, fpl[i].p.y);
		
		PolyBezierApprox poly(nSize / dMinError, nMaxSegments);
		
		poly.Fit(data);
		
		// Copy the skeleton approximation so that we can display it later
		m_bezierSegments[n].Resize(poly.m_knots.GetSize());
		
		for (i = 0; i < poly.m_knots.GetSize(); i++)
			m_bezierSegments[n][i] = poly.m_knots[i].seg;
	}
}

void SkeletalGraph::Deserialize(InputStream& is) 
{ 
	ASSERT(!m_pDDSGraph);

	m_pDDSGraph = new DDSGraph();

	m_pDDSGraph->Deserialize(is); 

	// Read piecewise skeletion approximation
	//m_lineSegments.Deserialize(is);
	//m_bezierSegments.Deserialize(is);
}

void SkeletalGraph::Serialize(OutputStream& os) const
{ 
	ASSERT(m_pDDSGraph);

	m_pDDSGraph->Serialize(os); 

	// Write piecewise skeletion approximation
	//m_lineSegments.Serialize(os);
	//m_bezierSegments.Serialize(os);
}
