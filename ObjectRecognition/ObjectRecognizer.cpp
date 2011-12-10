/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "ObjectRecognizer.h"
#include <MachineLearning/ObjectLearner.h>
#include <ShapeParsing/ShapeParser.h>
#include <ShapeMatching/ShapeMatcher.h>
#include <Tools/UserArguments.h>
#include <Tools/NamedColor.h>
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
void ObjectRecognizer::ReadParamsFromUserArguments()
{
	VisSysComponent::ReadParamsFromUserArguments();

	//g_userArgs.ReadArg("ObjectRecognizer", "modelDBPath", 
	//	"Path to the model object database", std::string(), &m_params.modelDBPath);

	g_userArgs.ReadArg("ObjectRecognizer", "matchingAlgoritm", 
		"Algorithm used to match graph-based representations of objects", 
		0, &m_params.matchingAlgorithm);

	g_userArgs.ReadBoolArg("ObjectRecognizer", "excludeQueryObjectFromDB", 
		"Check if query object is in model database and removes all its views?", 
		true, &m_params.excludeQueryObjectFromDB);

	g_userArgs.ReadBoolArg("ObjectRecognizer", "excludeQueryViewFromDB", 
		"Check if query object *view* is in model database and removes it?", 
		true, &m_params.excludeQueryViewFromDB);

	g_userArgs.ReadBoolArg("ObjectRecognizer", "onlySumModelNodeMatches", 
		"Sum only the matching distance of model nodes?", 
		false, &m_params.onlySumModelNodeMatches);
}

void ObjectRecognizer::Initialize(graph::node v)
{
	VisSysComponent::Initialize(v);

	m_pObjectLearner = FindParentComponent(ObjectLearner);
	m_pShapeParser = FindParentComponent(ShapeParser);
	m_pShapeMatcher = FindParentComponent(ShapeMatcher);

	std::list<UserCommandInfo> cmds0;

	ShapeParseGraph::GetSwitchCommands(cmds0);

	m_params.overlayWarpQuery = false;

	cmds0.push_back(UserCommandInfo("warp", "overlay warped query", 
		'w', &m_params.overlayWarpQuery));

	RegisterUserSwitchCommands(0, cmds0);	
}

void ObjectRecognizer::Run()
{
	if (!m_pObjectLearner)
	{
		ShowMissingDependencyError(ObjectLearner);
		return;
	}

	if (!m_pShapeParser)
	{
		ShowMissingDependencyError(ShapeParser);
		return;
	}

	if (!m_pShapeMatcher)
	{
		ShowMissingDependencyError(ShapeMatcher);
		return;
	}

	// Init the current ranking
	m_rankings.clear();

	// Only recognize object if we are performing the recognition task
	if (TaskName() != "Recognition")
		return;

	ShowStatus("Recognizing objects...");

	// Get the training info of the query shape if available only
	// if we have to exclude the query object from the model DB
	TrainingObjectData query_tod;
	bool hasQueryTOD = false;

	// hasQueryTOD is true only if we need to exclude something
	// from the database and the learner can provide the TOD
	if (m_params.excludeQueryObjectFromDB || m_params.excludeQueryViewFromDB)
	{
		hasQueryTOD = m_pObjectLearner->GetTrainingObjectData(&query_tod);

		StreamStatus("Matching query " << query_tod.ToString());
	}

	SPGMatch gmatch;
	unsigned dbg_match_counter = 0;

	const unsigned numQueryShapes = m_pShapeParser->NumShapes();

	const ModelHierarchy& modelHierarchy = m_pObjectLearner->GetModelHierarchy();
	const unsigned numModelViews = modelHierarchy.ModelViewCount();

	Timer matchingTimer;

	//DBG_DECLARE_TIMER(timer)
	
	// It is cleared above, so resize is okay here
	m_rankings.resize(numQueryShapes);
	
	//DBG_RESET_TIMER(timer)

	// For all query shapes in the ShapeParser
	for (unsigned s_q = 0; s_q < numQueryShapes; ++s_q)
	{
		QueryRanking& qr = m_rankings[s_q];

		// Get the parses of the query shape
		qr.queryParses = m_pShapeParser->GetShapeParses(s_q);
		
		qr.matchingTime = 0;
		matchingTimer.Reset();

		// There will be exactly one match info per model view
		// All matches are init to SPG_MATCH_NOT_SET (ie, -1)
		ASSERT(qr.matches.empty());
		qr.matches.resize(numModelViews);

		const unsigned numQueryParses = qr.queryParses.size();

		// For all parses of the query shape
		// Note: part of the objective is to find the best parse of the query, so
		// it is okay to have in the final ranking two different query parses that 
		// match to the same model view, and, possibly, even the same model view parse.
		for (gmatch.queryParseIdx = 0; gmatch.queryParseIdx < numQueryParses; 
			++gmatch.queryParseIdx)
		{
			const ShapeParseGraph& G_q = *qr.queryParses[gmatch.queryParseIdx];

			// For all model shapes in the model database
			for (gmatch.modelViewIdx = 0; gmatch.modelViewIdx < numModelViews; 
				++gmatch.modelViewIdx)
			{
				// See if we have to exclude the query object from the model DB
				if (hasQueryTOD)
				{
					const ModelHierarchy::ModelView& mv = 
						modelHierarchy.GetModelView(gmatch.modelViewIdx);

					//StreamStatus("Comparing to " 
					//			<< modelHierarchy.ToString(mv));

					// See if the current object view belongs to the query object
					if (modelHierarchy.HasParent(mv, query_tod.className, query_tod.objId))
					{
						// The query object is the same than the model object
						if (m_params.excludeQueryObjectFromDB)
						{
							StreamStatus("Skipping model object " 
								<< modelHierarchy.ToString(mv));

							continue;
						}
						else if (m_params.excludeQueryViewFromDB && 
							query_tod.Compare(mv.viewInfo))
						{
							// The query view is the same than the model view	
							StreamStatus("Skipping model view " 
								<< modelHierarchy.ToString(mv));

							continue;
						}
					}
				}

				// Get the number of parses of the current model view
				const unsigned numModelParses = 
					modelHierarchy.ShapeParseCount(gmatch.modelViewIdx);

				// Get the slot where we saved the model view parse that best matches
				// the current query view parse (set to SPG_MATCH_NOT_SET initially)
				SPGMatch& bestMatch = qr.matches[gmatch.modelViewIdx];

				// Note: do not init the best model match value found here because
				// the "best" is across model parses. It is init when resizing
				// ranking of models per queries

				// For all parses of the model shapes
				for (gmatch.modelParseIdx = 0; gmatch.modelParseIdx < numModelParses; 
					++gmatch.modelParseIdx)
				{
					const ShapeParseGraph& G_m = modelHierarchy.GetShapeParse(
						gmatch.modelViewIdx, gmatch.modelParseIdx);

					gmatch.value = m_pShapeMatcher->Match(G_q, G_m);

					if (m_params.onlySumModelNodeMatches)
						gmatch.value = m_pShapeMatcher->GetGraphDistanceS2F();

					/*if (dbg_match_counter > 0 &&
						dbg_match_counter / 1000.0 == int(dbg_match_counter / 1000.0))
					{
						StreamStatus("(query id " << s_q << ", query parse " 
							      << gmatch.queryParseIdx << ") -> "
							      << "(model id " << gmatch.modelViewIdx 
								  << ", model parse " << gmatch.modelParseIdx << ") " 
						          << "[match id " << dbg_match_counter << "]");

						DBG_PRINT_ELAPSED_TIME(timer, "Matched 1000 SPGs")
						DBG_RESET_TIMER(timer)
					}*/

					dbg_match_counter++;

					// @todo mark comparison place
					if (bestMatch.value == SPGMatch::ValueNotSet() 
						|| gmatch.value < bestMatch.value)
					{
						bestMatch.SetBasicInfo(gmatch);

						m_pShapeMatcher->GetF2SNodeMap(bestMatch.nodeMap);
					}
				}
			}
		}

		// Sort the matches of the current query shape
		// @todo mark comparison place
		std::sort(qr.matches.begin(), qr.matches.end(), std::less<SPGMatch>());

		qr.matchingTime = matchingTimer.ElapsedTime();

		StreamStatus("Matching done in " << qr.matchingTime << " seconds.");
	}
}

/*void CollectModelInfo(std::string& info, const ModelHierarchy::ModelObject& mo, unsigned level)
{
	info += "\n";
	info.append(mo.classInfo.name);

	//if (mo.type == ModelHierarchy::
}*/

void ObjectRecognizer::GetParameterInfo(int i, DoubleArray* pMinVals, 
	DoubleArray* pMaxVals, DoubleArray* pSteps) const
{
	InitArrays(2, pMinVals, pMaxVals, pSteps, 0, 0, 1);

	if (!m_rankings.empty())
		pMaxVals->at(0) = m_rankings.size() - 1;

	const ModelHierarchy& modelHierarchy = m_pObjectLearner->GetModelHierarchy();

	if (modelHierarchy.ModelViewCount() > 0)
		pMaxVals->at(1) = modelHierarchy.ModelViewCount() - 1;
}

/*!
	Draws the output of the component. This function is called from
	a "drawing" thread, which is different from the "processing" thread
	that calls Run(). In order to protect the shared resources between the
	threads, a mutex is used.
*/
void ObjectRecognizer::Draw(const DisplayInfoIn& dii) const
{
	unsigned q_idx = (unsigned)dii.params.at(0);
	unsigned m_idx = (unsigned)dii.params.at(1);

	if (q_idx >= m_rankings.size() || m_idx >= m_rankings[q_idx].matches.size())
		return;

	const SPGMatch& match = m_rankings[q_idx].matches[m_idx];

	// See if it's the query view window
	if (dii.displayId == 0)
	{
		SPGPtr query_spg = 
			m_rankings[q_idx].queryParses[match.queryParseIdx];
		
		query_spg->Draw(match.nodeMap);
	}
	// See if it's the model view window
	else if (dii.displayId == 1 && match.value > -1) 
	{
		const ModelHierarchy& modelHierarchy = m_pObjectLearner->GetModelHierarchy();

		const ShapeParseGraph& model_spg = modelHierarchy.GetShapeParse(
			match.modelViewIdx, match.modelParseIdx);

		model_spg.Draw();

		// Superimpose the warped query if requested
		if (m_params.overlayWarpQuery)
		{
			SPGPtr query_spg = 
				m_rankings[q_idx].queryParses[match.queryParseIdx];

			PointTransform ptp;
			ShapePartComp spc;
			graph::node u, v;

			spc.SetGraphs(*query_spg, model_spg);

			forall_nodes(u, *query_spg)
			{
				v = match.nodeMap[u].mappedNode;

				// If u or v are nil, the ptp returned is empty
				spc.GetTransformationParams(u, v, &ptp);

				// Returns a NULL pointer if the transform isn't possible (eg, empty ptp)
				ShapeDescPtr ptrSD = query_spg->inf(u).ptrDescriptor->Transform(ptp);

				if (ptrSD != NULL)
				{
					LineList ll;
					PointArray ptsQ;
					double dist = 0;
					
					ptrSD->GetPoints(&ptsQ);

					for (unsigned i = 0 ; i < ptp.P.size(); i++)
					{
						ll.push_back(std::make_pair(ptsQ[i], ptp.P[i]));
						dist += ptsQ[i].dist(ptp.P[i]);
					}
				
					DBG_PRINT1(dist / ptp.P.size())

					DrawLines(ll, 1.0);

					ptrSD->Draw(NamedColor("Green"));
				}
			}
		}
	}
}

/*!	
	Returns the basic information specifying the output of this component.
	It must provide an image, its type, and a text message. All of this parameters 
	are optional. For example, if there is no output image, the image type
	can be set to VOID_IMAGE.
*/
void ObjectRecognizer::GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const
{
	dio.imageType = VOID_IMAGE;
	dio.syncDisplayViews = true;

	unsigned q_idx = (unsigned)dii.params.at(0);
	unsigned m_idx = (unsigned)dii.params.at(1);

	if (q_idx >= m_rankings.size() || m_idx >= m_rankings[q_idx].matches.size())
		return;

	const SPGMatch& match = m_rankings[q_idx].matches[m_idx];
	std::ostringstream oss;

	// See if it's the query view window
	if (dii.displayId == 0)
	{
		oss << "Parse id: " << match.queryParseIdx 
			<< ", match value: " << match.value << ".";
	}
	// See if it's the model view window
	else if (dii.displayId == 1 && match.value > -1) 
	{
		oss << "Model id: " << match.modelViewIdx << ", parse id: "
			<< match.modelParseIdx << ". Node correspondences:\n";

		//const ShapeParseGraph& m_spg = modelHierarchy.GetShapeParse(
		//	match.modelViewIdx, match.modelParseIdx);

		for (unsigned i = 0; i < match.nodeMap.size(); ++i)
		{
			auto na = match.nodeMap.at(i);

			ASSERT(na.mappedNode == nil || i == na.srcNodeIdx);

			oss << i << " -> ";

			if (na.mappedNode == nil)
				oss << "U";
			else
				oss << na.tgtNodeIdx << " = " << na.nodeAttDist;
			
			if (i + 1 < match.nodeMap.size())
				oss << ", ";
		}

		/*for (unsigned i = 0; i < match.m2qMap.size(); ++i)
		{
			oss << i << " -> ";
			
			if (match.m2qMap[i] == UINT_MAX)
				oss << "U";
			else
				oss << match.m2qMap[i] << " = " << match.weights[i];
			
			oss << ", ";
		}*/
	}

	dio.message = oss.str();
}

