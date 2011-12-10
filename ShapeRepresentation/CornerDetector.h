/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _CORNER_DETECTOR_H_
#define _CORNER_DETECTOR_H_

#include "FluxSkeleton\include\ShapeBoundary.h"
#include "BoundaryCorner.h"
#include "Tools\HelperFunctions.h"

namespace vpl {

/*!

*/
struct MinAngleInfo
{
	enum ArmType {LEFT, RIGHT};

	struct AngleInfo 
	{ 
		double angleCos;    //! Cosine of the angle formed by *this* arm and another arm
		int armIdx;         //!< Local index of the *other* arm forming the angleCos
		Point arm;          //!< Normalized vector from source vertex to arm endpoint
		double length;      //!< Distance between source vertex and arm endpoint
		int circBndryIndex; //!< Circular boundary index of *this* arm's endpoint
		bool sign;          //!< Angle sign: negative (false) or positive (true)
	};

	struct ArmIndex 
	{ 
		ArmType armType; //<! Type of arm is either LEFT or RIGHT
		int armIdx;      //<! Local index of the arm
	};

	AngleInfo* minAnglesLeft;
	AngleInfo* minAnglesRight;
	ArmIndex minAngleIdx;
	double minAngleCos;
	int size;

	MinAngleInfo()
	{
		size = 0;
		minAnglesLeft = NULL;
		minAnglesRight = NULL;
		minAngleCos = -1;
	}

	void Initialize(int sz)
	{
		ASSERT(size == 0);

		size = sz;
		minAnglesLeft = new AngleInfo[sz];
		minAnglesRight = new AngleInfo[sz];
	}

	~MinAngleInfo()
	{
		delete[] minAnglesLeft;
		delete[] minAnglesRight;
	}

	AngleInfo& GetAngleLeft(int i)
	{
		ASSERT(i >= 0 && i < size);
		return minAnglesLeft[i];
	}

	AngleInfo& GetAngleRight(int i)
	{
		ASSERT(i >= 0 && i < size);
		return minAnglesRight[i];
	}

	/*!
		Gets the source and target arms as defined by the type. The source 
		arm correspons to the given type, and the target arm is the
		other arm. 
	*/
	void GetSourceAndTargetArms(ArmType type, 
		AngleInfo** src, AngleInfo** tgt) const
	{
		if (type == LEFT)
		{
			*src = minAnglesLeft;
			*tgt = minAnglesRight;
		}
		else
		{
			*src = minAnglesRight;
			*tgt = minAnglesLeft;
		}
	}

	/*!
		Once the angle information of each left and right arm is available, 
		this function is called to compute the minimum angle for the i'th
		left/right angle and each available right/left arm.
	*/
	void ComputeMinArmAngles(ArmType type, int idx)
	{
		AngleInfo *src, *tgt;
		GetSourceAndTargetArms(type, &src, &tgt);

		AngleInfo& ai = src[idx];
		double ac;

		ai.angleCos = -1; // smallest possible angle cosine (ie, max angle)

		for (int i = 0; i < size; i++)
		{
			ac = ai.arm.dot(tgt[i].arm);

			if (ac > ai.angleCos)
			{
				ai.angleCos = ac;
				ai.armIdx = i;
			}
		}

		// Set the angle sign
		if (ai.angleCos > -1)
		{
			const Point& otherArm = tgt[ai.armIdx].arm;

			ai.sign = ((ai.arm.x * otherArm.y - ai.arm.y * otherArm.x) < 0);

			if (type == LEFT)
				ai.sign = !ai.sign;
		}
	}

	/*!
		Finds the pairing of arms that yields the minimum angle. It is
		assumed that ComputeMinArmAngles(LEFT, *) and 
		ComputeMinArmAngles(RIGHT, *) where previously called.
	*/
	void ComputeMinAngle()
	{
		for (int i = 0; i < size; i++)
		{
			if (minAnglesLeft[i].angleCos > minAngleCos)
			{
				minAngleCos = minAnglesLeft[i].angleCos;
				minAngleIdx.armIdx = i;
				minAngleIdx.armType = LEFT;
			}

			if (minAnglesRight[i].angleCos > minAngleCos)
			{
				minAngleCos = minAnglesRight[i].angleCos;
				minAngleIdx.armIdx = i;
				minAngleIdx.armType = RIGHT;
			}
		}
	}

	/*!
		Compares the lenght of the LEFT and RIGHT arms of
		the minimum angle and returns the greater one.
	*/
	double GetMaxArmLengthOfMinAngle() const
	{
		AngleInfo *src, *tgt;
		GetSourceAndTargetArms(minAngleIdx.armType, &src, &tgt);

		const AngleInfo& ai = src[minAngleIdx.armIdx];

		double a = ai.length;
		double b = tgt[ai.armIdx].length;

		return MAX(a, b);
	}

	/*!
		Gets the arms that define the minimum angle
	*/
	void GetArmCircBndryIndicesOfMinAngle(int* pSrcIdx, int* pTgtIdx) const
	{
		AngleInfo *src, *tgt;
		GetSourceAndTargetArms(minAngleIdx.armType, &src, &tgt);
		const AngleInfo& ai = src[minAngleIdx.armIdx];

		*pSrcIdx = ai.circBndryIndex;
		*pTgtIdx = tgt[ai.armIdx].circBndryIndex;
	}

	/*!
		Gets the arms that define the minimum angle
	*/
	void GetArmEndpointsOfMinAngle(Point* pSrcArm, Point* pTgtArm) const
	{
		AngleInfo *src, *tgt;
		GetSourceAndTargetArms(minAngleIdx.armType, &src, &tgt);
		const AngleInfo& ai = src[minAngleIdx.armIdx];

		*pSrcArm = ai.arm;
		*pTgtArm = tgt[ai.armIdx].arm;
	}

	/*!
	*/
	bool GetMinAngleSign() const
	{
		if (minAngleIdx.armType == LEFT)
			return minAnglesLeft[minAngleIdx.armIdx].sign;
		else
			return minAnglesRight[minAngleIdx.armIdx].sign;
	}
};

/*!
	
*/
class CornerDetector
{
	const sg::ShapeBoundary* m_pShape;

	const int m_minArmSize;
	const int m_maxArmSize;

	static double s_minCornerAngle;

public:

	std::vector<MinAngleInfo> m_minAngles;
	CornerLabelArray m_labels;
	BoundaryCornerList m_concaveCorners;

protected:
	void ComputeMinimumAngles(const sg::ShapeBoundary* pShapeBdry);
	void ComputeCornerInformationAndlabels();

public:
	CornerDetector(int minSz, int maxSz) 
		: m_minArmSize(minSz), m_maxArmSize(maxSz)
	{
		m_pShape = NULL;
	}

	void FindConcaveCorners(const sg::ShapeBoundary* pShapeBdry);

	const CornerLabelArray& LabeledBoundaryPoints() const
	{
		return m_labels;
	}

	const BoundaryCornerList& ConcaveCorners() const
	{
		return m_concaveCorners;
	}

	bool IsCorner(int i) const
	{
		return m_minAngles[i].minAngleCos >= s_minCornerAngle;
	}

	double GetAngleCosine(int i) const
	{
		return m_minAngles[i].minAngleCos;
	}

	/*const double& operator[](int i) const
	{
		return m_minAngles[i].minAngleCos;
	}*/

	double GetRadius(int i) const
	{
		return m_minAngles[i].GetMaxArmLengthOfMinAngle();
	}

	/*!
		Checks if the i'th boundary point is concave wrt
		to the maximum-radius skeletal point whose
		emanating spoke ends at the boundary point.
	*/
	bool IsConcaveCorner(int i) const
	{
		return !m_minAngles[i].GetMinAngleSign();

		/*sg::BranchPointIndex bpi;

		if (m_pShape->getSpokesMap(i).FindMaxLengthSpokeOrigin(&bpi))
			return IsConcaveCorner(i, bpi);
		else
			return false;*/
	}

	/*!
	    The bpi is a skeletal point with a spoke terminating at the i'th 
		boundary point. Moreover, the *indices* of the spokes of the skeletal 
		point can be used to "imaginary" divide the	shape contour in two. Since 
		one of these spokes ends at the i'th boundary point (ie, the corner),
		the LEFT/RIGHT arms of this point must belong to *different* parts of the
		divided contour. If this is not the case, the arms go about the skeletal 
		point due to a U-shaped contour.
	*/
	bool CheckArmInterval(int i, const sg::BranchPointIndex& bpi) const
	{
		// Form a boundary interval with the spokes of the skeletal point
		unsigned int spokeIdx1, spokeIdx2;

		bpi.GetSpokeIndices(&spokeIdx1, &spokeIdx2);

		// The skeletal point is supposed to have a spoke that ends at
		// the i'th boundary point, so make sure it does
		ASSERT(i == spokeIdx1 || i == spokeIdx2);
		
		//DBG_PRINT2(spokeIdx1, spokeIdx2)

		sg::BoundaryInterval bi(m_pShape->size(), spokeIdx1, spokeIdx2);

		// Get the boundary indices of the arms
		int armCircIdx1, armCircIdx2;

		m_minAngles[i].GetArmCircBndryIndicesOfMinAngle(&armCircIdx1, &armCircIdx2);

		int armIdx1 = bi.TranslateCircularIndex(armCircIdx1);
		int armIdx2 = bi.TranslateCircularIndex(armCircIdx2);

		// If both arm indices are either inside or outside the boundary interval
		// formed by the spokes, return false
		bool arm1Inside = bi.Inside(armIdx1);
		bool arm2Inside = bi.Inside(armIdx2);

		return ((arm1Inside && !arm2Inside) || (!arm1Inside && arm2Inside));
	}

	/*!
		Checks if the i'th boundary point is concave wrt the 
		given skeletal point. The 'bpi' is a skeletal point with a 
		spoke terminating at the i'th boundary point.

		\image html IsConcaveCorner.gif
	*/
	bool IsConcaveCorner(int i, const sg::BranchPointIndex& bpi) const
	{
		//DBG_PRINT3(i, bpi.index, bpi.GetSkelPointCoord())

		Point vertex, arm1, arm2, intPt;

		m_pShape->getPoint(i, &vertex);

		m_minAngles[i].GetArmEndpointsOfMinAngle(&arm1, &arm2);

		Point armEndpt1 = arm1 + vertex;
		Point armEndpt2 = arm2 + vertex;
	
		Point skelPt = bpi.GetSkelPointCoord();

		if (!CheckArmInterval(i, bpi))
			return false;

		// The skeletal point must be outside the triangle formed
		// by the vertex and its arm endpoints in order to have a 
		// chance to be a concave corner
		if (IsInTriangle(skelPt, vertex, armEndpt1, armEndpt2))
			return false;

		int rv = FindLineSegmentIntersection(skelPt, vertex, 
			armEndpt1, armEndpt2, &intPt);

		// rv = 2 means that the segments overlap on several points
		ASSERT(rv != 2);

		// rv == 4 means that the second segments is intersected by the first line,
		// which generaly is the case of concave corners, but not always (see fig (c))
		// distance from intersection pt to corner must be smaller than to skel pt
		if (rv == 4)
			return (intPt.sqDist(vertex) < intPt.sqDist(skelPt));

		//  rv = 1 means that both segments intersect, the corner is NOT concave
		return (rv != 1);
	}

	static void ReadParamsFromUserArguments();
};

} //namespace vpl

#endif // _CORNER_DETECTOR_H_