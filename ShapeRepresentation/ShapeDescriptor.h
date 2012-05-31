/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/BasicTypes.h>
#include <Tools/MathUtils.h>
#include <Tools/STLUtils.h>
#include <Tools/ImageUtils.h>
#include <Tools/GenericParameters.h>
#include <Tools/Serialization.h>
#include "PointTransform.h"

namespace vpl {

class ShapeDescriptor;

typedef std::shared_ptr<ShapeDescriptor> ShapeDescPtr;

/*!
	Abstract class represeting a shape descriptor.

	The comparison of shape descriptors is done using an object
	of the ShapeDescriptorComp class.

	@see ShapeDescriptorComp
*/
class ShapeDescriptor
{
protected:
	unsigned m_boundaryLength;
	 
public:
	//! Constructor of a generic shape descriptor 
	ShapeDescriptor()
	{
		m_boundaryLength = 0;
	}

	//! The necessary virtual destructor
	virtual ~ShapeDescriptor()
	{
		
	}

	//! Reads static parameters for the creation of the shape descriptor
	virtual void ReadClassParameters() const = 0;

	//! Creates a shape descriptor
	virtual void Create(const PointArray& pts, const DoubleArray& tangents, unsigned boundaryLength)
	{
	}

	/*! 
		Returns the cost or similarity value that corresponds to
		the matching of this descriptor agains a nil descriptor.
	*/
	virtual double NilMatchValue() const = 0;

	virtual unsigned NumPoints() const
	{
		return 0;
	}
	
	virtual void Serialize(OutputStream& os) const
	{
	}

	virtual void Deserialize(InputStream& is)
	{
	}

	//! Draws some visualization of the shape descriptor
	virtual void Draw(const RGBColor& color) const
	{
		// It's an optional function. By default, do nothing.
	}

	// get the number of points on the boundary.
	// strangly enough, the sum of this for all parts in shape is
	// not equal to the number of points on the shape's whole
	// boundary.  It's very close though.  Investigate.
	// Example of this being set: 
	// ShapeParseGraph::ComputeShapeDescriptors()
	virtual unsigned int GetBoundaryLength() const
	{
		return m_boundaryLength;
	}

	//! Populates the point array with all N data points
	virtual void GetPoints(PointArray* pPts) const
	{
		pPts->clear();
	}
	
	/*!
	Creates a new shape descriptor that is the result of transforming
	this descriptor using 'params'. If the transformation is not
	possible, a NULL pointer is returned.
	*/
	virtual ShapeDescPtr Transform(const PointTransform& params) const
	{
		return NULL;
	}

	//! Computes an affine homography.
	Matrix ComputeHomography(const PointArray& tgtPts) const
	{
		PointArray srcPts;

		GetPoints(&srcPts);

		return ComputeHomography(srcPts, tgtPts);
	}

	PointArray WarpAffine(const PointArray& tgtPts) const
	{
		PointArray srcPts;

		GetPoints(&srcPts);

		return ApplyHomography(ComputeHomography(srcPts, tgtPts), srcPts);
	}

	//! [static] Point matrix must be 2 x N.
	static PointArray ToPointArray(const Matrix& m)
	{
		ASSERT(m.rows() == 2);

		PointArray pa(m.cols());

		for (unsigned i = 0; i < pa.size(); i++)
			pa[i].Set(m(0, i), m(1, i));

		return pa;
	}

	static Matrix ComputeHomography(const PointArray& srcPts, const PointArray& tgtPts);
	static PointArray ApplyHomography(const Matrix& H, const PointArray& srcPts);

	//! [static] Point matrices must be 2 x N.
	static Matrix ComputeHomography(const Matrix& srcPts, const Matrix& tgtPts)
	{
		return ComputeHomography(ToPointArray(srcPts), ToPointArray(tgtPts));
	}

	//! [static] Applies the homography H to the points.
	static Matrix ApplyHomography(const Matrix& H, const Matrix& srcPts);
};

} // namespace vpl
