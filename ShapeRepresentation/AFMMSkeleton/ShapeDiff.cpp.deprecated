#include "ShapeDiff.h"
#include <CImg/CImg.h>

#define DIST_UNKOWN -1

ShapeDiff::ShapeDiff(const DFIELD& skelPtsImg, FLAGS* shapeImg, DFIELD* bndryDistImg) 
	: m_pOrigShapeImg(shapeImg), m_pBndryDistImg(bndryDistImg),
	  m_origShapeImg(*shapeImg), m_bndryDistImg(*bndryDistImg), 
	  m_normDistImg(skelPtsImg.dimX(),skelPtsImg.dimY())
{
	int x, y;
	
	m_nPts = 0;
	m_nMaxPts = skelPtsImg.dimX() * skelPtsImg.dimY();
	m_dMaxError = 0;
	
	m_queryPt = annAllocPt(DIM);			// allocate query point 
	m_dataPts = annAllocPts(m_nMaxPts, DIM);	// allocate data points 

	// Add skeleton points
	for (x = 0; x < skelPtsImg.dimX(); x++)
		for (y = 0; y < skelPtsImg.dimY(); y++)
			if (skelPtsImg.value(x,y) != 0)
			{
				m_dataPts[m_nPts][0] = x;
				m_dataPts[m_nPts][1] = y;
				m_nPts++;
			}

	// Create KD-tree for efficient nearest neighbour search
	m_kdTree = new ANNkd_tree(m_dataPts, m_nPts, DIM); 

	// Precompute distances from all shape pts and accumulate error
	for (x = 0; x < shapeImg->dimX(); x++)
		for (y = 0; y < shapeImg->dimY(); y++)
		{
			// if skel pt, distance to closest skel pt is 0, so normDist = radius / radius = 1
			if (skelPtsImg.value(x,y) != 0) 
				m_normDistImg.value(x,y) = 1;
			// if pt in the shape, then we must find closest skel pt
			else if (shapeImg->alive(x,y) || shapeImg->narrowband(x,y))
			{
				m_normDistImg.value(x,y) = CompDistance(x,y);
				m_dMaxError += m_normDistImg.value(x,y);
			}
			// Otherwise, we don't need the dist. Assign DIST_UNKOWN == -1
			else
				m_normDistImg.value(x,y) = DIST_UNKOWN;	
		}
}

ShapeDiff::~ShapeDiff()
{
	delete m_kdTree; 
	annDeallocPt(m_queryPt);
	annDeallocPts(m_dataPts);
	annClose();
	
	delete m_pOrigShapeImg;
	delete m_pBndryDistImg;
}

double ShapeDiff::CompDistance(int x, int y)
{
	ANNidx idx;		// 1-element array of indices 
	ANNdist dist;		// 1-element array of NN distances
	double dNormError;
	float radius;
	
	m_queryPt[0] = x; 
	m_queryPt[1] = y;
	
	m_kdTree->annkSearch(m_queryPt, K, &idx, &dist); 
	radius = m_bndryDistImg.value((int)m_dataPts[idx][0], (int)m_dataPts[idx][1]);
	
	dist = sqrt(dist);
	
	if (dist < radius)
		dNormError = 1 - dist / radius;
	else
	{
		if (m_origShapeImg.alive(x,y) || m_origShapeImg.narrowband(x,y))
			dNormError = 0.1;
		else
			dNormError = m_bndryDistImg.value(x,y) / radius;
	}	
	
	//dNormError *= dNormError;
// 	if(dNormError < 0)
// 	{
// 		cerr << dNormError << endl;
// 		dNormError *= -1;
// 	}
	
	if (dNormError > 1) dNormError = 1;
	
	return dNormError;
}

double ShapeDiff::ComputeError(const FLAGS& newShapeImg)
{
	bool bShapePt1, bShapePt2;
	double dError = 0;
	int x, y;
	
	DFIELD diff(m_origShapeImg.dimX(), m_origShapeImg.dimY());

	for (x = 0; x < m_origShapeImg.dimX(); x++)
		for (y = 0; y < m_origShapeImg.dimY(); y++)
		{
			bShapePt1 = (newShapeImg.alive(x,y) || newShapeImg.narrowband(x,y));
			bShapePt2 = (m_origShapeImg.alive(x,y) || m_origShapeImg.narrowband(x,y));

			if (bShapePt1 != bShapePt2)
			{
				if (m_normDistImg.value(x,y) == DIST_UNKOWN)
					m_normDistImg.value(x,y) = CompDistance(x,y);
				
				dError += m_normDistImg.value(x,y);
				
				diff.value(x,y) = 1;
			}
			else
				diff.value(x,y) = 0;
		}
		
	//Test(diff);
	//Test(m_normDistImg);

	return dError / GetMaxError();
}

void ShapeDiff::Test(const DFIELD& field) const
{
	/*cimg_library::CImg<float> img(field.dimX(), field.dimY());

	img.dimy();*/

	static int n = 0;
	//using namespace cimg_library;
	
	cimg_library::CImg<float> img(field.dimX(), field.dimY());
	
	if (n++ == 8)
	{
		for (int x = 0; x < field.dimX(); x++)
			for (int y = 0; y < field.dimY(); y++)
				img(x,y) = field.value(x,y);
		
		//img.save_ascii("newDistErr.m");
		
		/*cimg_library::CImgDisplay main_disp(img.get_normalize(0,255),"Normalized Image");
		
		while (!main_disp.closed)
			main_disp.wait();*/
	}
}

