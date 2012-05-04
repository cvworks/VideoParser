/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "ShapeMatcher.h"
#include <MachineLearning/ObjectLearner.h>
#include <GraphTheory/ExactTreeMatcher.h>
#include <ShapeParsing/ShapeParser.h>
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
void ShapeMatcher::ReadParamsFromUserArguments()
{
	VisSysComponent::ReadParamsFromUserArguments();

	g_userArgs.ReadArg(Name(), "matchingAlgoritm", 
		"Algorithm used to match graph-based representations of objects", 
		0, &m_params.matchingAlgorithm);
}

/*!
	Clears the current graph matcher object.
*/
void ShapeMatcher::Clear()
{
	delete m_pGraphMatcher;
	m_pGraphMatcher = NULL;
}

void ShapeMatcher::Initialize(graph::node v)
{
	VisSysComponent::Initialize(v);

	m_pObjectLearner = FindParentComponent(ObjectLearner);
	m_pShapeParser = FindParentComponent(ShapeParser);

	ASSERT(!m_pGraphMatcher);
	m_pGraphMatcher = new ExactTreeMatcher;
}

void ShapeMatcher::Run()
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

	// Only recognize object if we are performing the recognition task
	//if (TaskName() != "Recognition")
	//	return;

	ShowStatus("Matching shapes...");

	
}

/*!
	Computes the full model shape distance matrix.

	@param indexMap optional mapping of indices used to randomize
	the order of the shapes in the database

	@return the index map used to create the matrix. It might be a trivial map,
	ie, [0, 1, ..., N] or a randomized map. Then, the matrix cell (i,j) stores
	the distance between database objects (map[i], map[j]).
*/
std::vector<unsigned> ShapeMatcher::ComputeDatabaseDistances(Matrix& m, 
	bool randomizeData, unsigned seed) const
{
	const ModelHierarchy& mh = m_pObjectLearner->GetModelHierarchy();
	const unsigned N = mh.NumShapes();

	// Create a trivial or a randomized index map
	std::vector<unsigned> indexMap = TrivialIndexMap(N);

	if (randomizeData)
		RandomArrayPermutation(indexMap, seed);

	// Create an NxN distance matrix
	m.set_size(N, N);

	// Fill in the diagonal
	for (unsigned i = 0; i < N; i++)
		m(i, i) = 0;

	// Fill the upper triangle and copy it to lower triangle
	for (unsigned i = 0; i < N; i++)
	{
		for (unsigned j = i + 1; j < N; j++)
		{
			m(i, j) = Match(mh[indexMap[i]].spg, 
				            mh[indexMap[j]].spg);

			m(j, i) = m(i, j);
		}
	}

	return indexMap;
}

double ShapeMatcher::Match(const ShapeParseGraph& g1, 
	const ShapeParseGraph& g2) const
{
	// Let the node similarity measurer know the graphs
	// that contain the nodes to compare
	const_cast<ShapePartComp&>(m_spsm).SetGraphs(g1, g2);

	return m_pGraphMatcher->Match(g1, g2, m_spsm);
}

/*!
	Gets the distance from the nodes in the second graph 
	to the modes in the first graph used in teh last called to Match().
*/
double ShapeMatcher::GetGraphDistanceS2F() const
{
	return m_pGraphMatcher->GetGraphDistanceS2F();
}

/*!
	Gets the correspondence map from the nodes in the first graph 
	to the modes in the second graph used in the last called to Match().
*/
void ShapeMatcher::GetF2SNodeMap(NodeMatchMap& nodeMap) const
{
	m_pGraphMatcher->GetF2SNodeMap(nodeMap);
}

/*!
	Draws the output of the component. This function is called from
	a "drawing" thread, which is different from the "processing" thread
	that calls Run(). In order to protect the shared resources between the
	threads, a mutex is used.
*/
void ShapeMatcher::Draw(const DisplayInfoIn& dii) const
{
	
}

/*!	
	Returns the basic information specifying the output of this component.
	It must provide an image, its type, and a text message. All of this parameters 
	are optional. For example, if there is no output image, the image type
	can be set to VOID_IMAGE.
*/
void ShapeMatcher::GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const
{
	dio.imageType = VOID_IMAGE;
	dio.syncDisplayViews = false;
}

