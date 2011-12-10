/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "MathUtils.h" // include first
#include "BasicUtils.h"
#include "Point.h"
#include "Exceptions.h"

#include <vector>
#include <array>

namespace vpl {

//! The Point class is essentially a 2D vector
typedef Point Vector2D;

//! Simple type to specify GUI user commands
struct UserCommandInfo
{
	std::string label;
	std::string tooltip;
	int keyCode;
	void* pParam;

	UserCommandInfo() { }

	//! Note: copy strings so that 'const char*' are valid params
	UserCommandInfo(std::string l, std::string t, int k, void* p) 
	{ 
		Set(l, t, k, p);
	}

	UserCommandInfo(const UserCommandInfo& rhs) 
	{ 
		operator=(rhs);
	}

	//! Sets all fields. Note: copy strings so that 'const char*' are valid params
	void Set(std::string l, std::string t, int k, void* p)
	{
		label = l;
		tooltip = t;
		keyCode = k;
		pParam = p;
	}

	void operator=(const UserCommandInfo& rhs)
	{
		label = rhs.label;
		tooltip = rhs.tooltip;
		keyCode = rhs.keyCode;
		pParam = rhs.pParam;
	}
};

//! Dimensions of an object's silhouette
struct ShapeDims
{
	double xmin, xmax, ymin, ymax;
};

//! Loose and tight parameter interval of type double
struct LTDouble
{
	double loose, tight;

	void Set(const double& l, const double& t)
	{
		loose = l;
		tight = t;
	}

	void Set(const double& v)
	{
		loose = v;
		tight = v;
	}

	LTDouble() { }

	LTDouble(const double& l, const double& t) { Set(l, t); }

	LTDouble(const double& v) { Set(v); }

	LTDouble Cos(bool bConvertToRadians) const;
};

//! 2D coordinate (x, y) of type unsigned int
template <typename T> struct Coordinate
{
	T x, y;

	Coordinate() { }
	Coordinate(const Coordinate<T>& rhs) : x(rhs.x), y(rhs.y) { }
	Coordinate(int xx, int yy) : x(xx), y(yy) { }

	bool operator==(const Coordinate<T>& rhs) const 
	{ 
		return (x == rhs.x && y == rhs.y);
	}

	bool operator!=(const Coordinate<T>& rhs) const 
	{
		return !operator==(rhs);
	}

	void operator=(const Coordinate<T>& rhs)
	{
		x = rhs.x;
		y = rhs.y;
	}

	T& operator[](int i) 
	{
		ASSERT(i >= 0 && i < 2); 
		return (i == 0) ? x : y; 
	}

	const T& operator[](int i) const 
	{ 
		ASSERT(i >= 0 && i < 2); 
		return (i == 0) ? x : y; 
	}
};

template <typename T> struct BoundingBox
{
	enum {DIM = 4, BEGIN_BRACKET = '{', END_BRACKET = '}'};

	T xmin, xmax, ymin, ymax;

	//! Makes sure than min and max values are set correctly
	bool Regularize()
	{
		bool swapped = false;

		if (xmin > xmax)
		{
			std::swap(xmin, xmax);
			swapped = true;
		}

		if (ymin > ymax)
		{
			std::swap(ymin, ymax);
			swapped = true;
		}

		return swapped;
	}

	BoundingBox()
	{ 
	}

	/*!
		Note: the coordinates are regularized so that min is always mi and max is always max.
	*/
	BoundingBox(const T& xn, const T& xx, const T& yn, const T& yx, bool regularizeCoords = true)
		: xmin(xn), xmax(xx), ymin(yn), ymax(yx)
	{ 
		if (regularizeCoords)
		{
			bool swapped = Regularize();

			DBG_PRINT_IF("Bounding box constructore regularized coordinates", swapped);
		}
	}

	BoundingBox(const std::array<T, 4>& rhs)
	{
		operator=(rhs);
	}

	const T& operator[](int i) const
	{
		ASSERT(i >= 0 && i < DIM);

		switch (i)
		{
			case 0: return xmin;
			case 1: return xmax;
			case 2: return ymin;
			case 3: return ymax;
		}
		
		return ymax;
	}

	T& operator[](int i)
	{
		ASSERT(i >= 0 && i < DIM);

		switch (i)
		{
			case 0: return xmin;
			case 1: return xmax;
			case 2: return ymin;
			case 3: return ymax;
		}
		
		return ymax;
	}

	unsigned int ni() const
	{
		return (xmax >= xmin) ? xmax - xmin + 1 : 0;
	}

	unsigned int nj() const
	{
		return (ymax >= ymin) ? ymax - ymin + 1 : 0;
	}

	void set(const T& xn, const T& xx, const T& yn, const T& yx)
	{
		xmin = xn;
		xmax = xx;
		ymin = yn;
		ymax = yx;

		Regularize();
	}

	Point top_left() const
	{
		return Point(xmin, ymin);
	}

	Point bottom_right() const
	{
		return Point(xmax, ymax);
	}

	BoundingBox& operator=(const std::array<T, 4>& rhs)
	{
		set(rhs.at(0), rhs.at(1), rhs.at(2), rhs.at(3));
		return *this;
	}

	BoundingBox& operator=(const BoundingBox& rhs)
	{
		set(rhs.xmin, rhs.xmax, rhs.ymin, rhs.ymax);
		return *this;
	}

	bool operator==(const BoundingBox& rhs) const
	{
		return (xmin == rhs.xmin && xmax == rhs.xmax && 
			    ymin == rhs.ymin && ymax == rhs.ymax);
	}

	bool operator!=(const BoundingBox& rhs) const
	{
		return !operator==(rhs);
	}

	template <typename T>
	friend std::ostringstream& operator<<(std::ostringstream& os, 
		const BoundingBox<T>& st)
	{ 
		os << (char)BoundingBox<T>::BEGIN_BRACKET;
		
		for (unsigned i = 1; i < DIM; ++i)
			os << st[i - 1] << ",";
		
		os << st[DIM - 1] << BoundingBox<T>::END_BRACKET; 

		return os;
	}

	template <typename T>
	friend std::istringstream& operator>>(std::istringstream& is, BoundingBox<T>& st) 
	{ 
		char ch;
		std::string buffer;

		// Read the 'begin bracked'
		is >> ch;

		if (ch != BoundingBox<T>::BEGIN_BRACKET)
			THROW_BASIC_EXCEPTION("Invalid BoundingBox 'begin bracket'");

		// Read all fields and their separators
		for (unsigned i = 0; i < DIM; ++i)
		{
			buffer.clear();

			while (!is.eof()) 
			{
				is >> ch;
			
				if (ch != ',' && ch != END_BRACKET)
					buffer.push_back(ch);
				else
					break;
			}

			st[i] = atoi(buffer.c_str());

			if (ch == BoundingBox<T>::END_BRACKET && i + 1 < DIM)
				THROW_BASIC_EXCEPTION("BoundingBox has missing field(s)");
		}
	
		// The last character read must be the 'end bracket'
		if (ch != BoundingBox<T>::END_BRACKET)
			THROW_BASIC_EXCEPTION("Invalid BoundingBox 'end bracket'");

		return is;
	}
};

//! Simple X-Y integer coordinates
struct XYCoord
{
	int x, y;

	XYCoord()
	{
		x = 0;
		y = 0;
	}

	XYCoord(int xx, int yy)
	{
		x = xx;
		y = yy;
	}

	void Set(int xx, int yy)
	{
		x = xx;
		y = yy;
	}

	bool operator==(const XYCoord& rhs) const 
	{ 
		return (x == rhs.x && y == rhs.y);
	}

	bool operator!=(const XYCoord& rhs) const 
	{
		return !operator==(rhs);
	}

	void operator=(const XYCoord& rhs)
	{
		x = rhs.x;
		y = rhs.y;
	}

	void operator=(const Point& pt)
	{
		x = ROUND_NUM(pt.x);
		y = ROUND_NUM(pt.y);
	}

	XYCoord& operator+=(const XYCoord& p)
	{
		(*this).x += p.x;
		(*this).y += p.y;

		return *this;
	}

	XYCoord& operator/=(const XYCoord & p)
	{
		(*this).x /= p.x;
		(*this).y /= p.y;

		return *this;
	}

	XYCoord operator+(const XYCoord & p) const
	{
		XYCoord r;
		r.x = x + p.x;
		r.y = y + p.y;

		return r;
	}

	XYCoord operator-(const XYCoord & p) const
	{
		XYCoord r;

		r.x = x - p.x;
		r.y = y - p.y;

		return r;
	}

	XYCoord operator*(const double & p) const
	{
		XYCoord r;

		r.x = (int)(x * p);
		r.y = (int)(y * p);

		return r;
	}

	XYCoord operator*(const float & p) const
	{
		XYCoord r;

		r.x = (int)(x * p);
		r.y = (int)(y * p);

		return r;
	}

	XYCoord operator/(const double & p) const
	{
		XYCoord r;

		r.x = (int)(x / p);
		r.y = (int)(y / p);

		return r;
	}

	//! Computes the squared distance between the points
	int SqDist(const XYCoord& rhs) const
	{
		int dx = x - rhs.x;
		int dy = y - rhs.y;

		return dx * dx + dy * dy;
	}

	double Norm() const
	{
		return sqrt(double(x * x + y * y));
	}

	friend int DotProduct(const XYCoord& a, const XYCoord& b)
	{
		return a.x * b.x + a.y * b.y;
	}
};

/*!
	Array of XY coordinates. Each coordinate is stored in a separate
	array.
*/
struct DiscreteXYArray
{
	std::vector<int> xa;
	std::vector<int> ya;

	DiscreteXYArray() { }
	DiscreteXYArray(const DiscreteXYArray& rhs) : xa(rhs.xa), ya(rhs.ya) { }

	unsigned int Size() const
	{
		ASSERT(xa.size() == ya.size());

		return xa.size();
	}

	DiscreteXYArray& operator=(const DiscreteXYArray& rhs)
	{
		xa = rhs.xa;
		ya = rhs.ya;
		return *this;
	}

	void ApplyOffset(unsigned dx, unsigned dy)
	{
		for (unsigned i = 0; i < xa.size(); i++)
		{
			xa[i] += dx;
			ya[i] += dy;
		}
	}

	bool operator==(const DiscreteXYArray& rhs) const
	{
		if (Size() != rhs.Size())
			return false;

		for (unsigned i = 0; i < Size(); i++)
		{
			if (xa[i] != rhs.xa[i] || ya[i] != rhs.ya[i])
				return false;
		}

		return true;
	}

	bool operator!=(const DiscreteXYArray& rhs) const
	{
		return !operator==(rhs);
	}
};

typedef Coordinate<unsigned int> UICoordinate;

typedef BoundingBox<unsigned int> UIBoundingBox;

} // namespace vpl

