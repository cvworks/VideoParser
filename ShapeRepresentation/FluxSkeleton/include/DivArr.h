#ifndef _DIV_ARRAY_H_
#define _DIV_ARRAY_H_

#include "DivPt.h"

namespace sg {

	class DivArr
	{
		static const DivPt m_defltValue;

		int m_xSize;
		int m_ySize;

		DivPt* m_pts;
		DivPt* m_endPt;

	public:
		typedef DivPt* iterator;

	public:

		DivArr(int xSize, int ySize)
		{
			m_xSize = xSize;
			m_ySize = ySize;

			int sz = xSize * ySize;
			m_pts = new DivPt[sz];
			m_endPt = m_pts + sz;
		}

		DivPt* begin() { return m_pts; }
		DivPt* end()   { return m_endPt; }

		~DivArr() 
		{ 
			delete[] m_pts; 
		}

		//! It's true iff x,y are within the array limits
		bool checkLimits(int x, int y) const
		{
			return (x >= 0 && x < m_xSize && y >= 0 && y < m_ySize);
		}

		const DivPt& operator()(int x, int y) const
		{
			if (x >= 0 && x < m_xSize && y >= 0 && y < m_ySize)
				return m_pts[y * m_xSize + x];
			else
				return m_defltValue;
		}

		DivPt& operator()(int x, int y)
		{
			ASSERT(checkLimits(x, y));

			return m_pts[y * m_xSize + x];
		}

		int getXSize() const { return m_xSize; }
		int getYSize() const { return m_ySize; }
	};
}

#endif // _DIV_ARRAY_H_
