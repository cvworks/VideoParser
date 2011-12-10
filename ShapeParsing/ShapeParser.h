/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _SHAPE_PARSER_H_
#define _SHAPE_PARSER_H_

#include <list>
#include "ShapeParsingModel.h"
#include <Tools/VisSysComponent.h>
#include <Tools/STLUtils.h>

namespace vpl {

class RegionAnalyzer;
class ImageProcessor;

/*!
	@brief Wrapper for a generic shape parser algorithm
*/
class ShapeParser : public VisSysComponent
{
	struct Params
	{
		unsigned maxNumParses;
	};

	static Params s_params;

protected:
	typedef std::list<SPM> ShapeList;

	ShapeList m_shapes;  //!< Parsing models for each shape in the image

	SkeletalGraphParams m_skeletonizationParams;

	std::shared_ptr<const RegionAnalyzer> m_pRegAnalyzer;
	std::shared_ptr<const ImageProcessor> m_pImgProcessor;
	
public:
	unsigned NumShapes() const { return m_shapes.size(); }

	const ShapeInformation& GetShapeInfo(unsigned i) const 
	{ 
		return element_at(m_shapes, i).GetShapeInfo(); 
	}

	const SCG& GetSCG(unsigned i) const 
	{ 
		return element_at(m_shapes, i).GetSCG(); 
	}

	const std::vector<SPGPtr>& GetShapeParses(unsigned i) const 
	{
		return element_at(m_shapes, i).GetShapeParses(); 
	}

public:	
	virtual void ReadParamsFromUserArguments();

	//! Reads the subset of user arguments associated with static parameters
	static void ReadStaticParameters();

	static unsigned MaxNumParses()
	{
		return s_params.maxNumParses;
	}

	virtual void Initialize(graph::node v);

	virtual std::string ClassName() const
	{
		return "ShapeParser";
	}

	virtual StrArray Dependencies() const
	{
		StrArray deps;

		deps.push_back("ImageProcessor");
		deps.push_back("RegionAnalyzer");

		return deps;
	}
	
	virtual void Run();

	virtual void Draw(const DisplayInfoIn& dii) const;

	/*!
		If i == 0, only iterate over the shapes parsed.

		If i > 0, iterate both over the shapes parsed and 
		their parses.
	*/
	virtual void GetParameterInfo(int i, DoubleArray* pMinVals, 
		DoubleArray* pMaxVals, DoubleArray* pSteps) const
	{
		if (i == 0) //only iterate over the shapes parsed
		{
			// Param 0 is the number of shapes being parsed
			InitArrays(1, pMinVals, pMaxVals, pSteps,
				0, m_shapes.size() - 1, 1);
		}
		else if (i == 1 || i == 2) //iterate over the shapes and their parses
		{
			// Param 0 is the number of shapes being parsed
			InitArrays(2, pMinVals, pMaxVals, pSteps,
				0, m_shapes.size() - 1, 1);
			
			// Param 1 is the number of parses
			pMaxVals->at(1) = MaxNumParses() - 1;
		}
		else //iterate over the shapes, their parses, and each part
		{
			// Param 0 is the number of shapes being parsed
			InitArrays(3, pMinVals, pMaxVals, pSteps,
				0, m_shapes.size() - 1, 1);
			
			// Param 1 is the number of parses
			pMaxVals->at(1) = MaxNumParses() - 1;

			// Number of parts
			pMaxVals->at(2) = 20;
		}
	}
	
	virtual int NumOutputImages() const 
	{ 
		return 4; 
	}

	virtual std::string GetOutputImageLabel(int i) const
	{
		switch (i)
		{
			case 0: return "Shape Information";
			case 1: return "Shape Cut Graph";
			case 2: return "Shape Parsing Model";
			case 3: return "Shape Parse Graph";
		}

		return "error";
	}

	virtual void GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const;
};

} // namespace vpl

#endif //_SHAPE_PARSER_H_
