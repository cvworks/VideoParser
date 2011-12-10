/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "ImageSegmenter.h"
#include <Tools/UserArguments.h>
#include <Tools/ImageUtils.h>
#include <Tools/IplImageView.h>
#include <Tools/LinearAlgebra.h>

#define POW_25_7 6103515625

using namespace vpl;

extern UserArguments g_userArgs;

//! Static array of colors used to display ALL segmentations
std::vector<RGBColor> ImageSegmenter::s_colors;

void ImageSegmenter::ReadParamsFromUserArguments()
{
	VisSysComponent::ReadParamsFromUserArguments();

	g_userArgs.ReadArg(Name(), "luminanceWeightFactor", 
		"Weight fuctor used to measure luminance differences.", 
		1.0, &m_colorSpaceParams.luminanceWeightFactor);

	g_userArgs.ReadArg(Name(), "chromaWeightFactor", 
		"Weight fuctor used to measure chroma differences.", 
		1.0, &m_colorSpaceParams.chromaWeightFactor);

	g_userArgs.ReadArg(Name(), "hueWeightFactor", 
		"Weight fuctor used to measure hue differences.", 
		1.0, &m_colorSpaceParams.hueWeightFactor);
}

void ImageSegmenter::Initialize(graph::node v)
{
	VisSysComponent::Initialize(v);

	m_pImgProcessor = FindParentComponent(ImageProcessor);
}

//: function de00 = deltaE2000(Labstd,Labsample, KLCH )
// Compute the CIEDE2000 color-difference between the sample between a reference
// with CIELab coordinates Labsample and a standard with CIELab coordinates 
// Labstd
// The function works on multiple standard and sample vectors too
// provided Labstd and Labsample are K x 3 matrices with samples and 
// standard specification in corresponding rows of Labstd and Labsample
// The optional argument KLCH is a 1x3 vector containing the
// the value of the parametric weighting factors kL, kC, and kH
// these default to 1 if KLCH is not specified.

// Based on the article:
// "The CIEDE2000 Color-Difference Formula: Implementation Notes, 
// Supplementary Test Data, and Mathematical Observations,", G. Sharma, 
// W. Wu, E. N. Dalal, submitted to Color Research and Application, 
// January 2004.
// available at http://www.ece.rochester.edu/~/gsharma/ciede2000/ 
double ImageSegmenter::ColorDiff(const LabColor& Labstd, const LabColor& Labsample) const
{
	double Lstd = (double) Labstd.R();
	double astd = (double) Labstd.G();
	double bstd = (double) Labstd.B();

	double Cabstd = sqrt(epow2(astd) + epow2(bstd));

	double Lsample = (double) Labsample.R();
	double asample = (double) Labsample.G();
	double bsample = (double) Labsample.B();

	double Cabsample = sqrt(epow2(asample) + epow2(bsample));

	double Cabarithmean = (Cabstd + Cabsample) / 2.0;

	double G = 0.5* ( 1 - sqrt( (Cabarithmean * 7.0) / ( (Cabarithmean * 7.0) + POW_25_7) ));

	double apstd = ((1 + G) * astd); // aprime in paper
	double apsample = ((1 + G) * asample); // aprime in paper
	double Cpsample = sqrt(epow2(apsample) + epow2(bsample));
	double Cpstd = sqrt(epow2(apstd) + epow2(bstd));

	// Compute product of chromas and locations at which it is zero for use later
	double Cpprod = (Cpsample * Cpstd);
	double zcidx = (Cpprod == 0);

	// Ensure hue is between 0 and 2pi
	// In C++, "Principal arc tangent of y/x, in the interval [-pi,+pi] radians."
	double hpstd = atan2(bstd, apstd);

	// DIEGO. Just in case, write the following line differently
	//hpstd = hpstd + 2 * M_PI * (hpstd < 0);  // rollover ones that come -ve
	if (hpstd < 0)
		hpstd += 2 * M_PI;
	
	// NOTE: MATLAB already defines atan2(0,0) as zero but explicitly set it
	// just in case future definitions change
	if ((abs(apstd) + abs(bstd)) == 0)
		hpstd = 0;

	double hpsample = atan2(bsample, apsample);

	// DIEGO. Just in case, write the following line differently
	//hpsample = hpsample+2 * M_PI * (hpsample < 0);
	if (hpsample < 0)
		hpsample += 2 * M_PI;

	if ( (abs(apsample) + abs(bsample)) == 0 )
		hpsample = 0;

	double dL = (Lsample-Lstd);
	double dC = (Cpsample-Cpstd);

	// Computation of hue difference
	double dhp = (hpsample - hpstd);

	// DIEGO dhp = dhp - 2.0 * M_PI * (dhp > M_PI );
	if (dhp > M_PI )
		dhp -= 2.0 * M_PI;


	// DIEGO dhp = dhp + 2.0 * M_PI * (dhp < (-M_PI) );
	if (dhp < (-M_PI))
		dhp += 2.0 * M_PI;

	// set chroma difference to zero if the product of chromas is zero
	if (zcidx)
		dhp = 0;

	// Note that the defining equations actually need
	// signed Hue and chroma differences which is different
	// from prior color difference formulae

	double dH = 2.0 * sqrt(Cpprod) * sin(dhp / 2.0);

	// weighting functions
	double Lp = (Lsample + Lstd) / 2.0;
	double Cp = (Cpstd + Cpsample) / 2.0;

	// Average Hue Computation
	// This is equivalent to that in the paper but simpler programmatically.
	// Note average hue is computed in radians and converted to degrees only 
	// where needed
	double hp = (hpstd + hpsample) / 2.0;

	// Identify positions for which abs hue diff exceeds 180 degrees 
	// DIEGO: hp = hp - ( abs(hpstd-hpsample)  > M_PI ) * M_PI;
	if (abs(hpstd-hpsample)  > M_PI)
		hp -= M_PI;
	
	// rollover ones that come -ve
	// DIEGO hp = hp + (hp < 0) * 2.0 * M_PI;
	if (hp < 0)
		hp += 2.0 * M_PI;
	
	// Check if one of the chroma values is zero, in which case set 
	// mean hue to the sum which is equivalent to other value
	if (zcidx)
		hp = hpsample + hpstd;

	double Lpm502 = epow2(Lp - 50);
	
	double Sl = 1 + 0.015 * Lpm502 / sqrt(20 + Lpm502);  
	
	double Sc = 1 + 0.045 * Cp;

	double T = 1 - 0.17*cos(hp - M_PI / 6 ) + 0.24 * cos(2 * hp) + 0.32 * cos(3 * hp + M_PI / 30.0)
		- 0.20 * cos(4 * hp - 63.0 * M_PI / 180.0);

	double Sh = 1 + 0.015 * Cp * T;

	double delthetarad = (30.0 * M_PI / 180.0) * exp(- epow2( (180.0 / M_PI * hp - 275.0) / 25.0 ));
	
	double Rc =  2 * sqrt( epow(Cp, 7) / (epow(Cp, 7) + POW_25_7) );
	
	double RT =  -sin(2 * delthetarad) * Rc;

	double kl = m_colorSpaceParams.luminanceWeightFactor;
	double kc = m_colorSpaceParams.chromaWeightFactor;
	double kh = m_colorSpaceParams.hueWeightFactor;

	double klSl = kl * Sl;
	double kcSc = kc * Sc;
	double khSh = kh * Sh;

	/*ASSERT_VALID_DENOMINATOR(klSl);
	ASSERT_VALID_DENOMINATOR(kcSc);
	ASSERT_VALID_DENOMINATOR(khSh);

	ASSERT_VALID_NUM(dL);
	ASSERT_VALID_NUM(dC);
	ASSERT_VALID_NUM(dH);

	ASSERT_VALID_NUM(RT);*/

	double det00_2 = epow2(dL / klSl) + epow2(dC / kcSc) + epow2(dH / khSh) + 
		RT * (dC / kcSc) * (dH / khSh);

	ASSERT(det00_2 >= 0);

	// The CIE 00 color difference
	double de00 = sqrt(det00_2);

	ASSERT_VALID_NUM(de00);

	return de00;
}
