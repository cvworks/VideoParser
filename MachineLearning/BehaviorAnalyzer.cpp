/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "BehaviorAnalyzer.h"
#include <VideoParser/ImageProcessor.h>
#include <ShapeParsing/ShapeParser.h>
#include <ObjectTracking/BlobTracker.h>
#include <ObjectTracking/KalmanBlobTracker/Trace.h>
#include <Tools/UserArguments.h>
#include <Tools/CvMatView.h>
#include <Tools/NamedColor.h>
#include <VideoParserGUI/DrawingUtils.h>
#include <GraphTheory/AttributedGraph.h>

using namespace vpl;

extern UserArguments g_userArgs;

/*!
	@param di pixel width
	@param dj pixel height
*/
inline RGBImg ImageZoom(RGBImg src, unsigned di, unsigned dj)
{
	RGBImg img(src.ni() * di, src.nj() * dj);
	CvMatView mat(img);
	RGBColor c;

	for (unsigned i = 0; i < src.ni(); i++)
	{
		for (unsigned j = 0; j < src.nj(); j++)
		{
			cv::Point p0(i * di, j * dj);
			cv::Point p1((i + 1) * di, (j + 1) * dj);

			c = src(i, j);

			cv::rectangle(mat, p0, p1, cv::Scalar(c.r, c.g, c.b), CV_FILLED);

			cv::rectangle(mat, p0, p1, cv::Scalar(192, 192, 192), 1);
		}
	}

	return img;
}

void BehaviorAnalyzer::ClusterEvents()
{
	AttributedGraph<TimeSlotData, int> g;
}

/*!
	Reads the parameters provided by the user. 

	It is called by VisSysComponent::Initialize(), which
	is in turn calledwhen the vis component is created.
	
	It is NOT an static function because the user is allowed
	to changed the parameters of a vision system component
	several times during the execution of a program. 
*/
void BehaviorAnalyzer::ReadParamsFromUserArguments()
{
	VisSysComponent::ReadParamsFromUserArguments();

	g_userArgs.ReadArg(Name(), "session_id", 
		"Id of the session to analyze", 0, 
		&m_params.session_id);

	g_userArgs.ReadArg(Name(), "numSmoothIterations", 
		"Number of smoothing operations to perform on each trace retrieved", 5u, 
		&m_params.numSmoothIterations);

	g_userArgs.ReadArg(Name(), "minProbLikelyEvents", 
		"The min probability of an event to be labeled VERY_LIKELY", 0.8, 
		&m_params.minProbLikelyEvents);

	g_userArgs.ReadArg(Name(), "maxProbUnlikelyEvents", 
		"The max probability of an event to be labeled VERY_UNLIKELY", 0.2, 
		&m_params.maxProbUnlikelyEvents);
}

void BehaviorAnalyzer::Initialize(graph::node v)
{
	VisSysComponent::Initialize(v);

	m_pImgProcessor = FindParentComponent(ImageProcessor);

	m_dbm.init(m_pSQLDatabase);
	
	// This component onlu runs in offline mode
	SetRunningMode(OFFLINE_RUNNING_MODE);
}

void BehaviorAnalyzer::GetParameterInfo(int i, DoubleArray* pMinVals, 
									 DoubleArray* pMaxVals, 
									 DoubleArray* pSteps) const
{
	if (i == TEMPORAL_SCALE || i == SEGMENT_IMG)
	{
		InitArrays(5, pMinVals, pMaxVals, pSteps, 0, 0, 1);

		pMaxVals->at(0) = m_tempPyr.ni(); // minus 1 if more that zero
		pMaxVals->at(1) = m_tempPyr.nj(); // minus 1

		pMaxVals->at(2) = 7;
		pMaxVals->at(3) = 1440 / m_eventCalendar.timeUnit();

		pMaxVals->at(4) = 2;

		SubtractOneFromNonZeroValues(pMaxVals);
	}
	else if (i == SPATIAL_SCALE)
	{
		InitArrays(5, pMinVals, pMaxVals, pSteps, 0, 0, 1);

		pMaxVals->at(0) = m_tempPyr.ni();
		pMaxVals->at(1) = m_tempPyr.nj();

		pMaxVals->at(2) = 7;
		pMaxVals->at(3) = 1440 / m_eventCalendar.timeUnit();

		pMaxVals->at(4) = m_tempPyr.numSpatialLevels();

		SubtractOneFromNonZeroValues(pMaxVals);
	}
	else if (i == TRACES_IMG)
	{
		InitArrays(2, pMinVals, pMaxVals, pSteps, 0, 0, 1);

		pMaxVals->at(0) = 7;
		pMaxVals->at(1) = 1440 / m_eventCalendar.timeUnit();

		SubtractOneFromNonZeroValues(pMaxVals);
	}
	else if (i == STATS_IMG)
	{
		InitArrays(6, pMinVals, pMaxVals, pSteps, 0, 0, 1);

		pMaxVals->at(0) = 6;
		pMaxVals->at(1) = 23;
		pMaxVals->at(2) = 59;
		pMaxVals->at(3) = 6;
		pMaxVals->at(4) = 23;
		pMaxVals->at(5) = 59;
	}
	else
	{
		VisSysComponent::GetParameterInfo(i, pMinVals, pMaxVals, pSteps);
	}
}

void BehaviorAnalyzer::Run()
{
	// Only work in "online learning" mode
	if (TaskName() != "Offline_learning")
		return;
	
	if (!m_pImgProcessor)
	{
		ShowMissingDependencyError(m_pImgProcessor);
		return;
	}

	if (!m_dbm || !m_dbm.readSessionInfo(m_params.session_id, m_sessionInfo))
	{
		ShowError("There is no session information");
		return;
	}

	if (m_sessionInfo.frame_width == 0 && m_sessionInfo.frame_height == 0)
	{
		m_sessionInfo.frame_width = 640;
		m_sessionInfo.frame_height = 480;
	}
	
	// Create the matrix of time x day for intervals of one minute
	ASSERT(m_data.empty());

	unsigned timeUnit = 15;

	m_data.clear();
	m_data.resize(7, 24 * 60 / timeUnit);

	ShowStatus("Reading traces from DB...");

	// Read the traces from the database and store in the multiscale histogram
	m_dbm.readBlobTraces(m_params.session_id, m_traces);
	
	ShowStatus("Done!");

	ShowStatus("Processing traces...");

	time_t min_timestamp = std::numeric_limits<time_t>::max();
	time_t max_timestamp = std::numeric_limits<time_t>::min();
	
	// Find the maximum and minimum timestamps
	for (auto traceIt = m_traces.begin(); traceIt != m_traces.end(); ++traceIt)
	{
		for (auto blobIt = traceIt->begin(); blobIt != traceIt->end(); ++blobIt)
		{
			const Blob& b = **blobIt;

			if (b.timestamp() < min_timestamp)
				min_timestamp = b.timestamp();

			if (b.timestamp() > max_timestamp)
				max_timestamp = b.timestamp();
		}
	}
	
	m_eventCalendar.init(min_timestamp, max_timestamp, timeUnit);

	unsigned traceId = 0;

	// Collect all the trace data
	for (auto traceIt = m_traces.begin(); traceIt != m_traces.end(); ++traceIt, ++traceId)
	{
		if (traceIt->size() < 2)
			continue;

		// Smooth the trace first
		smoothBlobTrace(*traceIt, m_params.numSmoothIterations);

		auto blobIt1 = traceIt->begin();
		auto blobIt0 = blobIt1++;

		for (; blobIt1 != traceIt->end(); ++blobIt1, ++blobIt0)
		{
			const Blob& b0 = **blobIt0;
			const Blob& b1 = **blobIt1;

			PointArray pts;
			RasterizeLine(b0.centerPoint(), b1.centerPoint(), &pts);
			
			// Push all the points [b0.centroid, b1.centroid). ie, remove last.
			pts.resize(pts.size() - 1);

			m_eventCalendar.add(b0.timestamp(), pts, traceId);

			// For visualization
			Time t(b0.timestamp());
			unsigned minutes = t.hour() * 60 + t.minutes();

			//if (t.weekday() == 1 && t.hour() == 0)
			//	DBG_PRINT5("x", b0.timestamp(), t.hour(), t.minutes(), minutes / timeUnit)

			m_data(t.weekday(), minutes / timeUnit).push_back(b0, b1);
		}
	}

	// Set a 7-day X 2-scale matrix of indices
	// Map each day to its corresponding day index at each scale
	//IrregularScale dayScale(7, 2, 0); // important: init all to zero
	IrregularScale dayScale(7, 1, 0); // important: init all to zero

	for (unsigned i = 0; i < 7; i++)
		dayScale(i, 0) = i;

	/*dayScale(0, 1) = 1;
	dayScale(6, 1) = 1;
	// other indices are set to zero above
	*/

	RegularScale timeScale;

	timeScale.push_back(0);
	/*timeScale.push_back(1);
	timeScale.push_back(2);
	timeScale.push_back(3); // +- 4 times units = 2 hour intervals
	*/

	m_tempPyr.init(m_eventCalendar, dayScale, timeScale,
		m_sessionInfo.frame_width, m_sessionInfo.frame_height, 6, 1);

	/*m_tempPyr.init(m_eventCalendar, dayScale, timeScale,
		m_sessionInfo.frame_width, m_sessionInfo.frame_height, 1, 8);*/

	m_tempPyr.processData(m_params.minProbLikelyEvents, m_params.maxProbUnlikelyEvents);

	ShowMsg("Done!");
}

/*!
	Draws the output of the component. This function is called from
	a "drawing" thread, which is different from the "processing" thread
	that calls Run(). In order to protect the shared resources between the
	threads, a mutex is used.
*/
void BehaviorAnalyzer::Draw(const DisplayInfoIn& dii) const
{	
	
}

/*!	
	Returns the text output for a given an output index 'i'
	with parameter 'param'. This function is called from
	a "drawing" thread, which is different from the "processing" thread
	that calls Run().
*/
void BehaviorAnalyzer::GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const
{
	if (m_sessionInfo.session_id < 0)
		return;

	if (dii.outputIdx == TEMPORAL_SCALE)
	{
		ASSERT(dii.params.size() == 5);

		unsigned dayScale  = (unsigned) dii.params.at(0);
		unsigned timeScale = (unsigned) dii.params.at(1);
		unsigned day       = (unsigned) dii.params.at(2);
		unsigned timeUnit  = (unsigned) dii.params.at(3);
		unsigned cluster   = (unsigned) dii.params.at(4);

		if (dayScale >= m_tempPyr.ni() || timeScale >= m_tempPyr.nj())
			return;

		RGBImg clusterImg;

		if (cluster > 0)
			clusterImg = m_tempPyr.clusterTimeSlots(dayScale, timeScale);

		auto m = m_tempPyr.get(dayScale, timeScale);

		const int di = 75;
		const int dj = 5;

		RGBImg img(m.ni() * di, m.nj() * dj);
		CvMatView mat(img);
		cv::Scalar color;

		img.fill(RGBColor(255, 255, 255));

		for (unsigned i = 0; i < m.ni(); i++)
		{
			for (unsigned j = 0; j < m.nj(); j++)
			{
				if (cluster == 0)
				{
					double pr = m.get(i, j).motionPerTimeUnit / double(m.get(i, j).samples);
					unsigned char c0, c1;
				
					if (pr < 0.5) 
					{
						c0 = (unsigned char)(pr * 255);
						c1 = 0;
					}
					else if (pr < 1)
					{
						c0 = 0;
						c1 = (unsigned char)(pr * 255);
					}
					else
					{
						c0 = 0;
						c1 = 255;
					}

					color = cv::Scalar(c1, c0, 0);

					/*if (m.get(i, j).motionPerTimeUnit != 0)
						color = cv::Scalar(255, 0, 0);
					else
						color = cv::Scalar(0, 0, 255);*/
				}
				else
				{
					auto c = clusterImg(i, j);
					color = cv::Scalar(c.r, c.g, c.b);
				}

				cv::rectangle(mat, cv::Point(i * di, j * dj), cv::Point((i + 1) * di, 
					(j + 1) * dj), color, CV_FILLED);
			}
		}

		if (day < m.ni() && timeUnit < m.nj())
		{
			std::ostringstream oss;

			oss << "DayTimeStats for day " << day << " and time unit " << timeUnit << "\n";

			oss << "motionPerTimeUnit: " << m(day, timeUnit).motionPerTimeUnit << ", "
				<< "anyMotion: " << m(day, timeUnit).anyMotion << ", "
				<< "samples: " << m(day, timeUnit).samples << ", "
				<< "minMotion: " << m(day, timeUnit).minMotion << ", "
				<< "maxMotion: " << m(day, timeUnit).maxMotion << ", "
				<< "traces: " << m(day, timeUnit).traces << ", "
				<< "minTraces: " << m(day, timeUnit).minTraces << ", "
				<< "maxTraces: " << m(day, timeUnit).maxTraces << ".";

			dio.message = oss.str();
		}

		dio.imageType = RGB_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(img);
	}
	else if (dii.outputIdx == SPATIAL_SCALE)
	{
		ASSERT(dii.params.size() == 5);

		unsigned dayScale  = (unsigned) dii.params.at(0);
		unsigned timeScale = (unsigned) dii.params.at(1);
		unsigned day       = (unsigned) dii.params.at(2);
		unsigned timeUnit  = (unsigned) dii.params.at(3);
		unsigned levelIdx  = (unsigned) dii.params.at(4);

		if (dayScale >= m_tempPyr.ni() || timeScale >= m_tempPyr.nj() 
			|| levelIdx >= m_tempPyr.numSpatialLevels())
		{
			return;
		}

		auto m = m_tempPyr.get(day, timeUnit, dayScale, timeScale).spatialPyramid;

		auto pyrLevel = m.get(levelIdx);
		unsigned di   = m.cellWidthInPixels(levelIdx);
		unsigned dj   = m.cellHeightInPixels(levelIdx);

		RGBImg img(m_tempPyr.imageWidth(), m_tempPyr.imageHeight());
		CvMatView mat(img);
		std::ostringstream oss;

		img.fill(RGBColor(0, 0, 0));

		for (unsigned i = 0; i < pyrLevel.ni(); i++)
		{
			for (unsigned j = 0; j < pyrLevel.nj(); j++)
			{
				cv::Point p0(i * di, j * dj);
				cv::Point p1((i + 1) * di, (j + 1) * dj);

				if (p1.x >= (int)img.ni())
					p1.x = img.ni() - 1;

				if (p1.y >= (int)img.nj())
					p1.y = img.nj() - 1;

				if (pyrLevel(i, j).uniqueEvents > 0)
					cv::rectangle(mat, p0, p1, cv::Scalar(255, 255, 255), CV_FILLED);

				cv::rectangle(mat, p0, p1, cv::Scalar(192, 192, 192), 1);

				oss << "(" << i << ", " << j << "): " << pyrLevel(i, j).uniqueEvents << "\n";
			}
		}

		dio.message = oss.str();

		dio.imageType = RGB_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(img);
	}
	else if (dii.outputIdx == HEATMAP_IMG)
	{
		FloatImg heatMap(m_sessionInfo.frame_width, m_sessionInfo.frame_height);

		heatMap.fill(0);
		PointArray pts;

		for (auto traceIt = m_traces.begin(); traceIt != m_traces.end(); ++traceIt)
		{
			if (traceIt->size() < 2)
				continue;

			auto it1 = traceIt->begin();
			auto it0 = it1++;

			for (; it1 != traceIt->end(); ++it1, ++it0)
			{
				const Blob& b0 = **it0;
				const Blob& b1 = **it1;

				RasterizeLine(b0.centerPoint(), b1.centerPoint(), &pts);

				for (unsigned i = 0; i < pts.size(); i++)
				{
					float& val = heatMap((unsigned)pts[i].x, (unsigned)pts[i].y);
					
					if (val < 10)
						val++;
				}
			}
		}

		dio.imageType = FLOAT_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(heatMap);
	}
	else if (dii.outputIdx == TRACES_IMG)
	{
		ASSERT(dii.params.size() == 2);

		unsigned day       = (unsigned) dii.params.at(0);
		unsigned timeUnit  = (unsigned) dii.params.at(1);

		if (day >= m_data.ni() || timeUnit >= m_data.nj())
			return;

		RGBImg image(m_sessionInfo.frame_width, m_sessionInfo.frame_height);

		image.fill(RGBColor(192, 192, 192));

		const SpatialDataList& sdl = m_data(day, timeUnit);

		for (auto dataIt = sdl.begin(); dataIt != sdl.end(); ++dataIt)
			image(dataIt->pt.x, dataIt->pt.y) = RGBColor(0, 0, 0);

		/*unsigned d = m_eventCalendar.timeUnit();

		for (unsigned t = timeUnit * d; t < (timeUnit + 1) * d; t++)
		{
			const SpatialDataList& sdl = m_data(day, t);

			for (auto dataIt = sdl.begin(); dataIt != sdl.end(); ++dataIt)
				image(dataIt->pt.x, dataIt->pt.y) = RGBColor(0, 0, 0);
		}*/

		dio.imageType = RGB_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(image);
	}
	else if (dii.outputIdx == STATS_IMG)
	{
		ASSERT(dii.params.size() == 6);

		int day  = (int) dii.params.at(0);
		int hour = (int) dii.params.at(1);
		int min  = (int) dii.params.at(2);
		int tm   = hour * 60 + min;

		int day_range  = (int) dii.params.at(3);
		int hour_range = (int) dii.params.at(4);
		int min_range  = (int) dii.params.at(5);
		int tm_range   = hour_range * 60 + min_range;

		int day_min = (day - day_range > 0) ? day - day_range : 0;
		int day_max = (day + day_range < 6) ? day + day_range : 6;

		int tm_min = (tm - tm_range > 0) ? tm - tm_range : 0;
		int tm_max = (tm + tm_range < 24 * 60 - 1) ? tm + tm_range : 24 * 60 - 1;

		FloatImg heatMap(m_sessionInfo.frame_width, 
			m_sessionInfo.frame_height);

		heatMap.fill(0);

		for (int d = day_min; d <= day_max; d++)
		{
			for (int m = tm_min; m <= tm_max; m++)
			{
				float& val = heatMap(m, d);
					
				if (val < 10)
					val++;
			}
		}

		/*int day_min = day - day_range;

		const SpatialDataList& sdl = m_data(hour * 60 + min, day);

		for (auto dataIt = sdl.begin(); dataIt != sdl.end(); ++dataIt)
		{
			float& val = heatMap(dataIt->pt.x, dataIt->pt.y);
					
			if (val < 10)
				val++;
		}*/

		dio.imageType = FLOAT_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(heatMap);
	}
	else if (dii.outputIdx == SEGMENT_IMG)
	{
		unsigned dayScale  = (unsigned) dii.params.at(0);
		unsigned timeScale = (unsigned) dii.params.at(1);
		unsigned day       = (unsigned) dii.params.at(2);
		unsigned timeUnit  = (unsigned) dii.params.at(3);

		const SpatialPyramid& spatialPyr = m_tempPyr.get(
			day, timeUnit, dayScale, timeScale).spatialPyramid;

		unsigned spatialLevelIdx = spatialPyr.size() - 1;

		AttributedGraph<TimeSlotData, int> g;

		m_tempPyr.getSlotGraph(g, dayScale, timeScale, spatialLevelIdx);

		auto sz = m_tempPyr.tableSize(dayScale, timeScale);

		NodeArray na(g);

		IntImg labelImg = g.inf(na[m_tempPyr.slotIndex(
			day, timeUnit, dayScale, timeScale)]).labelImg;

		RGBImg img(labelImg.ni(), labelImg.nj());

		auto it0 = labelImg.begin();
		auto it1 = img.begin();

		for (; it0 != labelImg.end(); ++it0, ++it1)
		{
			if (*it0 == SpatialStats::ALWAYS)
				*it1 = RGBColor(255, 0, 0);
			else if (*it0 == SpatialStats::VERY_LIKELY)
				//*it1 = RGBColor(255, 160, 0);
				*it1 = RGBColor(0, 255, 0);
			else if (*it0 == SpatialStats::SOMETIMES)
				*it1 = RGBColor(255, 255, 255);
			else if (*it0 == SpatialStats::VERY_UNLIKELY)
				//*it1 = RGBColor(0, 160, 255);
				*it1 = RGBColor(0, 0, 255);
			else if (*it0 == SpatialStats::NEVER)
				*it1 = RGBColor(0, 0, 0);
				//*it1 = RGBColor(0, 0, 255);
			else
				ASSERT(false);
		}

		unsigned di = spatialPyr.cellWidthInPixels(spatialLevelIdx);
		unsigned dj = spatialPyr.cellHeightInPixels(spatialLevelIdx);

		dio.imageType = RGB_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(ImageZoom(img, di, dj));
	}
}


