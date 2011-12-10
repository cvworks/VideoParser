/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "ModelHierarchy.h"

using namespace vpl;

/*!
	Loads the params used to parse the shapes in the database.

	It returns false if no params are stored. Otherwise, it
	returns true and *pParams contains the saved parameters.

	If there are more than one set of parameters saved, 
	it throws an exception.
*/
/*bool ModelHierarchy::LoadParsingParams(const SimpleDatabase& modelDB, 
	ShapeParseGraph::Params* pParams)
{
	SimpleDatabase::IndexInfo ii;
	
	if (!modelDB.LoadFirst(*pParams, ii))
		return false;

	// There should be exactly ONE set of params
	DBG_ONLY(ShapeParseGraph::Params auxPP);
	ASSERT(!modelDB.LoadNext(auxPP, ii));

	return true;
}*/

/*!
	The leaves of the hierarchy are shape parses. The next level up
	are shape views, which have one or more shape parses linked to them.
	All the levels above shape views are object classes. The children
	of an object class are either all shape views or all object classes.
	The type of children of an object class are indicated by an member
	enum variable with values OBJECT_VIEW or OBJECT_CLASS.

	@todo There might be multiple object-level metadatas with the same 
	information. All of them should be clustered into one, which their
	dependencies redirected appropriately.
*/
void ModelHierarchy::Load(const SimpleDatabase& modelDB)
{
	SimpleDatabase::IndexInfo ii;

	m_shapeParses.clear();
	m_modelViews.clear();
	m_modelObjects.clear();

	// Read the shape parse metadata (has the parse ID, etc)
	ShapeParseMetadata parseMeta;

	m_shapeParses.resize(modelDB.ObjectCount<ShapeParseMetadata>());

	for (modelDB.LoadFirst(parseMeta, ii); ii; modelDB.LoadNext(parseMeta, ii))
	{
		ShapeParse& sp = m_shapeParses[ii.objId];

		sp.parseInfo = parseMeta;

		// Read the shape parse graph
		if (!modelDB.Load(sp.spg, parseMeta.storDataId))
			ShowErrorAndNumber("Can't load shape parse graph with id", parseMeta.storDataId);
	}

	// Recover the information about each model object view
	ShapeViewMetadata shapeMeta;

	m_modelViews.resize(modelDB.ObjectCount<ShapeViewMetadata>());

	for (modelDB.LoadFirst(shapeMeta, ii); ii; modelDB.LoadNext(shapeMeta, ii))
	{
		ModelView& mv = m_modelViews[ii.objId];

		mv.viewInfo = shapeMeta;

		// Create a new shapeinfo object to store the data
		mv.ptrShapeContour.reset(new ShapeInformation);

		// Read the shape contour info
		if (!modelDB.Load(*mv.ptrShapeContour, shapeMeta.storDataId))
			ShowErrorAndNumber("Can't load shape info with id", shapeMeta.storDataId);
	}

	// Recover the information about each model object
	ObjectClassMetadata objMeta;

	m_modelObjects.resize(modelDB.ObjectCount<ObjectClassMetadata>());

	for (modelDB.LoadFirst(objMeta, ii); ii; modelDB.LoadNext(objMeta, ii))
	{
		m_modelObjects[ii.objId].classInfo = objMeta;
	}

	// Now, we can add the "edges" from parents to children in the hierarchy
	// and assign the share information between the objects
	unsigned i, parIdx;

	for (i = 0; i < m_shapeParses.size(); ++i)
	{
		parIdx = m_shapeParses[i].parseInfo.storParentMetadataId;

		m_modelViews[parIdx].shapeParses.push_back(i);

		// Get the shape contour info from the parent view
		m_shapeParses[i].spg.SetShapeInfo(m_modelViews[parIdx].ptrShapeContour);
	}

	for (i = 0; i < m_modelViews.size(); ++i)
	{
		parIdx = m_modelViews[i].viewInfo.storParentMetadataId;

		m_modelObjects[parIdx].modelViewsOrClasses.push_back(i);
	}

	for (i = 0; i < m_modelObjects.size(); ++i)
	{
		parIdx = m_modelObjects[i].classInfo.storParentMetadataId;

		// See if it has a parent "object". If it does, it means that its
		// parent is an object category rather than an object's name
		if (parIdx != INVALID_STORAGE_ID)
		{
			// The default type of the children is OBJECT_VIEW, so
			// if there are no children, that's normal. But, if there are
			// children, the value now should be OBJECT_CLASS, or else, we'll
			// be mixing views and classes in the list of children.
			ASSERT(m_modelObjects[parIdx].modelViewsOrClasses.empty() ||
				m_modelObjects[parIdx].type == ModelObject::OBJECT_CLASS);

			m_modelObjects[parIdx].type = ModelObject::OBJECT_CLASS;

			m_modelObjects[parIdx].modelViewsOrClasses.push_back(i);
		}
	}
}
