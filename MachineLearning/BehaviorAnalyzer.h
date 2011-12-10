/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/VisSysComponent.h>
#include <VideoParser/ImageProcessor.h>
#include <ObjectTracking/BlobTrackerDBManager.h>
#include <Tools/SimpleMatrix.h>
#include <Tools/SpatioTemporalScale.h>
#include <Tools/HelperFunctions.h>

namespace vpl {

/*!
	@brief 
*/
class BehaviorAnalyzer : public VisSysComponent
{
	enum {TEMPORAL_SCALE, SPATIAL_SCALE, HEATMAP_IMG, 
		TRACES_IMG, STATS_IMG, SEGMENT_IMG};

	struct Params
	{
		int session_id;
		unsigned numSmoothIterations;
		double minProbLikelyEvents;
		double maxProbUnlikelyEvents;
	};

	struct SpatialData
	{
		XYCoord pt;

		void set(const XYCoord& coord)
		{
			pt = coord;
		}

		void set(const Point& p)
		{
			pt.x = (int)p.x;
			pt.y = (int)p.y;
		}

		void set(int x, int y)
		{
			pt.x = x;
			pt.y = y;
		}
	};

	struct SpatialDataList : public std::list<SpatialData>
	{
		/*void push_back(const Blob& b)
		{
			SpatialData sd;
			
			sd.set(b.centroid());

			std::list<SpatialData>::push_back(sd);
		}*/

		/*! 
			Pushes all the points [b0.centroid, b1.centroid). ie, the
			b1.centroind is NOT added.
		*/
		void push_back(const Blob& b0, const Blob& b1)
		{
			PointArray pts;
			SpatialData sd;

			RasterizeLine(b0.centerPoint(), b1.centerPoint(), &pts);

			for (unsigned i = 1; i < pts.size(); i++)
			{
				sd.set(pts[i - 1]);
				std::list<SpatialData>::push_back(sd);
			}
		}
	};

	Params m_params;

protected:
	std::shared_ptr<const ImageProcessor> m_pImgProcessor;
	BlobTrackerDBManager m_dbm;

	BlobTraces m_traces;
	SimpleMatrix<SpatialDataList> m_data;

	TrackingSessionInfo m_sessionInfo;
	
	EventCalendar m_eventCalendar;
	DayTimeMultiscaleTable m_tempPyr;

	void ClusterEvents();

public:	
	virtual void Clear()
	{
		VisSysComponent::Clear();

		m_traces.clear();
		m_data.clear();

		m_sessionInfo.clear();
	}

	virtual void ReadParamsFromUserArguments();

	virtual void Initialize(graph::node v);

	virtual std::string ClassName() const
	{
		return "BehaviorAnalyzer";
	}

	virtual StrArray Dependencies() const
	{
		StrArray deps;

		deps.push_back("ImageProcessor");

		return deps;
	}
	
	virtual void Run();

	virtual void Draw(const DisplayInfoIn& dii) const;

	virtual void GetParameterInfo(int i, DoubleArray* pMinVals, 
		DoubleArray* pMaxVals, DoubleArray* pSteps) const;
	
	virtual int NumOutputImages() const 
	{ 
		return 6; 
	}

	virtual std::string GetOutputImageLabel(int i) const
	{
		switch (i)
		{
			case TEMPORAL_SCALE: return "Temporal Scale";
			case SPATIAL_SCALE: return "Spatial Scale";
			case HEATMAP_IMG: return "Trace heat map";
			case TRACES_IMG: return "All traces";
			case STATS_IMG: return "Date time stats";
			case SEGMENT_IMG: return "Segmentation image";
		}

		return "error";
	}

	virtual void GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const;
};

} // namespace vpl

