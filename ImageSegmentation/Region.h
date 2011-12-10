/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/BasicTypesSerialization.h>
#include <Tools/ImageSerialization.h>

namespace vpl {

typedef Coordinate<int> Offset;

/*!
	Pixel region recovered by an image segmenter.
*/
struct Region
{
	unsigned int numPts;
	double voteCount;
	RGBColor colorRGB; //!< The RGB color of the first pixel in the region
	double colorVal; //!< The gray color of the first pixel in the region
	RGBColor fakeColor; //!< The segmentation color
	UICoordinate firstPt;
	UIBoundingBox bbox;
	DiscreteXYArray boundaryPts;
	std::list<DiscreteXYArray> holeBndryPts;
	
	BoolImg mask;
	unsigned groupId; //!< Used to group regions selected by the user

	Region() { Clear(); }
	Region(const UIBoundingBox& bb) : bbox(bb) { Clear(); }

	void Clear()
	{
		numPts = 0; 
		voteCount = 0;
		groupId = 0;
	}

	void operator=(const Region& rhs)
	{
		numPts       = rhs.numPts;
		voteCount    = rhs.voteCount;
		colorRGB     = rhs.colorRGB;
		colorVal     = rhs.colorVal;
		fakeColor    = rhs.fakeColor;
		firstPt      = rhs.firstPt;
		bbox         = rhs.bbox;
		boundaryPts  = rhs.boundaryPts;
		holeBndryPts = rhs.holeBndryPts;
		groupId      = rhs.groupId;
	}

	void Serialize(OutputStream& os) const
	{
		::Serialize(os, numPts);
		::Serialize(os, voteCount);
		::Serialize(os, colorRGB);
		::Serialize(os, colorVal);
		::Serialize(os, fakeColor);
		::Serialize(os, firstPt);
		::Serialize(os, bbox);
		::Serialize(os, boundaryPts);
		::Serialize(os, holeBndryPts);
		::Serialize(os, groupId);
	}

	void Deserialize(InputStream& is) 
	{
		::Deserialize(is, numPts);
		::Deserialize(is, voteCount);
		::Deserialize(is, colorRGB);
		::Deserialize(is, colorVal);
		::Deserialize(is, fakeColor);
		::Deserialize(is, firstPt);
		::Deserialize(is, bbox);
		::Deserialize(is, boundaryPts);
		::Deserialize(is, holeBndryPts);
		::Deserialize(is, groupId);
	}

	void SetMaskSizeToBBox()
	{
		mask.set_size(bbox.ni(), bbox.nj());
		mask.fill(false);
	}

	void SetMaskPixel(const UICoordinate& c)
	{
		mask(c.x - bbox.xmin, c.y - bbox.ymin) = true;
	}

	void Unselect()
	{
		groupId = 0;
	}

	void SetGroupId(int id)
	{
		groupId = id;
	}

	int GetGroupId() const
	{
		return groupId;
	}

	bool IsSelected() const
	{
		return groupId > 0;
	}
};

} // namespace vpl

