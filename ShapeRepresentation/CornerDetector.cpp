/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "CornerDetector.h"
#include <Tools/UserArguments.h>

using namespace vpl;

extern UserArguments g_userArgs;

double CornerDetector::s_minCornerAngle = -0.876817; // ~= cos(151.2608)

/*!
	Reads parameters from global user's arguments
*/
void CornerDetector::ReadParamsFromUserArguments()
{
	g_userArgs.ReadArg("CornerDetector", "maxCornerAngle", 
		"Maximum angle of any detected corner", 151.2608, &s_minCornerAngle);

	s_minCornerAngle = cos(s_minCornerAngle);

	// State that we want to serialize the parameters
	g_userArgs.AddSerializableFieldKey("CornerDetector");
}

/*!
*/
void CornerDetector::ComputeMinimumAngles(const sg::ShapeBoundary* pShapeBdry)
{
	ASSERT(!m_pShape);

	m_pShape = pShapeBdry;

	const unsigned int bndrySize = m_pShape->size();
	const int armSize = m_maxArmSize - m_minArmSize + 1;

	Point p0, p1;
	int i;
	unsigned int bndryIdx;
	double armLen2;

	m_minAngles.clear();
	m_minAngles.resize(bndrySize);

	for (bndryIdx = 0; bndryIdx < bndrySize; bndryIdx++)
	{
		MinAngleInfo& mai = m_minAngles[bndryIdx];

		//std::cerr << "\n" << mai.size << ", " << bndryIdx << ", " << m_minAngles.size() << std::endl;

		mai.Initialize(armSize);

		m_pShape->getPoint(bndryIdx, &p0);

		// Collect info for LEFT arm
		for (i = 0; i < armSize; i++)
		{
			MinAngleInfo::AngleInfo& ai = mai.GetAngleLeft(i);

			ai.circBndryIndex = bndryIdx - i - m_minArmSize;

			m_pShape->getPointCircular(ai.circBndryIndex, &p1);

			ai.arm = p1 - p0;
			armLen2 = ai.arm.sqNorm(); // squared norm
			ai.length = sqrt(armLen2);
			ai.arm /= ai.length; // normalize arm
		}

		// Collect info for RIGHT arm
		for (i = 0; i < armSize; i++)
		{
			MinAngleInfo::AngleInfo& ai = mai.GetAngleRight(i);

			ai.circBndryIndex = bndryIdx + i + m_minArmSize;

			m_pShape->getPointCircular(ai.circBndryIndex, &p1);

			ai.arm = p1 - p0;
			armLen2 = ai.arm.sqNorm(); // squared norm
			ai.length = sqrt(armLen2);
			ai.arm /= ai.length; // normalize arm

			// Compute the minimum angle for this RIGHT arm
			mai.ComputeMinArmAngles(MinAngleInfo::RIGHT, i);
		}

		// Compute the minimum angle for each LEFT arm
		for (i = 0; i < armSize; i++)
			mai.ComputeMinArmAngles(MinAngleInfo::LEFT, i);

		mai.ComputeMinAngle();
	}	
}

/*!
*/
void CornerDetector::FindConcaveCorners(const sg::ShapeBoundary* pShapeBdry)
{
	ASSERT(m_concaveCorners.empty());

	ComputeMinimumAngles(pShapeBdry);

	const int sz = m_minAngles.size();

	ASSERT(sz == (int)pShapeBdry->size());

	BoundaryCorner ci;
	bool withinInterval = false;

	ci.interval.Set(sz, 0, 0);
	
	for (int i = 0; i < sz; i++)
	{
		if (IsCorner(i) && IsConcaveCorner(i))
		{
			if (withinInterval)
			{
				// Update interval with new last point
				ci.interval.SetLast(i);
			}
			else
			{
				// Create a new interval
				ci.interval.SetLimits(i, i);
				withinInterval = true;
			}
		}
		else if (withinInterval)
		{
			withinInterval = false;
			m_concaveCorners.push_back(ci);
			//DBG_PRINT6(i, ci.interval, IsCorner(i), IsConcaveCorner(i), GetAngleCosine(i), GetRadius(i))
		}
	}

	// If there is a pair of *adjacent* intervals at the front 
	// and back of the list, merge them
	if (m_concaveCorners.size() > 1 &&
		m_concaveCorners.front().interval.First() == 0 &&
		m_concaveCorners.back().interval.Last() == sz - 1)
	{
		m_concaveCorners.back().interval.SetLast(
			m_concaveCorners.front().interval.Last());

		m_concaveCorners.pop_front();
	}

	ComputeCornerInformationAndlabels();
}

/*!
	Fill in all the info about the corner intervals found
	and set the labels of each boundary point
*/
void CornerDetector::ComputeCornerInformationAndlabels()
{
	ASSERT(m_labels.empty());
	m_labels.resize(m_pShape->size(), CornerLabel(false, false));

	BoundaryCornerList::iterator it;
	int i, numSpokes;
	double angleCos;

	// Visit each corner interval and find the index and angle of the corner point
	for (it = m_concaveCorners.begin(); it != m_concaveCorners.end(); ++it)
	{
		BoundaryCorner& ci = *it;

		ci.spokeCount = -1;

		for (i = ci.interval.First(); ; i = ci.interval.Succ(i))
		{
			m_labels[i].isCorner = true;
			m_labels[i].cornerIt = it;

			numSpokes = m_pShape->getSpokesMap(i).SpokeCount();
			angleCos = m_minAngles[i].minAngleCos;

			if (numSpokes > ci.spokeCount || 
				(numSpokes == ci.spokeCount && angleCos > ci.angleCos))
			{
				ci.spokeCount = numSpokes;
				ci.angleCos = angleCos;
				ci.index = i;
			}

			if (i == ci.interval.Last())
				break;
		}
	}

	// Use the corner point index of each corner interval to set its 
	// "minimum concave corners" label to true
	for (it = m_concaveCorners.begin(); it != m_concaveCorners.end(); ++it)
	{
		m_labels[it->index].isMinimum = true;
	}
}

