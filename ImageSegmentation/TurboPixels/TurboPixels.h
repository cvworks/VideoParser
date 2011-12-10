/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/LinearAlgebra.h>
#include <Tools/ImageUtils.h>

namespace vpl {

class TurboPixels
{
	enum SPEED_TYPE {SUPERPIXELS, CURVATURE, GRADIENT};
	enum SEED_METHOD {GRID_SEEDS, RANDOM_SEEDS};

	Matrix m_phi;
	
	Matrix m_boundary;
	
protected:
	MatArray evolve_height_function_N(RGBImg rgbImg, FloatImg greyImg, double time_step, 
		unsigned num_iterations, SPEED_TYPE speed_type, unsigned numSuperPixels);

	Matrix get_initial_seeds(FloatImg img, unsigned num_seeds, Matrix speed, 
		SEED_METHOD method /*= GRID_SEEDS*/);

	Matrix get_seeds_random(FloatImg img, unsigned num_seeds);
	Matrix get_seeds_orig(FloatImg img, unsigned num_seeds, Matrix speed);
	Matrix get_superpixel_boundaries(Matrix phi, Matrix speed);
	Matrix get_speed_based_on_boundaries(Matrix phi, Matrix background_init);

	MatArray get_speed_based_on_gradient(FloatImg img, bool doDoublet /*= false*/, double normSigma /*= 5*/, 
		Matrix phi, Matrix in_speed, Matrix in_speed_x, Matrix in_speed_y);
public:
	void Initialize();
	void ComputeSuperpixels(RGBImg img, FloatImg greyImg);
};


} // namespace vpl

