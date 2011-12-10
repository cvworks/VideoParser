/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "TurboPixels.h"
//#include "../LevelSetMethod.h"

#include <VideoParserGUI/DrawingUtils.h>
#include <Tools/UserArguments.h>
#include <Tools/UserEvents.h>
#include <Tools/cv.h>

using namespace vpl;

extern UserArguments g_userArgs;

typedef Matrix& Ref;

Matrix height_function_der(const Matrix& m);

void TurboPixels::Initialize()
{
	double m_boundary_speed_interval = 6;
	unsigned m_numSuperPixels = 200;

	/*vnl_matrix<bool> m0;

	for (auto it0 = (m0).begin(); it0 != m0.end(); ++it0)
		x = y;*/

}

/*!
	Compute the superpixels given an image.
	img - the input image. Either RGB or Grayscale. Must be double in range
	0..1
	numSuperpixels - number of superpixels
	display_int - Interval of frames at which the progress of the evolution
	will be displayed. 0 if not display is needed (default).
	contour_color - color of the superpixel boundaries (default is red)
	
	Returns:
	    phi - the final evolved height function
	    boundary - a logical array representing superpixel boundaries
	    disp_img - the image with the boundaries overlaid on top of the image
	    frames - frames of the evolution in Matlab movie format if
	    display_int was above 0 and output frames parameter was given.
*/
void TurboPixels::ComputeSuperpixels(RGBImg rgbImg, FloatImg greyImg)
{
	unsigned numSuperpixels = 2000;

	double timeStep = 0.5;

	unsigned maxIterations = 500;

	MatArray hf = evolve_height_function_N(rgbImg, greyImg, timeStep, maxIterations, SUPERPIXELS, 
		numSuperpixels);

	m_phi = hf[0];
    
	// The grey image must be normalized to [0,1]
	hf = evolve_height_function_N(rgbImg, greyImg, 0.1, 10, CURVATURE, 0, 0);

	Ref smooth_img = hf[3];

	auto g = height_function_der(smooth_img * 255.0);

	auto mag = sqrt(epow2(g[0]) + epow2(g[1]));

	auto speed2 = exp(- mag / 5.0);

	m_boundary = get_superpixel_boundaries(phi, speed2);
}

/*!
	@return [phi, numIter, speed_grad, smooth_img]

		SUPERPIXELS <- [phi, numIter, speed_grad]
		CURVATURE <- [smooth_img]

*/
MatArray TurboPixels::evolve_height_function_N(RGBImg rgbImg, FloatImg greyImg, double time_step, 
	unsigned num_iterations, SPEED_TYPE speed_type, unsigned numSuperPixels)
{
    Matrix boundary_speed(1.0); // ie, it starts as the scalar 1
    Matrix band(1.0);
    BoolMatrix band_ind, select;

    unsigned MAX_BAND_SIZE = 5;
    double old_coveredArea = 0;
    unsigned alreadyPutSeeds = 0;
    double relativeAreaInc = inf;

    //background_init = [];
    //stoppingFrames = 0;
    
	// Prepare output variables
	MatArray out(4);

	Ref _phi        = out[0];
	Ref _numIter    = out[1];
	Ref _speed_grad = out[2];
	Ref _smooth_img = out[3];

    // Need that many consecutive frames that satisfy the stopping condition to stop
    int numStoppingFrames = 1;
    
	// Normalize to unit interval (all channels) both images
	// IDEA: create a matrix of doubles and work with that
    greyImg = clip(greyImg, 0, 1);
    
	_smooth_img.make_scalar(1.0);
    
	Matrix speed;

    if (speed_type == SUPERPIXELS)
	{
        // Compute the speed of propagation
        _smooth_img = evolve_height_function_N(rgbImg, greyImg, 0.1, 10, CURVATURE, 0, 0);
        
        double expSuperPixelDist = sqrt(greyImg.size() / double(numSuperPixels));

        auto normSigma = floor(expSuperPixelDist / 2.5);

        auto speedInfo = get_speed_based_on_gradient(
			_smooth_img, false, normSigma, _phi);

		_speed_grad      = speedInfo[0];
		Ref speed_grad_x = speedInfo[1];
		Ref speed_grad_y = speedInfo[2];

		speed = _speed_grad;

        // Place initial seeds and compute the initial signed distance function 
        auto seeds = get_initial_seeds(_smooth_img, numSuperPixels, _speed_grad);

        auto binary_img = get_binary_img_from_seeds(seeds, _smooth_img);

        auto background_init = ~binary_img;

        Matrix input_dist = cast<double, bool>(binary_img == 0); //??? check

        input_dist(input_dist == 0) = 999999;

        _phi = DT(input_dist);

        _phi = imfilter(_phi, fspecial("gaussian", make1x2(3,3), 0.5));
	}
    else // curvature, gradient
	{
        _phi = greyImg;
        speed = 1;
	}

	MatArray extension_fields;
	Matrix old_phi;
    
    for (unsigned i = 1; i < num_iterations; i++)
	{        
        bool hasToRecomputeBandFlag = hasToRecomputeBand(_phi, band, MAX_BAND_SIZE);
        
        // Extend the speed if needed
        if ((speed_type == GRADIENT || speed_type == SUPERPIXELS) && hasToRecomputeBandFlag)
		{
			extension_fields[0] = _speed_grad;
            extension_fields[1] = speed_grad_x;
            extension_fields[2] = speed_grad_y;
            
            auto fields2d = computeExtensionFields2d(_phi, extension_fields,
				make1x2(1,1), MAX_BAND_SIZE);

			Ref fm_phi = fields2d[0];
			
			// Note: the following is changed to...
			//Ref fields = fields2d[1];
            
			Ref speed_grad_extended = fields2d[1];
            Ref speed_grad_x_extended = fields2d[2];
            Ref speed_grad_y_extended = fields2d[3];

			select = speed_grad_extended > 0.0;
            
			band_ind = (select & abs(fm_phi) < (MAX_BAND_SIZE - 1));

            band = Zeros(fm_phi.rows(), fm_phi.cols());

            band(select) = abs(fm_phi(select));

            old_phi = _phi;
            
			_phi(select) = fm_phi(select);
            
            if (!isscalar(boundary_speed))
			{
				select = boundary_speed == 0;

                _phi(select) = old_phi(select);
			}
		}
        
        // Update the current speed of evolution
        if (speed_type == SUPERPIXELS)
		{
            if (i < 20)
                speed = speed_grad_extended;
            else
                speed = get_speed_based_on_gradient(smooth_img, 1, normSigma, _phi,
					speed_grad_extended, speed_grad_x_extended, speed_grad_y_extended);
		}

		MatArray hifun;

        // Evolve the height function one time step
        if (mod(i, m_boundary_speed_interval) == 1 && speed_type == SUPERPIXELS)
		{
            hifun = evolve_height_function(_phi, speed, Matrix(), time_step, speed_type, 
				band_ind, background_init);
		}
        else
		{
            hifun = evolve_height_function(_phi, speed, boundary_speed, 
				time_step, speed_type, band_ind);
		}
               
		_phi = hifun[0];
		boundary_speed = hifun[1];

        // Stop based on the relative area increase
        coveredArea = count(_phi(:) < 0);
        relativeAreaInc = (coveredArea - old_coveredArea) / size(_phi);
        old_coveredArea = coveredArea;
        
        if (speed_type == SUPERPIXELS && relativeAreaInc < 1e-4 && coveredArea / size(_phi) > 0.5)
            stoppingFrames++;
        else
            stoppingFrames = 0;
        
        if (stoppingFrames >= numStoppingFrames)
		{
            numIter = i;
            break;
		}
	}
}

Matrix TurboPixels::get_initial_seeds(FloatImg img, unsigned num_seeds, Matrix speed, 
	SEED_METHOD method /*= GRID_SEEDS*/)
{
	Matrix seeds;

	if (method == GRID_SEEDS)
		seeds = get_seeds_orig(img, num_seeds, speed);
	else if (method == RANDOM_SEEDS)
		seeds = get_seeds_random(img, num_seeds);
	else
		ASSERT(false);

	return seeds;
}

//! randomly pick num_seeds image locations that are not close to edges
//! (seed placement is not particularly "nice", but this is as random
//! as it gets...)
Matrix get_seeds_random(FloatImg img, unsigned num_seeds)
{
	nonedges = find(imfilter(im2double(edge(img,"canny")),
		fspecial("gaussian", make1x2(5,5), 1.5)) == 0);

	[I,J] = ind2sub(img.size(),
		nonedges(floor(length(nonedges) * rand(num_seeds,1) + 1)));
	
	Matrix seeds = [J I];

	return seeds;
}

//! start with seeds placed on a regular grid, then add a small random
//! displacement to each seed, them move them to local minima of gradient
Matrix get_seeds_orig(FloatImg img, unsigned num_seeds, Matrix speed)
{
    double size_grid = sqrt(img.size() / double(num_seeds));
    
    double rows = img.rows() / size_grid;
    double cols = img.cols() / size_grid;

    double size_grid_row = img.rows() / ceil(rows);
    double size_grid_col = img.cols() / ceil(cols);

    auto coords = meshgrid(0:ceil(rows-1), 0:ceil(cols-1)); //[y,x]

	Ref y = coords[0];
	Ref x = coords[1];

    x = x * size_grid_col + size_grid_col / 2.0;
    y = y * size_grid_row + size_grid_row / 2.0;
    
    auto mag = 1 - speed;
    
    // Pick only local minima for initialized seeds
    auto minDistBetweenSeeds = min(size_grid_row, size_grid_col);
    double seedRadius = 1;
    
    double maxShift = floor((minDistBetweenSeeds - 2 * seedRadius) / 2.0) - 1;

    auto d = local_min(mag, ceil(maxShift / 2.0)); //[dy,dx]

	Ref dy = d[0];
	Ref dx = d[1];

    auto new_x = dx(round(y(1,:)), round(x(:,1))).transpose();
    auto new_y = dy(round(y(1,:)), round(x(:,1))).transpose();

    x = new_x;
    y = new_y;

    off = 2;

    x = max(1 + off, min(x, img.cols() - off));
    y = max(1 + off, min(y, img.rows() - off));

    Matrix seeds(x.size(), 2);
	
	seeds.assign_as_column(0, x);
	seeds.assign_as_column(1, y);

	return seeds;
}

// Return the thinned superpixel boundaries
//function boundaries =
Matrix get_superpixel_boundaries(Matrix phi, Matrix speed)
{
    smallAreaThresh = 2e-5;
    
    if (islogical(phi))
	{
        boundaries = bwmorph(bwmorph(phi,"skel",Inf),"spur",Inf);
	}
    else
	{
        contour = zero_crossing(phi);
        background = (phi>=0 | contour);
        background_clean = bwmorph(bwmorph(background,"clean",Inf),"spur",Inf);                

        mask = double(background_clean) - 0.5;
        phi = computeDistanceFunction2d(phi, make1x2(1,1), mask);
        skeleton = doHomotopicThinning(phi, mask);
        
        // inside is the original superpixels plus the holes
        inside = (~background_clean | (background_clean & ~contour));
        
        // Fill narrow holes
        near_background = imfilter(double(~inside), Ones(3,3)) > 0;
        near_skeleton = imfilter(double(skeleton), Ones(3,3)) > 0;
        inside(near_skeleton & near_background) = 0;
        
        L = bwlabel(inside, 4);
        stats = regionprops(L, "Area");

        
        // Set small regions to be the background, so that they would be
        // thinned
        bad_superpixel = [stats(:).Area] < (smallAreaThresh * size(phi));

        for i=1:length(stats)
		{
            if (bad_superpixel(i))
                L(L==i) = 0;
		}

        inside = (L>0);

        phi = computeDistanceFunction2d(double(~inside)-0.5, make1x2(1,1));
        distance_ordering = phi;
        
        if (nargin > 1)            
		{
            distance_ordering = emul(1 - epow(speed, 0.25), 1 - speed) 
				+ emul(epow(speed, 0.25), ediv(phi max(phi(:))));
		}

        contour = zero_crossing(phi);
        background = (phi>=0 | contour);
        boundaries = bwmorph((doHomotopicThinning(distance_ordering,double(background)-0.5)),"clean", Inf);
        
		erode_kernel = [0, 1, 0; 1, 0, 1; 0, 1, 0];

        eroded_boundaries = imerode(boundaries, erode_kernel);
        boundaries(1,:) = eroded_boundaries(1,:);
        boundaries(:,1) = eroded_boundaries(:,1);
        boundaries(end,:) = eroded_boundaries(end,:);
        boundaries(:,end) = eroded_boundaries(:,end);
        boundaries = bwmorph(boundaries,"clean");
	}
}

Matrix get_speed_based_on_boundaries(Matrix phi, Matrix background_init)
{
    auto background = (phi >= 0 | zero_crossing(phi));

    mask = double(background & background_init) - 0.5;
    
	phi = computeDistanceFunction2d(phi, make1x2(1, 1), mask);
    
    auto skeleton = doHomotopicThinning(phi, mask);

    Matrix speed = Ones(phi.rows(), phi.cols());
	
    speed(skeleton) = 0;

	return speed;
}

/*!
	@param doDoublet Flag controlling relative vs. absolute gradient magnitude.
*/
MatArray get_speed_based_on_gradient(FloatImg img, bool doDoublet /*= false*/, double normSigma /*= 5*/, 
	Matrix phi, Matrix in_speed, Matrix in_speed_x, Matrix in_speed_y)
{
	MatArray out(5);

	Ref _speed = out[0];
	Ref _speed_x = out[1];
	Ref _speed_y = out[2];

	_speed = in_speed;
	_speed_x = in_speed_x;
	_speed_y = in_speed_y;

    if (_speed.empty())
	{
        auto g = height_function_der(255.0 * img);

		Ref gx = g[0];
		Ref gy = g[1];

        auto mag = sqrt(epow2(gx) + epow2(gy));

        auto ss_mag = corrDn(mag, make1x1(1), "repeat", make1x2(2,2));
        auto stdev = normSigma;
        auto nTaps = round(3 * stdev) * 2 + 1;
        
		auto lpImpResp = fspecial("gaussian", make1x2(1,nTaps), stdev);   // sample to 3 std devs
        
		// scale s.t. max value of impulse response is 1.0 (vs. sums to 1.0)
        lpImpResp = lpImpResp / max(lpImpResp);
        
		auto smooth_ssmag0 = imfilter(ss_mag, lpImpResp);
        
		auto smooth_ssmag = imfilter(smooth_ssmag0, lpImpResp.transpose());
        
		// upBlur  (check this since it looks like the respons
        // eis not as large as it should be.
        auto f = make3x1(0.5, 1.0, 0.5);

        auto res = upConv(smooth_ssmag, f, "reflect1", make1x2(2,1));

        smooth_mag = upConv(res, f.transpose(), "reflect1", make1x2(1,2));

        // scale so that a long strgiht edge does not compete with itself
        // i.e. with contrast normalization, a long straight edge should
        // produce max(max(mag)) == max(max(smooth_nag))
        smooth_mag = smooth_mag / (sqrt(2*pi) * stdev);

        if (smooth_mag.rows() != mag.rows())
            smooth_mag = smooth_mag(1:end-1,:);
        

        if (smooth_mag.cols() != mag.cols())
            smooth_mag = smooth_mag(:,1:end-1);

        // normalized response s.t.
        //   - we control the gradient mag that is mapped to half height
        //   - the result is mapped to [0,127]
        magHalfHeight = 10.0;

        normGradMag = 127 * ediv(mag, (magHalfHeight + smooth_mag));

        speed = exp(-normGradMag / 10.0);

        auto hf = height_function_der(speed);

		_speed_x = hf[0];
		_speed_y = hf[1];
	}
    
    if (doDoublet)
	{
        auto hf = height_function_der(phi);

		Ref dx = hf[0];
		Ref dy = hf[1];
		Ref dxx = hf[2];
		Ref dyy = hf[3];
		Ref dxy = hf[4];
        
        speed = get_full_speed(dx, dy, dxx, dyy, dxy, _speed, _speed_x, _speed_y, 1);
	}

	return out;
}
