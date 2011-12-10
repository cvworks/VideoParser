/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "ImageProcessor.h"
#include "VSCGraph.h"
#include <Tools/UserArguments.h>
#include <Tools/UserEvents.h>
#include <Tools/CvMatView.h>
#include <VideoParserGUI/DrawingUtils.h>

using namespace vpl;

extern UserArguments g_userArgs;

void ImageProcessor::ReadParamsFromUserArguments()
{
	VisSysComponent::ReadParamsFromUserArguments();

	g_userArgs.ReadArg(Name(), "bufferSize", "Number of "
		"previous frames stored. Set >= 1 for motion detection", 
		8u, &m_params.maxBufferSize);

	g_userArgs.ReadArg(Name(), "darkPixelValue", 
		"Pixel value that is considered the begining of darkness", 
		50.0f, &m_params.darkPixelValue);

	g_userArgs.ReadArg(Name(), "darknessThreshold", 
		"Level of darkness (1 == complete black) at which tracking is suspended", 
		0.7, &m_params.darknessThreshold);

	g_userArgs.ReadArg(Name(), "pixelDiffThreshold", 
		"Minimum intensity difference to consider that a pixel is noisy "
		"(for anti-noise mode only)", 20.0, &m_params.pixelDiffThreshold);

	g_userArgs.ReadArg(Name(), "maxTolerableNoiseLevel", 
		"Maximum image noise level [0,1] that does not activate noise mode "
		"(for anti-noise mode only)", 0.01, &m_params.maxTolerableNoiseLevel);
}

void ImageProcessor::Initialize(graph::node v)
{
	VisSysComponent::Initialize(v);

	m_stats.Clear();

	// Make sure that this component runs in all modes
	//SetRunningMode(ALL_RUNNING_MODES);
}

//! 
double ImageProcessor::ComputeDarknessLevel(FloatImg img) const
{
	unsigned level = 0;

	for (auto it = img.begin(); it != img.end(); ++it)
	{
		if (*it <= m_params.darkPixelValue)
			level++;
	}
	
	return double(level) / img.size();
}

//! 
double ImageProcessor::ComputeNoiseLevel(FloatImg img, FloatImg meanImg) const
{
	unsigned similarPixelCount = 0;

	// Compare the mean image with the true current image
	for (auto it0 = meanImg.begin(), it1 = img.begin(); 
		it0 != meanImg.end(); ++it0, ++it1)
	{
		if (fabs(*it0 - *it1) <= m_params.pixelDiffThreshold)
			similarPixelCount++;
	}

	return 1 - double(similarPixelCount) / meanImg.size();
}

void ImageProcessor::Run() 
{
	// Delete elements from the end if list is too long
	if (m_imgBuffer.size() > m_params.maxBufferSize)
		m_imgBuffer.pop_back(); 

	// Get the current images from the container graph
	const VSCGraph* pG = static_cast<const VSCGraph*>(ContainerGraph());

	const InputImageInfo& iii = pG->GetInputImageInfo();

	m_imgBuffer.push_front(iii);

	// See if the current frame is too dark or not
	m_stats.darknessLevel = ComputeDarknessLevel(iii.greyFrame);

	// When is offtime the image might be noisy when is not too dark. So,
	// remove noise and enhance the info by averaging the last frames.
	if (!IsTooDark() && IsOfftime() && m_imgBuffer.size() >= 2)
		SetAntiNoiseMode(true);

	// Note: the anti-noise mode is set above but also can be set by other components
	// in previous passes, because it's a shared variable of the vision system.
	if (AntiNoiseMode())
	{
		// Create a new "mean" image and keep the original current one, 
		// so that both can be compared.
		FloatImg meanImg(iii.greyFrame.ni(), iii.greyFrame.nj());

		meanImg.fill(0);

		for (auto it = m_imgBuffer.begin(); it != m_imgBuffer.end(); ++it)
		{
			auto srcIt = it->greyFrame.begin();
			auto tgtIt = meanImg.begin();

			for (; tgtIt != meanImg.end(); ++tgtIt, ++srcIt)
				*tgtIt += *srcIt;
		}

		for (auto it = meanImg.begin(); it != meanImg.end(); ++it)
			*it /= m_imgBuffer.size();

		// Compute the noise level based on the mean image
		m_stats.noiseLevel = ComputeNoiseLevel(iii.greyFrame, meanImg);

		if (IsNoisy())
		{
			// Recompute the darkness level
			m_stats.darknessLevel = ComputeDarknessLevel(meanImg);

			m_imgBuffer.front().greyFrame = meanImg;
		}
		else
		{
			SetAntiNoiseMode(false);
		}
	}
	else
	{
		// By default, the noise level is zero
		m_stats.noiseLevel = 0;
	}
}

/*!
	This function is called when the left mouse botton and
	the mouse pointer was on the component's view.

	@return zero if the even was not dealt with and 1 otherwise.
*/
bool ImageProcessor::OnGUIEvent(const UserEventInfo& uei)
{
	unsigned x = (unsigned)uei.coord.x;
	unsigned y = (unsigned)uei.coord.y;

	if (x >= GetRGBImage().ni())
		x = (GetRGBImage().ni() > 0) ? GetRGBImage().ni() - 1 : 0;

	if (y >= GetRGBImage().nj())
		y = (GetRGBImage().nj() > 0) ? GetRGBImage().nj() - 1 : 0;

	if (uei.id == EVENT_MOUSE_PUSH)
	{
		UIBoundingBox bbox(x, x, y, y);

		GetROISequence().push_back(bbox);
	}
	else if (uei.id == EVENT_MOUSE_DRAG)
	{
		GetROISequence().back().xmax = x;
		GetROISequence().back().ymax = y;
	}

	return true;
}

void ImageProcessor::GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const
{
	if (m_imgBuffer.empty())
		return;
		
	std::ostringstream oss;

	if (dii.outputIdx == 0) 
	{
		dio.imageType = RGB_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(m_imgBuffer.front().rgbFrame);

		if (IsTooDark())
			oss << "The image is too dark.";
	}
	else if (dii.outputIdx == 1)
	{
		dio.imageType = FLOAT_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(m_imgBuffer.front().greyFrame);

		
		oss << "Image noise level: " << m_stats.noiseLevel << "\n";
		oss << "Darkness level: " << m_stats.darknessLevel;
	}
	else if (dii.outputIdx == 2 && !GetROISequence().empty())
	{
		unsigned roiIdx = (unsigned)dii.params[0];

		RGBImg img = m_imgBuffer.front().rgbFrame;
		ByteImg mask(img.ni(), img.nj());

		mask.fill(0);
		GetROISequence().FillMask(roiIdx, mask);

		dio.imageType = BYTE_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(mask);
	}

	if (!GetROISequence().empty())
	{
		auto ra = GetROISequence();

		for (auto it = ra.begin(); it != ra.end(); ++it)
			oss << Tuple<unsigned, 4>(it->xmin, it->xmax, it->ymin, it->ymax) << ' ';
	}

	// Set the draggable property to fase so that the FL_DRAG event is sent to the component
	dio.specs.draggableContent = false;

	// Images should not be zoomed
	dio.specs.zoomableContent = false;

	dio.message = oss.str();
}
/*!
	Draws the output of the component. This function is called from
	a "drawing" thread, which is different from the "processing" thread
	that calls Run(). In order to protect the shared resources between the
	threads, a mutex is used.
*/
void ImageProcessor::Draw(const DisplayInfoIn& dii) const
{
	// Don't draw ROI when when showing masks
	if (dii.outputIdx == 2 || m_imgBuffer.empty())
		return;

	const ROISequence& roiSequence = m_imgBuffer.front().roiSequence;

	for (auto it = roiSequence.begin(); it != roiSequence.end(); ++it)
	{
		DrawSelection(Point(it->xmin, it->ymin), Point(it->xmax, it->ymax));
	}
}


