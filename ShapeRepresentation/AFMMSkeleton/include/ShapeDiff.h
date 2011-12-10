#ifndef _SHAPE_DIFF_H_
#define _SHAPE_DIFF_H_

#include "field.h"
#include "flags.h"

typedef FIELD<float> DFIELD;

//#define COMPUTE_SHAPE_DIFF

#ifdef COMPUTE_SHAPE_DIFF
#include <cstdlib>
#include <Tools/ANNSearch/include/ANN/ANN.h>

class ShapeDiff
{
	int m_nPts;			// actual number of data points 
	int m_nMaxPts;			// maximum number of data points 
	
	ANNpointArray m_dataPts;	// data points 
	ANNpoint m_queryPt;		// query point 
	ANNkd_tree* m_kdTree;		// search structure 
	
	double m_dMaxError;		// cumulative error from all shape points
	
	FLAGS* m_pOrigShapeImg;
	DFIELD* m_pBndryDistImg;
	
	const FLAGS& m_origShapeImg;    // original shape image
	const DFIELD& m_bndryDistImg;   // initial boundary distance 2D array
	DFIELD m_normDistImg;           // precomputed normalized distances to the boundary
	
	//static const int DIM = 2;     // assumed 2 in some places of this code
	//static const int K = 1;       // assumed 1 in some places of this code

	enum {DIM = 2};                 // assumed 2 in some places of this code
	enum {K = 1};                   // assumed 1 in some places of this code
public:
	ShapeDiff(const DFIELD& skelPtsImg, FLAGS* shapePtsImg, DFIELD* bndryDistImg);
	~ShapeDiff();

	double GetMaxError() const { return m_dMaxError; }
	double ComputeError(const FLAGS& newShapeImg);
	void Test(const DFIELD& field) const;
	
	inline double CompDistance(int x, int y);
};
#else
class ShapeDiff
{
public:
	ShapeDiff(const DFIELD& skelPtsImg, FLAGS* shapePtsImg, DFIELD* bndryDistImg) { }

	~ShapeDiff() { }

	double GetMaxError() const { return 0; }
	double ComputeError(const FLAGS& newShapeImg) { return 0; }
	void Test(const DFIELD& field) const { }
	
	inline double CompDistance(int x, int y) { return 0; }
};

#endif //COMPUTE_SHAPE_DIFF

#endif //_SHAPE_DIFF_H_
