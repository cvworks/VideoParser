/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _SYNTHETIC_VIDEO_H_
#define _SYNTHETIC_VIDEO_H_

#include "Video.h"
#include <Tools/BasicTempTypes.h> // for MotionData
#include <Tools/ParamFile.h>

namespace vpl {

class SyntheticVideo : public Video
{
	ParamFile m_params;
	unsigned int m_nSpriteCount;
	RGBImg m_curRGBFrame;
	float m_fMaskValue;
	bool m_bHaveBackground;

	typedef std::list<Point> Trajectory;
	typedef Trajectory::const_iterator TrajectoryIndex;

	std::list<BaseImgPtr> m_sprites;
	std::list<FloatImg> m_masks;
	std::vector<Trajectory> m_trajectories;
	std::vector<TrajectoryIndex> m_positions;

	void GenerateFrame(bool bPaintMaskOnly = false);
public:
	void Clear();
	bool Load(std::string strFilename);
	void ReadFirstFrame();
	void ReadNextFrame();
	void ReadFrame(fnum_t i);

	Trajectory MakeTrajectory(const std::string& strTrjType, 
		const PointList& points) const;
	
	bool IsLastFrame() const;
	RGBImg GetCurrentRGBFrame() const;
	FloatImg GetCurrentGreyScaleFrame() const;

	// Class-specific functions and types
public:
	RGBImg MakeRGBFrameFromMaskOnly(std::vector<MotionData>& mdVec);
};

} // namespace vpl

#endif //_SYNTHETIC_VIDEO_H_