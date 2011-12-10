DoubleArray indices;

	if (s_params.boundarySubsamplingScope == WHOLE_SHAPE)
	{
		bndrySegs.SubsampleExact(s_params.boundarySubsamplingValue, &pts, &tangents, &indices);
	}


		void SubsampleExact(unsigned numSamples, PointArray* pSamples, 
		DoubleArray* pTangents, DoubleArray* pIndices) const
	{
		ASSERT(pSamples && pTangents);

		pSamples->clear();
		pTangents->clear();
		pIndices->clear();

		if (m_pts.empty())
			return;
		
		double loopDist = m_pts.front().dist(m_pts.back());

		ASSERT(loopDist < 2);

		const double delta = (Length() + loopDist) / numSamples;

		pSamples->reserve(numSamples + 1);
		pTangents->reserve(numSamples + 1);
		pIndices->reserve(numSamples + 1);

		double d, t;
		double cumDist = 0;

		// Start by adding the first point
		pSamples->push_back(m_pts.front());
		pTangents->push_back(Tangent(0));
		pIndices->push_back(0);

		// The previous point might not always be one in m_pts. Initially,
		// it is the one that we just added, ie, the first.
		Point prevPt = m_pts.front();

		// Not that dist between first and last is included, so
		// the true last point is the first point, which is not reached twice
		for (unsigned i = 1; i < m_pts.size() && pSamples->size() < numSamples;)
		{
			const Point& pt = m_pts[i];

			d = pt.dist(prevPt);

			cumDist += d;

			if (cumDist == delta)
			{
				pSamples->push_back(pt);
				pTangents->push_back(Tangent(i));
				pIndices->push_back(i);

				cumDist = 0;
				prevPt = pt;
				i++;
			}
			else if (cumDist > delta) // inc total distance by d * t
			{
				t = (delta - cumDist + d) / d;
				ASSERT(t >= 0 && t <= 1);

				Point pt_prime = LineSegmentPoint(prevPt, pt, t);
				
				pSamples->push_back(pt_prime);

				// Compute the weighted average tangent using
				// the last added tangent and the next point's one
				double tan_prime = (prevPt == pSamples->back()) 
					? pTangents->back() : Tangent(i - 1);

				pTangents->push_back((1 - t) * tan_prime + t * Tangent(i));
				
				double t_prime = (pt_prime.x - m_pts[i - 1].x) / (pt.x - m_pts[i - 1].x);
				pIndices->push_back((1 - t_prime) * (i - 1) + t_prime * i);
				
				cumDist = 0;
				prevPt = pt_prime;
	
				// do not increment 'i' yet
			}
			else
			{
				prevPt = pt;
				i++;
			}
		}

		if (numSamples != pSamples->size())
		{
			DBG_PRINT4(numSamples, pSamples->size(), Length(), delta)
		}
	}
}
/*m_nFrame++;
	
	if (!IsLastFrame())
	{
		std::ostringstream fileName;
		
		fileName << m_strPrefix << m_nFrame << "." << m_strFormat;
		m_curImg = vil_load(fileName.str().c_str());

		if (m_curImg->size() == 0)
			ShowError1("Cannot read image frame:", fileName.str());
	}*/

	/*IplImage* image;                      // This is image pointer
 
		image = cvLoadImage(m_pathIt->c_str(), CV_LOAD_IMAGE_COLOR);

		RGBImg img;

		IplImageToVXLImage(image, img);

		m_curImg = ConvertToBaseImgPtr(img);*/

// We let the option-argument pair appear in any order, so
	// first, find the indices of the option and its argument
	// in the command line
	for (int i = 1; i < argc; i++)
	{
		for (int j = 0; j < numOptions; j++)
		{
			if (!)
			{
				// Only one opt argument should be given
				if (optIdx >= 0)
				{
					ShowError("Multiple option parameters given");
					ShowUsage("videoparser [-opt] parameter_file.txt");
					return 1;
				}

				optIdx = j;

				// The cmd-line index of the argument
				// is the one that is NOT 'i'
				paramFile = (i == 1) ? argv[2] : argv[1];

				break;
			}
		}
	}

else // or (c) we are missing the param file somehow
		{
			ShowError("Missing parameter file");
			ShowUsage("videoparser [-opt] parameter_file.txt");
			return 1;
		}

	int main(int argc, char **argv) 
{
	const int numOptions = 3;
	char* options[numOptions] = {"-nogui", "-e", "-p"};
	int optIdx = -1, argIdx = -1;
	std::string paramFile;

	// We let the option-argument pair appear in any order, so
	// first, find the indices of the option and its argument
	// in the command line
	for (int i = 1; i < argc; i++)
	{
		for (int j = 0; j < numOptions; j++)
		{
			if (!_stricmp(argv[i], options[j]))
			{
				// Only one opt argument should be given
				if (optIdx >= 0)
				{
					ShowError("Multiple option parameters given");
					ShowUsage("videoparser [-opt] parameter_file.txt");
					return 1;
				}

				optIdx = j;

				// The cmd-line index of the argument
				// is the one that is NOT 'i'
				paramFile = (i == 1) ? argv[2] : argv[1];

				argIdx = (i == 1) ? 2 : 1;
				break;
			}
		}
	}

	// If no option is provided, then
	if (argIdx < 0)
	{
		// there is only one argument
		if (argc == 2)
		{
			argIdx = 1;
		}
		else // or we are missing the param file
		{
			ShowError("Missing parameter file");
			ShowUsage("videoparser [-opt] parameter_file.txt");
			return 1;
		}
	}

	if (!g_userArgs.ReadParameters(argv[argIdx]))
	{
		ShowOpenFileError(argv[argIdx]);
		ShowUsage("videoparser [-opt] [parameter_file]");
		return 1;
	}

	// Get the verbose mode parameter from the file
	g_userArgs.ReadBoolArg("Main", "verboseMode", 
		"Whether to show all status messages or not", false, &g_verboseMode);

	// If optIdx >= 0, we are working without a GUI
	if (optIdx >= 0)
	{
		if (optIdx == 0 || optIdx == 1)
			RunExperiments(argv[argIdx]);
		else if (optIdx == 2)
			RunProcesses(argv[0]);

		return 0;
	}

	
	void SubsampleExact(unsigned numSamples, PointArray* pSamples, 
		DoubleArray* pTangents) const
	{
		ASSERT(pSamples && pTangents);

		pSamples->clear();
		pTangents->clear();

		if (m_pts.empty())
			return;
		
		double loopDist = m_pts.front().dist(m_pts.back());

		ASSERT(loopDist < 2);

		const double delta = (Length() + loopDist) / numSamples;

		pSamples->reserve(numSamples + 1);
		pTangents->reserve(numSamples + 1);

		double d, t;
		double cumDist = 0;

		Point prevPt = m_pts.front();

		pSamples->push_back(prevPt);
		pTangents->push_back(Comp);
		
		auto it1 = m_pts.begin();

		// Not that dist between first an dlast is included, so
		// the true last point is the first point, which is not reached twice
		for (++it1; it1 != m_pts.end() && pSamples->size() < numSamples;)
		{
			d = prevPt.dist(*it1);
			cumDist += d;

			if (cumDist == delta)
			{
				pSamples->push_back(*it1);
				cumDist = 0;
				prevPt = *it1;
				++it1;
				//dbgTotDist += d;
			}
			else if (cumDist > delta)
			{
				t = (delta - cumDist + d) / d;
				ASSERT(t >= 0 && t <= 1);

				prevPt = LineSegmentPoint(prevPt, *it1, t);
				pSamples->push_back(prevPt);
				cumDist = 0;
				//dbgTotDist += d * t;

				// do not move the next point yet!!!
			}
			else
			{
				prevPt = *it1;
				++it1;
				//dbgTotDist += d;
			}
		}

		if (numSamples != pSamples->size())
		{
			DBG_PRINT4(numSamples, pSamples->size(), Length(), delta)
		}
	}
	
	if (m_enforceSize)
	{
		unsigned wr = (unsigned)ceil(double(img.ni()) / double(m_frameWidth));
		unsigned hr = (unsigned)ceil(double(img.nj()) / double(m_frameHeight));
	
		unsigned factor = MAX(wr, hr);

		if (factor > 1)
		{
	
			img = vil_decimate(img, factor, factor);
		}
	}


		if (m_enforceSize)
	{
		unsigned wr = (unsigned)ceil(double(img.ni()) / double(m_frameWidth));
		unsigned hr = (unsigned)ceil(double(img.nj()) / double(m_frameHeight));
	
		unsigned factor = MAX(wr, hr);

		if (factor > 1)
		{
			DBG_LINE
			img = vil_decimate(img, factor, factor);
		}
	}

/*!
	Metadata used to recover serialized user arguments.
*/
struct UserArgumentsMetadata : public PersistentHierarchicalMetadata
{
	// nothing more to add
};

DECLARE_BASIC_SERIALIZATION(vpl::UserArgumentsMetadata)
	
	ShapeParseGraph::Params params;

		if (m_modelHierarchy.LoadParsingParams(
			m_pObjectLearner->GetModelDatabse(), &params))
		{
			ShowStatus("Updating parsing param from database");

			ShapeParseGraph::SetParams(params);
		}
	
	void SerializeUserArguments(OutputStream& os);
	void DeserializeUserArguments(InputStream& is);

/*!
	Serialize selected user arguments.
*/
void ShapeParser::SerializeUserArguments(OutputStream& os)
{
	
}

/*!
	Deserialize selected user arguments.
*/
void ShapeParser::DeserializeUserArguments(InputStream& is)
{
	g_userArgs.Deserialize(is);

	// Read the parameters again!
	ReadParamsFromUserArguments();
}

auto it = m_serializableFields.find(fieldKey);

		if (it != m_serializableFields.end())
			m_serializableFields.insert(fieldKey);

		void AddSegment(BoundarySegment* pSegment)
	{
		if (!m_segments.empty())
			m_length += m_segments.back()->LastPoint().dist(pSegment->FirstPoint());

		m_length += pSegment->Length();

		m_segments.push_back(pSegment);		

		pSegment->AppendTo(&m_pts);
	}



		
		PointArray pts;
		BoundarySegment* pCurrSeg;
		BoundarySegmentList::const_iterator segIt;
		
		// Length() might be less than we need (eg, diag of two points is sqrt(2))
		pts.reserve(Length() * 1.5);

		std_forall(segIt, m_segments)
		{
			pCurrSeg = *segIt;

			pCurrSeg->AppendTo(&pts);
		}



		{
		double dbg_init_dist = m_pts.front().dist(m_pts.back());
			double totDist = m_pts.front().dist(m_pts.back());

			{
				auto it1 = m_pts.begin();
				auto it0 = it1++;

				for (; it1 != m_pts.end(); ++it0, ++it1)
					totDist += it0->dist(*it1);
			}
		}
/////////////////////////////////////////////////////////////////////////////
double closingDist = 0;

		if (!m_segments.empty())
		{
			const Point& p0 = m_segments.front()->FirstPoint();
			const Point& p1 = m_segments.back()->LastPoint();

			if (p0 != p1)
				closingDist = p0.dist(p1);
		}

		return m_length + closingDist;
/////////////////////////////////////////////////////////////////////////////
		/*for (; i < lastIdxs[k]; ++i)
		{
			DrawString2D(sc.toCharPtr(nums[i]), (int)p0.x, (int)p0.y, font);
			p0.x += szs[i].x;

			if (i + 1 < lastIdxs[k])
			{
				DrawString2D(", ", (int)p0.x, (int)p0.y, font);
				p0.x += szSep.x;
			}
		}

		p0.x = tl.x + left;*/
/////////////////////////////////////////////////////////////////////////////
		DBG_PRINT1(inf(e).sharedVars.size());
		auto C = inf(e).sharedVars;

		for (auto _it = C.begin(); _it != C.end(); ++_it)
		{
			DBG_LINE
			DBG_PRINT1(*_it);
		}
		DBG_LINE
/////////////////////////////////////////////////////////////////////////////
std::list<unsigned>& shared = adj(inf(vi.parent).cliqueId, i);
//attribute(e).sharedVars.splice(shared.end(), shared);
/////////////////////////////////////////////////////////////////////////////
void DrawNumbersInBox(const std::list<int>& nums, const vpl::Point& tl, 
		      const vpl::Point& br, void* font, TEXT_H_ALIGNMENT hor_align, 
			  TEXT_V_ALIGNMENT ver_align, int maxNumDigits = 10);

void DrawNumbersInBox(const std::list<int>& nums, const vpl::Point& tl, 
		      const vpl::Point& br, void* font, TEXT_H_ALIGNMENT hor_align, 
			  TEXT_V_ALIGNMENT ver_align, int maxNumDigits)
{
	std::vector<int> nums2(nums.size());

	std::copy(nums.begin(), nums.end(), nums2.begin());

	DrawNumbersInBox(nums2, tl, br, font, hor_align, ver_align, maxNumDigits);
}
/////////////////////////////////////////////////////////////////////////////
Point tree_top_left(M, M), tree_bottom_right;
	Point tree_dim, tree_center;
	
	SetDrawingColor(NamedColor("DimGray"));

	i = 0;

	for (auto it = forest.begin(); it != forest.end(); ++it, ++i)
	{
		tree_dim.x = N_w * a_w(i);
		tree_dim.y = N_h * a_h(i);

		tree_bottom_right = tree_top_left;

		tree_bottom_right += tree_dim;

		tree_center = tree_top_left + tree_dim / 2.0; 

		//DrawTree(it->root, tree_center, N_w, N_h);
		DrawFilledRectangle(tree_top_left, tree_bottom_right);

		tree_top_left.x += tree_dim.x + N_w;
	}	
/////////////////////////////////////////////////////////////////////////////
	std::list<TreeAttributes> forest;

	TreeAttributes ta;
	ta.width = 2;
	ta.height = 4;
	forest.push_back(ta);
	ta.width = 5;
	ta.height = 8;
	forest.push_back(ta);
	ta.width = 3;
	ta.height = 7;
	forest.push_back(ta);

	DBG_PRINT6(I_w, I_h, tree_w(i), tree_h(i), p0, p1)


	//DoubleVector tree_w(N), tree_h(N);

/////////////////////////////////////////////////////////////////////////////
for (auto rootIt = m_roots.begin(); rootIt != m_roots.end(); ++rootIt)
	{
		DrawTree(*rootIt);

		DBG_LINE
	}

	node v;
	

	

	std::list<TreeAttributes> forest = compute_forest_info(*this);

	for (auto it = forest.begin(); it != forest.end(); ++it)
	{
		DBG_PRINT4(is_undirected(), index(it->root), it->width, it->height)
	}

	DBG_LINE
	auto ta = forest.front();

	forall_nodes(v, *this)
	{
		DBG_PRINT3(index(v), ta.attributes(v).level, ta.attributes(v).childOrder)
	}

	void set_visited(graph::node v, graph::node from)
	{
		(*ptrVisited)[v].tree = from;
	}
/////////////////////////////////////////////////////////////////////////////


	std::set<int> separator;
	std::set<int> residual;


	// Populate the "separator" and "residual" sets of each clique node
	std::list<unsigned>::const_iterator shrdVarIt;
	std::vector<int>::const_iterator varIt;

	forall_nodes(v, *this)
	{
		CVClique& cvc = attribute(v);

		// Populate the "separator" set of clique v
		forall_adj_edges(e, v)
		{
			std::list<unsigned>& shared = attribute(e).sharedVars;

			for (shrdVarIt = shared.begin(); shrdVarIt != shared.end(); ++shrdVarIt)
				cvc.separator.insert(*shrdVarIt);
		}

		// Populate the "residual" set of clique v
		for (varIt = cvc.variables.begin(); varIt != cvc.variables.end(); ++varIt)
		{
			if (cvc.separator.find(*varIt) == cvc.separator.end())
				cvc.residual.insert(*varIt);
		}
	}
/////////////////////////////////////////////////////////////////////////////

	// Create the needed edges between all maximal clique
	// that share cut variables
	edge e;

	for (unsigned i = 0; i < cliques.size(); ++i)
	{
		if (maximal[i])
		{
			// Note that (i,j) and (j,i) are the same edge, so
			// only add the (i,j) one
			for (unsigned j = i + 1; j < cliques.size(); ++j)
			{
				std::list<unsigned>& shared = adj(i, j);

				if (maximal[j] && !shared.empty())
				{
					e = new_edge(nodes[i], nodes[j], CVCliqueDependency());

					attribute(e).sharedVars.splice(shared.end(), shared);
				}
			}
		}
	}
/////////////////////////////////////////////////////////////////////////////
typedef std::tuple<unsigned, unsigned, node> VarInfo;
pq.push(VarInfo(0, i, nil));
/////////////////////////////////////////////////////////////////////////////
	// First, collect the common variables for each possible pair of cliques
	for (auto varIt = m_variables.begin(); varIt != m_variables.end(); ++varIt)
	{
		for (auto n0 = varIt->cliques.begin(); n0 != varIt->cliques.end(); ++n0)
		{
			auto n1 = n0;

			for (++n1; n1 != varIt->cliques.end(); ++n1)
			{
				adj(*n0, *n1).push_back(varIt->);
				//++adj(*n0, *n1);
				//++adj(*n1, *n0);
			}
		}
	}
/////////////////////////////////////////////////////////////////////////////
/*!
	Returns a list of all nodes collected in a depth-first search
	manner.
*/
void vpl::dfs_node_list(const graph& G, graph::NodeList& nl)
{
	graph::node v;

	G.UnvisitAllNodes();

	if (G.is_directed())
	{
		forall_nodes(v, G)
		{
			if (G.indeg(v) == 0)
				dfs_node_list(nl, v);
		}
	}
	else
	{
		forall_nodes(v, G)
		{
			if (!G.Visited(v))
				dfs_node_list(nl, v);
		}
	}
}

/*! 
	This function assumes that the visited attribute of all nodes
	has been initialized. Ie, it must be preceeded by UnvisitAllNodes()
*/
void vpl::dfs_node_list(const graph& G, graph::NodeList& nl, graph::node root)
{
	//if (nl.empty())
	//	UnvisitAllNodes();

	G.SetVisited(root, true);

	nl.push_back(root);

	graph::node v;

	forall_adj_nodes(v, root)
	{
		if (!G.Visited(v))
			dfs_node_list(G, nl, v);
	}
}
/////////////////////////////////////////////////////////////////////////////
if (s_params.drawLabels)
	{
		NodeList nl;
		node v;

		dfs_node_list(nl);

		int idx = 0;

		forall_node_items(v, nl)
		{
			/*if (degree(v) == 1)
				SetDrawingColor(NamedColor("Black"));
			else*/
				SetDrawingColor(NamedColor("Blue"));

			DrawNumber(idx++, inf(v).pt);

			//inf(v).idx
		}
	}
/////////////////////////////////////////////////////////////////////////////
	/*IntList::const_iterator it0, it1;
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
		// and use the length of teh line to determine the weight of
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

		if (partId == 0 || partCount == partId - 1)
		{
			if (colorIndices.empty())
			{
				SetDrawingColor(ColorMatrix(partCount, ColorMatrix::PASTELS));
			}
			else if (partCount < colorIndices.size() && colorIndices[partCount] < UINT_MAX)
			{
				SetDrawingColor(ColorMatrix(colorIndices[partCount], ColorMatrix::PASTELS));
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
			inf(v).ptrDescriptor->Draw();
		}
	}

	RGBColor nodeColor(255, 255, 255);

	forall_nodes(v, *this)
	{
		p0 = nodeCenters[index(v)];

		if (s_params.showLabels)
		{
			// Draw number first to get size of label, then
			// draw a filled disk of appropriate size
			SetDrawingColor(nodeColor);
			sz = DrawNumberCentered(index(v), p0, font);
			DrawDisk(p0, MAX(sz.x, sz.y) + 1);

			// Draw the number in front of the disk and a border
			// circle to frame the node
			SetDefaultDrawingColor();
			sz = DrawNumberCentered(index(v), p0, font);
			DrawCircle(p0, MAX(sz.x, sz.y) + 1);
		}
		else
		{
			
			SetDefaultDrawingColor();
			DrawDisk(p0, s_params.nodeRadius);
		}
	}*/
/////////////////////////////////////////////////////////////////////////////
ShapeParsingModel(ShapeInfoPtr ptrShapeInfo, SCGPtr ptrShapeCutGraph)
		: m_ptrShapeInfo(ptrShapeInfo), m_ptrShapeCutGraph(ptrShapeCutGraph)
	{
		
	}
/////////////////////////////////////////////////////////////////////////////
ptrModel.reset(new ShapeParsingModel(ptrShapeInfo, ptrCutGraph));
/////////////////////////////////////////////////////////////////////////////
void ShapeParsingModel::Create(ShapeInfoPtr ptrShapeInfo, SCGPtr ptrShapeCutGraph)
{
	ASSERT(empty());

	m_ptrShapeInfo = ptrShapeInfo;
	m_ptrShapeCutGraph = ptrShapeCutGraph;
/////////////////////////////////////////////////////////////////////////////
public:

	struct ShapeParseInfo
	{
		ShapeInfoPtr shapeInfo; //!< Basic shape information. eg, its boundary, corners, etc
		BCGPtr bcg; //!< Boundary cut graph of the shape

		ShapeParseInfo(const ShapeInfoPtr& ptrShape, const BCGPtr& ptrBCG)
			: shapeInfo(ptrShape), bcg(ptrBCG)
		{
		}

		//! Default contructor needed for serialization
		ShapeParseInfo() { }

		void operator=(const ShapeParseInfo& rhs)
		{
			shapeInfo = rhs.shapeInfo;
			bcg = rhs.bcg;
		}

		void Serialize(OutputStream& os) const
		{
			shapeInfo->Serialize(os);
		}

		void Deserialize(InputStream& is)
		{
			shapeInfo.reset(new ShapeInformation);

			shapeInfo->Deserialize(is);
		}
	};
/////////////////////////////////////////////////////////////////////////////
std::list<ShapeCutPath> ShapeParsingModel::FindMaxLen4Cycles(const BoundaryCutGraph& bcg)
{
	std::queue<ShapeCutPath> paths;
	ShapeCutPath p;
	node v;

	// Init queue
	forall_nodes(v, bcg)
	{
		paths.push(ShapeCutPath(v));
	}

	std::list<ShapeCutPath> cycles;
	graph::edge e, cycleEdge;
	node u;
	int k = 0;

	// Process queue
	while (!paths.empty())
	{
		p = paths.front();
		paths.pop();

		u = p.nodes.back();
		cycleEdge = nil;

		// Add the interior edges of the path (except for
		// the current "cycle" edge, which has to be
		// added only to subsequent the super-cycles
		forall_adj_edges(e, u)
		{
			// Dummy edges cannot be interior edges
			if (bcg.inf(e).isDummy)
				continue;
			
			v = opposite(u, e);

			if (v == p.nodes.front())
				cycleEdge = e;
			else if (p.ContainsProper(v))
				p.interiorEdges.push_back(e);
		}

		// Find cycle and candidate super-cycles
		bool isDummy;

		forall_adj_edges(e, u)
		{
			isDummy = bcg.inf(e).isDummy;

			// If edge is dummy and there is already a dummy
			// edge in the path, then we skip this edge
			if (isDummy && p.dummyEdge != nil)
				continue;

			v = opposite(u, e);

			if (v == p.nodes.front())
			{
				cycles.push_back(p);
				cycles.back().edges.push_back(e);

				if (isDummy)
					cycles.back().dummyEdge = e;
			}
			else if (!p.Contains(v))
			{
				// We have a candidate super-cycle of length > k
				paths.push(p);

				paths.back().nodes.push_back(v);
				paths.back().edges.push_back(e);

				// The "cycle edge" is an interior edge 
				// of the candidate super-cycle
				if (cycleEdge != nil)
					paths.back().interiorEdges.push_back(cycleEdge);

				if (isDummy) 
					paths.back().dummyEdge = e;
			}
		}
	}

	return cycles;
}
/////////////////////////////////////////////////////////////////////////////
struct ShapeCutAdj
{
	bool adj;
	graph::edge e;

	ShapeCutAdj()
	{
		adj = false;
		e = nil;
	}

	void Set(bool a, graph::edge b)
	{
		adj = a;
		e = b;
	}
};
/////////////////////////////////////////////////////////////////////////////
void Set(int i, const Point& p) 
	{ 
		idx = i; 
		pt = p;
	}

		SCGEdgeAtt(bool isDummyEdge = false)
	{
		isDummy = isDummyEdge;
	}
/////////////////////////////////////////////////////////////////////////////
EdgeMap<std::list<graph::node>> cuts2cliques(bcg);
CutVariable cvar;
cvc.cuts = cycleIt->edges;
		cvc.cuts.splice(cvc.cuts.end(), cycleIt->interiorEdges);		

		for (auto cutIt = cvc.cuts.begin(); 
			cutIt != cvc.cuts.end(); ++cutIt)
		{
			e = *cutIt;
			
			cuts2cliques[e].push_back(v);

			cvar.Set(bcg.index(source(e)), bcg.index(target(e)), false);
			
			cvc.variables.push_back(cvar);
		}
/////////////////////////////////////////////////////////////////////////////
//! Interior edges are added at the back pf the list!
	/*CVClique(const ShapeCutPath& p) 
	{
		cuts = p.edges;
		cuts.splice(cuts.end(), p.interiorEdges);
	}*/
/////////////////////////////////////////////////////////////////////////////

//VPLMatrix<ShapeCutAdj> m_shapeCutAdjMatrix;


//m_shapeCutAdjMatrix.clear();
	ASSERT(m_shapeCutAdjMatrix.size() == 0);

	m_shapeCutAdjMatrix.set_size(bcg.number_of_nodes(), bcg.number_of_nodes());

	edge e;
	int i, j;

	forall_edges(e, bcg)
	{
		i = index(source(e));
		j = index(target(e));

		m_shapeCutAdjMatrix(i, j).Set(true, e);
		m_shapeCutAdjMatrix(j, i).Set(true, e);
	}

	for (i = -1, j = 0; i < XYZ; ++j)
	{
		m_shapeCutAdjMatrix(i, j).Set(true, nil);
		m_shapeCutAdjMatrix(j, i).Set(true, nil);
	}
/////////////////////////////////////////////////////////////////////////////

			if (p.Contains(v))
			{
				if (v == p.nodes.front())
					cycleEdge = e;
				else if (v != u)
					p.interiorEdges.push_back(e);
			}

	/*! 
		Returns true if the nodes list contains v, but excludes 
		first and last elements in the list.
	*/
	bool ContainsProper(graph::node v) const
	{
		auto it = std::find(nodes.begin(), nodes.end(), v);

		return (it != nodes.begin() && it != nodes.end() &&
			++it != nodes.end());
	}
/////////////////////////////////////////////////////////////////////////////

for (auto nodeIt = na.begin(); nodeIt != na.end(); ++nodeIt)
	{
		if (*nodeIt == nil)
			continue;
/////////////////////////////////////////////////////////////////////////////
