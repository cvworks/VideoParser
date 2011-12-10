/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <memory>
#include "SkeletalGraph.h"
#include "CornerDetector.h"
#include "BoundaryCutter.h"
#include <Tools/BasicTypes.h>
#include <Tools/STLUtils.h>
#include <Tools/Serialization.h>

namespace vpl {

/*!
	Information recovered about shape defined by a given closed curve.

	The info includes:

	- The shape boundary
	- The medial axis of the shape
	- The negative curvature extrema points along teh boundary (ie, concave corners)
	- All possible boundary cuts according to some set of rules
*/
class ShapeInformation
{
	static bool s_drawSpokes;
	static bool s_drawMedialAxis;
	static bool s_drawCorners;
	static bool s_drawBoundaryMap;
	static bool s_drawBoundaryCuts;
	static bool s_drawLabels;

public:
	bool m_initialized;

	sg::ShapeBoundary* m_pBoundary; //!< Shape boundary (owned!)

	SkeletalGraph m_skelGraph; //!< Skeleton of the shape

	CornerDetector m_corners; //!< Corners along the boundary (not serialized)
	BoundaryCutter m_bndryCuts; //!< Boundary cuts (not serialized)

	UIBoundingBox m_bbox; //!< Bounding box of the shape
	unsigned m_imgWidth;  //!< Pixel width of the image that contains the shape
	unsigned m_imgHeight; //!< Pixel height of the image that contains the shape

public:
	ShapeInformation() : m_corners(4, 8)
	{ 
		m_initialized = false;
		m_pBoundary = NULL;
	}

	~ShapeInformation()
	{
		delete m_pBoundary;
	}

	/*!
		Sets the shape attributes that are "meta" in the sense that are either
		not intrinsic to the shape or that can be easily derived from it.
	*/
	void SetMetaAttributes(unsigned imgWidth, unsigned imgHeight,
		const UIBoundingBox& bbox)
	{
		m_imgWidth = imgWidth;
		m_imgHeight = imgHeight;
		m_bbox = bbox;
	}

	void GetImgDimensions(unsigned* w, unsigned* h) const
	{
		*w = m_imgWidth;
		*h = m_imgHeight;
	}

	//! Returns the "persistent" boundary points
	const PointArray& GetBoundaryPoints() const
	{
		return m_pBoundary->getPoints();
	}

	//! Returns the bounding box of the shape
	void GetBoundingBox(UIBoundingBox* bbox) const
	{
		*bbox = m_bbox;

		//m_skelGraph.GetBounds(xmin, xmax, ymin, ymax);
	}

	/*!
		Serializes the boundary and skeleton of the shape.

		The boundary corners and cuts are not serialized but
		can be recreated when deserializing by calling 
		ComputeBoundaryCornersAndCuts().
	*/
	void Serialize(OutputStream& os) const
	{
		::Serialize(os, m_imgWidth);
		::Serialize(os, m_imgHeight);

		// Serialize the shape boundary info. Note that
		// the mapping between boundary points and skeletal
		// points is NOT serialized. 
		m_pBoundary->Serialize(os);

		// In general, the skeletal graph would serialize its
		// ShapeBoundary, but since we took ownership of it, 
		// it won't in this case.
		m_skelGraph.Serialize(os);
	}

	/*!
		Deserializes the boundary and skeleton of the shape.

		To recreate the boundary corners and cuts, call
		ComputeBoundaryCornersAndCuts() after the deserialization.
	*/
	void Deserialize(InputStream& is)
	{
		::Deserialize(is, m_imgWidth);
		::Deserialize(is, m_imgHeight);

		if (!m_pBoundary)
			m_pBoundary = new sg::ShapeBoundary;

		// Deserialize the shape boundary info. Note that
		// the mapping between boundary points and skeletal
		// points is NOT deserialized and must be computed if
		// needed by calling m_pBoundary->setSkeletalPoints(pDDSGraph).
		m_pBoundary->Deserialize(is);

		// The sekelat graph won't deserialize its ShapeBoundary 
		// because we took ownership of it "before" we serialized the
		// skeleton. In any case, the deserialization of the 
		// ShapeBoundary would have been done by calling setSkeletalPoints().
		m_skelGraph.Deserialize(is);

		// Recreate the map between boundary points and skeletal points
		m_pBoundary->setSkeletalPoints(m_skelGraph.GetDDSGraph());

		// The shape info is initialized now
		m_initialized = true;

		// Recreate the boundary corners and cuts
		ComputeBoundaryCornersAndCuts();
	}

	//! Ensure that operator equal is NOT called
	void operator=(const ShapeInformation& rhs)
	{
		ASSERT(false);
	}

	void ComputeBoundaryCornersAndCuts()
	{
		ASSERT(m_initialized);

		m_corners.FindConcaveCorners(m_pBoundary);

		m_bndryCuts.ComputeCuts(m_pBoundary, m_corners.ConcaveCorners(),
			m_corners.LabeledBoundaryPoints());
	}

	/*!
		Creates an overcomplete representation of the given shape. 
		
		It can only be called once per object.
	*/
	bool ShapeInformation::Create(const DiscreteXYArray& boundaryPts, 
								  const SkeletalGraphParams& params)
	{	
		ASSERT(m_initialized == false);

		m_initialized = m_skelGraph.CreateUsingFluxSkeleton(boundaryPts);

		if (m_initialized)
		{
			ASSERT(!m_pBoundary);
			
			// Get ownership of the shape boundary object
			m_pBoundary = m_skelGraph.ReleaseShapeBoundary();

			ComputeBoundaryCornersAndCuts();
		}

		return m_initialized;
	}

	const sg::DDSGraph* GetDDSGraph() const 
	{ 
		return m_skelGraph.GetDDSGraph(); 
	}

	unsigned BoundarySize() const
	{
		return m_pBoundary->size();
	}

	void Draw() const;

	std::string GetOutputText() const;

	static void GetSwitchCommands(std::list<UserCommandInfo>& cmds);

	static void ReadParamsFromUserArguments();
};

//! Shared pointer to a ShapeInformation object
typedef std::shared_ptr<ShapeInformation> ShapeInfoPtr;

} // namespace vpl

