/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "FluxSkeleton/include/DDSGraphProject.h"
#include "FluxSkeleton/include/DDSGraphUtils.h"

#include <fstream>

#include <Tools/PiecewiseApprox/PolyLineTLSApprox.h>
#include <Tools/PiecewiseApprox/PolyBezierApprox.h>
#include <Tools/Serialization.h>

template<class T> class FIELD; // for AFMM skeleton in ImageInfo

namespace vpl {

struct ImageInfo
{
	std::string strFileName;
	FIELD<float>* pField;
	double noiseLevel;
	
	ImageInfo() { noiseLevel = 0; pField = NULL; }
	ImageInfo(const char* fn) : strFileName(fn) { noiseLevel = 0; pField = NULL; }
};

//! SkeletalGraph parameters for construction
struct SkeletalGraphParams
{	
	enum SKEL_ALGO {FluxAlgorithm, AFMMAlgorithm, VoronoiAlgorithm};

	int nSkelAlgorithm;

	//AFMM Skeleton Params
	double afmmTau;
	int nBndryRecErrorWeight, nStrucRecErrorWeight;
	int nSimplifyExternal, nSimplifyInternal;
	
	SkeletalGraphParams() { memset(this, 0, sizeof(SkeletalGraphParams)); }

	void ReadFromUserArguments();
};

/*!
	The SkeletalGraph class wrapps and extends the DDSGraph (aka FluxSkeleton)
	in the FluxSkeleton project. Currently, the AFMMSkeleton class inherits
	from the DDSGraph and so the wrapper may contain a FluxSkeleton
	or an AFMMSkeleton.
*/
class SkeletalGraph
{
protected:
	sg::DDSGraph* m_pDDSGraph;              //!< It may point to a DDSGraph or a AFMMSkeleton

	SmartArray<EuclideanLineSegmentArray> m_lineSegments;
	SmartArray<BezierSegmentArray> m_bezierSegments;

public:
	SkeletalGraph(sg::DDSGraph* p = NULL) 
	{ 
		m_pDDSGraph = p; 
	}
	
	~SkeletalGraph();

	bool Create(const ImageInfo& imgInfo, const SkeletalGraphParams& params, 
		ShapeDims* pDims = NULL);

	bool CreateUsingFluxSkeleton(const DiscreteXYArray& coords);

	bool CreateUsingFluxSkeleton(const char* szPPMFileName, ShapeDims* pDims = NULL);

	bool CreateUsingAFMMSkeleton(const ImageInfo& imgInfo, const SkeletalGraphParams& params, 
		ShapeDims* pDims = NULL);

	bool Regularize();

	const sg::DDSGraph* GetDDSGraph() const { return m_pDDSGraph; }
	sg::DDSGraph* GetDDSGraph() { return m_pDDSGraph; }
	//void SetSkeleton(sg::DDSGraph* p) { m_pDDSGraph = p; }

	void Serialize(OutputStream& os) const;
	void Deserialize(InputStream& is);
	
	void ApproxSkeletonWithLines(double dMinError, int nMaxSegments);
	void ApproxSkeletonWithCubicBeziers(double dMinError, int nMaxSegments);

	// Inline functions for display purposes
	SmartArray<EuclideanLineSegmentArray> GetLinearApproximation() const 
	{ 
		return m_lineSegments; 
	}

	void SetLinearApproximation(SmartArray<EuclideanLineSegmentArray> lineSegs)
	{ 
		m_lineSegments = lineSegs; 
	}

	SmartArray<BezierSegmentArray> GetBezierApproximation() const 
	{ 
		return m_bezierSegments; 
	}

	void SetBezierApproximation(SmartArray<BezierSegmentArray> bezierSegs) 
	{ 
		m_bezierSegments = bezierSegs; 
	}

	//!	Transfers ownership of the shape boundary to the caller
	sg::ShapeBoundary* ReleaseShapeBoundary()
	{
		return m_pDDSGraph->releaseShape();
	}

	//!	Returns a const pointer to the shape boundary
	const sg::ShapeBoundary* GetShapeBoundary() const
	{
		return m_pDDSGraph->getShape();
	}

	//!	Returns the number of points along the shape boundary
	unsigned int GetBoundarySize() const
	{
		return m_pDDSGraph->getShape()->size();
	}

	void GetBounds(double *xmin, double *xmax,
		double *ymin, double *ymax) const
	{
		ASSERT(m_pDDSGraph);

		m_pDDSGraph->getBounds(xmin, xmax, ymin, ymax);
	}

	//! Assign bndry info to each skeletal point
	/*void AssignBoundaryInfo()
	{
		m_pDDSGraph->SetBoundaryPoints();
	}*/

	const sg::SkelNodes& getNodes() const { return m_pDDSGraph->getNodes(); }
	sg::SkelNodes& getNodes()             { return m_pDDSGraph->getNodes(); }

	const sg::SkelEdges& getEdges() const { return m_pDDSGraph->getEdges(); }
	sg::SkelEdges& getEdges()             { return m_pDDSGraph->getEdges(); }
};

} // namespace vpl

