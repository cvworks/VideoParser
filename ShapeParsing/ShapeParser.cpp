/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "ShapeParser.h"
#include <ImageSegmentation/RegionAnalyzer.h>
#include <VideoParser/ImageProcessor.h>
#include <Tools/UserArguments.h>
#include <Tools/VSCDataSerializer.h>
#include <VideoParserGUI/DrawingUtils.h>

#include <ShapeRepresentation/ShapeContextComp.h>

using namespace vpl;

extern UserArguments g_userArgs;

ShapeParser::Params ShapeParser::s_params;
const bool DOING_TRECVID_RECOGNITION_WORK = false;

/*! 
	[static] Reads the subset of user arguments associated 
	with static parameters.
*/
void ShapeParser::ReadStaticParameters()
{
	// Read the parameters for computing the shape info
	ShapeInformation::ReadParamsFromUserArguments();

	// Read the parameters for the shape cut graph
	ShapeCutGraph::ReadParamsFromUserArguments();

	// Read the parameters for the shape parsing model class
	ShapeParsingModel::ReadParamsFromUserArguments();

	// Read the general parameters for parsing shapes
	g_userArgs.ReadArg("ShapeParser", "maxNumParses", 
		"Maximum number of parses per shape", 1u, &s_params.maxNumParses);
}

/*!
	Reads the parameters provided by the user. 

	It is called by VisSysComponent::Initialize(), which
	is in turn calledwhen the vis component is created.
	
	It is NOT an static function because the user is allowed
	to changed the parameters of a vision system component
	several times during the execution of a program. 
*/
void ShapeParser::ReadParamsFromUserArguments()
{
	VisSysComponent::ReadParamsFromUserArguments();

	// Read the parameters for computing the skeletons
	m_skeletonizationParams.ReadFromUserArguments();

	ShapeParser::ReadStaticParameters();
}

void ShapeParser::Initialize(graph::node v)
{
	VisSysComponent::Initialize(v);

	m_pRegAnalyzer = FindParentComponent(RegionAnalyzer);
	m_pImgProcessor = FindParentComponent(ImageProcessor);

	std::list<UserCommandInfo> cmds0, cmds1, cmds2, cmds3;

	ShapeInformation::GetSwitchCommands(cmds0);
	ShapeCutGraph::GetSwitchCommands(cmds1);
	ShapeParsingModel::GetSwitchCommands(cmds2);
	ShapeParseGraph::GetSwitchCommands(cmds3);

	RegisterUserSwitchCommands(0, cmds0);
	RegisterUserSwitchCommands(1, cmds1);
	RegisterUserSwitchCommands(2, cmds2);
	RegisterUserSwitchCommands(3, cmds3);
}

void ShapeParser::Run()
{
	if (!m_pRegAnalyzer)
	{
		ShowMissingDependencyError(RegionAnalyzer);
		return;
	}

	if (!m_pImgProcessor)
	{
		ShowMissingDependencyError(ImageProcessor);
		return;
	}

	ShapeInfoPtr ptrShapeInfo;

	ShowStatus("Parsing...");

	// Erase previous shapes
	m_shapes.clear();

	bool hasSavedData = false;

	// Try loading the parsing data for the current frame
	if (m_pDataSerializer)
	{
		hasSavedData = m_pDataSerializer->LoadComponentData(
			m_shapes, this, "ShapeParsingModel");
	}

	if (hasSavedData)
	{
		ShowStatus("We have saved data :-)");

		for (auto it = m_shapes.begin(); it != m_shapes.end(); ++it)
			it->ComputeShapeParses();
	}
	else
	{
		const RegionPyramid& regions = m_pRegAnalyzer->GetRegions();
		unsigned width, height;
		bool hasSkeleton;

		m_pRegAnalyzer->GetImgDimensions(&width, &height);

		RegionPyramid::const_iterator selRegIt = regions.end();
		unsigned maxPerim = 0;

		// @TODO This is temporary: select only largest region that is not
		// defined by the full image boundary
		int region_count = 0;
		for (auto it = regions.begin(); it != regions.end(); ++it)
		{
			if (it->bbox.xmin == 0 && it->bbox.ymin == 0)
				continue;

			if (it->boundaryPts.Size() > maxPerim)
			{
				maxPerim = it->boundaryPts.Size();
				selRegIt = it;
			}
			region_count++;
		}

		static int frame_count = 0;
		frame_count++;

		//////////////////////////////
		if (DOING_TRECVID_RECOGNITION_WORK)
		{
			if (frame_count % 5 == 0)
			{
				std::set<int> salient_region_ids = m_pRegAnalyzer->getSalientRegions();
				for (auto it = salient_region_ids.begin(); it != salient_region_ids.end(); ++it)
				{
					std::cout << "Salient reigon number: " << (*it) << " ------------------------------" << std::endl;
					Region r = regions.region(0, (*it));

					// Create a new shape info object, but don't add it yet
					ptrShapeInfo.reset(new ShapeInformation); // it self destroys if not used

					ShowStatus("Computing skeleton...");

					try 
					{
						hasSkeleton = ptrShapeInfo->Create(r.boundaryPts, m_skeletonizationParams);
					}
					catch (...)
					{
						hasSkeleton = false;
					}

					// See if the shape info can be created
					if (hasSkeleton)
					{
						ptrShapeInfo->SetMetaAttributes(width, height, r.bbox);

						ShowStatus("Computing shape parsing model...");

						m_shapes.push_back(ShapeParsingModel());

						// Create a shape parsing model with the shape cut graph
						m_shapes.back().Create(ptrShapeInfo, MaxNumParses());

						ShowStatus("Computing shape parse graphs...");
				
						// Find the K most probable shape parses
						m_shapes.back().ComputeShapeParses();

						ShowStatus("Shape parsing is done!");
					}
					else
					{
						auto frameMetadata = m_pImgProcessor->FrameMetadata();

						ASSERT(!frameMetadata.empty());

						StreamError("Cannot create shape representation for object (" <<
							frameMetadata.front().first << ", " <<
							frameMetadata.front().second << ")");
					}

				}
			}

		}
		/////////////////////////////
		else
		{
			for (auto it = regions.begin(); it != regions.end(); ++it)
			{
				if (it->bbox.xmin == 0 && it->bbox.ymin == 0)
					continue;

				if (it != selRegIt)
					continue;

				// Create a new shape info object, but don't add it yet
				ptrShapeInfo.reset(new ShapeInformation); // it self destroys if not used

				ShowStatus("Computing skeleton...");

				try 
				{
					hasSkeleton = ptrShapeInfo->Create(it->boundaryPts, m_skeletonizationParams);
				}
				catch (...)
				{
					hasSkeleton = false;
				}

				// See if the shape info can be created
				if (hasSkeleton)
				{
					ptrShapeInfo->SetMetaAttributes(width, height, it->bbox);

					ShowStatus("Computing shape parsing model...");

					m_shapes.push_back(ShapeParsingModel());

					// Create a shape parsing model with the shape cut graph
					m_shapes.back().Create(ptrShapeInfo, MaxNumParses());

					ShowStatus("Computing shape parse graphs...");
				
					// Find the K most probable shape parses
					m_shapes.back().ComputeShapeParses();

					ShowStatus("Shape parsing is done!");
				}
				else
				{
					auto frameMetadata = m_pImgProcessor->FrameMetadata();

					ASSERT(!frameMetadata.empty());

					StreamError("Cannot create shape representation for object (" <<
						frameMetadata.front().first << ", " <<
						frameMetadata.front().second << ")");
				}
			}
		}
	}

	// See if we have to save the data that we have
	if (!hasSavedData && m_pDataSerializer)
	{	
		m_pDataSerializer->SaveComponentData(
			m_shapes, this, "ShapeParsingModel");
	}
}

/*!
	Draws the output of the component. This function is called from
	a "drawing" thread, which is different from the "processing" thread
	that calls Run(). In order to protect the shared resources between the
	threads, a mutex is used.
*/
void ShapeParser::Draw(const DisplayInfoIn& dii) const
{
	if (!m_shapes.empty())
	{
		unsigned param0 = (unsigned) dii.params[0];
		unsigned param1 = (unsigned) dii.params[1];

		const ShapeParsingModel& spm = element_at(m_shapes, param0);

		switch (dii.outputIdx)
		{
			case 0:
				spm.GetShapeInfo().Draw(); break;
			case 1:
				spm.GetSCG().Draw(param1); 
				break;
			case 2:
				spm.Draw(param1); 
				break;
			default:
				unsigned param2 = (unsigned) dii.params[2];

				if (param1 < spm.NumberOfParses() && param2 <=
					(unsigned)spm.GetShapeParse(param1).number_of_nodes())
				{
					spm.GetShapeParse(param1).Draw(NodeMatchMap(), param2); 
				}
				break;
		}
	}
}

/*!	
	Returns the text output for a given an output index 'i'
	with parameter 'param'. This function is called from
	a "drawing" thread, which is different from the "processing" thread
	that calls Run().
*/
void ShapeParser::GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const
{
	RGBImg colImg;

	dio.imageType = VOID_IMAGE;
	dio.imagePtr = ConvertToBaseImgPtr(colImg);

	if (!m_shapes.empty())
	{
		unsigned param0 = (unsigned) dii.params[0];
		unsigned param1 = (unsigned) dii.params[1];

		const ShapeParsingModel& spm = element_at(m_shapes, param0);

		switch (dii.outputIdx)
		{
			case 0:
				dio.message = spm.GetShapeInfo().GetOutputText(); 
				break;
			case 1:
				dio.message = spm.GetSCG().GetOutputText(param1); 
				break;
			case 2:
				dio.message = spm.GetOutputText(); 
				break;
			default:
				if (param1 < spm.NumberOfParses())
					dio.message = spm.GetShapeParse(param1).GetOutputText(); 
				break;
		}
	}
}


