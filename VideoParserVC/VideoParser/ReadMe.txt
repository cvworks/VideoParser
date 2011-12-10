FloatImg img = m_imgBuffer.front().greyFrame;

		for (auto it = ++m_imgBuffer.begin(); it != m_imgBuffer.end(); ++it)
		{
			auto srcIt = it->greyFrame.begin();
			auto tgtIt = img.begin();

			for (; tgtIt != img.end(); ++tgtIt, ++srcIt)
				*tgtIt += *srcIt;
		}

		for (auto it = img.begin(); it != img.end(); ++it)
			*it /= m_imgBuffer.size();

		// Recompute the darkness level
		ComputeDarknessLevel();


		//cv::Ptr<cv::FilterEngine> filter = cv::createGaussianFilter(
		//	cv::DataType<float>::type, cv::Size(8, 8), 1.0);

		//CvMatView rgbMat(m_imgBuffer.front().rgbFrame);
		//DBG_LINE
		/*FloatImg origImg = m_imgBuffer.front().greyFrame;
		FloatImg blurImg(origImg.ni(), origImg.nj());

		CvMatView origMat(origImg);
		CvMatView blurMat(blurImg);

		//cv::blur(origMat, blurMat, cv::Size(9, 9));
		//cv::gaussianBlur(origMat, blurMat, cv::Size(0, 0), 4.0);
		cv::medianBlur(origMat, blurMat, 5);

		//blurMat.setTo(0);

		m_imgBuffer.front().greyFrame = blurImg;*/

		//m_imgBuffer.front().greyFrame.fill(0);

//! Gets the info associated with the current video frame
void VideoProcessor::GetInputImage(const GenericVideo& vid, InputImageInfo& imgInfo) const
{
	imgInfo.rgbFrame = m_video.GetCurrentRGBFrame();
	imgInfo.greyFrame = m_video.GetCurrentGreyScaleFrame();

	imgInfo.frameNumber = FrameNumber();
	imgInfo.framePos = m_video.FramePosition();
	
	imgInfo.frameInfo = m_video.FrameInfo();
	imgInfo.timestamp = m_video.CurrentFrameTime();
	imgInfo.roiSequence  = m_video.GetROISequence();
}

	//void ProcessNewFrame(fnum_t frameNumber, const double& framePos, 
	//	RGBImg rgbFrame, FloatImg greyFrame, const std::string& frameInfo);

void VSCGraph::ProcessNewFrame(fnum_t frameNumber, const double& framePos, 
							   RGBImg rgbFrame, FloatImg greyFrame,
							   const std::string& frameInfo)
{
	// Store the current frame data
	m_inImgInfo.frameNumber = frameNumber;
	m_inImgInfo.framePos = framePos;
	m_inImgInfo.rgbFrame = rgbFrame;
	m_inImgInfo.greyFrame = greyFrame;
	m_inImgInfo.frameInfo = frameInfo;
	}

HANDLE hFile = CreateFileA("output2.txt", GENERIC_READ | GENERIC_WRITE, 
	FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
	CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

if (hFile != INVALID_HANDLE_VALUE)
{
	int fp = _open_osfhandle((long)hFile, 0);
	FILE* fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	ios::sync_with_stdio();
}

std::cout << "Are you there" << std::endl;
	

	if (SetStdHandle(STD_OUTPUT_HANDLE, hFile) == FALSE)
	{
		ShowError("Cannot redirect output stream");
		return 1;
	}
	else
	{
		std::cout << "Are you there" << std::endl;
		return 1;
	}

	/*

	

	if (hFile == INVALID_HANDLE_VALUE)
	{
		ShowError("Cannot open output file");
		return 1;
	}
	else if (SetStdHandle(STD_OUTPUT_HANDLE, hFile) == FALSE 
		|| SetStdHandle(STD_OUTPUT_HANDLE, hFile) == FALSE)
	{
		ShowError("Cannot redirect output stream");
		return 1;
	}*/

bool operator=(const ShapeViewMetadata& rhs) const
	{
		bool unusued[NUM_PROPERTIES];

		for (unsigned i = 0; i < NUM_PROPERTIES; i++)
			unusued[i] = true;

		for (unsigned i = 0; i < NUM_PROPERTIES; i++)
		{
			bool hasProp = false;

			for (unsigned j = 0; j < NUM_PROPERTIES; j++)
			{
				if (unusued[j] && propId[i] == rhs.propId[j] && 
					strcmp(propName[i], rhs.propName[j]) == 0)
				{
					hasProp = true;
					break;
				}
			}

			if (!hasProp)
				return false;
		}

		return true;
	}

std::string className() const
	{
		ASSERT(!data.empty());
		return data.front().first;
	}

	int objId() const
	{
		ASSERT(!data.empty());
		return data.front().second;
	}

void Set(const std::string& name1, int id1, 
             const std::string& name2 = std::string(), int id2 = ID_NOT_SET, 
			 const std::string& name3 = std::string(), int id3 = ID_NOT_SET)
	{
		propId[0] = id1;
		string_copy(name1, propName[0], ID_NAME_SIZE + 1, ID_NAME_SIZE);

		propId[1] = id2;
		string_copy(name2, propName[1], ID_NAME_SIZE + 1, ID_NAME_SIZE);

		propId[2] = id3;
		string_copy(name3, propName[2], ID_NAME_SIZE + 1, ID_NAME_SIZE);
	}

#define ShowStatus(M) 

#define ShowStatus1(M,A) 

#define ShowStatus2(M,N,S)

#define StreamStatus(CMD)

std::string guiStr("-nogui");

		if (!stricmp(argv[1], "-e") || !stricmp(argv[1], "-p")
			|| !stricmp(argv[1], "-nogui"))
		{
			opt = argv[1];
			argfname = argv[2];
		}
		else if 



		if (guiStr == argv[1])
			argfname = argv[2];
		else if (guiStr == argv[2])
			argfname = argv[1];

if (maxNumDesc && (v1 == nil || v2 == nil))
	{		int x = 1;

		x++;
	}


struct SPGNodeMatch
{
	NodeArray m2qMap; //!< Query nodes to model node correspondences
	DoubleArray weights;   //!< Weight of node matches
};

typedef std::pair<graph::node, double> NodeCorrespondence;
typedef NodeMap<NodeCorrespondence> NodeCorrespondences;

forall_adj_edges(e1, v1)
	{
		u1 = opposite(v1, e1);

		if (u1 == ptrNodeAss->Parent(0))
			continue;

		forall_adj_edges(e2, v2)
		{
			u2 = opposite(v2, e2);

			if (u2 == ptrNodeAss->Parent(1))
				continue;

			

			// Handle virtual nodes!!!
			childIdx2++;
		}

		childIdx1++;
	}

	ASSERT(childIdx1 == maxNumDesc || childIdx2 == maxNumDesc);

	for (; childIdx1 < maxNumDesc; childIdx1++)
	{
		unsigned idx2 = 0;

		forall_adj_edges(e2, v2)
		{
			ptrChildAss.reset(new NodeAssignment(nil, u2));

			ComputeRootedTreeDistance(ptrChildAss);

			costs[childIdx1][idx2] = ptrChildAss->Distance();

			idx2++;
		}
	}

	for (; childIdx2 < maxNumDesc; childIdx2++)
	{
		unsigned idx1 = 0;

		forall_adj_nodes(u1, v1)
		{
			ptrChildAss.reset(new NodeAssignment(u1, nil));

			ComputeRootedTreeDistance(ptrChildAss);

			costs[idx1][childIdx2] = ptrChildAss->Distance();

			idx1++;
		}
	}

void ShapeParser::Run()
{
	if (!m_pRegAnalyzer)
	{
		ShowMissingDependencyError(RegionAnalyzer);
		return;
	}

	ShapeInfoPtr ptrShapeInfo;

	ShowStatus("Parsing...");

	//ShapeContextComp::Test();
	//ShapeParsingModel().ComputeShapeParses();
	//return;

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

		ASSERT(false);

		ShapeList::iterator it;

		std_forall(it, m_shapes)
		{
			//it->bcg = CreateBoundaryCutGraph(it->shapeInfo);
		}
	}
	else
	{
		const RegionSet& regions = m_pRegAnalyzer->GetRegions();
		SCGPtr ptrCutGraph;
		unsigned width, height;
		bool hasSkeleton;

		m_pRegAnalyzer->GetImgDimensions(&width, &height);

		RegionSet::const_iterator selRegIt = regions.end();
		unsigned maxPerim = 0;

		// @TODO This is temporary: select only largest region
		for (RegionSet::const_iterator it = regions.begin(); it != regions.end(); ++it)
		{
			if (it->bbox.xmin == 0 && it->bbox.ymin == 0)
				continue;

			if (it->boundaryPts.Size() > maxPerim)
			{
				maxPerim = it->boundaryPts.Size();
				selRegIt = it;
			}
		}

		for (RegionSet::const_iterator it = regions.begin(); it != regions.end(); ++it)
		{
			if (it->bbox.xmin == 0 && it->bbox.ymin == 0)
				continue;

			if (it != selRegIt)
				continue;

			// Create a new shape info object, but don't add it yet
			ptrShapeInfo.reset(new ShapeInformation); // it self destroys if not used

			ShowStatus("Computing skeleton...");

			try {
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
				
				ShowStatus("Computing shape cut graph...");

				// Create a shape cut graph with the shape information
				ptrCutGraph.reset(new ShapeCutGraph(ptrShapeInfo));

				ShowStatus("Computing shape parsing model...");

				m_shapes.push_back(ShapeParsingModel());

				// Create a shape parsing model with the shape cut graph
				m_shapes.back().Create(ptrShapeInfo, ptrCutGraph);

				ShowStatus("Computing shape parse graphs...");
				
				// Find the K most probable shape parses
				m_shapes.back().ComputeShapeParses();

				ShowStatus("Shape parsing is done!");
			}
			else
			{
				ShowError("Cannot create shape representation");
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

#include <ShapeParsing/ShapeParseGraph.h>

RGBImg img1;

	if (img.ni() > m_frameWidth || img.nj() > m_frameHeight)
	{
		ByteImg a = vil_convert_to_n_planes(3, m_curImg);
		ByteImg b;

		vil_resample_nearest(a, b, m_frameWidth, m_frameHeight);

		img1 = vil_convert_to_component_order(ConvertToBaseImgPtr(b));
	}
	else
	{
		img1 = img;


	}

	FloatImg img1;

	if (img.ni() > m_frameWidth || img.nj() > m_frameHeight)
	{
		vil_resample_nearest(img, img1, m_frameWidth, m_frameHeight);
	}
	else
	{
		img1 = img;
	}
/////////////////////////////////////////////////////////////////////////////
	if (!m_bJustPlayVideo)
	{
		Plotter p;

		p.Create(0, NULL);
	}

/////////////////////////////////////////////////////////////////////////////
	for (unsigned int i = 0; i < m_regionPyr.size(); ++i)
	{
		Region& r = m_regionPyr[i];

		if (r.boundaryPts.Size() > 0)
		{
			r.color = srcImg(r.boundaryPts.xa.front(), 
				r.boundaryPts.ya.front());
		}
		else
		{
			r.color = RGBColor(128,128,128);
		}
	}

	m_frameWidth = 0;
	m_frameHeight = 0;
	BaseImgPtr auxImg;

	for (auto it = m_framePaths.begin(); it != m_framePaths.end(); ++it)
	{
		auxImg = vil_load(it->c_str());

		if (auxImg->ni() > m_frameHeight)
			m_frameHeight = auxImg->ni();

		if (auxImg->nj() > m_frameWidth)
			m_frameWidth = auxImg->nj();
	}

	m_frameWidth += 2 * IMG_MARGIN;
	m_frameHeight += 2 * IMG_MARGIN;
	//m_curImg