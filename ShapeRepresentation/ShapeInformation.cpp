/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "ShapeInformation.h"
#include <Tools/UserArguments.h>
#include <VideoParserGUI/DrawingUtils.h>

using namespace vpl;

extern UserArguments g_userArgs;

bool ShapeInformation::s_drawSpokes = false;
bool ShapeInformation::s_drawMedialAxis = true;
bool ShapeInformation::s_drawCorners = true;
bool ShapeInformation::s_drawBoundaryMap = false;
bool ShapeInformation::s_drawBoundaryCuts = true;
bool ShapeInformation::s_drawLabels = false;

void ShapeInformation::ReadParamsFromUserArguments()
{
	CornerDetector::ReadParamsFromUserArguments();
}

//! [static] Update drawing parameters
void ShapeInformation::GetSwitchCommands(std::list<UserCommandInfo>& cmds)
{
	cmds.push_back(UserCommandInfo("spokes", "draw spokes", 's', &s_drawSpokes));
	cmds.push_back(UserCommandInfo("axis", "draw medial axis", 'm', &s_drawMedialAxis));
	cmds.push_back(UserCommandInfo("corners", "draw corners", '.', &s_drawCorners));
	cmds.push_back(UserCommandInfo("bndry map", "draw boundary map", 'b', &s_drawBoundaryMap));
	cmds.push_back(UserCommandInfo("cuts", "draw boundary cuts", 'k', &s_drawBoundaryCuts));
	cmds.push_back(UserCommandInfo("labels", "draw labels", 'l', &s_drawLabels));
}

/*!
	Draw the information associated with the shape representation
*/
void ShapeInformation::Draw() const
{
	if (!m_initialized)
		return;

	PointList pts;
	unsigned int numPts = m_pBoundary->size();
	Point bndryPt;
	LineList bndrySymLines;
	sg::SpokesMap::const_iterator branchIt;
	sg::SpokeInfoList::const_iterator spokeIt;

	// Draw corners
	if (s_drawCorners)
	{
		const CornerLabelArray& cla = m_corners.LabeledBoundaryPoints();

		//SetDrawingColor(RGBColor(255, 0, 0));

		for (unsigned int i = 0; i < numPts; i++)
		{
			if (cla[i].isCorner)// && m_corners.IsConcaveCorner(i)) // && m_corners.IsConcaveCorner(i)
			{
				m_pBoundary->getPoint(i, &bndryPt);

				if (cla[i].isMinimum)
					SetDrawingColor(RGBColor(255, 0, 0));
				else
					SetDrawingColor(RGBColor(0, 255, 0));

				if (s_drawLabels && cla[i].isMinimum)
					DrawNumber(i, bndryPt + Point(0, 2));

				DrawCircle(bndryPt, 1);
			}

			/*if (m_corners.IsCorner(i) && m_corners.IsConcaveCorner(i))
			{
				m_pBoundary->getPoint(i, &bndryPt);
				DrawCircle(bndryPt, 2); //m_corners.GetRadius(i)
			}*/

		}

		SetDefaultDrawingColor();
	}

	// Draw shape contour

	// first, copy points into an stl vector
	for (unsigned int i = 0; i < numPts; i++)
	{
		m_pBoundary->getPoint(i, &bndryPt);
		pts.push_back(bndryPt);

		if (s_drawBoundaryMap)
		{
			const sg::SpokesMap& sm = m_pBoundary->getSpokesMap(i);

			for (branchIt = sm.begin(); branchIt != sm.end(); ++branchIt)
			{
				const sg::SkelBranch* pBranch = branchIt->first;

				const sg::FluxPointArray& fpl = pBranch->getFluxPoints();
				const sg::BoundaryInfoArray& bil = pBranch->getBoundaryInfoArray();

				const sg::SpokeInfoList& sil = branchIt->second;

				for (spokeIt = sil.begin(); spokeIt != sil.end(); ++spokeIt)
				{
					bndrySymLines.push_back(std::make_pair(bndryPt, 
						bil[spokeIt->skelPtIndex][sg::FlipSide(spokeIt->side)].pt));
				}
			}
		}
	}

	DrawPolygon(pts);

	if (s_drawBoundaryMap)
	{
		SetDrawingColor(RGBColor(0, 255, 0));
		DrawLines(bndrySymLines);
		SetDefaultDrawingColor();
	}

	// Draw skeleton info
	const sg::DDSGraph* sk = m_skelGraph.GetDDSGraph();
	const sg::SkelBranch* pBranch;

	ASSERT(sk);

	// Draw spokes
	if (s_drawSpokes)
	{
		LineList lines1, lines2;
		RGBColor col1(255, 0, 0), col2(0, 0, 255);
		
		//leda::color c(51, 204, 102); // green (or pink: 243, 169, 161)

		forall_const_branches(pBranch, sk->getEdges())
		{
			const sg::FluxPointArray& fpl = pBranch->getFluxPoints();
			const sg::BoundaryInfoArray& bil = pBranch->getBoundaryInfoArray();

			for (unsigned int i = 0; i < bil.size(); i++)
			{
				// If m_nShowRays == 2, show external rays only
				//if (m_nShowRays == 2 && !(i == 0 || i + 1 == bil.size()))
				//	continue;
				lines1.push_back(std::make_pair(fpl[i].p, bil[i].first.pt));
				lines2.push_back(std::make_pair(fpl[i].p, bil[i].second.pt));

				SetDrawingColor(col1, 0.5);
				DrawLines(lines1);

				SetDrawingColor(col2, 0.5);
				DrawLines(lines2);
			}
		}

		SetDefaultDrawingColor();
	}

	// Draw skeletal points
	if (s_drawMedialAxis)
	{
		forall_const_branches(pBranch, sk->getEdges())
			DrawLineStrip(pBranch->getFluxPoints());
	}

	// Draw boundary cuts
	if (s_drawBoundaryCuts)
	{
		const BoundaryCutSet& bcs = m_bndryCuts.GetCuts();
		BoundaryCutSet::const_iterator it;
		LineList cutLines;
		Point p0, p1, c;

		SetDrawingColor(RGBColor(255, 128, 128));

		for (it = bcs.begin(); it != bcs.end(); ++it)
		{
			m_pBoundary->getPoint(it->From(), &p0);
			m_pBoundary->getPoint(it->To(), &p1);

			cutLines.push_back(std::make_pair(p0, p1));

			c = it->MedialPoint();
			DrawCircle(c, 1);
		}

		//DBG_MSG2("Number of cuts", cutLines.size())

		DrawLines(cutLines);

		SetDefaultDrawingColor();
	}
}

std::string ShapeInformation::GetOutputText() const
{
	const BoundaryCutSet& bcs = m_bndryCuts.GetCuts();
	std::ostringstream oss;

	oss << "There are " << bcs.size() << " boundary cuts"; 

	return oss.str();
}
