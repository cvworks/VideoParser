#ifndef _SHAPE_DIFF_H_
#define _SHAPE_DIFF_H_

#include "field.h"
#include "flags.h"
#include <cstdlib>
#include <../../ann_1.1/include/ANN/ANN.h>

#define DIST_UNKOWN -1

typedef FIELD<float> DFIELD;

class ShapeDiff
{
	int m_nPts;			// actual number of data points 
	int m_nMaxPts;			// maximum number of data points 
	
	ANNpointArray m_dataPts;	// data points 
	ANNpoint m_queryPt;		// query point 
	ANNkd_tree* m_kdTree;		// search structure 
	
	double m_dMaxError;		// cumulative error from all shape points
	
	FLAGS* m_pOrigShapeImg;	// original shape image
	DFIELD* m_pBndryDistImg;	// initial boundary distance 2D array
	DFIELD m_normDistImg;		// precomputed normalized distances to the boundary
	
	static const int DIM = 2;	// assumed 2 in some places of this code
	static const int K = 1;		// assumed 1 in some places of this code
public:
	ShapeDiff(const DFIELD& skelPtsImg, FLAGS* shapePtsImg, DFIELD* bndryDistImg);
	~ShapeDiff();

	double GetMaxError() const { return m_dMaxError; }
	double ComputeError(const FLAGS& newShapeImg);
	void Test(const DFIELD& field) const;
	
	inline double CompDistance(int x, int y);
};

#endif //_SHAPE_DIFF_H_
