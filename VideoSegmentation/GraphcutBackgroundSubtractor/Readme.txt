This is a C implementation of background subtraction given a set of background frames as a training set.
I use Vladimir Kolmogorov's graphcut source code from:
http://www.adastral.ucl.ac.uk/~vladkolm/software.html
The background model is per-pixel RGB space Gaussian, assuming independence between RGB channels.
OpenCV 1.0 is required for I/O purpose.

The code does the following:
(1) background RGB Gaussian model training. There is no maximum number limit for the training images. The minimum is 1 frame. In this case, the user should specify the desired variance for R,G,B channel for every pixel.
(2) shadow modelling. Please refer to my technical report (http://www.cs.unc.edu/~lguan/COMP255.files/FinalReport.htm) Section 3.1 and N. Martel-Brisson & A. Zaccarin's "Moving Cast Shadow Detection from a Gaussian Mixture Shadow Model" paper for details about shadow removal.
(3) graphcut cleaning. Please refer to Boykov et.al.'s graphcut papers for more detail. 
(4) non-recursive largest binary blob finding.

The code has been tested with sample datasets with the same set of parameters under WindowsXP environment. 
Dataset can be downloaded at http://www.cs.unc.edu/~lguan/Software/BackgroundSubtractionSampleData.zip

The main file is Segment.cpp (although cpp files are used, it is really C.)
The parameters that need to specify are as follows:

dir              -- background image files directory
inputext         -- background image & foregroundimage files format extension
bgFilename       -- background image files prefix, such as "bg"
digit            -- background image files & foreground image files numbering format, (how many digits)
backgroundStart  -- background image file starting number
backgroundEnd    -- background image file ending number (ending number should be at least the same as starting number)
dirI             -- foreground image files directory
fgFilename       -- foreground image & output image files prefix, such as "fg"
foregroundStart  -- foreground image file starting number
foregroundEnd    -- foreground image file ending number (ending number should be at least the same as starting number)

outputdir        -- output segmented image directory
outputext        -- output files format extension

defaultBackgroundVariance          -- (49.0/255.0/255.0) default background per-pixel Guassian variance.
expan                              -- expan for the deviation for background modes
PFprior                            -- (0.3) the prior for a pixel to be foreground pixel
PF                                 -- (1.0/100000.0) the possibility given a color to be foreground (ideally should be 1/(255)^3)
shadow_calculation_threshold_soft  -- (0.9) ranging [shadow_calculation_threshold_hard, 1]
shadow_calculation_threshold_hard  -- (0.3) ranging [0, shadow_calculation_threshold_soft]
shadow_sigma_soft                  -- (1.7)
shadow_sigma_hard                  -- (0.025)

CUT_alpha                          -- (0.2) graphcut smoothness control, namely the neighborhood N-node weight
CUT_fWeight                        -- (0.5) graphcut T-node (to the foreground label) weight


Only non-commercial use is permitted from the author.
For more information or questions, please feel free to contact lguan@cs.unc.edu

Li Guan
Oct 2nd, 2008