/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "ObjectLearner.h"
#include <VideoParser/ImageProcessor.h>
#include <ShapeParsing/ShapeParser.h>
#include <Tools/UserArguments.h>
#include <VideoParserGUI/DrawingUtils.h>

using namespace vpl;

extern UserArguments g_userArgs;

/*!
	Reads the parameters provided by the user. 

	It is called by VisSysComponent::Initialize(), which
	is in turn calledwhen the vis component is created.
	
	It is NOT an static function because the user is allowed
	to changed the parameters of a vision system component
	several times during the execution of a program. 
*/
void ObjectLearner::ReadParamsFromUserArguments()
{
	VisSysComponent::ReadParamsFromUserArguments();

	g_userArgs.ReadArg("ObjectLearner", "modelDBPath", 
		"Path to the model object database", std::string(), 
		&m_params.modelDBPath);
}

void ObjectLearner::Initialize(graph::node v)
{
	VisSysComponent::Initialize(v);

	m_pImgProcessor = FindParentComponent(ImageProcessor);
	m_pShapeParser = FindParentComponent(ShapeParser);

	std::list<UserCommandInfo> cmds;
	ShapeParseGraph::GetSwitchCommands(cmds);
	RegisterUserSwitchCommands(0, cmds);

	m_modeDBIsLoaded = false;
	m_savedParsingParams = false;	

	m_modelHierarchy.Clear();
	m_bModelHierarchyLoaded = false;
}

void ObjectLearner::GetParameterInfo(int i, DoubleArray* pMinVals, 
									 DoubleArray* pMaxVals, 
									 DoubleArray* pSteps) const
{
	unsigned objCount;

	if (!m_modeDBIsLoaded && !OpenModelDatabase())
		objCount = 0;
	else
		objCount = m_modelDB.ObjectCount<ShapeParseGraph>();
		

	InitArrays(2, pMinVals, pMaxVals, pSteps,
			0, (objCount > 0) ? objCount - 1 : 0, 1);

	// Param 1 is a generic maximum num of parts
	pMaxVals->at(1) = 50;
}

bool ObjectLearner::GetTrainingObjectData(TrainingObjectData* pData) const
{
	if (m_frameMetadata.empty())
		return false;

	pData->className.assign(m_objMeta.name);
	pData->objId = m_objMeta.id;

	pData->viewProps = m_viewMeta.Get();
	
	return true;
}

bool ObjectLearner::OpenModelDatabase()
{
	ASSERT(!m_modeDBIsLoaded);

	m_modeDBIsLoaded = m_modelDB.Create(
		m_params.modelDBPath.c_str());

	if (!m_modeDBIsLoaded)
		ShowOpenFileError(m_params.modelDBPath);

	return m_modeDBIsLoaded;
}

void ObjectLearner::Run()
{
	if (!m_pImgProcessor)
	{
		ShowMissingDependencyError(m_pImgProcessor);
		return;
	}

	if (!m_pShapeParser)
	{
		ShowMissingDependencyError(ShapeParser);
		return;
	}
	// The first thing to do is to open the model database
	// even if not learning new objects, since dependent components
	// might ask for it (eg, the ObjectRecognizer)
	if (!m_modeDBIsLoaded && !OpenModelDatabase())
		return;

	// Keep the current frame metadata regardless of whether we are learning
	// new models or not. This is used to analyze experimental results.
	m_frameMetadata = m_pImgProcessor->FrameMetadata();

	int foo = 1;
	if (!m_frameMetadata.empty())
	{
		// Save the object metadata. ie, the first field of the metadata (prefix, number)
		m_objMeta.Set(m_frameMetadata.front().first, m_frameMetadata.front().second);
		
		// The first key-value pair of the metadata corresponds to
		// the object name and id. All other fields are view properties.
		// We replace such first key-value pair with an additional view
		// property that indexes over the number of disconnected shapes 
		// that form the object view (set bellow).
		StrIntList viewProps = m_frameMetadata;

		viewProps.front().first = "*";
		viewProps.front().second = 0;		
		
		m_viewMeta.Set(viewProps);
	}
	else
	{
		m_objMeta.SetEmpty();
		m_viewMeta.SetEmpty();
	}

	// If the task is not "learning" learn the hierarchy of model objects
	if (TaskName() != "Learning")
	{
		if (!m_bModelHierarchyLoaded)
		{
			ShowStatus1("Reading model database: ", 
				GetModelDatabse().FileName());
		
			m_modelHierarchy.Load(GetModelDatabse());
		
			ShowStatus2("Database is in memory and has", 
				m_modelHierarchy.ModelViewCount(), "objects views");

			m_bModelHierarchyLoaded = true;
		}
	}
	else // The task is learning, so learn new model object views
	{
		ShowStatus("Learning model objects...");

		ASSERT(!m_frameMetadata.empty());

		// Make sure that we saved the params used to parse the shapes
		if (!m_savedParsingParams)
		{
			// Save the "serializable" user arguments selected by the 
			// ShapeParser component using ParamFile::AddSerializableFieldKey()
			m_modelDB.Save(g_userArgs);
			m_savedParsingParams = true;
		}

		// Init the storage info of the object and view metadata
		m_objMeta.SetStorageInfo(INVALID_STORAGE_ID);
		m_viewMeta.SetStorageInfo(INVALID_STORAGE_ID);

		// It is assumed that the current frames has one object that might be
		// formed by multiple "disconnected" shapes. The view data of the object
		// is recovered from the frame metadata. If the same object name-id
		// appears in another frame, there will be mutiple object metadatas
		// with the same object-level info.
		unsigned objMetaId = m_modelDB.Save(m_objMeta);

		for (unsigned shapeId = 0; shapeId < m_pShapeParser->NumShapes(); ++shapeId)
		{
			// Save the shape info, which represents a view of an object
			unsigned shapeDataId = m_modelDB.Save(m_pShapeParser->GetShapeInfo(shapeId));

			// Save the shape metadata
			m_viewMeta.propId[0] = shapeId;
			m_viewMeta.SetStorageInfo(shapeDataId, objMetaId);

			unsigned viewMetaId = m_modelDB.Save(m_viewMeta);

			// Save the shape parses
			auto shapeParses = m_pShapeParser->GetShapeParses(shapeId);

			for (unsigned parseId = 0; parseId < shapeParses.size(); ++parseId)
			{
				// Save the shape parse
				unsigned parseDataId = m_modelDB.Save(*shapeParses[parseId]);

				// Save the shape parse metadata
				m_parseMeta.Set(parseId);
				m_parseMeta.SetStorageInfo(parseDataId, viewMetaId);

				m_modelDB.Save(m_parseMeta);
			}
		}

		m_modelDB.FlushEverything();

		// This line can be used to count objects of certain type 
		// in the database
		//unsigned n = m_modelDB.ObjectCount<ShapeParseGraph>();
	}
}

/*!
	Draws the output of the component. This function is called from
	a "drawing" thread, which is different from the "processing" thread
	that calls Run(). In order to protect the shared resources between the
	threads, a mutex is used.
*/
void ObjectLearner::Draw(const DisplayInfoIn& dii) const
{	
	if (!m_modeDBIsLoaded && !OpenModelDatabase())
		return;

	// Read the shape parse metadata (has the parse ID, etc)
	ShapeParseMetadata m_parseMeta;

	if (!m_modelDB.Load(m_parseMeta, (unsigned)dii.params[0]))
	{
		ShowErrorAndNumber("Can't load parse metadata with id", (unsigned)dii.params[0]);
		return;
	}

	// Read the actual shape parse graph
	ShapeParseGraph spg;

	if (!m_modelDB.Load(spg, m_parseMeta.storDataId))
	{
		ShowErrorAndNumber("Can't load shape parse graph with id", m_parseMeta.storDataId);
		return;
	}

	// Find out the information about what model object view this is
	ShapeViewMetadata m_viewMeta;

	if (!m_modelDB.Load(m_viewMeta, m_parseMeta.storParentMetadataId))
	{
		ShowErrorAndNumber("Can't load shape metadata with id", m_parseMeta.storParentMetadataId);
		return;
	}

	// Find out the shape information needed to draw the shape
	ShapeInfoPtr ptrSI(new ShapeInformation);

	if (!m_modelDB.Load(*ptrSI, m_viewMeta.storDataId))
	{
		ShowErrorAndNumber("Can't load shape information with id", m_viewMeta.storDataId);
		return;
	}

	// Store the shape info into the SPG so that it can use it when drawing
	spg.SetShapeInfo(ptrSI);

	// Draw the whole SPG (param == 0) or one of its nodes (param == node id)
	spg.Draw((unsigned)dii.params[1]);
}

/*!	
	Returns the text output for a given an output index 'i'
	with parameter 'param'. This function is called from
	a "drawing" thread, which is different from the "processing" thread
	that calls Run().
*/
void ObjectLearner::GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const
{
	if (!m_modeDBIsLoaded && !OpenModelDatabase())
		return;

	// Set the basic output info first
	dio.imageType = VOID_IMAGE;
	dio.syncDisplayViews = true;

	// Set the output message
	ObjectClassMetadata m_objMeta;
	ShapeViewMetadata m_viewMeta;
	ShapeParseMetadata m_parseMeta;

	if (!m_modelDB.Load(m_parseMeta, (unsigned)dii.params[0]))
	{
		ShowErrorAndNumber("Can't load parse metadata with id", (unsigned)dii.params[0]);
		return;
	}

	if (!m_modelDB.Load(m_viewMeta, m_parseMeta.storParentMetadataId))
	{
		ShowErrorAndNumber("Can't load shape metadata with id", m_parseMeta.storParentMetadataId);
		return;
	}

	if (!m_modelDB.Load(m_objMeta, m_viewMeta.storParentMetadataId))
	{
		ShowErrorAndNumber("Can't load object metadata with id", m_viewMeta.storParentMetadataId);
		return;
	}

	std::ostringstream oss;

	oss << "Object: " << m_objMeta.name << ", " << m_objMeta.id
		<< ". View: " << m_viewMeta.ToString() 
		<< ". Parse id:" << m_parseMeta.parseId
		<< ". Display ID:" << dii.displayId;

	dio.message = oss.str();
}
