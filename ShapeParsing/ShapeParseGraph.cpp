/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "ShapeParseGraph.h"
#include <ShapeRepresentation/BoundarySegments.h>
#include <ShapeRepresentation/ShapeDescriptorComp.h>
#include <VideoParserGUI/DrawingUtils.h>
#include <Tools/UserArguments.h>
#include <Tools/NamedColor.h>
#include <Tools/ColorMatrix.h>
#include <Tools/UserArguments.h>
#include <GraphTheory/GraphUtils.h>

using namespace vpl;

extern UserArguments g_userArgs;

ShapeParseGraph::Params ShapeParseGraph::s_params;

void ShapePart::Serialize(OutputStream& os) const
{
	::Serialize(os, boundarySegments);
	::Serialize(os, center);
	::Serialize(os, nilMatchCost);

	::Serialize(os, descriptorType);
		
	if (ptrDescriptor)
		ptrDescriptor->Serialize(os);

}

void ShapePart::Deserialize(InputStream& is) 
{
	::Deserialize(is, boundarySegments);
	::Deserialize(is, center);
	::Deserialize(is, nilMatchCost);

	::Deserialize(is, descriptorType);

	ptrDescriptor.reset(NewShapeDescriptor(descriptorType));

	if (ptrDescriptor)
		ptrDescriptor->Deserialize(is);

}

/*!
	[static] Reads the ShapeParseGraph parameters provided by the user
*/
void ShapeParseGraph::ReadParamsFromUserArguments()
{
	s_params.showDescriptorInfo = false;
	s_params.showShapeInfo = false;
	s_params.showLabels = false;
	s_params.nodeRadius = 3.0;

	/*g_userArgs.ReadArg("ShapeParseGraph", "nodeRadius", 
		"Radius used when drawing nodes", 3.0, &s_params.nodeRadius);

	g_userArgs.ReadBoolArg("ShapeParseGraph", "showLabels", 
		"Whether to show node labels or not", false, &s_params.showLabels);*/

	g_userArgs.ReadArg("ShapeParseGraph", "shapeDescriptor", 
		ShapeDescriptorTypes(),	"Class of shape descriptors used", 
		SHAPE_CONTEXT, (int*)&s_params.descriptorType);

	g_userArgs.ReadArg("ShapeParseGraph", "boundarySubsamplingValue", 
		"Absolute or percentage value to define the subsampling rate", 
		25u, &s_params.boundarySubsamplingValue);

	g_userArgs.ReadArg("ShapeParseGraph", "corner_alpha", 
		"Number of points to sample on the boundary for shape context, before adding corners", 
		25u, &s_params.corner_alpha);

	g_userArgs.ReadArg("ShapeParseGraph", "boundarySubsamplingType", 
		Tokenize("none absolute percentage corner_count"), "Whether the 'value' given is absolute"
		" or a percentage", 1, (int*)&s_params.boundarySubsamplingType);

	g_userArgs.ReadArg("ShapeParseGraph", "boundarySubsamplingScope", 
		Tokenize("whole part"), "Whether the 'value' and 'type' refer"
		" to the boundary of the whole or of each part", 
		1, (int*)&s_params.boundarySubsamplingScope);

	g_userArgs.ReadBoolArg("ShapeParseGraph", "sampleShapeCutPoints", 
		"Sample the points of the shape cuts too?", 
		true, &s_params.sampleShapeCutPoints);

	g_userArgs.ReadArg("ShapeParseGraph", "nodeNilMatchScheme", 
		Tokenize("value part_comp"), "Scheme used to interpret the value of nodeNilMatchValue. "
		" 'value' means that nodeNilMatchValue should be used as is."
		" 'part_comp' means that ShapePartComp class must be consulted.", 
		1, (int*)&s_params.nodeNilMatchScheme);

	g_userArgs.ReadArg("ShapeParseGraph", "nodeNilMatchValue", 
		"Value used when a node is left unmatched. This is interpreted "
		" according to nodeNilMatchScheme", 
		0.0, &s_params.nodeNilMatchValue);

	// Also read the "static" parameters for the selected shape descriptor
	ReadShapeDescriptorParams(s_params.descriptorType);

	// State that we want to serialize the parameters
	g_userArgs.AddSerializableFieldKey("ShapeParseGraph");
}

//! [static] Update drawing parameters
void ShapeParseGraph::GetSwitchCommands(std::list<UserCommandInfo>& cmds)
{
	cmds.push_back(UserCommandInfo("labels", "show labels", 
		'l', &s_params.showLabels));

	cmds.push_back(UserCommandInfo("desc", "show shape descriptor info", 
		'd', &s_params.showDescriptorInfo));

	cmds.push_back(UserCommandInfo("info", "show shape info", 
		'i', &s_params.showShapeInfo));
}

unsigned int ShapeParseGraph::getNumberOfBoundaryPoints() const
{
	return m_ptrShapeInfo->BoundarySize();
}

/*!
	Creates a shape descriptor for each part using N evenly-spaced samples 
	from the part's boundary.
*/
void ShapeParseGraph::ComputeShapeDescriptors()
{
	std::vector<ConnectedBoundarySegments> vcbs(number_of_nodes());
	const PointArray& shapeBndry = m_ptrShapeInfo->GetBoundaryPoints();
	IntList::const_iterator it0, it1;
	sg::BoundaryInterval bi;
	node v;


	// Set the size of the shape boundary only once
	bi.SetBoundarySize(shapeBndry.size());

	// Initialize the sum of all shape parts' lengths
	double totalLength = 0;

	// Form the boundary of each shape part as a list of connected
	// boundary segments of appropriate classes to represent true
	// boundary intervals and boundary cuts.
	forall_nodes(v, *this)
	{
		const IntList& partBndry = attribute(v).boundarySegments;
		ConnectedBoundarySegments& bndrySegs = vcbs[index(v)];

		// Add the endpoint of each boundary segment
		for (it1 = partBndry.begin(), it0 = it1++; it1 != partBndry.end(); ++it0, ++it1)
		{
			// Set the first and last point of the boundary interval
			bi.SetLimits(*it0, *it1);

			// Add the points within the boundary interval
			bndrySegs.AddSegment(new DiscreteBoundarySegment(&shapeBndry, bi));

			// Move to the gap between bndry intervals (ie, we are NOT at the next interval yet!)
			it0 = it1++;

			// See if we reached the end of the list of segments
			if (it1 == partBndry.end())
			{
				break;
			}
			else if (s_params.sampleShapeCutPoints)
			{
				bndrySegs.AddSegment(new DiscreteBoundarySegment(
					shapeBndry[*it0], shapeBndry[*it1]));
			}
		}

		// Close the curve and accumulate its length
		bndrySegs.CloseCurve();
		totalLength += bndrySegs.Length();
	}

	PointArray pts;
	DoubleArray tangents; // array of angle cosines of the tangents at each sampled point
	unsigned numSamples;

	// Get N evenly-spaced samples from the part's boundary
	forall_nodes(v, *this)
	{
		ShapePart& sp = attribute(v);
		const ConnectedBoundarySegments& bndrySegs = vcbs[index(v)];

		// See if we have to subsample the bndry points
		if (s_params.boundarySubsamplingType == NO_SAMPLING)
		{
			// Subsample the points using the identity delta
			bndrySegs.CopyPoints(&pts);
			bndrySegs.CopyTangents(&tangents);
		}
		else
		{
			// The sampling number can be PERCENTAGE or ABSOLUTE. If absolute, 
			// the subsampling value IS the number of points. If
			// relative, it is a percentage of points.

			// Reset the sampling value
			numSamples = s_params.boundarySubsamplingValue;

			if (s_params.boundarySubsamplingScope == WHOLE_SHAPE)
			{

				// If the value is relative, convert it to ABSOLUTE type
				if (s_params.boundarySubsamplingType == PERCENTAGE_SAMPLING)
					numSamples = unsigned(numSamples * bndrySegs.Length() / 100);

				// Handle ABSOLUTE type by computing the number of samplings points
				// that correspond to the current part
				numSamples = (unsigned) ROUND_NUM(
					numSamples * bndrySegs.Length() / totalLength);
				
			}
			else if (s_params.boundarySubsamplingType == CORNER_COUNT_SAMPLING)
			{
				// get the number of corners for this shape part.
				IntList &part_boundary_indices = sp.boundarySegments;
				const BoundaryCornerList &corners = m_ptrShapeInfo->m_corners.ConcaveCorners();
				unsigned corner_count = 0;

				for (auto it = corners.begin(); it != corners.end(); ++it)
				{
					for (auto pt = part_boundary_indices.begin(); pt != part_boundary_indices.end(); ++pt)
					{
						if ((*it).interval.Includes((*pt)))
						{
							corner_count++;
							break;
						}
					}
				}
				numSamples = 4 * corner_count + s_params.corner_alpha;
				//sp.nilMatchCost
				//std::cout << "number of samples for this part: " << numSamples << std::endl;
			}

			// It's SHAPE_PART scope. The case of ABSOLUTE type needs no work
			// so we only handle the case of PERCENTAGE type
			else if (s_params.boundarySubsamplingType == PERCENTAGE_SAMPLING)
			{
				numSamples = unsigned(numSamples * bndrySegs.Length() / 100);
			}

			// Subsample the points
			bndrySegs.SubsampleExact(numSamples, &pts, &tangents);
		}

		DBG_STREAM_IF("Not the num samples requested: " 
			<< pts.size(), pts.size() != numSamples)

		// Create a new and *empty* shape descriptor object
		sp.descriptorType = s_params.descriptorType;
		sp.ptrDescriptor.reset(NewShapeDescriptor(sp.descriptorType));

		// Create a shape descriptor using the sampled points
		if (sp.ptrDescriptor)
			sp.ptrDescriptor->Create(pts, tangents, (unsigned int)bndrySegs.Length());

		// Set the cost of leaving the part unmacthed. ie, the 
		// cost of matching the part against a "nil" part.
		if (s_params.nodeNilMatchScheme == NIl_MATCH_VALUE)
			sp.nilMatchCost = s_params.nodeNilMatchValue;
		else if (s_params.nodeNilMatchScheme == NIL_MATCH_PART_COMP)
			sp.nilMatchCost = sp.ptrDescriptor->NilMatchValue();
		else
			ASSERT(false);

		// Compute center of mass of the part to obtain display 
		// coordinates for teh node representing it
		sp.center.Set(0,0);

		if (!pts.empty())
		{
			for (auto it = pts.begin(); it != pts.end(); ++it)
				sp.center += *it;

			sp.center /= pts.size();
		}
	}
}

/*!
	Draws the given SPG nodes. The array may contain 'nil' elements, which are ignored
*/
void ShapeParseGraph::Draw(const NodeMatchMap& nodeMap, unsigned partId) const
{
	IntList::const_iterator it0, it1;
	Point avgPt, p0, p1;
	PointList pts;
	PointArray nodeCenters(number_of_nodes());
	double lineLen, numPts;
	int bndryPtIdx;

	const PointArray& bndry = m_ptrShapeInfo->GetBoundaryPoints();
	unsigned partCount = 0;

	node v;
	
	forall_nodes(v, *this)
	{
		const IntList& partBndry = inf(v).boundarySegments;

		pts.clear();

		// Initialize the average point with the midpoint of the
		// line segment spanned by the first and last boundary points
		// and use the length of the line to determine the weight of
		// the point in the average
		p0 = bndry[partBndry.front()];
		p1 = bndry[partBndry.back()];

		lineLen = p0.dist(p1);
		avgPt = ((p0 + p1) / 2.0) * lineLen;
		numPts = lineLen;

		// Add the endpoint of each boundary segment
		for (it1 = partBndry.begin(), it0 = it1++; it1 != partBndry.end(); ++it0, ++it1)
		{
			for (bndryPtIdx = *it0; ; ++bndryPtIdx)
			{
				// Wrap around the endpoints of the circular boundary array
				if (bndryPtIdx == bndry.size()) 
					bndryPtIdx = 0;

				pts.push_back(bndry[bndryPtIdx]);
				avgPt += pts.back();
				++numPts;

				// We are done as soon as we added the last point
				if (bndryPtIdx == *it1)
					break;
			}

			// Move to the gap (we are NOT at the next segment yet!)
			it0 = it1++;

			// See if we reached the end of the list of segments
			if (it1 == partBndry.end())
			{
				break;
			}
			else
			{
				// Add the midpoint of the line weighted by the line length
				p0 = bndry[*it0];
				p1 = bndry[*it1];

				lineLen = p0.dist(p1);
				avgPt += ((p0 + p1) / 2.0) * lineLen;
				numPts += lineLen;
			}
		}

		// Note that the ids of parts start at 1, but partCount starts at 0
		if (partId == 0 || partCount == partId - 1)
		{
			if (nodeMap.empty())
			{
				SetDrawingColor(ColorMatrix(partCount, ColorMatrix::PASTELS));
			}
			else if (partCount < nodeMap.size() && 
				nodeMap.at(partCount).mappedNode != nil)
			{
				SetDrawingColor(ColorMatrix(
					nodeMap.at(partCount).tgtNodeIdx, ColorMatrix::PASTELS));
			}
			else
			{
				SetDrawingColor(RGBColor(255, 255, 255));
			}

			DrawFilledPolygon(pts);

			SetDrawingColor(RGBColor(0, 0, 0));
			DrawPolygon(pts);
		}

		nodeCenters[index(v)] = avgPt / numPts;

		partCount++;
	}

	SetDrawingColor(RGBColor(0, 0, 0));
	void* font = GetFont(TIMES_ROMAN_10_FONT);
	Point sz;

	graph::edge e;

	forall_edges(e, *this)
	{
		DrawLine(nodeCenters[index(source(e))], 
			     nodeCenters[index(target(e))]);
	}

	// Draw each part descriptor information (if requested)
	if (s_params.showDescriptorInfo)
	{
		forall_nodes(v, *this)
		{
			if (partId == 0 || index(v) == partId - 1)
				inf(v).ptrDescriptor->Draw(NamedColor("Red"));
		}
	}

	RGBColor nodeColor(255, 255, 255);
	double radius;

	double maxCost = 0;

	forall_nodes(v, *this)
	{
		if (inf(v).nilMatchCost > maxCost)
			maxCost = inf(v).nilMatchCost;
	}

	forall_nodes(v, *this)
	{
		//p0 = nodeCenters[index(v)];
		p0 = inf(v).center;

		if (s_params.showLabels)
		{
			STLInitVector<unsigned> lbl(index(v), 
				inf(v).ptrDescriptor->NumPoints(), 
				(int)ceil(inf(v).nilMatchCost));

			//num = index(v);
			//num = (int)ceil(inf(v).cost);
			//num = inf(v).ptrDescriptor->NumPoints();

			// Draw number first to get size of label, then
			// draw a filled disk of appropriate size
			SetDrawingColor(nodeColor);

			//sz = DrawNumberCentered(num, p0, font);
			//radius = MAX(sz.x, sz.y) + 1;

			radius = s_params.nodeRadius * 
				(inf(v).nilMatchCost / maxCost);

			// Draw node filling
			DrawDisk(p0, radius);

			// Draw the number in front of the disk and a border
			// circle to frame the node
			SetDefaultDrawingColor();
			//sz = DrawNumberCentered(num, p0, font);
			
			DrawNumbersInBox(lbl, p0, p0 + Point(100, 100), font, 
				LEFT_ALIGNMENT, TOP_ALIGNMENT, ",");

			DrawCircle(p0, radius);
		}
		else
		{			
			SetDefaultDrawingColor();
			DrawDisk(p0, s_params.nodeRadius);
		}
	}

	if (s_params.showShapeInfo)
		m_ptrShapeInfo->Draw();
}

std::string ShapeParseGraph::GetOutputText() const
{
	std::ostringstream oss;

	oss << "There are " << number_of_nodes() << " nodes and "
		<< number_of_edges() << " edges."; 

	return oss.str();
}

/*!
	[static]
*/
void ShapeParseGraph::ComputeNodeSimilarityMatrix(const ShapeParseGraph& g1, 
		const ShapeParseGraph& g2, DoubleMatrix& simWeights)
{
	/*node u, v;
	ShapeDescriptorComp* pMatcher; // = new int;

	simWeights.set_size(g1.number_of_nodes(), g2.number_of_nodes());

	forall_nodes(u, g1)
	{
		forall_nodes(v, g2)
		{
			simWeights[index(u)][index(v)] = pMatcher->Match(
				GetShapePart(u).ptrDescriptor, 
				GetShapePart(v).ptrDescriptor);
		}
	}*/
}

