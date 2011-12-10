/**------------------------------------------------------------------------
* All Rights Reserved
* Author: Diego Macrini
*-----------------------------------------------------------------------*/
#include "GraphcutBackgroundSubtractor.h"
#include <VideoParser/ImageProcessor.h>
#include <Tools/IplImageView.h>
#include <Tools/UserArguments.h>
#include "blob.h"
#include "common.h"

#define VERY_SMALL_WEIGHT 0.000000000000000000000000000000000000001

using namespace vpl;

extern UserArguments g_userArgs;

void GraphcutBackgroundSubtractor::ReadParamsFromUserArguments()
{
	// Read the parameters in for the base class first
	BackgroundSubtractor::ReadParamsFromUserArguments();

	defaultBackgroundVariance = 49.0/255.0/255.0; // deviation^2
	expan = 0.6;                                  // expan for the deviation for background modes
	PFprior = 0.3;                                // the prior for a pixel to be foreground pixel
	PF = 1.0/100000.0;                            // the possibility given a color to be foreground 
	                                              // (ideally should be 1/(255)^3)

	shadow_calculation_threshold_soft = 0.9;	  // 0 - 1
	shadow_calculation_threshold_hard = 0.3;	  // 0 - 1
	shadow_sigma_hard = 0.025;                    //shadow length [0 - sqrt(3)]
	shadow_sigma_soft = 1.7;                      //shadow length [0 - sqrt(3)]

	CUT_alpha = 0.2;
	CUT_fWeight = 0.5;
}

void GraphcutBackgroundSubtractor::Clear()
{
	BackgroundSubtractor::Clear();

	if (m_hasBackgroundModel)
	{
		cvReleaseImage(&mean_image);
		cvReleaseImage(&test1_image);
		cvReleaseImage(&test2_image);
		cvReleaseImage(&test3_image);
		cvReleaseImage(&test4_image);

		delete[] Mr; 
		delete[] Mg;
		delete[] Mb;
		delete[] Crr;
		delete[] Cgg;
		delete[] Cbb;

		delete[] finalWeight;
		delete[] backgroundWeight;
		delete[] shadowWeight;

		delete[] nodes;

		m_hasBackgroundModel = false;
	}
}

void GraphcutBackgroundSubtractor::AllocateMemory(int width, int height)
{
	ASSERT(!m_hasBackgroundModel);

	m_hasBackgroundModel = true;

	m_frameSize.Set(width, height);

	CvSize sz;
	sz.width = m_frameSize.width;
	sz.height = m_frameSize.height;

	mean_image = cvCreateImage(sz, IPL_DEPTH_8U, 3);
	test1_image = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	test2_image = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	test3_image = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	test4_image = cvCreateImage(sz, IPL_DEPTH_8U, 1);

	int arraySize = m_frameSize.width * m_frameSize.height;

	Mr = new double[arraySize];
	Mg = new double[arraySize];
	Mb = new double[arraySize];
	Crr = new double[arraySize];
	Cgg = new double[arraySize];
	Cbb = new double[arraySize];

	finalWeight = new double[arraySize];
	backgroundWeight = new double[arraySize];
	shadowWeight = new double[arraySize];

	nodes = new graphcut::Graph::node_id[arraySize];

	// Initialize memory
	int i, j, idx;

	for (i = 0; i < m_frameSize.height; i++) 
	{
		for (j = 0; j < m_frameSize.width; j++) 
		{
			idx = m_frameSize.width * i + j;

			Mr[idx] = 0;
			Mg[idx] = 0;
			Mb[idx] = 0;
			Crr[idx] = 0;
			Cgg[idx] = 0;
			Cbb[idx] = 0;
		}
	}
}

void GraphcutBackgroundSubtractor::LearnBackgroundModelOffline()
{
	BackgroundSubtractor::LearnBackgroundModelOffline();

	int i, j, idx;

	if (m_basicParams.trainingLength == 1)
	{
		for (i = 0; i < m_frameSize.height; i++) 
		{
			for (j = 0; j < m_frameSize.width; j++) 
			{
				idx = m_frameSize.width * i + j;

				Crr[idx] = defaultBackgroundVariance;
				Cgg[idx] = defaultBackgroundVariance;
				Cbb[idx] = defaultBackgroundVariance;
			}
		}
	}

	//store deviation in Cxx 
	for (i = 0; i < m_frameSize.height; i++) 
	{
		for (j=0; j < m_frameSize.width; j++) 
		{
			idx = m_frameSize.width * i + j;

			Crr[idx] = sqrt(Crr[idx]);
			Cgg[idx] = sqrt(Cgg[idx]);
			Cbb[idx] = sqrt(Cbb[idx]);

			((unsigned char *)(mean_image->imageData + mean_image->widthStep*i))[j*3] = (char)(Mr[idx] * 255.0);
			((unsigned char *)(mean_image->imageData + mean_image->widthStep*i))[j*3+1] = (char)(Mg[idx] * 255.0);
			((unsigned char *)(mean_image->imageData + mean_image->widthStep*i))[j*3+2] = (char)(Mb[idx] * 255.0);
		}
	}

	//mean_image is ready here -> it's the background image
}

/*!
	 @see Section 3.1 in Documentation/Background subtraction/FinalReport.htm
*/
void GraphcutBackgroundSubtractor::ProcessBackgroundFrame(RGBImg sample, 
	FloatImg greyImg, fnum_t frameIndex)
{
	if (!m_hasBackgroundModel)
		AllocateMemory(sample.ni(), sample.nj());

	double R, G, B;
	double oldMeanR, oldMeanG, oldMeanB;
	int i, j, idx;

	IplImageView iplView(sample, false);

	frame1 = iplView;

	// Create a DOUBLE variable to hold an increment of the frame index
	double dFrameIdxPlus1 = double(frameIndex + 1);

	for (i = 0; i< m_frameSize.height; i++) 
	{
		for (j = 0; j < m_frameSize.width; j++) 
		{
			idx = m_frameSize.width * i + j;

			R = ((unsigned char *)(frame1->imageData + frame1->widthStep * i))[j*3];
			G = ((unsigned char *)(frame1->imageData + frame1->widthStep * i))[j*3+1];
			B = ((unsigned char *)(frame1->imageData + frame1->widthStep * i))[j*3+2];

			R /= 255.0;
			G /= 255.0;
			B /= 255.0;

			oldMeanR = Mr[idx];
			oldMeanG = Mg[idx];
			oldMeanB = Mb[idx];

			Mr[idx] = (Mr[idx] * frameIndex + R) / dFrameIdxPlus1;
			Mg[idx] = (Mg[idx] * frameIndex + G) / dFrameIdxPlus1;
			Mb[idx] = (Mb[idx] * frameIndex + B) / dFrameIdxPlus1;

			//Variance = ((B-Mean).*(B-Mean) + Variance*(num-1) + (num-1)*(oldMean - Mean).*(oldMean - Mean))/num;
			Crr[idx] = ((R - Mr[idx]) * (R - Mr[idx]) + Crr[idx] * frameIndex 
				+ frameIndex * (oldMeanR - Mr[idx]) * (oldMeanR - Mr[idx])) / dFrameIdxPlus1;

			Cgg[idx] = ((G - Mg[idx]) * (G - Mg[idx]) + Cgg[idx] * frameIndex 
				+ frameIndex * (oldMeanG - Mg[idx])*(oldMeanG - Mg[idx])) / dFrameIdxPlus1;

			Cbb[idx] = ((B - Mb[idx]) * (B - Mb[idx]) + Cbb[idx] * frameIndex 
				+ frameIndex * (oldMeanB - Mb[idx]) * (oldMeanB - Mb[idx])) / dFrameIdxPlus1;
		}
	}
}

/*!
	 @see Section 3.1 in Documentation/Background subtraction/FinalReport.htm
*/
void GraphcutBackgroundSubtractor::FindPreliminaryForeground()
{
	int i, j, idx;
	double R, G, B;
	double forR, forG, forB;
	double R1, G1, B1, crr, cgg, cbb;
	double nR, nG, nB, ratio;

	for (i = 0; i< m_frameSize.height; i++) 
	{
		for (j = 0; j < m_frameSize.width; j++) 
		{
			idx = m_frameSize.width * i + j;

			R = ((unsigned char *)(frame1->imageData + frame1->widthStep*i))[j*3];
			G = ((unsigned char *)(frame1->imageData + frame1->widthStep*i))[j*3+1];
			B = ((unsigned char *)(frame1->imageData + frame1->widthStep*i))[j*3+2];

			R /= 255.0;
			G /= 255.0;
			B /= 255.0;

			R1 = R - Mr[idx];
			G1 = G - Mg[idx];
			B1 = B - Mb[idx];

			crr = Crr[idx]*expan;
			cgg = Cgg[idx]*expan;
			cbb = Cbb[idx]*expan;

			if(crr<1e-5)
				crr = 1e-5;

			forR = R1*R1/crr/crr;

			if(cgg<1e-5)
				cgg = 1e-5;

			forG = G1*G1/cgg/cgg;

			if(cbb<1e-5)
				cbb = 1e-5;

			forB = B1*B1/cbb/cbb;

			double index = forR + forG + forB;		

			backgroundWeight[idx] = exp(-0.5*index) /2.0 / pi / sqrt(2.0 * pi) / crr / cgg / cbb;

			backgroundWeight[idx] = backgroundWeight[idx] * (1.0 - PFprior) 
				/ (backgroundWeight[idx] * (1.0 - PFprior) + PF * PFprior);

			if(backgroundWeight[idx] < VERY_SMALL_WEIGHT)
				backgroundWeight[idx] = VERY_SMALL_WEIGHT;

			((unsigned char *)(test1_image->imageData + test1_image->widthStep*i))[j] = 
				(unsigned char)(backgroundWeight[idx] * 255.0);

			// shadow probability ------------------------------------------------------------------------------------------
			// here normalized to (0,0,0)-(1,1,1) cube
			shadowWeight[idx]=0.0;

			//only test when shadow might be mistakenly set as foreground
			//if(((unsigned char *)(test1_image->imageData + test1_image->widthStep*i))[j] == 0){
			nR = R / Mr[idx];
			nG = G / Mg[idx];
			nB = B / Mb[idx];
			ratio = sqrt(nR * nR + nG * nG + nB * nB) / sqrt(3.0);

			// intensity not too small, and still smaller than mean pixel intensity, trigue shadow calculation
			if(ratio>shadow_calculation_threshold_hard && ratio<=shadow_calculation_threshold_soft)
			{	
				double CD = sqrt((nR-ratio)*(nR-ratio)+(nG-ratio)*(nG-ratio)+(nB-ratio)*(nB-ratio))*ratio;
				shadowWeight[idx]=exp(-(CD*CD)/shadow_sigma_hard/shadow_sigma_hard*0.1);
				//shadowWeight[idx]=CD*3.0;
			}

			// intensity not too small, and still smaller than mean pixel intensity, trigue shadow calculation
			if(ratio>shadow_calculation_threshold_soft && ratio<1.0)
			{	
				//according to http://www.cs.unc.edu/~lguan/COMP255.files/FinalReport.htm
				double CD = sqrt(nR*nR+nG*nG+nB*nB-(nR+nG+nB)*(nR+nG+nB)/9.0); 
				shadowWeight[idx]=exp(-(CD*CD)/shadow_sigma_soft/shadow_sigma_soft*0.5);
				//shadowWeight[idx]=CD*3.0;
			}
			//}

			((unsigned char *)(test2_image->imageData + test2_image->widthStep*i))[j] = 
				(unsigned char)(shadowWeight[idx]*255.0);

			finalWeight[idx] = shadowWeight[idx]>backgroundWeight[idx]?shadowWeight[idx]:backgroundWeight[idx];				

			//((unsigned char *)(test3_image->imageData + test3_image->widthStep*i))[j] = (unsigned char)(((finalWeight[idx]>0.1)?1:0)*255.0);

			((unsigned char *)(test3_image->imageData + test3_image->widthStep*i))[j] 
				= 255 - (unsigned char)(finalWeight[idx]*255.0);

			//if (finalWeight[idx]<0.1)
			//	((unsigned char *)(test4_image->imageData + test4_image->widthStep*i))[j] = 255;
			//else
			//	((unsigned char *)(test4_image->imageData + test4_image->widthStep*i))[j] = 0;
		}
	}
}

/*!
*/
void GraphcutBackgroundSubtractor::FindBlobs()
{
	double CUT_bWeight;

	// Begin graphcut
	graphcut::Graph g;

	int i, j, idx;

	for (i = 0; i< m_frameSize.height; i++) 
	{
		for (j = 0; j < m_frameSize.width; j++) 
		{
			idx = m_frameSize.width * i + j;

			CUT_bWeight = 1 - finalWeight[idx];

			nodes[idx] = g.add_node();

			g.set_tweights(nodes[idx], CUT_fWeight, CUT_bWeight);

			// using four connection
			if (i != 0) //not on the first row of the image
			{
				g.add_edge(nodes[idx], nodes[m_frameSize.width * (i - 1) + j], 
					CUT_alpha, CUT_alpha);
			}

			if (j != 0)	//not on the first row of the image
			{
				g.add_edge(nodes[idx], nodes[m_frameSize.width * i + (j - 1)], 
					CUT_alpha, CUT_alpha);
			}
		}
	}

	double flow = g.maxflow();

	for (i = 0; i< m_frameSize.height; i++) 
	{
		for (j = 0; j < m_frameSize.width; j++) 
		{
			if (g.what_segment(nodes[m_frameSize.width*i+j]) == graphcut::Graph::SOURCE)
				((unsigned char *)(test4_image->imageData + test4_image->widthStep*i))[j] = 0;
			else
				((unsigned char *)(test4_image->imageData + test4_image->widthStep*i))[j] = 128;
		}
	}

	// Preserve the largest blob of the binary image
	findLargestBlob((unsigned char *)(test4_image->imageData), 
		m_frameSize.height, m_frameSize.width);
}

/*!
	We discriminate between foreground and background pixels
	by building and maintaining a model of the background.
	Any pixel which does not fit this model is then deemed
	to be foreground.
*/
void GraphcutBackgroundSubtractor::FindForeground(RGBImg newFrame, FloatImg greyImg)
{
	IplImageView iplView(newFrame, false);

	frame1 = iplView;

	FindPreliminaryForeground();
	
	FindBlobs();

	// Possible output is
	//cvShowImage("Mean Image", frame1);
	//cvShowImage("Backgound Weight", test1_image);
	//cvShowImage("Shadow Weight", test2_image);
	//cvShowImage("Final Weight", test3_image);
	//cvShowImage("Final Cut Result", test4_image);

	//IplImageToVXLImage(test4_image, m_foreground, 1);
	IplImageToVXLImage(test1_image, m_foreground, 1);
	IplImageToVXLImage(mean_image, m_background);
}
