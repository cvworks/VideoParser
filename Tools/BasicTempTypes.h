#ifndef __BASIC_TEMP_TYPES_H__
#define __BASIC_TEMP_TYPES_H__

template<typename T> class OrderedPair
{
protected:
	T first;
	T second;
	
public:
	OrderedPair() { first = second = 0; }
	OrderedPair(const T& x, const T& y) : first(x), second(y) { }
	void Set(const T& x, const T& y)   { first = x; second = y; }

	void Print(std::ostream& os = std::cout, bool bEndOfLine = true) const
	{
		os << "(" << first << ", " << second << ")";
		if (bEndOfLine) os << std::endl;
	}
};

template<typename T> class TwoDimCoord : public OrderedPair<T>
{
public:
	TwoDimCoord() { }
	TwoDimCoord(const T& x, const T& y) : OrderedPair<T>(x, y) { }
	
	const T& x() const { return this->first; }
	const T& y() const { return this->second; }
};

template<typename T> class TwoDimShift : public OrderedPair<T>
{
public:
	TwoDimShift() { }
	TwoDimShift(const T& x, const T& y) : OrderedPair<T>(x, y) { }
	
	const T& dx() const { return this->first; }
	const T& dy() const { return this->second; }
};

typedef TwoDimCoord<int> PixelCoord;
typedef TwoDimShift<double> MotionData;

#endif //__BASIC_TEMP_TYPES_H__
