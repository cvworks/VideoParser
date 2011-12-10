/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "IplImageView.h"

#include <Tools/CvUtils.h> // gets cvConvertImage() function

/*!
	If swapRb is true, it deep copies VXL images and then swaps 
	the read and blue channels as required by the IPL image.
	Otherwise, it views the IPL image without swapping the RB channels.
*/
IplImageView::IplImageView(RGBImg& rgbImg, bool swapRB)
{
	if (swapRB)
	{
		RGBImg img = vil_copy_deep(rgbImg);

		m_vxlView = ConvertToBaseImgPtr(img);		

		CreateHeader(img.ni(), img.nj(), IPL_DEPTH_8U, 3,
			(char*) img.top_left_ptr());

		// Swap read and blue channels
		cvConvertImage(m_pIplImage, m_pIplImage, CV_CVTIMG_SWAP_RB);
	}
	else
	{
		m_vxlView = ConvertToBaseImgPtr(rgbImg);

		CreateHeader(rgbImg.ni(), rgbImg.nj(), IPL_DEPTH_8U, 3,
			(char*) rgbImg.top_left_ptr());
	}
}
