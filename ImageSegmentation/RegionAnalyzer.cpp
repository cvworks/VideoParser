/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "RegionAnalyzer.h"
#include <ImageSegmentation/ImageSegmenter.h>
#include <VideoSegmentation/BackgroundSubtractor.h>
#include <ObjectTracking/BlobTracker.h>
#include <vil/algo/vil_blob_finder.h>
//#include <vil/algo/vil_trace_8con_boundary.h>
//#include <vil/algo/vil_trace_8con_boundary.cxx>

#include <Tools/VSCDataSerializer.h>
#include <VideoParserGUI/DrawingUtils.h>
#include <Tools/UserEvents.h>
#include <Tools/cv.h>

#define NORTH 0
#define SOUTH 1
#define EAST  2
#define WEST  3

using namespace vpl;

extern UserArguments g_userArgs;

void RegionAnalyzer::Initialize(graph::node v)
{
	// Base init calls Clear(), ReadParamsFromUserArguments(),...
	VisSysComponent::Initialize(v);

	m_pImgSegmenter = FindParentComponent(ImageSegmenter);

	m_pImgProcessor = FindParentComponent(ImageProcessor);

	// The BackgroundSubtractor is an OPTIONAL component
	m_pBkdSubtractor = FindParentComponent(BackgroundSubtractor);
	m_hasForegroundMask = (m_pBkdSubtractor != NULL);

	// The BlobTracker is an OPTIONAL component
	m_pBlobTracker = FindParentComponent(BlobTracker);
	m_hasBlobMask = (m_pBlobTracker != NULL);
}

void RegionAnalyzer::ReadParamsFromUserArguments()
{
	VisSysComponent::ReadParamsFromUserArguments();

	g_userArgs.ReadBoolArg(Name(), "saveUserSelection", 
		"Whether to save the user selected regions or not", 
		false, &m_saveUserSelection);
}

void RegionAnalyzer::Run()
{
	if (!m_pImgSegmenter)
	{
		ShowMissingDependencyError(ImageSegmenter);
		return;
	}

	if (!m_pImgProcessor)
	{
		ShowMissingDependencyError(m_pImgProcessor);
		return;
	}

	// First thing to do is clear any data from previous a pass
	Clear();
	
	m_srcRGBImg = m_pImgProcessor->GetRGBImage();
	m_srcGreyImg = m_pImgProcessor->GetGreyImage();

	if (m_hasForegroundMask)
		m_foregroundImg = m_pBkdSubtractor->Foreground();

	if (m_hasBlobMask)
		m_blobImg = m_pBlobTracker->GetBlobMask();

	/*if (m_countVotes)
	{
		m_saliencyImg = m_pBkdSubtractor->Foreground();

		//m_saliencyImg.deep_copy(m_pBkdSubtractor->Foreground());
		//vil_math_scale_values(m_saliencyImg, 1.0 / 255.0);
	}*/

	m_inputImgs.resize(m_pImgSegmenter->NumSegmentations());
	

	for (unsigned i = 0; i < m_pImgSegmenter->NumSegmentations(); i++)
	{
		m_regionPyr.AddLevel(FindRegions(
			m_pImgSegmenter->Regions(i), 
			m_pImgSegmenter->NumRegions(i),
			m_pImgSegmenter->ColoredSegmentation(i)));

		// Save a ptr to the segmentation id's for later
		m_inputImgs[i] = m_pImgSegmenter->Regions(i);
	}

	LoadUserData();
}

/*!
	Finds the region information of a segmentation.

	Get the segmentation id's of each pixel.
	Gets the segmentation colores assigned to each pixel/regions.
	Uses the source image to find the true "color" of each region.
*/
RegionArray RegionAnalyzer::FindRegions(IntImg inputSegImg, int num_regions,
	RGBImg coleredSegImg)
{
	RegionArray regions;

	// Init regions and their b-boxes
	const UIBoundingBox bbox(inputSegImg.ni(), 0, inputSegImg.nj(), 0, false);

	// Init the size of the array of regions to the number of regions that
	// we know are there...
	regions.assign(num_regions, bbox);

	UICoordinate c;

	for (c.y = 0; c.y < inputSegImg.nj(); ++c.y)
	{
		for (c.x = 0; c.x < inputSegImg.ni(); ++c.x)
		{
			Region& r = regions[inputSegImg(c.x, c.y)];

			// If this is the first pixel that we see from region 'r'
			// set the basic attributes of 'r'...
			if (r.numPts == 0)
			{
				r.firstPt = c;
				r.colorRGB = m_srcRGBImg(c.x, c.y);
				r.colorVal = m_srcGreyImg(c.x, c.y);
				r.fakeColor = coleredSegImg(c.x, c.y);
			}

			// Count the number of pixels seen from this region
			++r.numPts;

			// Shrink/grow the bounding box of the region
			if(c.x < r.bbox.xmin) r.bbox.xmin = c.x;
			if(c.x > r.bbox.xmax) r.bbox.xmax = c.x;
			if(c.y < r.bbox.ymin) r.bbox.ymin = c.y;
			if(c.y > r.bbox.ymax) r.bbox.ymax = c.y;

			// Integrate foreground information if available
			if (m_hasBlobMask && m_blobImg(c.x, c.y) == 255)
			{
				r.voteCount++;
			}
			/*else if (m_hasForegroundMask && m_foregroundImg(c.x, c.y) == 255)
			{
				r.voteCount++;
			}*/
		}
	}

	for (unsigned int i = 0; i < regions.size(); ++i)
		regions[i].SetMaskSizeToBBox();

	for (c.y = 0; c.y < inputSegImg.nj(); ++c.y)
	{
		for (c.x = 0; c.x < inputSegImg.ni(); ++c.x)
		{
			regions[inputSegImg(c.x, c.y)].SetMaskPixel(c);
		}
	}

	vil_blob_finder finder;
	DiscreteXYArray xya;

	// Find the boundary of each region from its mask image
	for (unsigned int i = 0; i < regions.size(); ++i)
	{
		Region& r = regions[i];

		// Provide the region's mask to the blob finder
		finder.set_image(r.mask);

		// Find the boundary of the outer "true" pixels
		//finder.next_4con_region(r.boundaryPts.xa, r.boundaryPts.ya);
		finder.next_8con_region(r.boundaryPts.xa, r.boundaryPts.ya);
		
		// Update the boundary points coordinates so that they 
		// correspond to the boundary points of the region instead 
		// of those of it's mask
		r.boundaryPts.ApplyOffset(r.bbox.xmin, r.bbox.ymin);

		// Found the boundary of all nested regions
		//while (finder.next_4con_region(xya.xa, xya.ya))
		while (finder.next_8con_region(xya.xa, xya.ya))
		{	
			xya.ApplyOffset(r.bbox.xmin, r.bbox.ymin);

			r.holeBndryPts.push_back(xya);
		}		
	}

	return regions;
}

/*!
	If regionIdx < 0, then ALL regions in segmentation segmentIdx are requested.

	Note: regionIdx is not constrained to be a valid region index. So, check
	if it's a valid ID for the requested level.
*/
ByteImg RegionAnalyzer::CreateContourImage(int segmentIdx, int regionIdx) const
{
	if (m_inputImgs.empty())
		return ByteImg();

	ByteImg img(m_inputImgs.front().ni(), m_inputImgs.front().nj());

	img.fill(255);

	const RegionArray& ra = m_regionPyr.level(segmentIdx);

	if (regionIdx < 0) // show all regions
	{
		unsigned i;

		for (auto it0 = ra.begin(); it0 < ra.end(); ++it0)
		{
			//if ((m_hasForegroundMask || m_hasBlobMask) && it0->voteCount == 0)
			//	continue;

			const DiscreteXYArray& xya = it0->boundaryPts;
			
			for (i = 0; i < xya.xa.size(); i++)
			{
				img(xya.xa[i], xya.ya[i]) = 0;
			}
		}
	}
	else if (regionIdx < (int) ra.size()) // show the requested region
	{
		const DiscreteXYArray& xya = ra[regionIdx].boundaryPts;
			
		for (unsigned i = 0; i < xya.xa.size(); i++)
		{
			img(xya.xa[i], xya.ya[i]) = 0;
		}
	}

	return img;
}

/*!
	If regionIdx < 0, then ALL regions in segmentation segmentIdx are requested.

	Note: regionIdx is not constrained to be a valid region index. So, check
	if it's a valid ID for the requested level.
*/
RGBImg RegionAnalyzer::CreateSalientRegionImage(int segmentIdx, int regionIdx) const
{
	if (m_inputImgs.empty())
		return RGBImg();

	RGBImg salImg(m_inputImgs.front().ni(), m_inputImgs.front().nj());
	RGBImg colImg = m_pImgSegmenter->ColoredSegmentation(0);

	RGBColor bgdCol(0, 0, 0), voteCol(255, 255, 255);

	UICoordinate c;

	salImg.fill(bgdCol);

	if (0)
	{
		const RegionArray& ra = m_regionPyr.level(segmentIdx);

		IntImg inputImg = m_inputImgs[segmentIdx];

		for (c.y = 0; c.y < inputImg.nj(); ++c.y)
		{
			for (c.x = 0; c.x < inputImg.ni(); ++c.x)
			{
				if (regionIdx >= 0 && regionIdx != inputImg(c.x, c.y))
					continue;

				const Region& r = ra[inputImg(c.x, c.y)];

				if (r.voteCount > 0 && r.voteCount / r.numPts >= 0.5)
					salImg(c.x, c.y) = colImg(c.x, c.y);
			}
		}
	}


	IntImg region_img = m_pImgSegmenter->Regions(0);
	// ok... here it is basically just echoing back the foreground...
	int pixels_checked = 0;
	for (unsigned i = 0; i < salImg.ni(); ++i) // x
	{
		for (unsigned j = 0; j < salImg.nj(); ++j) // y
		{
			
			if ((int)m_foregroundImg(i, j) == 255)
			{
				salImg(i, j) = voteCol;
			}
		}
	}

	return salImg;
}

std::set<int> RegionAnalyzer::getSalientRegions() const
{
	std::set<int> salient_regions;
	IntImg region_img = m_pImgSegmenter->Regions(0);
	for (unsigned i = 0; i < m_inputImgs.front().ni(); ++i) // x
	{
		for (unsigned j = 0; j < m_inputImgs.front().nj(); ++j) // y
		{
			
			if ((int)m_foregroundImg(i, j) == 255)
			{
				salient_regions.insert(region_img(i, j));
			}
		}
	}
	return salient_regions;
}

bool RegionAnalyzer::pointInRegion(int x, int y, DiscreteXYArray &pts) const
{
	//typedef boost::geometry::model::d2::point_xy<int> point_type;
	//typedef boost::geometry::model::polygon<point_type> polygon_type;

	/*polygon_type poly;
	for (unsigned i = 0; i < pts.Size(); ++i)
	{
		point_type pt(pts.xa[i], pts.ya[i]);
		boost::geometry::append(poly, pt);
	}

	point_type pt(x, y);*/
	return true;
	//return boost::geometry::within(pt, poly);
}

/*!
	Draws the output of the component. This function is called from
	a "drawing" thread, which is different from the "processing" thread
	that calls Run(). In order to protect the shared resources between the
	threads, a mutex is used.
*/
void RegionAnalyzer::Draw(const DisplayInfoIn& dii) const
{
	PointList pts;
	for (unsigned i = 20; i < 40; ++i)
	{
		for (unsigned j = 20; j < 40; ++j)
		{
			pts.push_back(Point(i, j));
		}
	}
	DrawFilledPolygon(pts);
#if 0
	if (m_regionPyr.empty())
		return;

	RGBColor selCol(255, 0, 0), regCol;

	unsigned segmentIdx = (unsigned) dii.params[0];

	ASSERT(segmentIdx < m_regionPyr.height());

	const RegionArray& ra = m_regionPyr.level(segmentIdx);

	// Draw all the SELECTED regions
	for (auto it0 = ra.begin(); it0 < ra.end(); ++it0)
	{
		if (!it0->IsSelected())
			continue;

		PointList pts;

		const DiscreteXYArray& xya = it0->boundaryPts;
			
		for (unsigned i = 0; i < xya.xa.size(); i++)
			pts.push_back(Point(xya.xa[i], xya.ya[i]));

		// Find out the color of the first region in the group
		regCol = ra[it0->GetGroupId() - 1].fakeColor;

		SetDrawingColor(regCol);
		DrawFilledPolygon(pts);

		// Drar a well-defined red boundary
		SetDrawingColor(selCol);
		DrawPolygon(pts);
	}

	SetDefaultDrawingColor();

	/*if (!m_shapes.empty())
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
	}*/
#endif
}

/*!
	This function is called when the left mouse botton and
	the mouse pointer was on the component's view.

	@return zero if the even was not dealt with and 1 otherwise.
*/
bool RegionAnalyzer::OnGUIEvent(const UserEventInfo& uei)
{
	// The mouse should not have any effect if we can't save
	// the user data that it would create
	if (!m_pDataSerializer)
		return false;

	if (uei.id == EVENT_MOUSE_PUSH)
	{
		ASSERT(!uei.params.empty());

		unsigned segmentIdx = (unsigned) uei.params[0];

		ASSERT(segmentIdx < m_inputImgs.size());

		IntImg inputImg = m_inputImgs[segmentIdx];

		// See if the click is within the image limits
		if (uei.coord.x >= inputImg.ni() || uei.coord.y >= inputImg.nj())
			return false;

		// Find the ID of the region that contains the mouse click
		int regId = inputImg((unsigned)uei.coord.x, (unsigned)uei.coord.y);

		// Find the region information
		Region& r = m_regionPyr.region(segmentIdx, regId);

		// See if which mouse button was pushed
		if (uei.value == EVENT_VALUE_MOUSE_BUTTON1)
		{
			// The current "selection" state of the region
			// determines the action to take...
			if (r.IsSelected())
			{
				// Unselect the region
				r.Unselect();
			}
			// If there are no selections, or if the current
			// segmentation is different from the active segmenation, 
			// then a left click acts as a right click
			else if (m_currentGroupId == 0 || 
				m_currentSegmentationId != segmentIdx)
			{
				// Perform a right click, ie start a new group
				UserEventInfo uei2 = uei;

				uei2.value = EVENT_VALUE_MOUSE_BUTTON2;

				OnGUIEvent(uei2);
			}
			else
			{
				// Add the selected region to the current group
				r.SetGroupId(m_currentGroupId);
			}
		}
		else if (uei.value == EVENT_VALUE_MOUSE_BUTTON2)
		{
			// If the region is already selected, its group id 
			// becomes the current one 
			if (r.IsSelected())
			{
				m_currentGroupId = r.GetGroupId();
			}
			else // otherwise, we start a new group
			{
				m_currentSegmentationId = segmentIdx;

				// The ID of the group is the ID of the
				// region used to created plus 1. The +1 is
				// needed because the region zero exists, but
				// group ID zero meands that the region is not
				// selected
				m_currentGroupId = regId + 1; 

				r.SetGroupId(m_currentGroupId);
			}
		}
	}
	else if (uei.id == EVENT_FINISHED_FRAME)
	{
		SaveUserData();
	}

	return true;
}

/*!
	Loads existing user data from the system's cache (if it's active).
*/
void RegionAnalyzer::LoadUserData()
{
	if (!m_pDataSerializer)
		return;

	// Try to recover the regions selected by the user in a previous
	// iteration trough the video frame. First, try loading the parsing 
	// data for the current frame.
	if (!m_pDataSerializer->LoadComponentData(
		m_savedRegionPyr, this, "UserSelectedRegions"))
	{
		//ShowStatus("There are no saved region labellings.");
		m_savedRegionPyr.clear();
		return;
	}

	// Next, read the saved data and compare it against
	// the current data. If they describe the exact same
	// regions, then it means that the saved labelling is 
	// valid for this data.
	
	//ShowStatus("Region labellings have been found.");

	//DBG_PRINT2(m_savedRegionPyr.size(), m_regionPyr.size())

	// We should have the same number of regions in each level 
	// of the pyramid. The order (ie, the region ID) within 
	// each level and the boundary of each region should be the same
	if (m_savedRegionPyr == m_regionPyr)
	{
		// If we do have the same data, we copy the saved group id
		ShowStatus("User labelling has been loaded.");

		auto it0 = m_regionPyr.begin();
		auto it1 = m_savedRegionPyr.begin();
	
		for (; it0 != m_regionPyr.end(); ++it0, ++it1)
			it0->groupId = it1->groupId;
	}
	else
	{
		WARNING2(true, "The number of current regions and "
			"saved regions is different",
			m_regionPyr.height(), m_savedRegionPyr.height());

		// Clear the data retrieved to indicate that we don't have any
		m_savedRegionPyr.clear();
	}
}

/*!
	Saves user data to the system's cahce (if active) and to
	a YAML/XML file (if requested by the user).

	It checks if there is user data to save and, if there is, it 
	saves its. It checks whether there was previous data and if there is
	at leat one selected region.
*/
void RegionAnalyzer::SaveUserData()
{
	//cv::FileStorage fs("selected_regions.yml", cv::FileStorage::WRITE);

	////////////////////////////////////////////////////////
	// Deal with saving the data to the cache
	if (!m_pDataSerializer)
		return;

	bool hasUserData = false;

	// If there is previously saved user data, see if it has been modified
	if (!m_savedRegionPyr.empty())
	{
		ASSERT(m_savedRegionPyr == m_regionPyr);

		auto it0 = m_regionPyr.begin();
		auto it1 = m_savedRegionPyr.begin();
	
		for (; it0 != m_regionPyr.end(); ++it0, ++it1)
		{
			if (it0->groupId != it1->groupId)
			{
				hasUserData = true;
				break;
			}
		}	
	}
	else // no saved data. See if there are labels to save
	{
		// See if at least one regions has been selected by the user
		for (auto it = m_regionPyr.begin(); it != m_regionPyr.end(); ++it)
		{
			if (it->IsSelected())
			{
				hasUserData = true;
				break;
			}
		}	
	}

	// See if there is data to save
	if (hasUserData)
	{	
		unsigned status;

		status = m_pDataSerializer->SaveComponentData(
			m_regionPyr, this, "UserSelectedRegions");

		// Updating the saved regions in memory may not be needed
		// but it's always good to have consitent data, isn't it?
		m_savedRegionPyr = m_regionPyr;

		if (status != INVALID_STORAGE_ID)
			ShowStatus("Region labelling has been saved (but not indexed yet).");
		else
			ShowError("Could not save region labelling.");
	}
}

