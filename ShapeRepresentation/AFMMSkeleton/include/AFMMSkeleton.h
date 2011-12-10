/*
	This class will implement the method necessary for 
	simplifying the skeleton structure

	author: Matthijs von Eede
*/

#ifndef _AFMMSKELETON_H_
#define _AFMMSKELETON_H_

#include <list>
#include <memory>
#include "DDSGraph.h"
#include "DivergenceSkeletonMaker.h"
#include "SkelCompParams.h"
#include "ShapeDiff.h"
#include "Tools/BasicUtils.h"

//typedef sg::DDSEdge sgDDSEdge;
//typedef std::pair< sg::DDSEdge*, double> DDSEdgeAndError;


struct DDSEdgeAndError : public std::pair<sg::DDSEdge*, double>
{
	bool operator < (const DDSEdgeAndError& rhs) const
	{
		return (second < rhs.second);
	}
};

//first argument will hold the index of the edge in 'edges' the
//second argument will hold the number of points in that edge
//which will be used as a measure for the length of an edge
struct IndexAndLength : public std::pair<int,int>
{
	bool operator < (const IndexAndLength& rhs) const
	{
		return (second < rhs.second);
	}
};

typedef std::list<IndexAndLength> InternalEdgeList;
typedef FIELD<float> DFIELD;
typedef std::list<sg::DDSEdge*> DDSEdgeList;
typedef std::pair<DDSEdgeList, double> DDSkelAndError;
typedef std::list<DDSEdgeAndError> DDSEdgeAndErrorList;
typedef std::list<DDSkelAndError> DDSkelAndErrorList;
typedef std::list<sg::DDSNode*> DDSNodeList;
typedef sg::DivergenceMap DMap;
typedef std::shared_ptr<DDSNodeList> DDSNodeListPtr;
typedef std::shared_ptr<DDSEdgeList> DDSEdgeListPtr;
typedef std::shared_ptr<DDSEdgeAndErrorList> DDSEdgeAndErrorListPtr;
typedef std::shared_ptr<DDSkelAndErrorList> DDSkelAndErrorListPtr;

class Skeleton;


/*!
	The AFMMSkeleton class has the same functionality than
	the DDSGraph class but it is created from
	AFMM Star skeletons instead of FluxSkeletons
*/
class AFMMSkeleton : public sg::DDSGraph
{
private:
	//! member variables for the weights of the simplification
	double m_xmin, m_xmax, m_ymin, m_ymax;
	
	//! variable which will hold the reconstruction error of a skeleton at a given moment in time
	double m_dRecError;
	
	double m_dRecErrorWeightBnd, m_dRecErrorWeightStr;
	
	DFIELD* m_pOriginalDTMap; //!< stores the original distance transform map
	
	DFIELD* m_pCurrentSkeletonField; //!< stores the current skeleton field
	
	FLAGS* m_pOriginalAFMMFlags; //!< stores the raw output of the AFMM algorithm
	
	Skeleton* m_pAlexSkeleton; //!< stores an object of the old skeleton code
	
	ShapeDiff* m_pSD; //!< computes diff between orig and reconstructed skeleton
		
	bool ExternalEdgesSimplified(); //!< tries to simplify external branches
	
	bool InternalEdgesSimplified(); //!< tries to simplify internal branches
	
	DDSEdgeListPtr GetExternalEdges();
	
	void CreateFieldWithoutExternalEdge
		(sg::DDSEdge* pExternalEdge, 
		DFIELD* pFieldWithoutEdge);
		
	void MarkSkeletonWithoutEdge
		(sg::DDSEdge* pExternalEdge,
		DFIELD* pFieldWithoutEdge);
		
	unsigned int GetEdgeCount()
	{
		return getEdges().size();
	}
		
	void Init();
	
	void InitializeSkeletonField();
	
	void CreateShapeDiff();
	
	void RemoveExternalEdgeFromField(const sg::DDSEdge* pExternalEdge);
	
	void AddExternalEdgeToField(const sg::DDSEdge* pExternalEdge);
	
	int DetermineEdgesToBeRemovedCount(DDSEdgeAndErrorListPtr ptrEdgeErrorValues);
	
	void RemoveEdge(sg::DDSEdge* pExternalEdge);
	
	void RemoveNode(sg::DDSNode* pNode);
	
	void RemoveReferenceToEdge(sg::DDSEdge* pExternalEdge);
	
	int RemoveReferenceFromNode(sg::DDSNode* pNode, sg::DDSEdge* pExternalEdge);
	
	void UpdateSkeleton();
	
	void EnsureFlowAway(sg::DDSEdge* pEdge, sg::DDSNode* pFlowAwayNode);
	
	void EnsureMergedEdges(sg::DDSEdge* pEdge);
	
	void MergeEdge(sg::DDSEdge* pEdge);
	
	void FindLigatureEdges(InternalEdgeList& ligatureEdges);
	
	bool IsInternalEdge(sg::DDSEdge* pEdge);
	
	bool IsExternalEdge(sg::DDSEdge* pEdge);
	
	bool IsLigature(sg::DDSEdge* pEdge);
	
	bool IsOuterInsideEdge(sg::DDSEdge* pEdge);
	
public:
	AFMMSkeleton();
	AFMMSkeleton(sg::ShapeBoundary* pShape); //constructor
	
	static AFMMSkeleton* MakeSkeleton(const SkelCompParams& skelParams);

	virtual ~AFMMSkeleton(); //destructor

	void SimplifyExternal(); //simplify the external branches
	
	void SimplifyInternal(); //simplify the internal branches

	void Simplify(); //simplify both external and internal branches
	
	void SetRecErrorWeightBnd(double dRecErrorWeightBnd)
	{
		m_dRecErrorWeightBnd = dRecErrorWeightBnd;
	}
	
	void SetRecErrorWeightStr(double dRecErrorWeightStr)
	{
		m_dRecErrorWeightStr = dRecErrorWeightStr;
	}
	
	void SetDistanceTransformMap(DFIELD* pDTMap)
	{
		m_pOriginalDTMap = pDTMap;
	}
	
	void SetOriginalAFMMFlags(FLAGS* pFLAGS)
	{
		m_pOriginalAFMMFlags = pFLAGS;
	}
	
	void SetDimensions(double xmin, double xmax, double ymin, double ymax)
	{
		m_xmin = xmin;
		m_xmax = xmax;
		m_ymin = ymin;
		m_ymax = ymax;
	}
	
	const void GetDimensions(double* xmin, double* xmax, double* ymin, double* ymax)
	{
		*xmin = m_xmin;
		*xmax = m_xmax;
		*ymin = m_ymin;
		*ymax = m_ymax;
	}
};

inline AFMMSkeleton* computeAFMMkeleton(sg::DivArr& da, sg::ShapeBoundary* pShape)
{
	colour_skeleton_array(da);
	
	AFMMSkeleton* skeleton = new AFMMSkeleton(pShape);

	if (skeleton)
		sg::buildDDSGraph(da, skeleton);
	
	return skeleton;
}

#endif //_AFMMSKELETON_H_
