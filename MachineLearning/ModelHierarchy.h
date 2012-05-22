/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "ModelMetadata.h"
#include <ShapeParsing/ShapeParseGraph.h>

namespace vpl {

/*!
	Hierarchy of model object silluettes and their parses.

	The leaves of the hierarchy are shape parses. The next level up
	are shape views, which have one or more shape parses linked to them.
	All the levels above shape views are object classes. The children
	of an object class are either all shape views or all object classes.
	The type of children of an object class are indicated by an member
	enum variable with values OBJECT_VIEW or OBJECT_CLASS.

	The metadata of each node has a database index to retrieve a data block
	and an index that indicates the parent of the data block. The parent index
	can also be used to find the parent of a model hierarchy node because
	the database indices and the model hierarchy indices are guaranteed 
	to be the same.

	Note, the 'parseIdx' is a relative 'shapeIdx'. That is, 'parseIdx' is
	an index over the parses of some object view. In contrast, a 'shapeIdx'
	is an index over all the parses of all the views of all objects.
*/
class ModelHierarchy
{
public:
	struct ModelHierarchyNode
	{
	public:
	};

	struct ShapeParse : public ModelHierarchyNode
	{
		ShapeParseMetadata parseInfo;
		ShapeParseGraph spg;
	};

	struct ModelView : public ModelHierarchyNode
	{
		ShapeViewMetadata viewInfo;
		ShapeInfoPtr ptrShapeContour; //!< Shared with 1+ spgs
		std::vector<unsigned> shapeParses;

		ModelView()
		{
			shapeParses.reserve(10);
		}
	};

	struct ModelObject : public ModelHierarchyNode
	{
		enum ChildrenType {OBJECT_VIEW, OBJECT_CLASS};

		ObjectClassMetadata classInfo;
		ChildrenType type; //!< Tells whether it has children that are views or objects
		std::list<unsigned> modelViewsOrClasses; //!< The child views or child object classes

		//! Sets OBJECT_VIEW as the default children type
		ModelObject() { type = OBJECT_VIEW; }

		//! Returns true of the object is a top object class
		bool IsRootObjectClass() const 
		{ 
			return classInfo.storParentMetadataId == INVALID_STORAGE_ID; 
		}
	};

protected:
	std::vector<ShapeParse> m_shapeParses;
	std::vector<ModelView> m_modelViews;
	std::vector<ModelObject> m_modelObjects;

public:
	//! Iterator over all shapes
	typedef std::vector<ShapeParse>::const_iterator const_iterator; 

	//! First iterator over all shapes
	const_iterator begin() const
	{
		return m_shapeParses.begin();
	}

	//! Last iterator over all shapes
	const_iterator end() const
	{
		return m_shapeParses.end();
	}

	//! Array subscript operator over all shapes
	const ShapeParse& operator[](unsigned shapeIdx) const
	{
		return m_shapeParses.at(shapeIdx);
	}

	unsigned NumShapes() const
	{
		return m_shapeParses.size();
	}

public:
	void Load(const SimpleDatabase& modelDB);

	void Clear()
	{
		m_shapeParses.clear();
		m_modelViews.clear();
		m_modelObjects.clear();
	}

	unsigned ModelObjectCount() const
	{
		return m_modelObjects.size();
	}

	const ModelObject& GetModelObject(unsigned i) const
	{
		return m_modelObjects[i];
	}

	unsigned ModelViewCount() const
	{
		return m_modelViews.size();
	}

	const ModelView& GetModelView(unsigned viewIdx) const
	{
		//std::cout << " Size of model database: " << m_modelViews.size() << std::endl;
		return m_modelViews[viewIdx];
	}

	unsigned ShapeParseCount(unsigned viewIdx) const
	{
		return m_modelViews[viewIdx].shapeParses.size();
	}

	const ShapeParseGraph& GetShapeParse(unsigned viewIdx, unsigned parseIdx) const
	{
		return m_shapeParses[m_modelViews[viewIdx].shapeParses[parseIdx]].spg;
	}

	unsigned ModelObjectCount(const std::string& name) const
	{
		unsigned count = 0;

		for (auto it = m_modelObjects.begin(); it != m_modelObjects.end(); ++it)
			if (name == it->classInfo.name)
				count++;

		return count;
	}

	std::string getModelViewClass(const ModelView &mv) const
	{
		unsigned parent_id = mv.viewInfo.storParentMetadataId;
		const ModelObject &parent = m_modelObjects[parent_id];
		return parent.classInfo.name;
	}

	std::string ToString(const ModelView& mv) const
	{
		std::ostringstream oss;

		unsigned parIdx = mv.viewInfo.storParentMetadataId;

		oss << "(";

		if (parIdx != INVALID_STORAGE_ID)
		{
			const ModelObject& parent = m_modelObjects[parIdx];

			oss << parent.classInfo.name << "," 
				<< parent.classInfo.id << ",";
		}

		oss << mv.viewInfo.ToString();
		
		oss << ")";

		return oss.str();
	}

	bool HasParent(const ModelView& mv, const std::string& name, int id) const
	{
		unsigned parIdx = mv.viewInfo.storParentMetadataId;

		if (parIdx == INVALID_STORAGE_ID)
			return false;

		const ModelObject& parent = m_modelObjects[parIdx];

		return (name == parent.classInfo.name && id == parent.classInfo.id);
	}

	bool HasAncestor(const ModelView& mv, const std::string& name) const
	{
		unsigned parIdx = mv.viewInfo.storParentMetadataId;

		if (parIdx == INVALID_STORAGE_ID)
			return false;

		const ModelObject& parent = m_modelObjects[parIdx];

		return (name == parent.classInfo.name) ? true : HasAncestor(parent, name);
	}

	bool HasAncestor(const ModelObject& mo, const std::string& name) const
	{
		unsigned parIdx = mo.classInfo.storParentMetadataId;

		if (parIdx == INVALID_STORAGE_ID)
			return false;

		const ModelObject& parent = m_modelObjects[parIdx];

		return (name == parent.classInfo.name) ? true : HasAncestor(parent, name);
	}
};

} // namespace vpl
