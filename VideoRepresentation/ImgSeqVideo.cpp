/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "ImgSeqVideo.h"

#include <vil/vil_load.h> 
//#include <vul/vul_file.h> // for vul_file::get_cwd();
//#include <vil/vil_resample_nearest.h>
#include <vil/vil_decimate.h>
#include <Tools/DirWalker.h>
#include <Tools/Tuple.h>
#include <Tools/CvMatView.h>
#include <Tools/Exceptions.h>

//#include <Tools/CvUtils.h> // to load img with opencv

using namespace vpl;

template <typename T> void ImgSeqVideo::EnsureFrameSize(T& img) const
{
	/*if (m_enforceSize || img.ni() != m_frameWidth || img.nj() != m_frameHeight)
	{
		double wr = ceil(double(m_frameWidth) / double(img.ni()));
		double hr = ceil(double(m_frameHeight) / double(img.nj()));
	
		// factor is greater than 1 if image is smaller than the default, and less
		// than 1 if the image is larger that the default.
		const double f = MIN(wr, hr);

		cv::Size sz(cvRound(f * img.ni()), cvRound(f * img.nj()));

		if ((unsigned)sz.width > m_frameWidth) 
			sz.width = m_frameWidth;

		if ((unsigned)sz.height > m_frameHeight)
			sz.height = m_frameHeight;

		T aux(sz.width, sz.height);

		CvMatView src(img), tgt(aux);

		cv::resize(src, tgt, sz, 0, 0, cv::INTER_LANCZOS4);

		img = aux;
	}*/

	if (m_enforceSize || img.ni() != m_frameWidth || img.nj() != m_frameHeight)
	{
		unsigned wr = (unsigned)ceil(double(img.ni()) / double(m_frameWidth));
		unsigned hr = (unsigned)ceil(double(img.nj()) / double(m_frameHeight));
	
		unsigned factor = MAX(wr, hr);

		if (factor > 1)
		{
			if (m_showScalingWarning)
			{
				StreamMsg("Scaling image " << img.ni() << " x " << img.nj()
					<< " by " << factor << ". Frame size is " 
					<< m_frameWidth << " x " << m_frameHeight << ".");
			}
			
			img = vil_decimate(img, factor, factor);
		}
	}
}


bool ImgSeqVideo::Load(std::string strFilename)
{
	Clear();

	
	if (!m_params.ReadParameters(strFilename.c_str()))
	{
		ShowOpenFileError(strFilename);
		return false;
	}

	ShowStatus("Image sequence loaded");

	std::string rootDir;
	unsigned maxLevel;
	vpl::Tuple<double, 3, '(', ')'> rgbColor(255, 255, 255);

	// First, try reading all optinal parameters so that their usage strings are known
	//m_params.ReadArg("count", "Number of images to read [-1 for all]", -1, &m_frameCount);

	m_params.ReadArg("rootDir", "Root directory", std::string("."), &rootDir);

	// See if the images have relative directories and if the video file
	// has an absolute path
	if (!rootDir.empty() && rootDir[0] == '.') // it's relative dir
	{
		std::string path = DirWalker::GetPath(strFilename.c_str());

		if (!path.empty() && path[0] != '.') // it's absolute dir
			rootDir = DirWalker::ReplacePath(rootDir.c_str(), path.c_str());
	}

	m_params.ReadArg("frameRate", "Number of frames per second", 
		5.0, &m_frameRate);

	m_params.ReadArg("frameWidth", "Width used instead of that of the first frame", 
		0u, &m_frameWidth);

	m_params.ReadArg("frameHeight", "Height used instead of that of the first frame", 
		0u, &m_frameHeight);

	m_params.ReadBoolArg("enforceSize", "Ensure that all frames have the same size?", 
		true, &m_enforceSize);

	m_params.ReadBoolArg("showScalingWarning", "Shows a warning when an image is scaled", 
		true, &m_showScalingWarning);

	m_params.ReadArg("marginSize", "Number of margin pixels around each video frame", 
		0u, &m_marginSize);

	m_params.ReadArg("marginColor", "Color of the image margin used for all frames", 
		rgbColor, &rgbColor);

	m_marginColor.r = (unsigned char)rgbColor[0];
	m_marginColor.g = (unsigned char)rgbColor[1];
	m_marginColor.b = (unsigned char)rgbColor[2];

	m_params.ReadArg("maxRecurseLevels", "Maximum number of subdirectory levels to visit", 
		0u, &maxLevel);

	StrList globs, defaultGlobs;

	defaultGlobs.push_back("*.bmp");
	defaultGlobs.push_back("*.jpg");
	
	m_params.ReadArgs("glob", "Globbing on filenames (like a command line glob)",
		defaultGlobs, &globs);

	// We can now collect, parse, amd sort the filenames
	// To allow for multiple globs, eg, *.bmp and *.jpg
	// we collect for each glob separatelly.
	std_forall(globIt, globs)
	{
		DirWalker::CollectFileNames(rootDir, *globIt, maxLevel, m_framePaths);
	}

	if (m_framePaths.empty())
	{
		ShowError("No video frame images were found. Is the glob spec correct?");
	}

	// Sort the files names by interpreting numeric characters as numbers
	DirWalker::SortFileNamesNumerically(m_framePaths);

	// Set the required video information in the base class
	SetVideoInfo(strFilename, m_framePaths.size(),
		DirWalker::ReadCreationTime(strFilename));

	// Read the ROI info0rmation if provided and associate it to the video
	ROISequence boxes;

	m_params.ReadArgs("roi", "List of regions of interest {xmin,xmax,ymin,ymax}", 
		boxes, &boxes);

	if (!boxes.empty())
		SetROISequence(boxes);
	
	return true;
}

/*!
	[protected] Reads the image file referred by m_pathIt.
*/
void ImgSeqVideo::ReadCurrentImageFile()
{
	if (IsLastFrame())
		return;
	
	m_curImg = vil_load(m_pathIt->c_str());

	if (m_curImg == NULL || m_curImg->size() == 0)
	{
		ShowError1("Cannot read image frame:", *m_pathIt);

		RGBImg img(50, 50);

		img.fill(RGBColor(255, 255, 255));

		m_curImg = ConvertToBaseImgPtr(img);
	}

	// Set the frame width and height if not set yet
	// or if frames can have different sizes
	if (m_frameWidth == 0 || !m_enforceSize)
		m_frameWidth = m_curImg->ni();

	if (m_frameHeight == 0 || !m_enforceSize)
		m_frameHeight = m_curImg->nj();
}

void ImgSeqVideo::ReadFirstFrame()
{
	m_currentFrameNumber = 0;

	m_pathIt = m_framePaths.begin();

	ReadCurrentImageFile();
}

void ImgSeqVideo::ReadNextFrame()
{
	++m_currentFrameNumber;

	++m_pathIt;

	ReadCurrentImageFile();
}

void ImgSeqVideo::ReadFrame(fnum_t i)
{
	WARNING(i < 0 || i >= m_frameCount, "Invalid frame number requested");

	for (m_currentFrameNumber = 0, m_pathIt = m_framePaths.begin();
		m_currentFrameNumber < i && !IsLastFrame(); ++m_currentFrameNumber, ++m_pathIt);

	// It might be the last frame if 'i' was invalid... so check that
	// Note: we always have to init m_pathIt because is used by IsLastFrame()
	if (!IsLastFrame())
		ReadCurrentImageFile();
}

bool ImgSeqVideo::IsLastFrame() const
{
	return m_pathIt == m_framePaths.end();
}

RGBImg ImgSeqVideo::GetCurrentRGBFrame() const
{
	// Handle easy case first: don't need to check the size or add a margin
	if (!m_enforceSize && m_marginSize == 0)
	{
		return vil_convert_to_component_order(
			vil_convert_to_n_planes(3, m_curImg));
	}
	
	RGBImg img = vil_convert_to_component_order(
		vil_convert_to_n_planes(3, m_curImg));
	
	EnsureFrameSize(img); // operates only if m_enforceSize is true

	RGBImg img2(m_frameWidth + 2 * m_marginSize, 
		m_frameHeight + 2 * m_marginSize);

	img2.fill(m_marginColor);

	// Copy to interior window with top-left corner (m_marginSize,m_marginSize)
	vil_copy_to_window(img, img2, m_marginSize, m_marginSize);

	return img2;
}

FloatImg ImgSeqVideo::GetCurrentGreyScaleFrame() const
{
	// Handle easy case first: don't need to check the size or add a margin
	if (!m_enforceSize && m_marginSize == 0)
	{
		return ConvertToGreyImage(m_curImg);
	}

	FloatImg img = ConvertToGreyImage(m_curImg);

	EnsureFrameSize(img); // operates only if m_enforceSize is true

	FloatImg img2(m_frameWidth + 2 * m_marginSize, 
		m_frameHeight + 2 * m_marginSize);

	img2.fill(m_marginColor.grey());

	// Copy to interior window with top-left corner (m_marginSize,m_marginSize)
	vil_copy_to_window(img, img2, m_marginSize, m_marginSize);

	return img2;
}


