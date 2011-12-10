#include <stdio.h>
#include <Tools/MathUtils.h>
#include <string.h>

#include "AffineMotionEstimator.h"

#include "robust.h"
#include "pyramid.h"
#include "gm2.h"
#include "outliers.h"
#include "ctf-image.h"
#include "basicUtils.h"
#include "affine-warp.h"

#define MIN_NUM_INLIERS 25

using namespace vpl;

void MotionMaps::Init(int nx, int ny)
{
	for (int i = 0; i < NUM_MAPS; i++)
		maps[i].set_size(nx, ny);

	// Init vals for outlier and weight maps
	for(int i = 0; i < nx; i++)
      for(int j = 0; j < ny; j++)
      {
		maps[OUTLIERS](i,j) = 1.0; // => outlier_img
		maps[WEIGHTS](i,j) = 0.0; // => weights
      }
}

void MotionMaps::operator=(const MotionMaps& rhs)
{
	for (int i = 0; i < NUM_MAPS; i++)
		maps[i].deep_copy(rhs.maps[i]);
}

MotionMaps::PTRS MotionMaps::GetPtrs()
{
	PTRS pp;

    pp.outlier_img = maps[OUTLIERS].top_left_ptr();
    pp.weights     = maps[WEIGHTS].top_left_ptr();
    pp.lap1        = maps[LAP1].top_left_ptr();
    pp.lap2        = maps[LAP2].top_left_ptr();
    pp.Ix          = maps[IX].top_left_ptr();
    pp.Iy          = maps[IY].top_left_ptr();
    pp.It          = maps[IT].top_left_ptr();
    pp.warped      = maps[WARPED].top_left_ptr();
    pp.mean        = maps[MEAN].top_left_ptr();
    pp.used_pts    = maps[USED_PTS].top_left_ptr();
    
    ASSERT(USED_PTS == NUM_MAPS - 1);
    
    return pp;
}

AffineMotionEstimator::AffineMotionEstimator()
{
	SetDefaultParams();
}

void AffineMotionEstimator::SetDefaultParams()
{
    sigma = 15.0; 		//ending value of sigma
    sigma_init = 25.0; 	//Starting value of sigma
    omega = 1.995f; 
    rate = 0.95f;		//rate at which to decrease sigma
    levels = 3; 		//number of levels in the pyramid
    iters = 30; 		//max number of iterations at each level in the pyramid
    filter = 0; 		//whether the image is filtered or not [0,1]
    affine = 1; 		//fit an affine (1) or constant (0) model
    multiple = 1; 		//number of motions to find (1, 2, or 3)
    
    // Generic initial values
    u=0.0; v=0.0;
    a0=0.0; a1=0.0; a2=0.0; a3=0.0; a4=0.0; a5=0.0; 
    nEstimatedMotions = 0;
}

bool AffineMotionEstimator::EnoughInliers(float* mask)
{
	int i, j, nInliers = 0, nTotal = 0, nZeros = 0, nOnes = 0, nOutliers = 0;
	
	for(i = 0; i < ny; i++)
    	for(j = 0; j < nx; j++)
		{
      		if (mask[(i * nx) + j] >= 0.5)
      			nInliers++;
				
			if (mask[(i * nx) + j] < 0.5)
      			nOutliers++;
				
			if (mask[(i * nx) + j] == 0.0)	
				nZeros++;
				
			if (mask[(i * nx) + j] == 1.0)	
				nOnes++;	
				
			nTotal++;
		}

	if (Verbose())
	{
		DBG_SHOW(nInliers)
		DBG_SHOW(nOutliers)
		DBG_SHOW(nZeros)
		DBG_SHOW(nOnes)
		DBG_SHOW(nTotal)
	}

	return nInliers >= MIN_NUM_INLIERS;
}

bool AffineMotionEstimator::Estimate(FloatImg curFrame, FloatImg prevFrame)
{
	ASSERT(prevFrame.ni() == curFrame.ni() && prevFrame.nj() == curFrame.nj());
	
	// Init all parameters...
	SetDefaultParams();

    nx = prevFrame.ni();
	ny = prevFrame.nj();	
	
	u=0.0; v=0.0;
	a0=u; a1=0.0; a2=0.0;
    a3=v; a4=0.0; a5=0.0;
    
    nEstimatedMotions = 0;
      
	sigma = outlier_to_sigma(sigma);
    sigma_init = outlier_to_sigma(sigma_init);
    
    float* image1 = prevFrame.top_left_ptr();
    float* image2 = curFrame.top_left_ptr();
    
    curMaps.Init(nx, ny);
    MotionMaps::PTRS p = curMaps.GetPtrs();
    
    // Ready to start estimating the first motion...    
	pyramid_regress(image1, image2, levels, &a0, &a1, &a2, &a3, &a4, &a5,
		    p.Ix, p.Iy, p.It, nx, ny, 
		    iters, sigma, sigma_init, omega, p.outlier_img, p.weights,
		    filter, affine, rate);
		    
	// Save the inlier mask of motion 1 (inlier => outlier(i,j)>=0.5), 
	copy_image(p.used_pts, p.outlier_img, nx, ny);
	
	// The inverse of such mask is the "initial" inliers of motion 2
	invert_image(p.outlier_img, nx, ny);

	// Warp, average and substract corresponding images
	affine_warp_image(image1, p.warped, a0, a1, a2, a3, a4, a5, nx, ny);
	average_images(p.warped, image2, p.mean, nx, ny);
	subtract_image(p.warped, image2, nx, ny);
	
	// Save current maps
	motionMaps[nEstimatedMotions++] = curMaps;
	
	if (multiple > 1 && EnoughInliers(p.outlier_img))
	{	  
		a0=u; a1=0.0; a2=0.0;
		a3=v; a4=0.0; a5=0.0;
		
		pyramid_regress(image1, image2, levels, &a0, &a1, &a2, &a3, &a4, &a5,
				p.Ix, p.Iy, p.It, nx, ny, 
				iters, sigma, sigma_init, omega, 
				p.outlier_img, p.weights, filter, affine, rate);
				
		affine_warp_image(image1, p.warped, a0, a1, a2, a3, a4, a5, nx, ny);		
		average_images(p.warped, image2, p.mean, nx, ny);
		subtract_image(p.warped, image2, nx, ny);
		
		motionMaps[nEstimatedMotions++] = curMaps;
	}
	
	// Here, p.outlier_img has the inliers of motion 2. Note that its "inverse"
	// is a "superset" of the inliers of motion 3. To prune this set,
	// we need to also substract the inliers of motion 1.

    if (multiple > 2)
    {   	
		add_image(p.used_pts, p.outlier_img, nx, ny);
		copy_image(p.outlier_img, p.used_pts, nx, ny);
		invert_image(p.outlier_img, nx, ny);
	}
		
	if (multiple > 2 && EnoughInliers(p.outlier_img))
	{	
		a0=u; a1=0.0; a2=0.0;
		a3=v; a4=0.0; a5=0.0;
		
		pyramid_regress(image1, image2, levels, &a0, &a1, &a2, &a3, &a4, &a5,
				p.Ix, p.Iy, p.It, nx, ny, 
				iters, sigma, sigma_init, omega, p.outlier_img, p.weights,
				filter, affine, rate);
		
		affine_warp_image(image1, p.warped, a0, a1, a2, a3, a4, a5, nx, ny);		
		average_images(p.warped, image2, p.mean, nx, ny);		
		subtract_image(p.warped, image2, nx, ny);
		
		motionMaps[nEstimatedMotions++] = curMaps;
	}
	
	return true;
}
