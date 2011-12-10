/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/STLUtils.h>
#include "ShapeDescriptor.h"

struct CvContourTree;
struct CvMemStorage;
struct CvContour;
struct CvPoint;

namespace vpl
{
/*!
	
*/
class ContourTree : public ShapeDescriptor
{
public:
	//! Parameters for the construction of a shape context
	struct Params
	{
		double accuracyThreshold;   //<! Creation param
		double similarityThreshold; //<! Matching param

		void ReadFromUserArguments();
	};

protected:
	CvContourTree* m_pTree;     //!< Binary tree
    CvMemStorage* m_pStorage;   //!< Memory storage for the tree
	CvContour* m_pContourSeqHeader; //!< Pointer to a CvContour object (ie, a specialized CVSeq)
	CvPoint* m_pContourPts;     //!< Array of contour points
	unsigned m_numPoints;       //!< Number of contour points

	PointArray m_ptsArray; //!< Original contour points stored in an array

	static Params s_params; //!< Static parameters for the construction of shape contexts

	void CopyPointsToOpenCVSeq(const PointArray& pts);
	void Create();

public:
	ContourTree()
	{
		m_pTree = NULL;
		m_pStorage = NULL;
		m_pContourSeqHeader = NULL;
		m_pContourPts = NULL;
	}

	~ContourTree();

	static const Params& GetParams()
	{
		return s_params;
	}
	
	//! Cast the wrapper class
	operator const CvContourTree*() const 
	{ 
		ASSERT(m_pTree);

		return m_pTree; 
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

	//! Reads static parameters for the creation of the shape descriptor
	virtual void ReadClassParameters() const
	{
		s_params.ReadFromUserArguments();
	}

	//! Draws some visualization of the shape descriptor
	virtual void Draw(const RGBColor& color) const;

	void operator=(const ContourTree& rhs)
	{
		// Call the assignment operator in the base class
		ShapeDescriptor::operator=(rhs);

		s_params = rhs.s_params;
	}

	//! Creates a shape context for the boundary 'pts'
	virtual void Create(const PointArray& pts, const DoubleArray& tangents);

	//! Defines the serialize function required by the abstract base class
	virtual void Serialize(OutputStream& os) const;

	//! Defines the deserialize function required by the abstract base class
	virtual void Deserialize(InputStream& is);
};

} //namespace vpl

