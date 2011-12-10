/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/VisSysComponent.h>
#include <Tools/SimpleDatabase.h>
#include <Tools/STLUtils.h>
#include "ModelMetadata.h"
#include "ModelHierarchy.h"

namespace vpl {

class ImageProcessor;
class ShapeParser;

struct TrainingObjectData
{
	std::string className;
	int objId;
	StrIntList viewProps;

	void operator=(const TrainingObjectData& rhs)
	{
		className = rhs.className;
		objId = rhs.objId;
		viewProps = rhs.viewProps;
	}

	StrIntPair viewProp(unsigned i) const
	{
		ASSERT(i < viewProps.size());

		return element_at(viewProps, i);
	}

	std::string viewPropName(int i) const
	{
		return viewProp(i).first;
	}

	int viewPropId(int i) const
	{
		return viewProp(i).second;
	}

	bool Compare(const ShapeViewMetadata& rhs) const
	{
		StrIntList viewProps2(viewProps);

		// We don't care about the first property
		viewProps2.front().first = rhs.Get(0).first;
		viewProps2.front().second = rhs.Get(0).second;

		return rhs.Compare(viewProps2);
	}

	std::string ToString() const
	{
		std::stringstream ss;

		ss << "(" << className << "," << objId;
		
		for (auto it = viewProps.begin(); it != viewProps.end(); ++it)
			ss << "," << it->first << "," << it->second;

		ss << ")";

		return ss.str();
	}
};

/*!
	@brief Wrapper for a generic object recognition algorithm
*/
class ObjectLearner : public VisSysComponent
{
	typedef std::pair<std::string, int> KeyValuePair;
	typedef std::list<KeyValuePair> KeyValueList;

	struct Params
	{
		std::string modelDBPath;
	};

	Params m_params;

protected:
	std::shared_ptr<const ImageProcessor> m_pImgProcessor;
	std::shared_ptr<const ShapeParser> m_pShapeParser;
	SimpleDatabase m_modelDB;
	bool m_modeDBIsLoaded;
	KeyValueList m_frameMetadata;

	ObjectClassMetadata m_objMeta;
	ShapeViewMetadata m_viewMeta;
	ShapeParseMetadata m_parseMeta;

	bool m_savedParsingParams;

	ModelHierarchy m_modelHierarchy;
	bool m_bModelHierarchyLoaded;

	bool OpenModelDatabase();

	/*! Some times we need to open the database when drawing
	    and before the component is run, so we need a const
		version of OpenModelDatabase() to call.
	*/
	bool OpenModelDatabase() const
	{
		return const_cast<ObjectLearner*>(this)->OpenModelDatabase();
	}

public:	
	bool GetTrainingObjectData(TrainingObjectData* pData) const;

	const SimpleDatabase& GetModelDatabse() const
	{
		WARNING(!m_modeDBIsLoaded, "Model database isn't loaded");

		return m_modelDB;
	}

	//! Returns the model objects provided by the ObjectLearner
	const ModelHierarchy& GetModelHierarchy() const
	{
		return m_modelHierarchy;
	}

	//! Filename of the model database
	std::string ModelDBPath() const
	{
		return m_params.modelDBPath;
	}

	virtual void Clear()
	{
		VisSysComponent::Clear();

		m_frameMetadata.clear();
	}

	virtual void ReadParamsFromUserArguments();

	virtual void Initialize(graph::node v);

	virtual std::string ClassName() const
	{
		return "ObjectLearner";
	}

	virtual StrArray Dependencies() const
	{
		StrArray deps;

		deps.push_back("ImageProcessor");
		deps.push_back("ShapeParser");

		return deps;
	}
	
	virtual void Run();

	virtual void Draw(const DisplayInfoIn& dii) const;

	virtual void GetParameterInfo(int i, DoubleArray* pMinVals, 
		DoubleArray* pMaxVals, DoubleArray* pSteps) const;
	
	virtual int NumOutputImages() const 
	{ 
		return 1; 
	}

	virtual std::string GetOutputImageLabel(int i) const
	{
		switch (i)
		{
			case 0: return "Models";
		}

		return "error";
	}

	virtual void GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const;
};

} // namespace vpl

