/**------------------------------------------------------------------------
* All Rights Reserved
* Author: Diego Macrini
*-----------------------------------------------------------------------*/
#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

namespace vpl {

	/*!
		@brief Wrapper for the Felzenszwalb-Huttenlocher graph segmentation algorithm.

		The algorith is described in the paper "Effcient Graph-Based Image Segmentation"
		by Pedro F. Felzenszwalb and Daniel P. Huttenlocher.
	*/
	class FHGraphSegmenter
	{
	public:
		struct edge 
		{
			float w;
			int a, b;

			bool operator<(const edge &rhs) const
			{
				return w < rhs.w;
			}
		};

		typedef std::vector<edge> Edges;

		struct uni_elt
		{
			int rank;
			int p;
			int size;
		};

		class universe 
		{
		public:
			universe() 
			{
			}

			void init(int elements) 
			{
				elts.resize(elements);

				num = elements;

				for (int i = 0; i < elements; i++) 
				{
					elts[i].rank = 0;
					elts[i].size = 1;
					elts[i].p = i;
				}
			}

			bool empty() const
			{
				return elts.empty();
			}

			int find(int x) 
			{
				int y = x;

				while (y != elts[y].p)
					y = elts[y].p;

				elts[x].p = y;
				
				return y;
			}

			void join(int x, int y) 
			{
				if (elts[x].rank > elts[y].rank) {
					elts[y].p = x;
					elts[x].size += elts[y].size;
				} else {
					elts[x].p = y;
					elts[y].size += elts[x].size;
					if (elts[x].rank == elts[y].rank)
						elts[y].rank++;
				}
				num--;
			}

			int size(int x) const { return elts[x].size; }
			int num_sets() const { return num; }

			unsigned num_elements() const { return elts.size(); }

		private:
			std::vector<uni_elt> elts;
			int num;
		};

	protected:
		universe m_universe;
		std::vector<edge> m_edges;
		std::vector<float> m_thresholds;

	public:

		void init(int numVertices, int maxNumEdges)
		{
			// make a disjoint-set forest
			m_universe.init(numVertices);

			m_thresholds.resize(numVertices);

			m_edges.resize(maxNumEdges);
		}

		Edges& edges()
		{
			return m_edges;
		}

		int maxNumEdges() 
		{
			return m_edges.size();
		}

		/*!
			Segment a graph. Create a disjoint-set forest representing the segmentation.

			@param num_edges number of m_edges in graph
			@param c constant for treshold function.
		*/
		void segment_graph(int num_edges, float c) 
		{ 
			ASSERT(num_edges <= maxNumEdges());
			ASSERT(!m_universe.empty() && !m_thresholds.empty() 
				&& !m_edges.empty());

			// sort m_edges by weight
			std::sort(m_edges.begin(), m_edges.end());

			// init thresholds
			for (unsigned i = 0; i < m_universe.num_elements(); i++)
				m_thresholds[i] = c;

			// for each edge, in non-decreasing weight order...
			for (int i = 0; i < num_edges; i++) 
			{
				edge *pedge = &m_edges[i];

				// components connected by this edge
				int a = m_universe.find(pedge->a);
				int b = m_universe.find(pedge->b);

				if (a != b) 
				{
					if ((pedge->w <= m_thresholds[a]) &&
						(pedge->w <= m_thresholds[b])) 
					{
						m_universe.join(a, b);
						a = m_universe.find(a);
						m_thresholds[a] = pedge->w + c / m_universe.size(a);
					}
				}
			}
		}

		void postProcessSmallComponents(int num_edges, int min_size)
		{
			ASSERT(!m_universe.empty());

			for (int i = 0; i < num_edges; i++) 
			{
				int a = m_universe.find(m_edges[i].a);
				int b = m_universe.find(m_edges[i].b);

				if ((a != b) && ((m_universe.size(a) < min_size) 
					|| (m_universe.size(b) < min_size)))
				{
					m_universe.join(a, b);
				}
			}
		}

		int find(int id) 
		{
			ASSERT(!m_universe.empty());

			return m_universe.find(id);
		}

		int num_sets() const
		{
			ASSERT(!m_universe.empty());

			return m_universe.num_sets();
		}
	};

} // namespace vpl


