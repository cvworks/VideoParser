/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <memory>
#include <Tools/ImageUtils.h>
#include <GraphTheory/AttributedGraph.h>
#include <Tools/STLUtils.h>
#include "InputImageInfo.h"

namespace vpl {

class VisSysComponent;

typedef std::shared_ptr<VisSysComponent> VSCPtr;
typedef std::shared_ptr<const VisSysComponent> ConstVSCPtr;

#ifdef USE_LEDA_GRAPH_CLASS
typedef leda::GRAPH<VSCPtr, double> VSCBaseGraph;
#else
typedef AttributedGraph<VSCPtr, double> VSCBaseGraph;
#endif

//! Possible running modes with values defined to perform bitwise combinations
enum RUNNING_MODE {ONLINE_RUNNING_MODE = 1, OFFLINE_RUNNING_MODE = 2, 
	ALL_RUNNING_MODES = 15};

/*!
*/
struct SharedComponentData
{
	bool antiNoiseMode;        //!< True if anti-noise algorithms should be applied

	SharedComponentData()
	{
		antiNoiseMode = false;
	}
};

/*!
	VSC Graph with very general member variables and functions.
*/
class GenericVSCGraph : public VSCBaseGraph
{
protected:
	RUNNING_MODE m_runningMode;  //!< The current running mode
	std::string m_taskName;      //!< Name of the task to perform
	time_t m_commonTimestamp;    //!< The current timestamp common to all components
	std::string m_videoFilename; //!< File path used to load the video
	InputImageInfo m_inImgInfo;  //!< Info of the current frame being processed
	std::shared_ptr<SharedComponentData> m_sharedCompData;

public:
	GenericVSCGraph() : m_sharedCompData(new SharedComponentData)
	{

	}

	GenericVSCGraph& operator=(const GenericVSCGraph& rhs)
	{
		m_runningMode = rhs.m_runningMode;
		m_taskName = rhs.m_taskName;
		m_commonTimestamp = rhs.m_commonTimestamp;
		m_videoFilename = rhs.m_videoFilename;
		m_inImgInfo = rhs.m_inImgInfo;
		*m_sharedCompData = *rhs.m_sharedCompData;

		return *this;
	}

	void SetAntiNoiseMode(bool mode) const
	{
		m_sharedCompData->antiNoiseMode = mode;
	}

	bool GetAntiNoiseMode() const
	{
		return m_sharedCompData->antiNoiseMode;
	}

	void SetRunningMode(RUNNING_MODE mode)
	{
		m_runningMode = mode;
	}

	RUNNING_MODE GetRunningMode() const
	{
		return m_runningMode;
	}

	//! Sets the file path used to load the video
	void SetVideoFilename(std::string strFilename)
	{
		m_videoFilename = strFilename;
	}

	//! Gets the file path used to load the video
	const std::string& GetVideoFilename() const
	{
		return m_videoFilename;
	}

	fnum_t GetFrameNumber() const
	{
		return m_inImgInfo.frameNumber;
	}

	std::string TaskName() const
	{
		return m_taskName;
	}

	void SetTimestamp(time_t t)
	{
		m_commonTimestamp = t;
	}

	time_t GetTimestamp() const
	{
		return m_commonTimestamp;
	}

	const InputImageInfo& GetInputImageInfo() const
	{
		return m_inImgInfo;
	}
};

/*!
	Basic information about the output of a component
*/
struct OutputImageInfo
{
	VSCPtr component;
	std::string label;
	int index;

	void operator=(const OutputImageInfo& rhs)
	{
		component = rhs.component;
		label = rhs.label;
		index = rhs.index;
	}
};

/*!
*/
struct OutputImageParam
{
	int index;
	DoubleArray params;

	void operator=(const OutputImageParam& rhs)
	{
		index = index;
		params = rhs.params;
	}

	friend std::ostream& operator<<(std::ostringstream& os, const OutputImageParam& oip)
	{ 
		// @TODO fix this!
		double param = (oip.params.empty()) ? 0 : oip.params.front();

		os << " (" << oip.index << ", " << param << ")";

		return os; 
	}

	friend std::istream& operator>>(std::istream &is, OutputImageParam& oip)
	{ 
		char a, b, c;
		double param;

		is >> a >> oip.index >> b >> param >> c;
		
		if (a != '(' || b != ',' || c != ')')
			THROW_BASIC_EXCEPTION("Invalid syntax when reading OutputImageParam");

		// @TODO fix this!
		oip.params.resize(1, param);

		return is; 
	}
};

/*!
	Information about the current status of the display. When multiple
	displays are available, their id's are specified in 'displayId'. This
	can be used to send/draw different outputs as a function of the display.
*/
struct DisplayInfoIn
{
	int outputIdx;      //!< Index of the vision component output selected
	DoubleArray params; //!< Parameters for the vision component selected
	int displayId;      //!< ID of the display making the output request

	DisplayInfoIn() 
	{ 
		// The display ID is optional, so set a default value
		outputIdx = -1;
		displayId = 0; 
	}

	DisplayInfoIn(const DisplayInfoIn& rhs)
	{
		operator=(rhs);
	}

	void Clear()
	{
		outputIdx = -1;
		params.clear();
		displayId = 0;
	}

	void operator=(const DisplayInfoIn& rhs)
	{
		outputIdx = rhs.outputIdx;
		params = rhs.params;
		displayId = rhs.displayId;
	}
};

/*!
	Specifications about how to display the output
	of the component.
*/
struct DisplaySpecs
{
	bool draggableContent;
	bool zoomableContent;

	DisplaySpecs()
	{
		Clear();
	}

	void Clear()
	{
		draggableContent = true;
		zoomableContent = true;
	}
};

/*!
	Information about what the component wants to display.
*/
struct DisplayInfoOut
{
	ImageType imageType;
	BaseImgPtr imagePtr;
	std::string message;
	bool syncDisplayViews;
	DisplaySpecs specs;

	DisplayInfoOut()
	{
		imageType = VOID_IMAGE;
		syncDisplayViews = false;
	}

	void Clear()
	{
		imageType = VOID_IMAGE;
		imagePtr = BaseImgPtr();
		message.clear();
		syncDisplayViews = false;
		specs.Clear();
	}

	void operator=(const DisplayInfoOut& rhs)
	{
		imageType = rhs.imageType;
		imagePtr = rhs.imagePtr;
		message = rhs.message;
		syncDisplayViews = rhs.syncDisplayViews;
		specs = rhs.specs;
	}
};

/*!
	Information describing a user event and its
	associated parameters.
*/
struct UserEventInfo : public DisplayInfoIn
{
	int id;
	int value;
	Point coord;

	UserEventInfo(int eventID, int val) : coord(-1, -1)
	{ 
		id = eventID;
		value = val;
	}

	UserEventInfo(const DisplayInfoIn& dii, int eventID, int val, 
		Point pt = Point(0,0)) : DisplayInfoIn(dii)
	{
		id = eventID;
		value = val;
		coord = pt;
	}

	void operator=(const UserEventInfo& rhs)
	{
		DisplayInfoIn::operator=(rhs);

		id = rhs.id;
		value = rhs.value;
		coord = rhs.coord;
	}
};

} // vpl namespcace