/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "SyntheticVideo.h"

#include <vil/vil_load.h>
#include <Tools/Num2StrConverter.h>
#include <Tools/Exceptions.h>
#include <Tools/DirWalker.h>

using namespace vpl;

void SyntheticVideo::Clear()
{
	Video::Clear();
	
	m_nSpriteCount = 0;
	m_bHaveBackground = true;
	m_sprites.clear();
	m_trajectories.clear();
	m_positions.clear();
}

bool SyntheticVideo::Load(std::string strFilename)
{
	Clear();

	try
	{
		m_params.ReadParameters(strFilename.c_str());

		// Read basic parameter values
		m_frameCount = m_params.GetIntValue("frameCount");
		m_nSpriteCount = m_params.GetIntValue("spriteCount");
		m_fMaskValue = m_params.GetFloatValue("maskValue");
		m_bHaveBackground = m_params.GetBoolValue("haveBackground");

		std::string strPath = m_params.GetStrValue("path");

		// Give the requested size to the canvas
		m_curRGBFrame.set_size(m_params.GetIntValue("width"), m_params.GetIntValue("hight"));
	
		// Read sprite images and trajectories
		BaseImgPtr img;

		m_trajectories.resize(m_nSpriteCount);
		std::string spriteFilename, maskFilename;
		Num2StrConverter cv(100);
	
		for (unsigned int i = 0; i < m_nSpriteCount; i++)
		{
			spriteFilename = strPath + m_params.GetStrValue(cv.toCharPtr("sprite", i));
			img = vil_load(spriteFilename.c_str());
			m_sprites.push_back(img);
			
			if (i == 0 && m_bHaveBackground)
			{
				m_masks.push_back(FloatImg());
			}
			else
			{
				maskFilename = strPath + m_params.GetStrValue(cv.toCharPtr("mask", i));
				img = vil_load(maskFilename.c_str());
				m_masks.push_back(ConvertToGreyImage(img));
			}

			m_trajectories[i] = MakeTrajectory(
				m_params.GetStrValue(cv.toCharPtr("trajectoryType", i)),
				m_params.GetPointValues(cv.toCharPtr("trajectoryParams", i)));
		}
	}
	catch(BasicException e)
	{
		e.Print();
		return false;
	}

	// Set the required video information in the base class
	SetVideoInfo(strFilename, m_frameCount, DirWalker::ReadCreationTime(strFilename));
	
	return true;
}

SyntheticVideo::Trajectory SyntheticVideo::MakeTrajectory(
	const std::string& strTrjType, const PointList& points) const
{
	Trajectory trj;

	if (strTrjType == "points")
		trj = points;
	else if (strTrjType == "increment")
	{
		ASSERT(m_frameCount > 0);

		if (points.size() != 2)
			THROW_BASIC_EXCEPTION("Invalid number of parameters for trajectory type: increment");
		
		PointList::const_iterator it = points.begin();
		Point pt = *it++;
		Point inc = *it;
		
		for (int i = 0; i < m_frameCount; i++)
		{
			trj.push_back(pt);
			pt.x += inc.x;
			pt.y += inc.y;
		}
	}
	else
		THROW_BASIC_EXCEPTION("Invalid trajectory type");

	return trj;
}

void SyntheticVideo::ReadFirstFrame()
{
	m_currentFrameNumber = 0;
	m_positions.resize(m_nSpriteCount);
	
	for (unsigned int i = 0; i < m_nSpriteCount; i++)
		m_positions[i] = m_trajectories[i].begin();

	GenerateFrame();
}

void SyntheticVideo::ReadNextFrame()
{
	m_currentFrameNumber++;

	for (unsigned int i = 0; i < m_nSpriteCount; i++)
	{
		m_positions[i]++;
		if (m_positions[i] == m_trajectories[i].end())
			m_positions[i]--;
	}

	GenerateFrame();
}

/*!
	Generates a frame by combin ing the sprite and mask information.
	If bPaintMaskOnly == true, only the mask are painted with their
	index as their pixel value. Thus, mask number 3 will have all
	its valid pixels painted with value 3. The inices are defined in
	the interval [(0,0,0), (255,255,255)]. When using a default
	background (ie, with no mask for it), its pixels are assigned
	index (255,255,255) and its motion is assumed to be (0,0).
*/
void SyntheticVideo::GenerateFrame(bool bPaintMaskOnly /*=false*/)
{
	RGBImg sprite;
	FloatImg mask;
	unsigned int i, j, k;
	int ii, jj, maxi, maxj;
	double di, dj;
	
	m_curRGBFrame.fill(255);
	maxi = m_curRGBFrame.ni();
	maxj = m_curRGBFrame.nj();

	std::list<BaseImgPtr>::const_iterator itSprite = m_sprites.begin();
	std::list<FloatImg>::const_iterator itMask = m_masks.begin();

	for (k = 0; k < m_nSpriteCount; k++, itSprite++, itMask++)
	{
		sprite = *itSprite;
		mask   = *itMask;
		
		di = m_positions[k]->x;
		dj = m_positions[k]->y;

		for (i = 0; i < sprite.ni(); i++)
		{
			for (j = 0; j < sprite.nj(); j++)
			{
				ii = (int)(i + di);
				jj = (int)(j + dj);

				if ((ii >= 0 && ii < maxi && jj >= 0 && jj < maxj) &&
				   ((m_bHaveBackground && k == 0) || mask(i,j) < m_fMaskValue))
					m_curRGBFrame(ii,jj) = (bPaintMaskOnly) ? ind2rgb(k):sprite(i, j);
			}
		}
	}
}

bool SyntheticVideo::IsLastFrame() const
{
	return m_currentFrameNumber >= m_frameCount;
}

RGBImg SyntheticVideo::GetCurrentRGBFrame() const
{
	return m_curRGBFrame;
}

FloatImg SyntheticVideo::GetCurrentGreyScaleFrame() const
{
	FloatImg curGreyFrame;
	vil_convert_rgb_to_grey(m_curRGBFrame, curGreyFrame);
	return curGreyFrame;
}

RGBImg SyntheticVideo::MakeRGBFrameFromMaskOnly(std::vector<MotionData>& mdVec)
{
	RGBImg origImg = m_curRGBFrame;
	GenerateFrame(true);
	RGBImg maskOnlyImg = m_curRGBFrame;
	m_curRGBFrame = origImg;

	// Now add the motion info
	mdVec.resize(m_nSpriteCount);
	TrajectoryIndex curr, prev;
	
	for (unsigned int k = 0; k < m_nSpriteCount; k++)
	{
		prev = curr = m_positions[k];
		if (m_currentFrameNumber > 0) prev--;
		mdVec[k].Set(curr->x - prev->x, curr->y - prev->y);
	}

	return maskOnlyImg;
}

void SyntheticVideo::ReadFrame(fnum_t i)
{
	if (m_currentFrameNumber == i)
		return;

	if (i == 0)
	{
		ReadFirstFrame();
	}
	else
	{
		m_currentFrameNumber = i - 1;
		ReadFirstFrame();
	}
}
