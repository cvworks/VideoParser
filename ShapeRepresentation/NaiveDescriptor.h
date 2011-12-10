/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "ShapeDescriptor.h"

namespace vpl {

class NaiveDescriptor : public ShapeDescriptor
{
protected:
	PointArray m_pts;

public:
	/*! 
		Assignment operator. Must be overwritten in the derived classes,
		which in turn must call this base function.
	*/
	NaiveDescriptor& operator=(const NaiveDescriptor& rhs)
	{
		ShapeDescriptor::operator =(rhs);

		m_pts = rhs.m_pts;

		return *this;
	}

	const PointArray& Points() const
	{
		return m_pts;
	}

	/*! 
		Returns the cost or similarity value that corresponds to
		the matching of this descriptor agains a nil descriptor.
	*/
	virtual double NilMatchValue() const
	{
		ASSERT(false);
		return 0;
	}

	virtual unsigned NumPoints() const
	{
		return m_pts.size();
	}

	//! Reads static parameters for the creation of the shape descriptor
	virtual void ReadClassParameters() const
	{
		// there are no params to read
	}

	//! Creates a shape descriptor
	virtual void Create(const PointArray& pts, const DoubleArray& tangents)
	{
		m_pts = pts;
	}
	
	virtual void Serialize(OutputStream& os) const
	{
		::Serialize(os, m_pts);
	}

	virtual void Deserialize(InputStream& is)
	{
		::Deserialize(is, m_pts);
	}

	//! Draws some visualization of the shape descriptor
	virtual void Draw(const RGBColor& color) const;
};

} // namespace vpl
