/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <set>

#include "SimpleMatrix.h"
#include "ImageUtils.h"
//#include "STL2DArray.h"
#include "Time.h"
#include "BasicTypes.h"
#include "STLUtils.h"
#include <ImageSegmentation/FHSegmenter/FHGraphSegmenter.h>
#include <GraphTheory/LemonGraph.h>
#include <GraphTheory/AttributedGraph.h>

namespace vpl {

//! Array of scale "deltas". scale index --> delta
typedef std::vector<unsigned> RegularScale;

/*! 
	Matrix mapping each index in I to a corresponding scale in S and index' in I'. The matrix
    has size |I| x |S|, and the elements of the matrix are values in I'.
*/
typedef SimpleMatrix<unsigned> IrregularScale;

//! List of the coordinates where the motion events were detected
struct MotionEventSet
{
	PointList coords;
	std::set<unsigned> traceIds;
};

/*!
*/
struct SpatialStats
{
	//! The relative order of the labels matters
	enum LABEL {ALWAYS, VERY_LIKELY, SOMETIMES, VERY_UNLIKELY, NEVER};

	unsigned totalEvents;
	unsigned uniqueEvents;
	LABEL label;
	bool visited;

	SpatialStats()
	{
		totalEvents = 0;
		uniqueEvents = 0;
		label = SOMETIMES;
		visited;
	}

	/*void operator+=(const SpatialStats& rhs)
	{
		events += rhs.events;
	}*/
};

struct MotionEvent
{
	time_t timestamp;
	Point coord;

	MotionEvent() 
	{ 
	}

	MotionEvent(time_t t, Point c) : timestamp(t), coord(c)
	{
	}
};

struct TimeSlotData
{
	IntImg labelImg;

	void operator=(const TimeSlotData& rhs)
	{
		labelImg = rhs.labelImg;
	}
};

/*!
*/
class SpatialPyramid : public std::vector<SimpleMatrix<SpatialStats>>
{
protected:
	unsigned m_imageWidth;
	unsigned m_imageHeight;

	std::vector<std::vector<unsigned>> m_labelCounters;

public:
	typedef SimpleMatrix<SpatialStats> SpatialLevel;
	typedef std::vector<SimpleMatrix<SpatialStats>> ParentClass;

	const SpatialLevel& get(unsigned i) const { return operator[](i); }
	SpatialLevel& get(unsigned i) { return operator[](i); }

	SpatialPyramid()
	{
	}

	SpatialPyramid(SpatialPyramid&& rhs) 
		: ParentClass(rhs), m_labelCounters(rhs.m_labelCounters)
	{
		m_imageWidth = rhs.m_imageWidth;
		m_imageHeight = rhs.m_imageHeight;
	}

	SpatialPyramid(unsigned ni, unsigned nj, unsigned levels)
	{
		init(ni, nj, levels);
	}
	
	void operator=(const SpatialPyramid& rhs)
	{
		ParentClass::operator=(rhs);

		m_labelCounters = rhs.m_labelCounters;

		m_imageWidth = rhs.m_imageWidth;
		m_imageHeight = rhs.m_imageHeight;
	}

	void init(unsigned ni, unsigned nj, unsigned levels)
	{
		ASSERT(empty());
		ASSERT(m_labelCounters.empty());
		ASSERT(ni > 0 && nj > 0 && levels > 0);

		m_imageWidth = ni;
		m_imageHeight = nj;

		resize(levels);

		m_labelCounters.resize(levels); // nested arrays are init to empty

		// Set the number of elements in each level of the pyramid
		unsigned delta = 1;

		for (auto it = begin(); it != end(); ++it)
		{
			it->resize(delta, delta);
			delta *= 2;
		}
	};

	unsigned imageWidth() const
	{
		return m_imageWidth;
	}

	unsigned imageHeight() const
	{
		return m_imageHeight;
	}

	unsigned ni(unsigned level) const
	{
		return get(level).ni();
	}

	unsigned nj(unsigned level) const
	{
		return get(level).nj();
	}

	unsigned cellWidthInPixels(unsigned level) const
	{
		return imageWidth() / ni(level);
	}

	unsigned cellHeightInPixels(unsigned level) const
	{
		return imageHeight() / nj(level);
	}

	void resetVisitedFlags()
	{
		for (auto it0 = begin(); it0 != end(); ++it0)
			for (auto it1 = it0->begin(); it1 != it0->end(); ++it1)
				it1->visited = false;
	}

	void add(const MotionEventSet& mes)
	{
		double cx, cy;
		unsigned twoToLevel = 1; // stores pow(2, level)

		resetVisitedFlags();

		for (unsigned level = 0; level < size(); level++)
		{
			for (auto it = mes.coords.begin(); it != mes.coords.end(); ++it)
			{
				cx = (it->x * twoToLevel) / m_imageWidth;
				cy = (it->y * twoToLevel) / m_imageHeight;

				SpatialStats& ss = get(level).get(unsigned(cx), unsigned(cy));

				ss.totalEvents++;

				if (!ss.visited)
				{
					ss.visited = true;
					ss.uniqueEvents++;
				}
			}

			twoToLevel *= 2;
		}
	}

	/*!
		Assign labels as follows. If probability (event/samples) is 1, the label is ALWAYS. If it
		is 0, the label is NEVER. If the probability is greater or equal than tau_likely, the label is
		VERY_LIKELY, while if it is smaller or equal than tau_unlikely, the label is VERY_UNLIKELY. Finally,
		if the probability is in [tau_likely, tau_unlikely], the label is SOMETIMES.
	*/
	void assignLabels(double samples, const double& tau_likely, const double& tau_unlikely)
	{
		double pr;

		for (unsigned k = 0; k < size(); k++)
		{
			SpatialPyramid::SpatialLevel& levelImg = get(k);

			for (unsigned i = 0; i < levelImg.ni(); i++)
			{
				for (unsigned j = 0; j < levelImg.nj(); j++)
				{
					pr = levelImg(i, j).uniqueEvents / samples;

					ASSERT_UNIT_INTERVAL(pr);

					if (pr == 1)
						levelImg(i, j).label = SpatialStats::ALWAYS;
					else if (pr == 0)
						levelImg(i, j).label = SpatialStats::NEVER;
					else if (pr >= tau_likely)
						levelImg(i, j).label = SpatialStats::VERY_LIKELY;
					else if (pr <= tau_unlikely)
						levelImg(i, j).label = SpatialStats::VERY_UNLIKELY;
					else
						levelImg(i, j).label = SpatialStats::SOMETIMES;
				}
			}
		}
	}

	void setLabelCounters()
	{
		for (unsigned k = 0; k < size(); k++)
		{
			const SpatialPyramid::SpatialLevel& img = get(k);

			for (auto it = img.begin(); it != img.end(); ++it)
				incrementLabelCounter(k, it->label);
		}
	}

	/*!
		Increments by one the counter of label "label" at level "level".

		The label counter array is resized s.t. the counter of "label" is
		at position "label".
	*/
	void incrementLabelCounter(unsigned level, unsigned label)
	{
		ASSERT(level <= size());

		std::vector<unsigned>& counter = m_labelCounters[level];

		if (label >= counter.size())
			counter.resize(label + 1, 0);

		counter[label]++;
	}

	unsigned labelCount(unsigned level, unsigned label) const
	{
		ASSERT(level < size());

		const std::vector<unsigned>& counter = m_labelCounters[level];

		return (label< counter.size()) ? counter[label] : 0;
	}

	unsigned bottomLabelCount(unsigned label) const
	{
		return (label < m_labelCounters.back().size()) ? m_labelCounters.back()[label] : 0;
	}

	/*!
		Initializes the pyramid as the merge of the given pair of 
		pyramids.
	*/
	void initFromMerge(const SpatialPyramid& sp0, const SpatialPyramid& sp1)
	{
		ASSERT(!sp0.empty() && !sp1.empty());

		ASSERT(sp0.imageWidth() == sp1.imageWidth() && 
			   sp0.imageHeight() == sp1.imageHeight() &&
			   sp0.size() == sp1.size());

		init(sp0.imageWidth(), sp0.imageHeight(), sp0.size());

		int l0, l1;

		for (unsigned k = 0; k < size(); k++)
		{
			const SpatialPyramid::SpatialLevel& img0 = sp0.get(k);
			const SpatialPyramid::SpatialLevel& img1 = sp1.get(k);
			SpatialPyramid::SpatialLevel& img2 = get(k);

			SpatialStats::LABEL lbl;

			for (unsigned i = 0; i < img2.ni(); i++)
			{
				for (unsigned j = 0; j < img2.nj(); j++)
				{
					l0 = img0(i, j).label;
					l1 = img1(i, j).label;

					if ((l0 <= SpatialStats::SOMETIMES && l1 >= SpatialStats::SOMETIMES) ||
						(l0 >= SpatialStats::SOMETIMES && l1 <= SpatialStats::SOMETIMES))
					{
						lbl = SpatialStats::SOMETIMES;
					}
					else if (l0 == SpatialStats::ALWAYS && l1 == SpatialStats::ALWAYS)
					{
						lbl = SpatialStats::ALWAYS;
					}
					else if (l0 == SpatialStats::NEVER && l1 == SpatialStats::NEVER)
					{
						lbl = SpatialStats::NEVER;
					}
					else if (l0 < SpatialStats::SOMETIMES && l1 < SpatialStats::SOMETIMES)
					{
						lbl = SpatialStats::VERY_LIKELY;
					}
					else if (l0 > SpatialStats::SOMETIMES && l1 > SpatialStats::SOMETIMES)
					{
						lbl = SpatialStats::VERY_UNLIKELY;
					}
					else
					{
						ASSERT(false);
						lbl = SpatialStats::SOMETIMES;
					}

					img2(i, j).label = lbl;

					incrementLabelCounter(k, lbl);
				}
			}
		}
	}

	/*void add(const MotionEvent& me)
	{
		double cx, cy;
		unsigned twoToLevel = 1; // stores pow(2, level)

		for (unsigned level = 0; level < size(); level++)
		{
			cx = (me.coord.x * twoToLevel) / m_imageWidth;
			cy = (me.coord.y * twoToLevel) / m_imageHeight;

			get(level).get(unsigned(cx), unsigned(cy)) += me.stats;

			twoToLevel *= 2;
		}
	}*/
};

class EventCalendar
{
protected:
	//time_t m_min_timestamp;
	//time_t m_max_timestamp;

	unsigned m_firstTimeSlotIdx;
	unsigned m_lastTimeSlotIdx;

	unsigned m_timeUnit; //!< Number of minutes in a time unit

	std::vector<MotionEventSet> m_calendar;

public:
	EventCalendar()
	{ 
		m_timeUnit = 1; 
	}

	void init(time_t min_timestamp, time_t max_timestamp, unsigned timeUnit)
	{
		m_timeUnit = timeUnit;
		//m_min_timestamp = min_timestamp;
		//m_max_timestamp = max_timestamp;

		// Truncate the min and max
		m_firstTimeSlotIdx = unsigned(min_timestamp / 60) / m_timeUnit;
		m_lastTimeSlotIdx  = unsigned(max_timestamp / 60) / m_timeUnit;

		m_calendar.clear();
		m_calendar.resize(m_lastTimeSlotIdx - m_firstTimeSlotIdx + 1);
	}

	//! Converts index to its corresponding absolute timestamp.
	time_t toTime(unsigned i) const
	{
		return (i + m_firstTimeSlotIdx) * m_timeUnit * 60;
	}

	//! @param timestamp is an absolute time in seconds 
	unsigned toIndex(time_t timestamp) const
	{
		unsigned absIdx = unsigned(timestamp / 60) / m_timeUnit;

		ASSERT(absIdx >= m_firstTimeSlotIdx && absIdx <= m_lastTimeSlotIdx);

		return absIdx - m_firstTimeSlotIdx;
	}

	MotionEventSet& array_get(unsigned i)
	{
		return m_calendar[i];
	}

	const MotionEventSet& array_get(unsigned i) const
	{
		return m_calendar[i];
	}

	MotionEventSet& time_get(time_t absoluteTime)
	{
		return m_calendar[toIndex(absoluteTime)];
	}

	const MotionEventSet& time_get(time_t absoluteTime) const
	{
		return m_calendar[toIndex(absoluteTime)];
	}

	void add(time_t timestamp, const PointArray& coords, unsigned traceId)
	{
		MotionEventSet& mes = time_get(timestamp);

		mes.traceIds.insert(traceId);

		for (auto it = coords.begin(); it != coords.end(); ++it)
			mes.coords.push_back(*it);
	}

	unsigned timeUnit() const
	{
		return m_timeUnit;
	}

	/*time_t minTimestamp() const
	{
		return m_min_timestamp;
	}

	time_t maxTimestamp() const
	{
		return m_max_timestamp;
	}*/

	unsigned size() const
	{
		return m_calendar.size();
	}
};

struct DayTimeMerge
{
	SpatialPyramid spatialPyramid;
	bool redundant;
	unsigned day0, time0, day1, time1;

	DayTimeMerge(unsigned i0, unsigned j0, unsigned i1, unsigned j1,
		const SpatialPyramid& sp) : spatialPyramid(sp)
	{
		redundant = false;
		day0 = i0;
		time0 = j0;
		day1 = i1;
		time1 = j1;
	}

	/*DayTimeMerge(DayTimeMerge&& rhs) : spatialPyramid(rhs.spatialPyramid)
	{
		redundant = rhs.redundant;
	}*/

	DayTimeMerge(const DayTimeMerge& rhs) : spatialPyramid(rhs.spatialPyramid)
	{
		redundant = rhs.redundant;
		day0      = rhs.day0;
		time0     = rhs.time0;
		day1      = rhs.day1;
		time1     = rhs.time1;
	}

	void operator=(const DayTimeMerge& rhs)
	{
		spatialPyramid = rhs.spatialPyramid;

		redundant = rhs.redundant;
		day0      = rhs.day0;
		time0     = rhs.time0;
		day1      = rhs.day1;
		time1     = rhs.time1;
	}
};

/*!
*/
struct DayTimeStats
{
	unsigned samples;

	unsigned motionPerTimeUnit;
	unsigned anyMotion;
	unsigned minMotion;
	unsigned maxMotion;

	SpatialPyramid spatialPyramid;
	bool spatialPyramidLabeled;

	std::list<DayTimeMerge> merges;

	unsigned traces;
	unsigned minTraces;
	unsigned maxTraces;

	DayTimeStats()
	{
		samples = 0;
		spatialPyramidLabeled = false;

		motionPerTimeUnit = 0;
		anyMotion = 0;
		minMotion = std::numeric_limits<unsigned>::max();
		maxMotion = std::numeric_limits<unsigned>::min();

		traces = 0;
		minTraces = std::numeric_limits<unsigned>::max();
		maxTraces = std::numeric_limits<unsigned>::min();
	}

	/*!
		Assign labels as follows. If probability (event/samples) is 1, the label is ALWAYS. If it
		is 0, the label is NEVER. If the probability is greater or equal than tau_likely, the label is
		VERY_LIKELY, while if it is smaller or equal than tau_unlikely, the label is VERY_UNLIKELY. Finally,
		if the probability is in [tau_unlikely, tau_likely], the label is SOMETIMES.
	*/
	void assignLabels(const double& tau_likely, const double& tau_unlikely)
	{
		spatialPyramidLabeled = true;

		spatialPyramid.assignLabels(samples, tau_likely, tau_unlikely);
	}

	/*DayTimeStats computeMerge(const DayTimeStats& rhs) const
	{
		ASSERT(spatialPyramidLabeled && rhs.spatialPyramidLabeled);


	}*/

	IntImg getLabelImage(unsigned levelIdx) const
	{
		ASSERT(spatialPyramidLabeled);

		const SpatialPyramid::SpatialLevel& levelImg = spatialPyramid.get(levelIdx);
		IntImg labelImg(levelImg.ni(), levelImg.nj());

		for (unsigned i = 0; i < levelImg.ni(); i++)
			for (unsigned j = 0; j < levelImg.nj(); j++)
				labelImg(i, j) = levelImg(i, j).label;

		return labelImg;
	}

	/*!
		
	*/
	void add(const MotionEventSet& mes)
	{
		samples++;

		// If the coords is empty, it means that there is no motion
		if (mes.coords.empty())
			return;

		motionPerTimeUnit++;
			
		anyMotion += mes.coords.size();
			
		traces += mes.traceIds.size();

		spatialPyramid.add(mes);

		// Min-max motion
		if (mes.coords.size() < minMotion)
			minMotion = mes.coords.size();

		if (mes.coords.size() > maxMotion)
			maxMotion = mes.coords.size();

		// Min-max traces
		if (mes.traceIds.size() < minTraces)
			minTraces = mes.traceIds.size();

		if (mes.traceIds.size() > maxTraces)
			maxTraces = mes.traceIds.size();
	}
};

/*!
*/
class DayTimeTable : public SimpleMatrix<DayTimeStats>
{
	typedef SimpleMatrix<DayTimeStats> ParentClass;

	void resize(unsigned, unsigned);
	void resize(unsigned, unsigned, const DayTimeStats&);

	std::list<DayTimeMerge> m_salientMerges;

public:
	void init(unsigned ni_day, unsigned nj_time, unsigned ni_img, unsigned nj_img, 
		unsigned spatialPyramidLevels)
	{
		ParentClass::resize(ni_day, nj_time);

		for (auto it = begin(); it != end(); ++it)
			it->spatialPyramid.init(ni_img, nj_img, spatialPyramidLevels);
	}

	/*!
		Should be called once all the data has been collected

		Assign labels as follows. If probability (event/samples) is 1, the label is ALWAYS. If it
		is 0, the label is NEVER. If the probability is greater or equal than tau_likely, the label is
		VERY_LIKELY, while if it is smaller or equal than tau_unlikely, the label is VERY_UNLIKELY. Finally,
		if the probability is in [tau_unlikely, tau_likely], the label is SOMETIMES.
	*/
	void assignLabels(const double& tau_likely, const double& tau_unlikely)
	{
		for (auto it = begin(); it != end(); ++it)
			it->assignLabels(tau_likely, tau_unlikely);
	}

	void mergeEvents(unsigned i0, unsigned j0, unsigned i, unsigned j,
		SimpleMatrix<SpatialPyramid>& cm, SimpleMatrix<unsigned char>& rm) const
	{
		const SpatialPyramid& sp_ij = get(i, j).spatialPyramid;

		ASSERT(i0 != i || j0 != j);

		unsigned i1, j1;

		unsigned next_row = (i0 + 1 < ni()) ? i0 + 1 : 0;

		if (i == i0 || (i == next_row && j < j0)) // second condition means wrap around time slots
		{
			// Go to the previous time slot (might involve changing the day)
			if (j > 0)
			{
				i1 = i;     // same day
				j1 = j - 1; // previous time slot
			}
			else // j == 0, so go to previous day and last time slot
			{
				i1 = (i > 0) ? i - 1 : ni() - 1; // previous day
				j1 = nj() - 1;                   // last time slot in that day
			}

			// Just merge with the previous column
			cm(i, j).initFromMerge(sp_ij, cm(i1, j1));

			// If the parent slot has the same number of "sometimes" labels, 
			// then it is redundant because the new marge has larger time span
			if (cm(i, j).bottomLabelCount(SpatialStats::SOMETIMES) == 
				cm(i1, j1).bottomLabelCount(SpatialStats::SOMETIMES))
			{
				rm(i1, j1) = true;
			}
		}
		else if (j == j0) 
		{
			// i == 0 means that previous is a wrap around days
			i1 = (i > 0) ? i - 1 : ni() - 1;
			j1 = j;

			// Just merge with the previous row
			cm(i, j).initFromMerge(sp_ij, cm(i1, j1));

			// If the parent slot has the same number of "sometimes" labels, 
			// then it is redundant because the new marge has larger time span
			if (cm(i, j).bottomLabelCount(SpatialStats::SOMETIMES) == 
				cm(i1, j1).bottomLabelCount(SpatialStats::SOMETIMES))
			{
				rm(i1, j1) = true;
			}
		}
		else
		{
			// Merge with the previous row and with the previous column
			

			// Go to the previous time slot (might involve changing the day)
			if (j > 0)
			{
				i1 = i;     // same day
				j1 = j - 1; // previous time slot
			}
			else // j == 0, so go to previous day and last time slot
			{
				i1 = (i > 0) ? i - 1 : ni() - 1; // previous day
				j1 = nj() - 1;                   // last time slot in that day
			}

			SpatialPyramid sp_aux;
			
			sp_aux.initFromMerge(sp_ij, cm(i1, j1));

			// If the parent slot has the same number of "sometimes" labels, 
			// then it is redundant because the new marge has larger time span
			if (sp_aux.bottomLabelCount(SpatialStats::SOMETIMES) == 
				cm(i1, j1).bottomLabelCount(SpatialStats::SOMETIMES))
			{
				rm(i1, j1) = true;
			}

			// Now go to previous day and the same time slot
			i1 = (i > 0) ? i - 1 : ni() - 1;
			j1 = j;

			cm(i, j).initFromMerge(sp_aux, cm(i1, j1));

			// If the parent slot has the same number of "sometimes" labels, 
			// then it is redundant because the new marge has larger time span
			if (cm(i, j).bottomLabelCount(SpatialStats::SOMETIMES) == 
				cm(i1, j1).bottomLabelCount(SpatialStats::SOMETIMES))
			{
				rm(i1, j1) = true;
			}
		}
	}

	void mergeEvents(unsigned i0, unsigned j0)
	{
		SimpleMatrix<SpatialPyramid> cumulativeMerges(ni(), nj());
		SimpleMatrix<unsigned char> redundantMerges(ni(), nj(), false);
		unsigned i1, j1;

		DayTimeStats& dts0 = get(i0, j0);

		// Init the base case of the cumulative process
		cumulativeMerges(i0, j0) = dts0.spatialPyramid;
		cumulativeMerges(i0, j0).setLabelCounters();

		for (unsigned di = 0; di < ni(); di++) // days
		{
			for (unsigned dj = 0; dj < nj(); dj++) // time slots
			{
				if (di == 0 && dj == 0)
					continue;

				// Days and times are circular, but the time also advances the day
				i1 = i0 + di;
				j1 = j0 + dj;

				if (j1 >= nj()) // if past midnight, restart on the next day
				{
					j1 -= nj();
					i1++;
				}

				if (i1 >= ni()) // treat days as circular list
					i1 -= ni();

				mergeEvents(i0, j0, i1, j1, cumulativeMerges, redundantMerges);
			}
		}

		unsigned numPix = dts0.spatialPyramid.back().ni() * dts0.spatialPyramid.back().nj();

		for (i1 = 0; i1 < ni(); i1++) // days
		{
			for (j1 = 0; j1 < nj(); j1++) // time slots
			{
				if (!redundantMerges(i1, j1) && !cumulativeMerges(i1, j1).empty() 
					&& cumulativeMerges(i1, j1).bottomLabelCount(SpatialStats::SOMETIMES) * 2 < numPix)
				{
					dts0.merges.push_back(DayTimeMerge(i0, j0, i1, j1, cumulativeMerges(i1, j1)));
				}
			}
		}
	}

	void mergeEvents()
	{
		DBG_LINE

		for (unsigned i = 0; i < ni(); i++)
			for (unsigned j = 0; j < nj(); j++)
				mergeEvents(i, j);

		DBG_LINE
		// todo: eliminate redundant events

		m_salientMerges.clear();

		for (unsigned i = 0; i < ni(); i++)
			for (unsigned j = 0; j < nj(); j++)
				for (auto it = get(i, j).merges.begin(); it != get(i, j).merges.end(); ++it)
					if (!it->redundant)
						m_salientMerges.push_back(*it);

		DBG_LINE
	}
};

/*!
*/
class DayTimeMultiscaleTable
{
protected:
	SimpleMatrix<DayTimeTable> m_levels;

	IrregularScale m_dayScale;
	RegularScale m_timeScale;
	
	unsigned m_numSpatialLevels;
	unsigned m_imgWidth;
	unsigned m_imgHeight;

	static std::vector<RGBColor> s_colors;

protected:
	/*!
		@param ssr is optional.
	*/
	void add(unsigned day, unsigned minutes, const MotionEventSet& mes)
	{
		int ct, dt, T;
		unsigned d;

		for (unsigned ds = 0; ds < m_levels.ni(); ds++) // day scale index
		{
			for (unsigned ts = 0; ts < m_levels.nj(); ts++) // time scale index
			{
				DayTimeTable& dtt = m_levels(ds, ts);
				d = m_dayScale(day, ds);

				ASSERT(d < dtt.ni() && minutes < dtt.nj());

				dt = (int) m_timeScale[ts];
				T = dtt.nj();
	
				for (int t = int(minutes) - dt; t <= int(minutes) + dt; t++)
				{
					// Convert t to a "circular time" ct
					if (t < 0) ct = t + T;
					else if (t >= T) ct = t - T;
					else ct = t;

					dtt(d, ct).add(mes);
				}
			}
		}
	}

public:
	DayTimeMultiscaleTable()
	{
		
	}

	DayTimeMultiscaleTable(const EventCalendar& eventCalendar, 
		const IrregularScale& dayScale, const RegularScale& timeScale,
		unsigned ni_img, unsigned nj_img, unsigned spatialPyramidLevels) 
	{
		init(eventCalendar, dayScale, timeScale, ni_img, nj_img, 
			spatialPyramidLevels);
	}

	void init(const EventCalendar& eventCalendar, 
		const IrregularScale& dayScale, const RegularScale& timeScale, 
		unsigned ni_img, unsigned nj_img, unsigned spatialPyramidLevels)
	{
		ASSERT(m_levels.empty());

		m_numSpatialLevels = spatialPyramidLevels;
		m_imgWidth = ni_img;
		m_imgHeight = nj_img;

		unsigned ni = dayScale.ni(); // number of days (usually 7)
		unsigned nj = 24 * 60 / eventCalendar.timeUnit();

		m_dayScale = dayScale;
		m_timeScale = timeScale;

		m_levels.resize(dayScale.nj(), timeScale.size());

		// Iterate over all "day scales" (ds) and "time scales" (ts), and
		// find out the number of rows and columns needed at each scale
		for (unsigned ds = 0; ds < m_levels.ni(); ds++)
		{
			for (unsigned ts = 0; ts < m_levels.nj(); ts++)
			{
				m_levels(ds, ts).init(dayScale.maxj(ds) + 1, nj, ni_img, nj_img, 
					spatialPyramidLevels);
			}
		}

		// Add the set of motion events in each cell. There might be no motion
		// for a cell, but it must still be added to increase the count of samples
		for (unsigned i = 0; i < eventCalendar.size(); i++)
		{
			const MotionEventSet& mes = eventCalendar.array_get(i);

			time_t t0 = eventCalendar.toTime(i);

			Time t = eventCalendar.toTime(i);

			const int minutes = t.hour() * 60 + t.minutes();

			//if (t.weekday() == 1 && t.hour() == 0)
			//	DBG_PRINT5("y", eventCalendar.toTime(i), t.hour(), t.minutes(), minutes / eventCalendar.timeUnit())

			add(t.weekday(), minutes / eventCalendar.timeUnit(), mes);
		}
	}

	/*!
		Should be called once all the data has been collected. It assigns labels and merges events.

		Assign labels as follows. If probability (event/samples) is 1, the label is ALWAYS. If it
		is 0, the label is NEVER. If the probability is greater or equal than tau_likely, the label is
		VERY_LIKELY, while if it is smaller or equal than tau_unlikely, the label is VERY_UNLIKELY. Finally,
		if the probability is in [tau_unlikely, tau_likely], the label is SOMETIMES.
	*/
	void processData(const double& tau_likely, const double& tau_unlikely)
	{
		for (auto it = m_levels.begin(); it != m_levels.end(); ++it)
		{
			it->assignLabels(tau_likely, tau_unlikely);
			it->mergeEvents();
		}
	}

	unsigned numSpatialLevels() const
	{
		return m_numSpatialLevels;
	}

	unsigned imageWidth() const
	{
		return m_imgWidth;
	}

	unsigned imageHeight() const
	{
		return m_imgHeight;
	}

	unsigned ni() const 
	{ 
		return m_levels.ni(); 
	}

	unsigned nj() const 
	{ 
		return m_levels.nj(); 
	}

	/*void addMotionEvent(const MotionEvent& me)
	{
		ASSERT(me.timestamp >= m_min_timestamp && me.timestamp <= m_max_timestamp);

		double relativeMinutes = (me.timestamp - m_min_timestamp) / 60.0;
		unsigned idx = unsigned(relativeMinutes / m_timeUnit);

		m_calendar[idx].push_back(me.coord);

		DayTimeStats dts;

		if (!m_calendar[idx])
		{
			m_calendar[idx] = true;
			dts.motionPerTimeUnit = 1;
		}

		dts.anyMotion = 1;

		Time t(me.timestamp);
		const int minutes = t.hour() * 60 + t.minutes();

		add(t.weekday(), minutes / m_timeUnit, dts, &me);
	}*/

	float compare(const DayTimeStats& lhs, const DayTimeStats& rhs) const
	{
		bool hasMotion0 = lhs.motionPerTimeUnit != 0;
		bool hasMotion1 = rhs.motionPerTimeUnit != 0;

		return (hasMotion0 == hasMotion1) ? 1.0f : 0.0f;
	}

	std::pair<unsigned, unsigned> tableSize(unsigned dayScaleIdx, 
		unsigned timeScaleIdx) const
	{
		const DayTimeTable& m = get(dayScaleIdx, timeScaleIdx);

		return std::make_pair(m.ni(), m.nj());
	}

	unsigned slotIndex(unsigned day, unsigned time, 
		unsigned dayScaleIdx, unsigned timeScaleIdx) const
	{
		const DayTimeTable& m = get(dayScaleIdx, timeScaleIdx);

		ASSERT(day < m.ni() && time < m.nj());

		return time * m.ni() + day;
	}

	void getSlotGraph(AttributedGraph<TimeSlotData, int>& g, 
		unsigned dayScaleIdx, unsigned timeScaleIdx, unsigned spatialLevelIdx) const
	{
		g.clear();
		g.set_directed(false);

		const DayTimeTable& m = get(dayScaleIdx, timeScaleIdx);
		const int width = m.ni();
		const int height = m.nj();
		const int NN = 4;
		TimeSlotData tsd;

		for (int time = 0; time < height; time++) 
		{
			for (int day = 0; day < width; day++) 
			{
				const DayTimeStats& dts = m.get(m_dayScale(day, dayScaleIdx), time);

				tsd.labelImg = dts.getLabelImage(spatialLevelIdx);

				g.new_node(tsd);
			}
		}

		NodeArray nodes(g);

		XYCoord neig[NN];

		for (int y = 0; y < height; y++) 
		{
			for (int x = 0; x < width; x++) 
			{
				neig[0].Set((x < width - 1) ? x + 1 : 0, y); // x + 1
				neig[1].Set((x > 0) ? x - 1 : width - 1, y); // x - 1

				// y - 1
				if (y > 0)
					neig[2].Set(x, y - 1);
				else if (x > 0)
					neig[2].Set(x - 1, height - 1);
				else 
					neig[2].Set(width - 1, height - 1);

				// y + 1
				if (y < height - 1)
					neig[3].Set(x, y + 1);
				else if (x < width - 1)
					neig[3].Set(x + 1, 0);
				else
					neig[3].Set(0, 0);

				for (int i = 0; i < NN; i++)
				{
					g.new_edge(
						nodes[y * width + x], 
						nodes[neig[i].y * width + neig[i].x], 
						1);
				}
			}
		}
	}

	RGBImg clusterTimeSlots(unsigned dayScaleIdx, unsigned timeScaleIdx) const
	{
		const DayTimeTable& m = get(dayScaleIdx, timeScaleIdx);
		const int width = m.ni();
		const int height = m.nj();
		const int NN = 4;

		LemonGraph<> g;
		LemonGraph<>::NodeMapT compMap(g);

		g.reserve(width * height, width * height * NN);
		
		auto nodes = g.addNodes(width * height, 0);

		XYCoord neig[NN];
		int num = 0;
		float sim;
		int edgeCount = 0;

		for (int y = 0; y < height; y++) 
		{
			for (int x = 0; x < width; x++) 
			{
				neig[0].Set((x < width - 1) ? x + 1 : 0, y); // x + 1
				neig[1].Set((x > 0) ? x - 1 : width - 1, y); // x - 1

				// y - 1
				if (y > 0)
					neig[2].Set(x, y - 1);
				else if (x > 0)
					neig[2].Set(x - 1, height - 1);
				else 
					neig[2].Set(width - 1, height - 1);

				// y + 1
				if (y < height - 1)
					neig[3].Set(x, y + 1);
				else if (x < width - 1)
					neig[3].Set(x + 1, 0);
				else
					neig[3].Set(0, 0);

				//neig[0].Set(x, y - 1);
				//neig[1].Set(x + 1, y);
				//neig[2].Set(x, y + 1);
				//neig[3].Set(x - 1, y);

				for (int i = 0; i < NN; i++)
				{
					if (neig[i].x >= 0 && neig[i].x < width && 
						neig[i].y >= 0 && neig[i].y < height)
					{
						sim = compare(m.get(x, y), m.get(neig[i].x, neig[i].y));

						if (sim > 0)
						{
							g.addEdge(nodes[y * width + x], nodes[neig[i].y * width + neig[i].x], 1);
							edgeCount++;
						}
					}
				}
			}
		}

		int numComp = g.connectedComponents(compMap);

		//DBG_PRINT3(g.edgeNum(), edgeCount, numComp)

		/*FHGraphSegmenter gs;

		gs.init(width * height, width * height * NN);

		FHGraphSegmenter::Edges& edges = gs.edges();

		XYCoord neig[NN];
		int num = 0;
		float sim;

		for (int y = 0; y < height; y++) 
		{
			for (int x = 0; x < width; x++) 
			{
				//neig[0].Set(x, (y > 0) ? y - 1 : height - 1);
				//neig[1].Set((x < width-1) ? x + 1 : 0, y);
				//neig[2].Set(x, (y < height-1) ? y + 1 : 0);
				//neig[3].Set((x > 0) ? x - 1 : width - 1, y);

				neig[0].Set(x, y - 1);
				neig[1].Set(x + 1, y);
				neig[2].Set(x, y + 1);
				neig[3].Set(x - 1, y);

				for (int i = 0; i < NN; i++)
				{
					if (neig[i].x >= 0 && neig[i].x < width && 
						neig[i].y >= 0 && neig[i].y < height)
					{
						sim = compare(m.get(x, y), m.get(neig[i].x, neig[i].y));

						if (sim > 0)
						{
							edges[num].a = y * width + x;
							edges[num].b = neig[i].y * width + neig[i].x;
							edges[num].w = sim;
							num++;
						}
					}
				}
			}
		}
		
		gs.segment_graph(num, 10);
		int num_ccs = gs.num_sets();
		*/

		IntImg segImg(width, height);

		std::vector<int> regionIds(width * height, -1);
		int comp, counter = 0;

		for (int y = 0; y < height; y++) 
		{
			for (int x = 0; x < width; x++) 
			{
				//comp = gs.find(y * width + x);
				comp = compMap[nodes[y * width + x]];

				int& id = regionIds[comp];

				if (id == -1)
					id = counter++;

				segImg(x, y) = id;
			}
		}

		unsigned int sz = segImg.size();

		// Make sure that there will be enough colors
		// available by assuming that each pixel is a region
		if (s_colors.size() < sz)
		{
			s_colors.resize(sz);

			for (unsigned int i = 0; i < sz; ++i)
				s_colors[i] = RandomRGBColor();
		}

		RGBImg colImg(segImg.ni(), segImg.nj());
			
		for (unsigned int j = 0; j < segImg.nj(); ++j)
			for (unsigned int i = 0; i < segImg.ni(); ++i)	
				colImg(i, j) = s_colors[segImg(i, j)];

		return colImg;
	}

	const DayTimeTable& get(unsigned dayScaleIdx, unsigned timeScaleIdx) const
	{
		return m_levels(dayScaleIdx, timeScaleIdx);
	}

	const DayTimeStats& get(unsigned day, unsigned time, 
		unsigned dayScaleIdx, unsigned timeScaleIdx) const
	{
		return m_levels(dayScaleIdx, timeScaleIdx).get(
			m_dayScale(day, dayScaleIdx), time);
	}
};

} // namespace vpl

