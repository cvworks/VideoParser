/*
	This class will implement the method necessary for 
	simplifying the skeleton structure

	author: Matthijs von Eede
*/

#include  "include/AFMMSkeleton.h"
#include "skeleton.h"
#include <Tools/MathUtils.h>

#include <vector>
#include <algorithm>
#include <iostream>

#ifndef WIN32
#define max std::max
///#else
///void reverse(sg::FluxPointArray::iterator it0, sg::FluxPointArray::iterator it1)
///{
///	ASSERT(false);
///}
#endif

#ifdef PI
#undef PI
#endif

#define APROX_ERROR_FIX 0.00025f
#define PI 3.141592653589793238462643

/*!
*/
AFMMSkeleton::AFMMSkeleton()
{
	Init();
}

/*!
*/
AFMMSkeleton::AFMMSkeleton(sg::ShapeBoundary* pShape)
		: sg::DDSGraph(pShape)
{
	Init();
}
 
/*!
*/
void AFMMSkeleton::Init()
{
	m_xmin = 0;
	m_xmax = 0;
	m_ymin = 0;
	m_ymax = 0;
	m_dRecError = 0;
	m_dRecErrorWeightBnd = 0;
	m_dRecErrorWeightStr = 0;
	m_pOriginalDTMap = NULL;
	m_pSD = NULL;
	m_pCurrentSkeletonField = NULL;
	m_pOriginalAFMMFlags = NULL;
	m_pAlexSkeleton = new Skeleton();
}

/*!
*/
AFMMSkeleton::~AFMMSkeleton()
{
	delete m_pOriginalDTMap;
	delete m_pSD;
	delete m_pCurrentSkeletonField;
	delete m_pOriginalAFMMFlags;
	delete m_pAlexSkeleton;
}

/*!
*/
/*static*/ AFMMSkeleton* AFMMSkeleton::MakeSkeleton(const SkelCompParams& skelParams)
{
	AFMMSkeleton* pDDS = Skeleton().get_DDSkeleton(skelParams); 
	
	//make a field that marks all the skeleton points
	pDDS->InitializeSkeletonField();
	
	return pDDS; 
}


/*!
*/
DDSEdgeListPtr AFMMSkeleton::GetExternalEdges()
{
	DDSEdgeListPtr ptrExternalEdges(new DDSEdgeList);
	const int numNodes = (int)getNodes().size();
	
	for(int i = 0; i < numNodes; i++)
		if(getNodes()[i]->edges.size() == 1)
			ptrExternalEdges->push_back(getNodes()[i]->edges[0]);
	
	return ptrExternalEdges;
}

/*!
	Sets all points in the field to 0 (zero)
*/
void InitializeField(DFIELD* pFieldWithoutEdge)
{
	for(int x = 0; x < pFieldWithoutEdge->dimX(); x++)
		for(int y = 0; y < pFieldWithoutEdge->dimY(); y++)
			pFieldWithoutEdge->value(x, y) = 0;
}

/*!
	Marks the given edge in the skeleton field
*/
void MarkEdgeInField
	(sg::DDSEdge* pEdge,
	DFIELD* pFieldWithoutEdge)
{
	//mark the flux points
	for(int i = 0; i < int(pEdge->flux_points.size()); i++)
		pFieldWithoutEdge->value(int(pEdge->flux_points[i].p.x),
							int(pEdge->flux_points[i].p.y)) = 1;
	
	//mark the two nodes... node 1...
	pFieldWithoutEdge->value(int(pEdge->n1->fp.p.x), int(pEdge->n1->fp.p.y)) = 1;
	//and node 2
	pFieldWithoutEdge->value(int(pEdge->n2->fp.p.x), int(pEdge->n2->fp.p.y)) = 1;
}

/*!
	This method will initialize the skeleton field
	with all the fluxpoints and DDSNodes of
	the edges in the AFMMSkeleton
	
	The distance transform  map is created
	when the object is created, so we can use
	the dimensions of m_pOriginalDTMap
*/
void AFMMSkeleton::InitializeSkeletonField()
{
	m_pCurrentSkeletonField = new DFIELD(m_pOriginalDTMap->dimX(), m_pOriginalDTMap->dimY());
	
	//initialize the points in the Skeleton field to zero
	InitializeField(m_pCurrentSkeletonField);

	const int numEdges = (int)getEdges().size();
	
	//add all edges to the skeleton
	for(int i = 0; i < numEdges; i++)
		MarkEdgeInField(getEdges()[i], m_pCurrentSkeletonField);
}

/*!
	Marks the skeleton in the field with the exeption of
	the given external edge
*/
void AFMMSkeleton::MarkSkeletonWithoutEdge
	(sg::DDSEdge* pExternalEdge,
	DFIELD* pFieldWithoutEdge)
{
	const int numEdges = (int)getEdges().size();

	for(int i = 0; i < numEdges; i++)
		if(getEdges()[i] != pExternalEdge)
			MarkEdgeInField(getEdges()[i], pFieldWithoutEdge);
}

/*!
*/
void AFMMSkeleton::RemoveExternalEdgeFromField(const sg::DDSEdge* pExternalEdge)
{
	//remove the actual edge
	for(int i = 0; i < int(pExternalEdge->flux_points.size()); i++)
		m_pCurrentSkeletonField->value(int(pExternalEdge->flux_points[i].p.x),
								int(pExternalEdge->flux_points[i].p.y)) = 0;
	
	//re-mark the node of the edge that actually was attached to other edges too
	//this one would have been removed in the previous step since the nodes are the
	//extreme points in the flux_points list of the edge
	if(int(pExternalEdge->n1->edges.size()) != 1)
		m_pCurrentSkeletonField->value(int(pExternalEdge->n1->fp.p.x),
			int(pExternalEdge->n1->fp.p.y)) = 1;
	
	if(int(pExternalEdge->n2->edges.size()) != 1)
		m_pCurrentSkeletonField->value(int(pExternalEdge->n2->fp.p.x),
			int(pExternalEdge->n2->fp.p.y)) = 1;
}

/*!
*/
void AFMMSkeleton::AddExternalEdgeToField(const sg::DDSEdge* pExternalEdge)
{
	//add the actual edge
	for(int i = 0; i < int(pExternalEdge->flux_points.size()); i++)
		m_pCurrentSkeletonField->value(int(pExternalEdge->flux_points[i].p.x),
								int(pExternalEdge->flux_points[i].p.y)) = 1;
	
	//add the two end nodes
	m_pCurrentSkeletonField->value(int(pExternalEdge->n1->fp.p.x),
		int(pExternalEdge->n1->fp.p.y)) = 1;
	m_pCurrentSkeletonField->value(int(pExternalEdge->n2->fp.p.x),
		int(pExternalEdge->n2->fp.p.y)) = 1;
}


/*!
	given the errors for the external edges, this function will return the number of 
	edges that need to be removed
*/
int AFMMSkeleton::DetermineEdgesToBeRemovedCount(DDSEdgeAndErrorListPtr ptrEdgeErrorValues)
{
	int nCount = 0;
	double dCurrentCombinedError = GetEdgeCount() + 
		(m_dRecErrorWeightBnd * m_dRecError);
	double dMinimumError = dCurrentCombinedError;
	double dCumulativeRecError = m_dRecError;
	int nCurrentEdgeCount = GetEdgeCount();
	
	DDSEdgeAndErrorList::iterator it;
	for(it = ptrEdgeErrorValues->begin(); it != ptrEdgeErrorValues->end(); ++it)
	{
		nCurrentEdgeCount--;
		dCumulativeRecError += (*it).second;
		dCurrentCombinedError = nCurrentEdgeCount + 
			(m_dRecErrorWeightBnd * dCumulativeRecError);
		if(dCurrentCombinedError < dMinimumError)
		{
			dMinimumError = dCurrentCombinedError;
			nCount++;
		}
		else
			break;
	}
	
	return nCount;
}

/*!
*/
int AFMMSkeleton::RemoveReferenceFromNode(sg::DDSNode* pNode, 
	sg::DDSEdge* pExternalEdge)
{
	std::vector<sg::DDSEdge*>::iterator it;
	for(it = pNode->edges.begin(); it != pNode->edges.end(); ++it)
		if(*it == pExternalEdge)
		{
			pNode->edges.erase(it);
			return 1;
		}
	
	return 0;
}

/*!
*/
void AFMMSkeleton::RemoveReferenceToEdge(sg::DDSEdge* pExternalEdge)
{
	int nRemovals = 0;
	
	nRemovals += RemoveReferenceFromNode(pExternalEdge->n1, pExternalEdge);
	nRemovals += RemoveReferenceFromNode(pExternalEdge->n2, pExternalEdge);
	
	ASSERT(nRemovals == 2);
}


/*!
*/
void AFMMSkeleton::RemoveNode(sg::DDSNode* pNode)
{
	bool bNodeRemoved = false;

	for(int i = int(pNode->edges.size()) - 1; i >= 0; i--)
		 RemoveEdge(pNode->edges[i]);
	
	std::vector<sg::DDSNode*>::iterator itNode;
	
	for(itNode = getNodes().begin(); itNode != getNodes().end() && !bNodeRemoved; ++itNode)
	{
		if(*itNode == pNode)
		{
			getNodes().erase(itNode);
			bNodeRemoved = true;
			break;
		}
	}

	ASSERT(bNodeRemoved);

	delete pNode;
}

/*!
*/
void AFMMSkeleton::RemoveEdge(sg::DDSEdge* pExternalEdge)
{

	//remove the references to this edge
	RemoveReferenceToEdge(pExternalEdge);

	bool bEdgeRemoved = false;
	std::vector<sg::DDSEdge*>::iterator it;
	
	for(it = getEdges().begin(); it != getEdges().end() && !bEdgeRemoved; ++it)
	{
		if((*it) == pExternalEdge)
		{
			getEdges().erase(it);
			bEdgeRemoved = true;
			break;
		}
	}
			
	ASSERT(bEdgeRemoved);

	delete pExternalEdge;
}


/*!
*/
void AFMMSkeleton::CreateFieldWithoutExternalEdge
	(sg::DDSEdge* pExternalEdge,
	DFIELD* pFieldWithoutEdge)
{
	InitializeField(pFieldWithoutEdge);
	
	MarkSkeletonWithoutEdge(pExternalEdge, pFieldWithoutEdge);
}


/*!
	This method is given a pointer to an edge which has
	only 1 real neighbor (not including itself) at node n2
	(so pEdge->n2->edges.size() == 2)
	Given this, this method will concatenate this nbr 
	to the given edge, and remove and delete information about
	this neighbor from the skeleton after it has been merged
*/
void AFMMSkeleton::MergeEdge(sg::DDSEdge* pEdge)
{
	sg::DDSEdge* pSecondEdge = (pEdge->n2->edges[0] == pEdge)?pEdge->n2->edges[1]:
		pEdge->n2->edges[0];
		
	//the last element from the fluxpoint list of pEdge is the same as the first from pSecondEdge
	pEdge->flux_points.pop_back();
	
	unsigned int i;
	//add fluxpoints of pSecondEdge to the fluxpoints of pEdge
	for(i = 0; i < pSecondEdge->flux_points.size(); i++)
		pEdge->flux_points.push_back(pSecondEdge->flux_points[i]);
	
	//update info from end node of pSecondEdge
	//instead of pointing to pSecondEdge it should now point to pEdge
	bool bEdgeSwapped = false;
	sg::DDSEdgeVect::iterator it;
	for(it = pSecondEdge->n2->edges.begin(); it != pSecondEdge->n2->edges.end()
		&& !bEdgeSwapped; ++it)
	{
		if(*it == pSecondEdge)
		{
			pSecondEdge->n2->edges.erase(it);
			pSecondEdge->n2->edges.push_back(pEdge);
			bEdgeSwapped = true;
			break;
		}
	}
	
	//the reconstruction error of this edge is now unknown
	pEdge->m_dEgdeRecError = -1;
	
	ASSERT(bEdgeSwapped);
	
	sg::DDSNode* pNodeTBDeleted = pEdge->n2;
	pEdge->n2 = pSecondEdge->n2;
	
	//remove and delete the node that was in the middle
	bool bNodeRemoved = false;
	sg::DDSNodeVect::iterator itNode;
	for(itNode = getNodes().begin(); itNode != getNodes().end() && !bNodeRemoved; ++itNode)
	{
		if(*itNode == pNodeTBDeleted)
		{
			getNodes().erase(itNode);
			bNodeRemoved = true;
			break;
		}
	}

	ASSERT(bNodeRemoved);

	delete pNodeTBDeleted;
	
	//remove and delete the edge that we just concatenated
	bool bEdgeRemoved = false;
	for(it = getEdges().begin(); it != getEdges().end() && !bEdgeRemoved; ++it)
	{
		if((*it) == pSecondEdge)
		{
			getEdges().erase(it);
			bEdgeRemoved = true;
			break;
		}
	}
			
	ASSERT(bEdgeRemoved);

	delete pSecondEdge;
	
}

/*!
	Pre condition is that EnsureFlowAway has been called
	on the skeleton which means that the flow of the skeleton
	will be correct

	This method gets a pointer to an edge and will check
	whether the second node of the given edge has exactly
	2 neighbors. If so we will merge these two edges
	and call this function again on 
	
	-the new incident edges of the current edge if it now 
	has more then 2 nbrs,
	
	-on itself if again the edge has 2 nbrs
	
	-return if it only has itself as nbr
*/
void AFMMSkeleton::EnsureMergedEdges(sg::DDSEdge* pEdge)
{
	//we move with the flow, so only look at n2 nodes
	if(pEdge->n2->edges.size() == 1)
		return;
		
	//we don't have to merge this edge to other edges if it has at least 2
	//nbrs other then itself (so at least 3 nbrs including itself)
	sg::DDSEdgeVect::iterator it;
	if(pEdge->n2->edges.size() > 2)
		for(it = pEdge->n2->edges.begin(); it != pEdge->n2->edges.end(); ++it)
			if(*it != pEdge)
				EnsureMergedEdges(*it);
	
	//we will need to merge the current edge its nbr
	if(pEdge->n2->edges.size() == 2)
	{
		//merge the two edges
		MergeEdge(pEdge);
		
		//check the new edge again
		EnsureMergedEdges(pEdge);
	}
}

/*!
	This method gets a pointer to an edge and a pointer
	to one of its nodes. It will make sure that the fluxpoints
	of this edge are arranged in such a way that they
	will flow away from the given node.
*/
void AFMMSkeleton::EnsureFlowAway(sg::DDSEdge* pEdge, sg::DDSNode* pFlowAwayNode)
{
	unsigned int nFPListSize = pEdge->flux_points.size();
	sg::DDSNode* pOtherNode = (pEdge->n1 == pFlowAwayNode)?pEdge->n2:pEdge->n1;
	sg::DDSEdgeVect::iterator it;

	//make sure the flow goes from n1 -> n2
	if(pEdge->n2 == pFlowAwayNode)
	{
		//swap n1 and n2
		sg::DDSNode* pSwap = pEdge->n1;
		pEdge->n1 = pEdge->n2;
		pEdge->n2 = pSwap;
// DBG_PRINT1("\nNodes swapped in EnsureFlowAway");
	}
	//verify that the fluxpoints go from n1 to n2
	if(!((pEdge->n1->fp == pEdge->flux_points[0]) &&
		(pEdge->n2->fp == pEdge->flux_points[nFPListSize - 1])))
	{
		//wrong flow, but n1 is the correct node, so only reverse the vector
		reverse(pEdge->flux_points.begin(), pEdge->flux_points.end());

// DBG_PRINT1("\nFluxPointList reversed in EnsureFlowAway");
// DBG_PRINT4("\nEdge: ", pEdge->n1->fp, " ", pEdge->n2->fp);
// DBG_PRINT2("\nNumber of points in the fluxpoint list: ", pEdge->flux_points.size()); 
// DBG_PRINT4("\nEndpoint 1 ", pEdge->n1->fp, " has edge count ", pEdge->n1->edges.size()); 
// DBG_PRINT4("\nEndpoint 2 ", pEdge->n2->fp, " has edge count ", pEdge->n2->edges.size());
	}
	//the flow is correct, now ensure the correct flow for its neighbors
	for(it = pOtherNode->edges.begin(); it != pOtherNode->edges.end(); ++it)
		if(*it != pEdge)
			EnsureFlowAway(*it, pOtherNode);
}

/*!
	This function will be called after edges have been deleted from
	the skeleton. The skeleton might need to be updated to make 
	sure that the flow of points is still correct, that there are no 
	nodes with only 2 neighbors (this node would just be a break
	within one single edge) and that the first edge of the skeleton
	in the edge vector is an external edge
*/
void AFMMSkeleton::UpdateSkeleton()
{
	ASSERT(GetEdgeCount() >= 1);
	
	if(GetEdgeCount() == 1)
		return;
	
	//if the first edge in the skeleton is not an external edge
	//we will have to swap it with an edge
	//that is an external edge
	if(getEdges()[0]->n1->edges.size() != 1 &&
		getEdges()[0]->n2->edges.size() != 1)
	{
		//the skeleton has at least 2 edges and the the first one wasn't
		//external, so there should be one somewhere else
		bool bNextExFound = false;

		for(unsigned int i = 1; i < getEdges().size() && !bNextExFound; i++)
		{
			if(getEdges()[i]->n1->edges.size() == 1 ||
				getEdges()[i]->n2->edges.size() == 1)
			{
				bNextExFound = true;
				sg::DDSEdge* pTemp = getEdges()[0];
				getEdges()[0] = getEdges()[i];
				getEdges()[i] = pTemp;
			}
		}
	}
	
	//start by correcting the flow of points in the skeleton
	if(getEdges()[0]->n1->edges.size() == 1)
		EnsureFlowAway(getEdges()[0], getEdges()[0]->n1);
	else
		EnsureFlowAway(getEdges()[0], getEdges()[0]->n2);
	
	//flow is correct now, we just need to merge edges when necessary
	EnsureMergedEdges(getEdges()[0]);
}

/*!
*/
bool AFMMSkeleton::ExternalEdgesSimplified()
{
	if(GetEdgeCount() == 1)
		return false;
		
	bool bSimplified = false;

	DDSEdgeListPtr ptrExternalEdges = GetExternalEdges();
	
	//get the reconstruction error for removing a single external edge
	DDSEdgeList::iterator it;
	DDSEdgeAndErrorListPtr ptrEdgeErrorValues(new DDSEdgeAndErrorList);
	DDSEdgeAndError edgeError;
	for(it = ptrExternalEdges->begin(); it != ptrExternalEdges->end(); ++it)
	{
		DBG_FLUSH(".");
		//initialize a field and place all edges but the given on in the field
		RemoveExternalEdgeFromField(*it);
		
		//only get the reconstruction error if it is unknown
		if((*it)->m_dEgdeRecError == -1)
			(*it)->m_dEgdeRecError = max((float)m_pAlexSkeleton->GetReconstructionError(
				m_pCurrentSkeletonField, m_pOriginalDTMap, *m_pSD),APROX_ERROR_FIX);

		edgeError.first = (*it);
		edgeError.second = (*it)->m_dEgdeRecError;
		ptrEdgeErrorValues->push_back(edgeError);
		
		AddExternalEdgeToField(*it);
	}
	
	//edgeErrorValues.sort();
	ptrEdgeErrorValues->sort();
	
	int nCount = DetermineEdgesToBeRemovedCount(ptrEdgeErrorValues);
	
	if(nCount == 0)
		return false;
		
	//ATTENTION!!! at the moment we will not remove all the edges at the same time anymore
	//this will mean that if the edgecount of edges we want to remove is greater then 0
	//we will only remove 1 edge and perhaps another in the next iteration
	if(nCount >= 1)
		nCount = 1;
	
	//remove the edges we want to remove
	int i;
	DDSEdgeAndErrorList::iterator itEAEL;
	for(i = 0, itEAEL = ptrEdgeErrorValues->begin(); i <  nCount && itEAEL != ptrEdgeErrorValues->end(); i++, ++itEAEL)
	{
		if((*itEAEL).first->n1->edges.size() == 1)
			RemoveNode((*itEAEL).first->n1);
		else if((*itEAEL).first->n2->edges.size() == 1)
			RemoveNode((*itEAEL).first->n2);
	}
	
	bSimplified = true;
	UpdateSkeleton();
	
	return bSimplified;
}

/*!
	Creates the initial shape diff from
	the skeleton we start with initially
*/
void AFMMSkeleton::CreateShapeDiff()
{
	//only create it if it wasn't created before
	if(m_pSD != NULL)
		return;

	//the following variables will be deleted by ShapeDiff
	DFIELD* pInitialDT;
	FLAGS* pInitialFlags;

	m_pAlexSkeleton->GetDTByInflation(m_pCurrentSkeletonField, 
		m_pOriginalDTMap, pInitialFlags, pInitialDT);
	
	m_pSD = new ShapeDiff(*m_pCurrentSkeletonField, pInitialFlags, pInitialDT);
}


/*!
*/
void AFMMSkeleton::SimplifyExternal()
{
	//make sure InitializeSkeletonField has been invoked already
	ASSERT(m_pCurrentSkeletonField != NULL);
	
	DBG_FLUSH("\nBeginning external edge simplification...\n");
	//create initial ShapeDiff needed for the errors of the inflated objects
	CreateShapeDiff();

	//get error for the skeleton as we got it (with initial threshold)
	//DBG_FLUSH(":");

	m_dRecError = m_pAlexSkeleton->GetReconstructionError(m_pCurrentSkeletonField, 
		m_pOriginalDTMap, *m_pSD);
	
	bool bExtSimpPerformed = ExternalEdgesSimplified();
	
	while(bExtSimpPerformed)
		bExtSimpPerformed = ExternalEdgesSimplified();
		
	DBG_FLUSH("\nExternal edges simplified...\n");
}


/*!
	Checks whether the edge has an external endpoint.
	In that case the edge is not internal
*/
bool AFMMSkeleton::IsInternalEdge(sg::DDSEdge* pEdge)
{
	//if one of the endpoints is external it's no internal edge
	if(pEdge->n1->edges.size() == 1 || pEdge->n2->edges.size() == 1)
		return false;

	return true;
}

/*!
	Checks whether the edge has an external endpoint.
	In that case the edge is external
*/
bool AFMMSkeleton::IsExternalEdge(sg::DDSEdge* pEdge)
{
	//if one of the endpoints is external it's no internal edge
	if(pEdge->n1->edges.size() == 1 || pEdge->n2->edges.size() == 1)
		return true;

	return false;
}


/*!
	This function will calculate the overlap between the
	two endpoints of the edge. If they overlap and they
	overlap by at least 75% we will consider this edge
	to be a ligature edge.
*/
bool AFMMSkeleton::IsLigature(sg::DDSEdge* pEdge)
{
	//c = distance between middle circle end1 and middle circle end2
	double x0, x1, y0, y1, r0, r1, c, CBA, CBD, CAB, CAD, overlapArea, smallArea;
	
	x0 = pEdge->n1->fp.p.x;
	y0 = pEdge->n1->fp.p.y;
	r0 = pEdge->n1->fp.dist;
	x1 = pEdge->n2->fp.p.x;
	y1 = pEdge->n2->fp.p.y;
	r1 = pEdge->n2->fp.dist;
	
	c = sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0));
	
	if(c <= (r0 + r1)) //in case they overlap
	{
		CBA = acos((r1*r1 + c*c - r0*r0)/(2*r1*c));
		CBD = 2 * CBA;
		CAB = acos((r0*r0 + c*c - r1*r1)/(2*r0*c));
		CAD = 2 * CAB;
		overlapArea = 0.5 * CBD * r1*r1 - 0.5 * r1*r1 * sin(CBD) +
					0.5 * CAD * r0*r0 - 0.5 * r0*r0 * sin(CAD);
		smallArea = (r0 < r1) ? PI * r0 * r0 : PI * r1 * r1;
		
		return (overlapArea/smallArea >= 0.75);
	}
	
	//if they don't overlap we surely don't deal with a ligature edge
	return false;
	
}


/*!
	This function will go through all the edges of the skeleton and
	if the edge is internal (doesn't have an external endpoint) and the 
	overlap between the two endpoints of the edge is at least 75%
	we will mark this edge as a ligature edge. In that case we store
	the index of the edge and the number of points in the edge (to reflect
	the edge's length)
*/
void AFMMSkeleton::FindLigatureEdges(InternalEdgeList& ligatureEdges)
{
	IndexAndLength idxAndLength;
	
	for(unsigned int i = 0; i < getEdges().size(); i++)
	{
		if(IsInternalEdge(getEdges()[i]) && IsLigature(getEdges()[i]))
		{
			idxAndLength.first = i;
			idxAndLength.second = getEdges()[i]->flux_points.size();
			ligatureEdges.push_back(idxAndLength);
		}
	}
}

/*!
	We define an outer inside edge as an inside edge
	that has exclusively external edges on 1 (or both)
	sides of its edge. 
	Seen from the outside of the skeleton these are the
	first inside edges we encounter when walking towards
	the middle of the skeleton
*/
bool AFMMSkeleton::IsOuterInsideEdge(sg::DDSEdge* pEdge)
{
	bool bOuterInsideEdge = true;
	unsigned int i;
	for(i = 0; i < pEdge->n1->edges.size(); i++)
	{
		if( (pEdge->n1->edges[i] != pEdge) && 
			!IsExternalEdge(pEdge->n1->edges[i]) )
			bOuterInsideEdge = false;
	}
	
	if(bOuterInsideEdge)
		return true;
	
	bOuterInsideEdge = true;
	for(i = 0; i < pEdge->n2->edges.size(); i++)
	{
		if( (pEdge->n2->edges[i] != pEdge) && 
			!IsExternalEdge(pEdge->n2->edges[i]) )
			bOuterInsideEdge = false;
	}
	
	if(bOuterInsideEdge)
		return true;
	else
		return false;
}


/*!
	This function will look at all the internal edges and determine
	whether the internal edges are suitable for removal. If so we
	will see whether we can remove the smallest of the inside edges.
	This might be a problem if this inside edge has other inside edges
	as neighbors at both endpoints and they also are marked to be removed.
	In that case we will find the edges that is situated most externally and
	try to remove that one.
*/
bool AFMMSkeleton::InternalEdgesSimplified()
{
	if(GetEdgeCount() == 1)
		return false;

	bool bSimplified = false;
	
	//we should first get a list of internal edges that are removable
	//since we only remove 1 edge at the time, we will use the index in the
	//edges list to point them out
	InternalEdgeList ligatureEdges;
	FindLigatureEdges(ligatureEdges);
	DBG_MSG2("Ligature count: ", ligatureEdges.size())
	
	if(ligatureEdges.size() == 0) //no internal edges to simplify
		return false;
	
	//sort the ligature edges according to length of the edge
	ligatureEdges.sort();
	
	InternalEdgeList::iterator it = ligatureEdges.begin();
	//determine which ligature edge to try and remove
	//if the smallest ligature edge has only external edges at one
	//side it's safe to remove that edge
#ifdef _DEBUG
	if(!IsOuterInsideEdge(getEdges()[it->first]))
	{
		DBG_MSG1("Smallest ligature candidate is not an outer inside branch")
	}
#endif
	
	return bSimplified;
}


/*!
*/
void AFMMSkeleton::SimplifyInternal()
{
	//make sure InitializeSkeletonField has been invoked already
	ASSERT(m_pCurrentSkeletonField != NULL);
	
	DBG_FLUSH("\nBeginning internal edge simplification...\n");

	//create initial ShapeDiff needed for the errors of the inflated objects
	CreateShapeDiff();
	
	bool bIntSimpPerformed = InternalEdgesSimplified();
	
	while(bIntSimpPerformed)
		bIntSimpPerformed = InternalEdgesSimplified();
	
	DBG_FLUSH("\nInternal edges simplified...\n");
}

/*!
*/
void AFMMSkeleton::Simplify()
{
	SimplifyExternal();
	SimplifyInternal();
}


