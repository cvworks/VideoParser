/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "ShapeDescriptor.h"

//struct CvMoments;

namespace cv {
class Mat;
}

namespace vpl
{
/*!
	
*/
class HuMoments : public ShapeDescriptor
{
public:
	//! Parameters for the construction of a shape context
	struct Params
	{
		int methodId;

		void ReadFromUserArguments();
	};

protected:
	cv::Mat* m_pPtsMat;

	static Params s_params; //!< Static parameters for the construction of shape contexts

public:
	HuMoments()
	{
		m_pPtsMat = NULL;
	}

	virtual ~HuMoments();
	
	//! Cast the wrapper class
	operator const cv::Mat&() const 
	{ 
		ASSERT(m_pPtsMat);

		return *m_pPtsMat; 
	}

	static const Params& GetParams()
	{
		return s_params;
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

	//! Creates a shape context for the boundary 'pts'
	virtual void Create(const PointArray& pts, const DoubleArray& tangents);

	//! Defines the serialize function required by the abstract base class
	virtual void Serialize(OutputStream& os) const;

	//! Defines the deserialize function required by the abstract base class
	virtual void Deserialize(InputStream& is);
};

} //namespace vpl

