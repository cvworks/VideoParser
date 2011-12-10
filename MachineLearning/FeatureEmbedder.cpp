#include "FeatureEmbedder.h"
#include <ShapeMatching/ShapeMatcher.h>
#include <MachineLearning/ObjectLearner.h>

using namespace vpl;

void FeatureEmbedder::Initialize(graph::node v)
{
	VisSysComponent::Initialize(v);

	m_pObjectLearner = FindParentComponent(ObjectLearner);
	m_pShapeMatcher = FindParentComponent(ShapeMatcher);
}

