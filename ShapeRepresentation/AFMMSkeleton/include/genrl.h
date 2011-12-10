#ifndef SKEL_GENRL_H
#define SKEL_GENRL_H


#include <map>
#include <Tools/MathUtils.h>

#undef INFINITY
#undef SQR

const float INFINITY = 1.0e7f;
const float eps  	 = 1.0e-6f;
const float eps2 	 = eps*eps;
const int   CONN_UNKNOWN = -1;


#define CHECKZ(x,y) ((fabs(y)>1.0e-8)? (x/y) : 0)


inline float SQR(float x) 			{ return x*x; }
inline int   SQR(int x)                 	{ return x*x; }
inline float max(float x,float y) 		{ return (x<y)? y : x; }
inline float min(float x,float y)		{ return (x<y)? x : y; } 
inline float INTERP(float a,float b,float t)	{ return a*(1-t)+b*t;  }



struct Coord 
            { 
		        int i,j; 
		        Coord(int i_,int j_):i(i_),j(j_) {}; 
                        Coord() {} 
		        bool operator==(const Coord& c) const { return i==c.i && j==c.j; } 
                        float dist(const Coord& c) const { return (float)sqrt((double)(c.i-i)*(c.i-i)+(c.j-j)*(c.j-j)); }
			bool operator <(const Coord& c) const { return (i==c.i)? (j<c.j) : (i<c.i); }
			int distance_m(const Coord& c) const { return int(fabs((double)i-c.i)+fabs((double)j-c.j)); }
            };


struct OrigSet : public std::multimap<float,Coord>
	     {
	        void initialize(int i,int j);
	     };







#endif
