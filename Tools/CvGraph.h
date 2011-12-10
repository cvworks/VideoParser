/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _VPL_GRAPH_H_
#define _VPL_GRAPH_H_

#include <Tools/cv.h>

namespace vpl {

typedef CvGraphVtx* node;
typedef CvGraphEdge* edge;

/*!
*/
class graph
{
protected:
	CvGraph* m_pCvGraph;
	CvMemStorage* m_pStorage;

	void Create(bool directed, int nodeSize, int edgeSize)
	{
		m_pStorage = cvCreateMemStorage();

		m_pCvGraph = cvCreateGraph(directed ? CV_ORIENTED_GRAPH : CV_GRAPH, 
			sizeof(CvGraph), nodeSize, edgeSize, m_pStorage);
	}

	graph(bool directed, int nodeSize, int edgeSize)
	{
		Create(directed, nodeSize, edgeSize);
	}

public:
	graph(bool directed = true)
	{
		Create(directed, sizeof(CvGraphVtx), sizeof(CvGraphEdge));
	}

	~graph()
	{
		cvReleaseMemStorage(&m_pStorage);
	}

	int index(node v) const
	{
		return cvGraphVtxIdx((CvGraph*) m_pCvGraph, v);
	}

	node new_node()
	{
		node v;

		cvGraphAddVtx(m_pCvGraph, NULL, &v);

		return v;
	}

	edge new_edge(node u, node v)
	{
		edge e;

		cvGraphAddEdgeByPtr(m_pCvGraph, u, v, NULL, &e);

		return e;
	}

	operator const CvGraph*() const { return m_pCvGraph; }
	operator CvGraph*() { return m_pCvGraph; }
};

/*!
*/
template <typename V_TYPE, typename E_TYPE> class AttributedGraph : graph
{
	struct AttributedCvGraphVertex : public CvGraphVtx
	{
		V_TYPE data; //!< The additional node attribute data

		AttributedCvGraphVertex() { }
		AttributedCvGraphVertex(const V_TYPE& x) : data(x) { }
	};

	struct AttributedCvGraphEdge : public CvGraphEdge
	{
		E_TYPE data; //!< The additional edge attribute data

		AttributedCvGraphEdge() { }
		AttributedCvGraphEdge(const E_TYPE& x) : data(x) { }
	};

public:
	AttributedGraph(bool directed = true) : graph(directed, 
		sizeof(AttributedCvGraphVertex), sizeof(AttributedCvGraphEdge))
	{
	}

	node new_node(const V_TYPE& x)
	{
		AttributedCvGraphVertex av(x);
		node v;

		// this function copies the 'data' field in av
		cvGraphAddVtx(m_pCvGraph, &av, &v);

		return v;
	}

	edge new_edge(node u, node v, const E_TYPE& x)
	{
		AttributedCvGraphEdge ae(x);
		edge e;

		// this function copies the 'data' field in ae
		cvGraphAddEdgeByPtr(m_pCvGraph, u, v, &ae, &e);

		return e;
	}

	operator const CvGraph*() const { return m_pCvGraph; }
	operator CvGraph*() { return m_pCvGraph; }
};

} // namespace vpl

#endif //_VPL_GRAPH_H_
