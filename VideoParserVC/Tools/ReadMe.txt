if (*it0 == DayTimeStats::ALWAYS)
				*it1 = NamedColor("LightSkyBlue");
			else if (*it0 == DayTimeStats::VERY_LIKELY)
				*it1 = NamedColor("MediumBlue");
			else if (*it0 == DayTimeStats::SOMETIMES)
				*it1 = NamedColor("PaleVioletRed");
			else // NEVER
				*it1 = NamedColor("DarkRed");

			/*if (numVertices != m_numVertices)
			{
				delete m_universe;
				delete[] m_thresholds;

				m_numVertices = numVertices;

				// make a disjoint-set forest
				m_universe = new universe(m_numVertices);

				m_thresholds = new float[m_numVertices];
			}

			if (maxNumEdges > m_maxNumEdges)
			{
				delete[] m_edges;

				m_maxNumEdges = maxNumEdges;

				m_edges = new edge[m_maxNumEdges];
			}*/

						/*m_universe = NULL;
			m_edges = NULL;
			m_thresholds = NULL;*/


		ASSERT(absoluteTime >= m_min_timestamp && absoluteTime <= m_max_timestamp);

		double relativeMinutes = (absoluteTime - m_min_timestamp) / 60.0;

		return unsigned(relativeMinutes / m_timeUnit);


		for (time_t ts = m_eventCalendar.minTimestamp(); ts <= m_eventCalendar.maxTimestamp(); 
			ts += 60 * timeUnit)
		{
			Time t(ts);

			const int minutes = t.hour() * 60 + t.minutes();

			add(t.weekday(), minutes / timeUnit, dts, NULL);
		}

/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "SimpleMatrix.h"
//#include "STL2DArray.h"
#include "Time.h"
#include "BasicTypes.h"

namespace vpl {

//! Array of scale "deltas". scale index --> delta
typedef std::vector<unsigned> RegularScale;

/*! 
	Matrix mapping each index in I to a corresponding scale in S and index' in I'. The matrix
    has size |I| x |S|, and the elements of the matrix are values in I'.
*/
typedef SimpleMatrix<unsigned> IrregularScale;

/*!
*/
struct SpatialStats
{
	unsigned events;

	SpatialStats()
	{
		events = 0;
	}

	void operator+=(const SpatialStats& rhs)
	{
		events = rhs.events;
	}
};

struct SpatialStatsRef
{
	Point coord;
	SpatialStats stats;
};

/*!
*/
class SpatialPyramid : public std::vector<SimpleMatrix<SpatialStats>>
{
protected:
	typedef SimpleMatrix<SpatialStats> SpatialLevel;
	unsigned m_imageWidth;
	unsigned m_imageHeight;

public:
	const SpatialLevel& get(unsigned i) const { return operator[](i); }
	SpatialLevel& get(unsigned i) { return operator[](i); }

	SpatialPyramid()
	{
	}

	SpatialPyramid(unsigned ni, unsigned nj, unsigned levels)
	{
		init(ni, nj, levels);
	}

	void init(unsigned ni, unsigned nj, unsigned levels)
	{
		m_imageWidth = ni;
		m_imageHeight = nj;

		resize(levels);

		unsigned delta = 1;

		for (auto it = begin(); it != end(); ++it)
		{
			it->resize(delta, delta);
			delta *= 2;
		}
	};

	void add(const SpatialStatsRef& ssr)
	{
		double cx, cy;

		for (unsigned level = 0; level < size(); level++)
		{
			cx = ssr.coord.x * (level * 2.0) / m_imageWidth;
			cy = ssr.coord.y * (level * 2.0) / m_imageHeight;

			get(level).get(unsigned(cx), unsigned(cy)) += ssr.stats;
		}
	}
};

/*!
*/
struct DayTimeStats
{
	unsigned motionPerTimeUnit;
	unsigned anyMotion;
	unsigned samples;

	const SpatialStatsRef* pStatsToAdd;

	SpatialPyramid spatialPyramid;

	DayTimeStats()
	{
		motionPerTimeUnit = 0;
		anyMotion = 0;
		samples = 0;
		pStatsToAdd = NULL;
	}

	void operator+=(const DayTimeStats& rhs)
	{
		motionPerTimeUnit += rhs.motionPerTimeUnit;
		anyMotion += rhs.anyMotion;
		samples += rhs.samples;

		if (rhs.pStatsToAdd)
			spatialPyramid.add(*rhs.pStatsToAdd);
	}
};

/*!
*/
class DayTimeTable : public SimpleMatrix<DayTimeStats>
{
	typedef SimpleMatrix<DayTimeStats> ParentClass;

	void resize(unsigned, unsigned);
	void resize(unsigned, unsigned, const DayTimeStats&);

public:
	void init(unsigned ni_day, unsigned nj_time, unsigned ni_img, unsigned nj_img, 
		unsigned spatialPyramidLevels)
	{
		ParentClass::resize(ni_day, nj_time);

		for (auto it = begin(); it != end(); ++it)
			it->spatialPyramid.init(ni_img, nj_img, spatialPyramidLevels);
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
	unsigned m_timeUnit; //!< Number of minutes in a time unit
	time_t m_min_timestamp;
	time_t m_max_timestamp;
	std::vector<bool> m_calendar;

protected:
	void add(unsigned day, unsigned time, const DayTimeStats& val)
	{
		int ct, dt, T;
		unsigned d;

		for (unsigned ds = 0; ds < m_levels.ni(); ds++) // day scale index
		{
			for (unsigned ts = 0; ts < m_levels.nj(); ts++) // time scale index
			{
				DayTimeTable& dtt = m_levels(ds, ts);
				d = m_dayScale(day, ds);

				ASSERT(d < dtt.ni() && time < dtt.nj());

				dt = (int) m_timeScale[ts];
				T = dtt.nj();

				for (int t = int(time) - dt; t <= int(time) + dt; t++)
				{
					// Convert t to a "circular time" ct
					if (t < 0) ct = t + T;
					else if (t >= T) ct = t - T;
					else ct = t;

					dtt(d, ct) += val;
				}
			}
		}
	}

public:
	DayTimeMultiscaleTable()
	{
		m_timeUnit = 1;
	}

	DayTimeMultiscaleTable(time_t min_timestamp, time_t max_timestamp, unsigned timeUnit, 
		const IrregularScale& dayScale, const RegularScale& timeScale,
		unsigned ni_img, unsigned nj_img, unsigned spatialPyramidLevels) 
	{
		init(min_timestamp, max_timestamp, timeUnit, dayScale, timeScale, 
			ni_img, nj_img, spatialPyramidLevels);
	}

	void init(time_t min_timestamp, time_t max_timestamp, unsigned timeUnit, 
		const IrregularScale& dayScale, const RegularScale& timeScale, 
		unsigned ni_img, unsigned nj_img, unsigned spatialPyramidLevels)
	{
		ASSERT(m_levels.empty());

		m_timeUnit = timeUnit;
		m_min_timestamp = min_timestamp;
		m_max_timestamp = max_timestamp;

		double numMinutes = (max_timestamp - min_timestamp) / 60.0;
		unsigned numUnits = unsigned(numMinutes / timeUnit) + 1;

		m_calendar.assign(numUnits, false);

		unsigned ni = dayScale.ni(); // number of days (usually 7)
		unsigned nj = 24 * 60 / timeUnit;

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

		// Fill in the number of samples
		DayTimeStats dts;
		
		dts.samples = 1;

		for (time_t ts = min_timestamp; ts <= max_timestamp; ts += 60 * timeUnit)
		{
			Time t(ts);

			const int minutes = t.hour() * 60 + t.minutes();

			add(t.weekday(), minutes / timeUnit, dts);
		}
	}

	unsigned ni() const 
	{ 
		return m_levels.ni(); 
	}

	unsigned nj() const 
	{ 
		return m_levels.nj(); 
	}

	unsigned getTimeUnit() const
	{
		return m_timeUnit;
	}

	void addMotionEvent(time_t timestamp, const SpatialStatsRef& ssr)
	{
		ASSERT(timestamp >= m_min_timestamp && timestamp <= m_max_timestamp);

		double relativeMinutes = (timestamp - m_min_timestamp) / 60.0;
		unsigned idx = unsigned(relativeMinutes / m_timeUnit);

		DayTimeStats dts;

		if (!m_calendar[idx])
		{
			m_calendar[idx] = true;
			dts.motionPerTimeUnit = 1;

			dts.pStatsToAdd = &ssr;
		}

		dts.anyMotion = 1;

		Time t(timestamp);
		const int minutes = t.hour() * 60 + t.minutes();

		add(t.weekday(), minutes / m_timeUnit, dts);
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

/*class DayTimePyramid : public MultiscaleMatrix
{
	IrregularScale m_dayScale;
	RegularScale m_timeScale;

public:
	DayTimePyramid()
	{
	}

	DayTimePyramid(unsigned ni, unsigned nj, const IrregularScale& dayScale, 
		const RegularScale& timeScale) 
	{
		init(ni, nj, dayScale, timeScale);
	}

	void init(unsigned ni, unsigned nj, const IrregularScale& dayScale, 
		const RegularScale& timeScale)
	{
		ASSERT(dayScale.ni() == ni);

		m_dayScale = dayScale;
		m_timeScale = timeScale;

		m_levels.resize(dayScale.nj(), timeScale.size());

		// Iterate over all "day scales" (ds) and "time scales" (ts), and
		// find out the number of rows and columns needed at each scale
		for (unsigned ds = 0; ds < m_levels.ni(); ds++)
			for (unsigned ts = 0; ts < m_levels.nj(); ts++)
				m_levels(ds, ts).resize(dayScale.maxj(ds), (unsigned)ceil(double(nj) / timeScale[ts]), 0);
	}

	void add(unsigned day, unsigned time, unsigned val)
	{
		for (unsigned ds = 0; ds < m_levels.ni(); ds++) // day scale index
		{
			for (unsigned ts = 0; ts < m_levels.nj(); ts++) // time scale index
			{
				m_levels(ds, ts).get(m_dayScale(day, ds), 
					time / m_timeScale[ts]) += val;
			}
		}
	}

	unsigned get(unsigned day, unsigned time, unsigned dayScaleIdx, unsigned timeScaleIdx) const
	{
		return m_levels(dayScaleIdx, timeScaleIdx).get(m_dayScale(day, dayScaleIdx), 
					time / m_timeScale[timeScaleIdx]);
	}
};*/

} // namespace vpl


	
		/*int dw = (int) (m_sessionInfo.frame_width / (double) m_tempPyr.get(dayScale, timeScale).ni());
		int dh = (int) (m_sessionInfo.frame_height / (double) m_tempPyr.get(dayScale, timeScale).nj());

		for (unsigned i = 0; i < m_sessionInfo.frame_width; i += dw)
		{
			cv::line(mat, cv::Point(i, 0), cv::Point(i, m_sessionInfo.frame_height - 1), 
				cv::Scalar(255, 0, 0));	
		}

		for (unsigned j = 0; j < m_sessionInfo.frame_height; j += dh)
		{
			cv::line(mat, cv::Point(0, j), cv::Point(m_sessionInfo.frame_width, j), 
				cv::Scalar(255, 0, 0));
		}*/

		/*for (unsigned i = 0; i < m_sessionInfo.frame_width; i++)
		{
			for (unsigned j = 0; j < m_sessionInfo.frame_height; j++)
			{
				//img(i, j) = (float)m_spacetime(i, j, day, minutes, spatialScale, temporalScale);
			}
		}*/

	DayTimeMultiscaleTable(unsigned ni, unsigned nj, const IrregularScale& dayScale, 
		const RegularScale& timeScale) 
	{
		init(ni, nj, dayScale, timeScale);
	}

			void init(unsigned ni, unsigned nj, const IrregularScale& dayScale, 
		const RegularScale& timeScale)
	{
		ASSERT(dayScale.ni() == ni);

		m_dayScale = dayScale;
		m_timeScale = timeScale;

		m_levels.resize(dayScale.nj(), timeScale.size());

		// Iterate over all "day scales" (ds) and "time scales" (ts), and
		// find out the number of rows and columns needed at each scale
		for (unsigned ds = 0; ds < m_levels.ni(); ds++)
			for (unsigned ts = 0; ts < m_levels.nj(); ts++)
				m_levels(ds, ts).resize(dayScale.maxj(ds) + 1, nj, 0);
	}
			
			//m_spacetime.plusOne((unsigned)pts[i].x, (unsigned)pts[i].y, 
				//	t.day(), minutes);

void add(unsigned day, unsigned time, unsigned val)
	{
		for (unsigned ds = 0; ds < m_levels.ni(); ds++) // day scale index
		{
			for (unsigned ts = 0; ts < m_levels.nj(); ts++) // time scale index
			{
				CellMatrix& cm = m_levels(ds, ts);
				unsigned dd = m_dayScale(day, ds);

				for (unsigned i = 0; i < cm.ni(); i++)
				{
					if (m_dayScale(i, ds) == dd)
					{
						int dt = (int) m_timeScale[ts];
						int T = cm.nj();

						for (int t = time - dt; t < time + dt; t++)
						{
							if (t < 0) t += T;
							else if (t >= T) t -= T;

							cm(i, t) += val;
						}
					}
				}
			}
		}
	}

for (unsigned d = 0; d < m_dayScale.ni(); d++)
				{
					if (m_dayScale(d, i)) // THIS IS WRONG. FIX ME.!!!!
					{
						for (unsigned t = time - m_timeScale[j]; t > time + m_timeScale[j]; t++)
							m_levels(i, j).get(m_dayScale(day, i), t) += val;
					}
				}
			}
		}

		// Determine the number of levels that we need for the days. ie,
		// what is the maximum scale number referenced.
		unsigned maxDayScale = dayScale.maxValue();

		// Find out how many days there are in each scale
		std::vector<unsigned> dayScaleDims(maxDayScale + 1, 0);

		for (unsigned day = 0; day < ni; day++)
			for (auto it = dayScale[day].begin(); it != dayScale[day].end(); ++it)
				dayScaleDims[*it]++;

		// Init the scale levels
		m_levels.resize(dayScaleDims.size(), timeScale.size());

		for (unsigned i = 0; i < m_levels.ni(); i++)
			for (unsigned j = 0; j < m_levels.nj(); j++)
				m_levels(i, j).resize(dayScaleDims[i], ceil(double(nj) / timeScale[j]));

class DayTimeScale : public SimpleMatrix<DayTimeTable>
{
public:
	DayTimeScale() : SimpleMatrix<DayTimeTable>(1, 1)
	{
	}

	DayTimeScale(const ScaleParams& x_params, const ScaleParams& y_params)
	{
	}

	/*! 
		Determine the appropriate d' from and scale i. For example, at the
		bottom scale, d'==d. */
	unsigned binDay(unsigned i, unsigned d) 
	{ 
		ASSERT(i < ni());

		/*switch (i)
		{
		case 0: return d;
		case 1: (d == 0 || d == 6) ? 0 : 1;
		default: ASSERT(false);
		}*/

		return d; 
	}

	unsigned binTime(unsigned j, unsigned t) 
	{ 
		ASSERT(j < nj());

		return t; 
	}

	void plusOne(unsigned d, unsigned t)
	{
		for (unsigned i = 0; i < ni(); i++)
			for (unsigned j = 0; j < nj(); j++)
				get(i, j).get(binDay(i, d), binTime(j, t))++;
	}
};

class SpaceTimeTable : public SimpleMatrix<DayTimeScale>
{
public:
	SpaceTimeTable()
	{
	}

	SpaceTimeTable(unsigned ni, unsigned nj) : SimpleMatrix<DayTimeScale>(ni, nj)
	{
	}

	void plusOne(unsigned x, unsigned y, unsigned d, unsigned t)
	{
		get(x, y).plusOne(d, t);
	}
};

class SpatioTemporalScale : public SimpleMatrix<SpaceTimeTable>
{
public:
	SpatioTemporalScale() : SimpleMatrix<SpaceTimeTable>(1, 1)
	{
	}

	SpatioTemporalScale(unsigned ni, unsigned nj) : SimpleMatrix<SpaceTimeTable>(ni, nj)
	{
	}

	unsigned binXCoord(unsigned i, unsigned x) 
	{ 
		ASSERT(i < ni());

		return x; 
	}

	unsigned binYCoord(unsigned j, unsigned y) 
	{ 
		ASSERT(j < nj());

		return y; 
	}

	void plusOne(unsigned x, unsigned y, unsigned d, unsigned t)
	{
		for (unsigned i = 0; i < ni(); i++)
			for (unsigned j = 0; j < nj(); j++)
				get(i, j).get(binXCoord(i, x), binYCoord(j, y)).plusOne(d, t);
	}

	/*int get(unsigned x, unsigned y, unsigned d, unsigned t) const
	{
		get(i, j).get(binXCoord(i, x), binYCoord(j, y)).plusOne(d, t);
	}*/
};

/*!
	Specified in terms of parameters type P, and 
	data type T.
*/
template <typename P, typename T> class ParametricScale 
	: public std::vector<SimpleMatrix<T>>
{
protected:
	typedef P ParamType;
	typedef T DataType;
	typedef SimpleMatrix<T> MatrixType;
	typedef std::vector<SimpleMatrix<T>> MatrixPyramid;
	typedef std::vector<P> ParameterArray;
	
	ParameterArray m_params;

public:
	ParametricScale()
	{
	}

	ParametricScale(const ParameterArray& params) 
	{
		init(params);
	}

	size_t height() const
	{
		return MatrixPyramid::size();
	}

	virtual void init(const ParameterArray& params, 
		const DataType& val = DataType()) = 0;

	virtual void remapIndices(size_t scale, size_t* i, size_t* j) const = 0;

	T& operator()(size_t scale, size_t i, size_t j)
	{
		remapIndices(scale, &i, &j);

		return at(scale)(i, j);
	}

	const T& operator()(size_t scale, size_t i, size_t j) const
	{
		remapIndices(scale, &i, &j);

		return at(scale)(i, j);
	}

	void tally_one_vote(size_t i, size_t j)
	{
		for (size_t scale = 0; scale < size(); scale++)
			operator()(scale, i, j)++;
	}
};

/*!
	Matrix of day x minutes
*/
class TemporalScale : public ParametricScale<int, int>
{
public:
	/*!
		The params specify steps in minutes. We also add
		a scale for weekend and weekdays.
	*/
	virtual void init(const ParameterArray& params, 
		const DataType& val = DataType())
	{
		const size_t N = params.size();

		m_params = params;

		MatrixPyramid::resize(N * 2);

		// Create matrices with SEVEN days
		for (size_t i = 0; i < N; i++)
			at(i).resize(7, 24 * 60 / m_params[i], val);

		// Create matrices with TWO days
		for (size_t i = 0; i < N; i++)
			at(i + N).resize(2, 24 * 60 / m_params[i], val);
	}

	virtual void remapIndices(size_t scale, size_t* i, size_t* j) const
	{
		ASSERT(scale >= 0 && scale < m_params.size() * 2);

		if (scale < m_params.size())
		{
			*j /= m_params[scale];
		}
		else
		{
			*j /= m_params[scale - m_params.size()];

			Time::WEEKDAY wd = (Time::WEEKDAY)(int) i;

			if (wd == Time::SUNDAY || wd == Time::SATURDAY)
				*i = 0;
			else 
				*i = 1;
		}
	}
};

/*!
	The parameter type is double and the data type is TemporalScale<double, int>.
*/
class SpatioTemporalScale : private ParametricScale<XYCoord, TemporalScale>
{
	typedef ParametricScale<XYCoord, TemporalScale> ParentClass;

public:
	SpatioTemporalScale()
	{

	}

	/*!
		The params are specified such that the first one (front)
		is the spatial dimensions (ni,nj) for the 0th scale, and all
		subsiquent paramters are x-y denominators x,y, such that the
		the dimention of the i+1-th scale is (ni/x, nj/y).
	*/
	virtual void init(const ParameterArray& params, 
		const DataType& val = DataType())
	{
		m_params = params;

		MatrixPyramid::resize(m_params.size());

		const unsigned ni = m_params.front().x;
		const unsigned nj = m_params.front().y;
		
		// Set the 0-th scale
		at(0).resize(ni, nj, val);

		// Set all other scales
		for (size_t i = 1; i < m_params.size(); i++)
			at(i).resize(ni / m_params[i].x, nj / m_params[i].y, val);
	}

	virtual void remapIndices(size_t scale, size_t* i, size_t* j) const
	{
		ASSERT(scale >= 0 && scale < m_params.size());

		if (scale > 0)
		{
			*i /= m_params[scale].x;
			*j /= m_params[scale].y;
		}
	}

	//! Height of the spatial and temporal pyramids
	std::pair<size_t, size_t> height() const
	{
		size_t spat, temp;

		if (empty() || front().empty())
		{
			spat = 0;
			temp = 0;
		}
		else
		{
			spat = ParentClass::size();
			temp = front()(0, 0).height();
		}

		return std::make_pair(spat, temp);
	}

	//! Number of rows at the bottom opf the pyramid
	size_t rows() const
	{
		return empty() ? 0 : front().ni();
	}

	//! Number of columns at the bottom opf teh pyramid
	size_t cols() const
	{
		return empty() ? 0 : front().nj();
	}

	void init(const std::vector<double>& spatialRatios, 
		const std::vector<double>& temporalRatios, const int& val)
	{
		/*ParentClass::init(spatialRatios);

		for (size_t i = 0; i < size(); i++)
		{
			MatrixType& mat = at(i);

			for (unsigned i = 0; i < mat.ni(); i++)
				for (unsigned j = 0; j < mat.nj(); j++)
					mat(i, j).init(temporalRatios, val);
		}*/
	}

	void convertToDayAndMinutes(time_t tt, unsigned* day, unsigned* minutes) const
	{
		Time t(tt);

		*day = t.day();
		*minutes = t.minutes();
	}

	int& operator()(unsigned i, unsigned j, unsigned day, unsigned minutes, 
		unsigned spatialScale, unsigned temporalScale)
	{
		return ParentClass::operator()(spatialScale, i, j)(temporalScale, day, minutes);
	}

	const int& operator()(unsigned i, unsigned j, unsigned day, unsigned minutes, 
		unsigned spatialScale, unsigned temporalScale) const
	{
		return ParentClass::operator()(spatialScale, i, j)(temporalScale, day, minutes);
	}

	void tally_one_vote(size_t i, size_t j, unsigned day, unsigned minutes)
	{
		for (size_t spatialScale = 0; spatialScale < size(); spatialScale++)
			ParentClass::operator()(spatialScale, i, j).tally_one_vote(day, minutes);
	}
};
/////////////////////////////////////


		const size_t N = params.size();

		m_params.reserve(N * 2);

		m_params = params;

		m_params.insert(params.begin() + N, 
			params.begin(), params.end());

		MatrixPyramid::resize(N * 2);

		for (size_t i = 0; i < N; i++)
			at(i).resize(7, 24 * 60 / m_params[i], val);

		for (size_t i = N; i < N; i++)
			at(i).resize(7, 24 * 60 / m_params[i], val);

	double avgPixelVal;
	double min1 = -1, max1 = -1, min2 = -1, max2 = -1;

avgPixelVal = rgbImage(i, j).grey();

			if (min1 == -1 || grayImage(i, j) < min1)
				min1 = grayImage(i, j);

			if (max1 == -1 || grayImage(i, j) > max1)
				max1 = grayImage(i, j);

			if (min2 == -1 || avgPixelVal < min2)
				min2 = avgPixelVal;

			if (max2 == -1 || avgPixelVal > max2)
				max2 = avgPixelVal;

DBG_PRINT4(min1, max1, min2, max2);

if (fabs(grayImage(i, j) - candBuff.mean()) <= m_params.pixelDiffThreshold)
					{
						//candBuff.add_confidence(m_params.positiveLearningStep);

						if (candBuff.get_confidence() > modelBuff.get_confidence())
						{
							//std::swap(candBuff, modelBuff);
							//m_backgroundModel(i, j) = (float) modelBuff.mean();

							//modelBuff.push_back(candBuff.mean());
							
							modelBuff = candBuff;
							m_backgroundModel(i, j) = (float) modelBuff.mean();
							candBuff.clear();
							
							/*const double& w = 0.02;//m_params.backgroundModelAdaptationRate;

							m_backgroundModel(i, j) = float((1.0 - w) * modelBuff.mean() 
								+ w * candBuff.mean());*/
						}
					}

void AdaptiveBackgroundSubtractor::processImage(RGBImg rgbImage, FloatImg grayImage,
	bool updateModel)
{
	int ls;

	m_stats.Clear();

	auto maskIt = m_foreground.begin();
	auto bmIt = m_backgroundModel.begin();
	auto confIt = m_modelConfidence.begin();
	auto pixelIt = grayImage.begin();

	auto bmColorIt = m_background.begin();
	auto colorPixelIt = rgbImage.begin();

	auto modelIt = m_modelBuffer.begin();
	auto candiIt = m_candidateBuffer.begin();

	for (unsigned i = 0; i < m_backgroundModel.ni(); i++)
	{
		for (unsigned j = 0; j < m_backgroundModel.nj(); j++)
		{
		}
	}

	// Iterate over each pixel
	while (pixelIt != grayImage.end())
	{
		if( ((*pixelIt - *bmIt) > m_params.pixelDiffThreshold) ||
			((*bmIt - *pixelIt) > m_params.pixelDiffThreshold) )
		{
			*maskIt = 255;
			m_stats.numberWhitePixels++;
			ls = -m_params.negativeLearningStep;

			// It might be a new candidate or a new sample of the current candidate
			if (*confIt > 0)
			{
				m_modelBuffer.clear(candiIt,
			}
		} 
		else 
		{
			*maskIt = 0;
			ls = m_params.positiveLearningStep; 
		}

		if((*pixelIt - *bmIt) > m_params.pixelDiffThreshold)
			m_stats.numberPixelsAboveThreshold++;
		else if((*bmIt - *pixelIt) > m_params.pixelDiffThreshold)
			m_stats.numberPixelsBelowThreshold++;
		else
			m_stats.numberPixelsInThreshold++;

		// Update the Confidence Level Image and optionally, the background model
		if (*confIt + ls <= 0)
		{
			*confIt = m_params.positiveLearningStep - m_params.negativeLearningStep;

			if (updateModel)
			{
				*bmIt = *pixelIt;
				*bmColorIt = *colorPixelIt;
			}
		} 
		else 
		{
			// Update the confidence
			*confIt += ls;

			// Cap the confidence s.t. it's not too high
			if(*confIt > m_params.maxConfidenceLevel)
				*confIt = m_params.maxConfidenceLevel;

			if(updateModel && ls > 0)
			{
				*bmIt = (float)(*bmIt * (1.0 - m_params.backgroundModelAdaptationRate)
					 + *pixelIt * m_params.backgroundModelAdaptationRate);
			}
		}

		++maskIt;
		++pixelIt;
		++bmIt;
		++confIt;
		++colorPixelIt;
		++modelIt;
		++candiIt;
	}

	//IplImageView img(m_foreground);
	//cv::Mat binMat(img);

	// Binary Image post processing
	CvMatView binMat(m_foreground);

    cv::erode(binMat, binMat, cv::Mat(), cv::Point(-1,-1), m_params.erodeIterations);
    cv::dilate(binMat, binMat, cv::Mat(), cv::Point(-1,-1), m_params.dilateIterations);
}

struct iterator
	{
		unsigned i;
		unsigned j;
		PixelwiseFrameBuffer* pMat;

		iterator(unsigned a, unsigned b, PixelwiseFrameBuffer* p)
			: i(a), j(b), pMat(p) { }

		iterator& operator++() 
		{ 
			if (++i >= pMat->ni())
			{
				i = 0;
				j++;
			}

			return *this; 
		}

		operator const T*() const 
		{ 
			return &ptr->data; 
		}
	};

	struct const_iterator
	{
		unsigned i;
		unsigned j;
		const PixelwiseFrameBuffer* pMat;

		iterator(unsigned a, unsigned b, const PixelwiseFrameBuffer* p)
			: i(a), j(b), pMat(p) { }

		const_iterator& operator++() 
		{ 
			if (++i >= pMat->ni())
			{
				i = 0;
				j++;
			}

			return *this; 
		}
	};

	
	struct const_iterator
	{
		unsigned i;
		unsigned j;
		const PixelwiseFrameBuffer* pMat;

		const_iterator(unsigned a, unsigned b, const PixelwiseFrameBuffer* p)
			: i(a), j(b), pMat(p) { }

		const_iterator& operator++() 
		{ 
			if (++i >= pMat->ni())
			{
				i = 0;
				j++;
			}

			return *this; 
		}
	};

/*ASSERT(m_params.frameStep > 1);

			for (int i = 0; i < m_params.frameStep; i++)
			{
				m_video.ReadNextFrame();

				m_nFrame++;

				if (IsLastFrame())
					break;
			}*/

try {
		
	}
	catch(BasicException e) {
		e.Print();
		ShowUsage("Output images must be specified as a list of (index,param) values");
		m_saveResults = false;
	}


/*const std::string& front()
	{ 
		m_mutex.Lock();
		return queryList.front(); 
		m_mutex.Release();
	}

	void pop_front()
	{ 
		m_mutex.Lock();
		queryList.pop_front(); 
		m_mutex.Release();
	}

	bool push_back(const std::string& val) 
	{ 
		if (state == READY || state == RUNNING)
		{
			m_mutex.Lock();
			queryList.push_back(val); 
			m_mutex.Release();

			return true;
		}

		return false;
	}*/

// Note, non-blocking mode must be set to false if
	// the thread is not created for whatever reason
	if (!m_pThreadData)
	{
		m_pThreadData = new SQLThreadData;
	}
	else if (!m_pThreadData->empty() && m_pThreadData->sql() != m_pMySql)
	{
		ShowError("There are pending queries from another database");
		m_nonBlockingMode = false;
		return true; // continue, but in non-blocking mode
	}

	// Pass the database info to the new thread
	m_pThreadData->set_sql(m_pMySql);

	int threadStatus = fl_create_thread(m_threadId, ProcessQueries, m_pThreadData);

	if (threadStatus <= 0)
	{
		ShowError("Cannot open database in non-blocking mode");
		m_nonBlockingMode = false;
		return true; // continue, but in non-blocking mode
	}

/*void SQLDatabase::Insert(std::string table, std::string fields, std::string values)
{
	ASSERT(IsOpen());

	std::ostringstream cmd;

	cmd << "insert into " << table << "(" << fields << ") values(" 
		<< values << ")";

    if (!::mysql_query(m_conn, cmd.str().c_str()))
        THROW_BASIC_EXCEPTION("Cannot insert data into database table");
}*/

	Mutex::HANDLE GetMutexHandle() const
	{
		ASSERT(m_pProcDrawMutex);

		return m_pProcDrawMutex->Handle();
	}

/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "BasicUtils.h"
#include <cxcore.h>
#include "ImageUtils.h"

//namespace cv {
//	class Mat;
//}

struct CvMat;
//typedef _IplImage IplImage;

/*!
	Wrapper for an IplImage whose data is owned by a VXL image.

	It is similar to the CvImage class declared in cxcore.hpp. However,
	CvImage counts references to the wrapped IplImage, while ShallowIplImage
	is just a IplImage* and assumes that the *imageData is ownd by some
	VXL image. A ShallowIplImage object is only valid while the VXL image
	does not change.
*/
class CvMatView : cv::Mat
{
protected:
	CvMat* m_pMat; //! It is valid as long as m_vxlView is
	BaseImgPtr m_vxlView;  //! Needed to ensure that Mat data is valid

	void CreateHeader(int width, int height, int type,
		int elemByteSize, char* imageData)
	{
		m_pMat = cvCreateMatHeader(height, width, type);

		int rowByteSize = width * elemByteSize;

		cvSetData(m_pMat, imageData, rowByteSize);
	}

public:
	CvMatView() { m_pMat = NULL; }

	CvMatView(FloatImg& img)
	{
		m_vxlView = ConvertToBaseImgPtr(img);

		ASSERT(img.jstep() > 0);

		CreateHeader(img.ni(), img.nj(), IPL_DEPTH_32F, sizeof(float),
			(char*) img.top_left_ptr());
	}

	CvMatView(ByteImg& img)
	{
		m_vxlView = ConvertToBaseImgPtr(img);

		CreateHeader(img.ni(), img.nj(), IPL_DEPTH_8U, sizeof(vxl_byte),
			(char*) img.top_left_ptr());
	}

	CvMatView(IntImg& img)
	{
		m_vxlView = ConvertToBaseImgPtr(img);

		CreateHeader(img.ni(), img.nj(), IPL_DEPTH_32S, sizeof(int),
			(char*) img.top_left_ptr());
	}

	CvMatView(RGBImg& rgbImg, bool swapRB);

	CvMatView(LabImg& img)
	{
		m_vxlView = ConvertToBaseImgPtr(img);

		CreateHeader(img.ni(), img.nj(), IPL_DEPTH_32F, sizeof(LabColor),
			(char*) img.top_left_ptr());
	}

	~CvMatView() { cvReleaseMatHeader(&m_pMat); }

	operator const CvMat*() const { return m_pMat; }

    operator CvMat*() { return m_pMat; }
};



// End comparison functions
//////////////////////////////////////////////////////////////////////////////

template <class T>
vnl_matrix<T> sqrt(const vnl_matrix<T>& m)
{
	vnl_matrix<T> r(lhs.rows(), lhs.cols());

	forall_cells2(r, m)
		*it0 = sqrt(*it1);

	return r;
}

//! Returns true  if S is a 1 x 1 matrix
//! and false  otherwise.
template <class T>
inline bool isscalar(const vnl_matrix<T>& m)
{
	return m.size() == 1;
}

//! Get the size as a 'double' type
template <class T>
inline double size(const vnl_matrix<T>& m)
{
	return m.size();
}

//! Count all the true elements of the bool matrix
template <class T>
inline unsigned count(const vnl_matrix<bool>& m)
{
	unsigned n = 0;

	forall_cells1(m)
		if (*it0)
			n++;

	return n;
}

/*!
	Array of references to matrices.
*/
template <typename T> class MatRefArray : public std::vector<vnl_matrix<T>*>
{
	typedef vnl_matrix<T>* P;
	typedef std::vector<P> BASE;

	MatRefArray& operator=(const MatRefArray& rhs) = delete;

public:
	void operator=(const std::vector<vnl_matrix<T>>& rhs)
	{
		CHECK_SIZE(*this, rhs);

		forall_cells2(*this, rhs)
			**it0 = *it1;
	}

	MatRefArray(P e0) : BASE(1, e0) 
	{ 
	}
	
	MatRefArray(P e0, P e1) 
		: BASE(2) 
	{ 
		operator[](0) = e0;
		operator[](1) = e1;
	}

	MatRefArray(P e0, P e1, P e2) 
		: BASE(3) 
	{ 
		operator[](0) = e0;
		operator[](1) = e1;
		operator[](2) = e2;
	}

	MatRefArray(P e0, P e1, P e2, P e3) 
		: BASE(4) 
	{ 
		operator[](0) = e0;
		operator[](1) = e1;
		operator[](2) = e2;
		operator[](3) = e3;
	}

	MatRefArray(P e0, P e1, P e2, P e3, P e4) 
		: BASE(5) 
	{ 
		operator[](0) = e0;
		operator[](1) = e1;
		operator[](2) = e2;
		operator[](3) = e3;
		operator[](4) = e4;
	}

	MatRefArray(P e0, P e1, P e2, P e3, P e4,
		P e5) : BASE(6) 
	{ 
		operator[](0) = e0;
		operator[](1) = e1;
		operator[](2) = e2;
		operator[](3) = e3;
		operator[](4) = e4;
		operator[](5) = e5;
	}

	MatRefArray(P e0, P e1, P e2, P e3, P e4,
		P e5, P e6) : BASE(7) 
	{ 
		operator[](0) = e0;
		operator[](1) = e1;
		operator[](2) = e2;
		operator[](3) = e3;
		operator[](4) = e4;
		operator[](5) = e5;
		operator[](6) = e6;
	}

	MatRefArray(P e0, P e1, P e2, P e3, P e4,
		P e5, P e6, P e7) : BASE(8) 
	{ 
		operator[](0) = e0;
		operator[](1) = e1;
		operator[](2) = e2;
		operator[](3) = e3;
		operator[](4) = e4;
		operator[](5) = e5;
		operator[](6) = e6;
		operator[](7) = e7;
	}
};

template <class T>
vnl_matrix<bool> operator>(const vnl_matrix<T>& lhs, const T& rhs)
{
	VPLMatrix<bool> r(lhs.rows(), lhs.cols());
		
	forall_cells2(r, lhs)
		*it0 = *it1 > rhs;

	return r;
}

template <class T>
vnl_matrix<bool> operator<(const vnl_matrix<T>& lhs, const T& rhs)
{
	VPLMatrix<bool> r(lhs.lows(), lhs.cols());
		
	forall_cells2(r, lhs)
		*it0 = *it1 < rhs;

	return r;
}

template <class T>
vnl_matrix<bool> operator==(const vnl_matrix<T>& lhs, const T& rhs)
{
	VPLMatrix<bool> r(lhs.rows(), lhs.cols());
		
	forall_cells2(r, lhs)
		*it0 = *it1 == rhs;

	return r;
}

template <class T>
vnl_matrix<bool> operator!=(const vnl_matrix<T>& lhs, const T& rhs)
{
	VPLMatrix<bool> r(lhs.rows(), lhs.cols());
		
	forall_cells2(r, lhs)
		*it0 = *it1 != rhs;

	return r;
}

VPLMatrix<bool> operator>(const BASE& rhs) const
	{
		VPLMatrix<bool> r(rhs.rows(), rhs.cols());
		
		forall_cells3(r, *this, rhs)
			*it0 = *it1 > *it2;

		return r;
	}

	VPLMatrix<bool> operator<(const BASE& rhs) const
	{
		VPLMatrix<bool> r(rhs.rows(), rhs.cols());
		
		forall_cells3(r, *this, rhs)
			*it0 = *it1 < *it2;

		return r;
	}
	
	VPLMatrix<bool> operator==(const BASE& rhs) const
	{
		VPLMatrix<bool> r(rhs.rows(), rhs.cols());
		
		forall_cells3(r, *this, rhs)
			*it0 = *it1 == *it2;

		return r;
	}

	VPLMatrix<bool> operator!=(const BASE& rhs) const
	{
		VPLMatrix<bool> r(rhs.rows(), rhs.cols());
		
		forall_cells3(r, *this, rhs)
			*it0 = *it1 != *it2;

		return r;
	}

	VPLMatrix<bool> operator>(const T& rhs) const
	{
		VPLMatrix<bool> r(rhs.rows(), rhs.cols());
		
		forall_cells2(r, *this)
			*it0 = *it1 > rhs;

		return r;
	}

	VPLMatrix<bool> operator<(const T& rhs) const
	{
		VPLMatrix<bool> r(rhs.rows(), rhs.cols());
		
		forall_cells2(r, *this)
			*it0 = *it1 < rhs;

		return r;
	}
	
	VPLMatrix<bool> operator==(const T& rhs) const
	{
		VPLMatrix<bool> r(rhs.rows(), rhs.cols());
		
		forall_cells2(r, *this)
			*it0 = *it1 == rhs;

		return r;
	}

	VPLMatrix<bool> operator!=(const T& rhs) const
	{
		VPLMatrix<bool> r(rhs.rows(), rhs.cols());
		
		forall_cells2(r, *this)
			*it0 = *it1 != rhs;

		return r;
	}

	/*CellRef& operator=(const vnl_matrix<T>& m)
	{
		ASSERT(cells.size() == m.size());

		auto matIt = m.begin();
		auto refIt = refs.begin();

		for (; matIt != m.end(); ++matIt, ++refIt)
			**refIt = *matIt;
	}
	*/

			/*CHECK_ROWS_AND_COLS(*this, rhs);

		VPLMatrix<bool> vals(rhs.rows(), rhs.cols());

		unsigned i, j;

		for (i = 0; i < rows(); i++)
			for (j = 0; j < cols(); j++)
				vals(i, j) = (*this)(i, j) > rhs(i, j);

		return vals;*/

		CHECK_SIZE(cells, m);

			auto matIt = m.begin();
			auto refIt = refs.begin();

			for (; matIt != m.end(); ++matIt, ++refIt)
				**refIt = *matIt;

/*std::vector<unsigned> vpl::RandomArrayPermutation(unsigned M)
{
	std::vector<unsigned> indexMap(M);

	for (unsigned i = 0; i < M; i++)
		indexMap[i] = i;

	//NOTE: it is probably better to use std::random_shuffle() instead
	// of code right bellow...
	// Shuffle an array a of n elements:
	// @see Fisher–Yates shuffle in wikipedia
    for (unsigned i = M - 1; i >= 1; i--)
	{
		//std::uniform_int_distribution<int> dist(0, i);

		// Get random integer j with 0 <= j <= i
		unsigned j = unsigned((rand() / double(RAND_MAX)) * i); 
		
		//exchange a[j] and a[i];
		std::swap(indexMap[i], indexMap[j]);
	}

	return indexMap;
}*/

	/*!
		Reads the ENUMERATION values associated with the property 'propKey'
		of the field 'fieldKey'. If the property or the field 
		do not exist, they are added with the provided default
		values. If there is an error reading the field:property, 
		the usage message is printed.
	*/
	template <typename T, unsigned DIM, char BEGIN_BRACKET, char END_BRACKET>
	bool ReadArgs(const Keyword& fieldKey, 
		const Keyword& propKey, const std::vector<StrArray>& enumLabels,
		const char* szUsageMsg, const std::vector<IntList>& defVals, 
		std::vector<IntList>* pValues)
	{
		template <class T> bool ReadArgs(const Keyword& fieldKey, 
		const Keyword& propKey, const char* szUsageMsg, 
		const std::list<T>& defVals, std::list<T>* pValues)
	}

class InputStream : public std::istream
{
public:
};

class OutputStream : public std::ostream
{
public:
};


	unsigned char** m_table;
	unsigned m_numRows;

	unsigned char m_numStates;
	unsigned char m_numVariables;


	CPT()
	{
		m_table = NULL;
	}

	~CPT()
	{
		delete[] m_table;
	}

	CPT(const unsigned char numStates, const unsigned char numVars)
	{
		Init(numStates, numVars);
	}

	void Init(const unsigned char numStates, const unsigned char numVars);
	
	
	void SubsampleExact(unsigned numSamples, BoolArray* pStates, 
		DoubleArray* pTangents) const
	{
		ASSERT(pStates && pTangents);

		pStates->resize();
		pTangents->clear();
	}

	/*!
Cardinality:
	numStates^numVars
*/
/*void CPT::Init(const unsigned char numStates, const unsigned char numVars)
{
	int gen_result;       // return value of generation functions
	unsigned set_counter; // counter of generated subsets

	m_numRows = (unsigned) pow((double)numStates, (double)numVars);

	m_table = new unsigned char*[m_numRows];

	for (unsigned i = 0; i < m_numRows; i++)
		m_table[i] = new unsigned char[numVars];

	if (!m_vector)
		return;

	//initialize
	set_counter = 0;
	gen_result = gen_vari_rep_init(m_vector, numStates, numVars);

	if(gen_result == GEN_EMPTY)
		set_counter++;

	//generate all successors
	while (gen_result == GEN_NEXT)
	{
		set_counter++;

		for(int x = 0; x < numVars; x++)
			printf("%u ", m_vector[x]);

		printf("(%u)\n", set_counter);

		gen_result = gen_vari_rep_next(m_vector, numStates, numVars);
	}
}*/

std::ostringstream oss;

		unsigned parIdx = mv.viewInfo.storParentMetadataId;

		oss << "{";

		if (parIdx == INVALID_STORAGE_ID)
		{
			return std::make_pair(std::string(), 
				ObjectClassMetadata::ID_NOT_SET);
		}
		else
		{
			const ModelObject& parent = m_modelObjects[parIdx];

			return std::make_pair(std::string(parent.classInfo.name),
				parent.classInfo.id);
		}

//std::streampos m_temporaryClosingPos;

	// Save the temporary closing character position
		m_temporaryClosingPos = m_resultsFile.tellp();
		m_resultsFile << "\n}\n";
////////////////////////////////////////////////////////////////////////////
void operator=(const SCGEdgeAtt& rhs)
	{
		isDummy = rhs.isDummy;
		isIgnored = rhs.isIgnored;
	}

unsigned NumberOfParses() const
	{
		return m_ptrShapeCutGraph->NumberOfParses();
	}

	const std::vector<SPGPtr>& GetShapeParses() const
	{
		return m_ptrShapeCutGraph->GetShapeParses();
	}

	const ShapeParseGraph& GetShapeParse(unsigned parseId) const
	{
		return m_ptrShapeCutGraph->GetShapeParse(parseId);
	}


	configs[i].Init(GetSCG(), false);
////////////////////////////////////////////////////////////////////////////
//! Random binary cut variable
struct CutVariable
{
	graph::edge cutEdge; //!< Edge in the shape cut graph
	int endpoint1;       //!< Boundary ID of cut endpoint 1
	int endpoint2;       //!< Boundary ID of cut endpoint 2

	std::list<unsigned> cliques; //!< Cliques that containt the variable

	std::list<graph::node> nodes; //!< Nodes that contain the variable

	void Set(graph::edge e, int ept1, int ept2)
	{
		cutEdge = e;
		endpoint1 = ept1;
		endpoint2 = ept2;
	}
};

			m_variables[scg.index(e)].Set(e, 
				scg.inf(source(e)).idx, scg.inf(target(e)).idx);

////////////////////////////////////////////////////////////////////////////
			else if (!r[i] &&
				relLenPr == m_params.m_equalOrLongerLenghtPr &&
				parPr > m_params.m_nonParallelPr)
			{
				pr = adjPr * parPr;
			}

if (cutState && x[j])
				{
					*pParPr = ParallelPr(cutEdge, *roleIt);

					ASSERT(*pParPr >= m_params.m_nonParallelPr);
				}
////////////////////////////////////////////////////////////////////////////
double ratio = (len1 <= len2) ? len1 / len2 : len2 / len1;
		
		double pr = (m_params.m_equalLenghtPr - m_params.m_maxRelLenghtPr) 
			* ratio + m_params.m_maxRelLenghtPr;

		return (len1 <= len2) ? pr : m_params.m_equalLenghtPr;
////////////////////////////////////////////////////////////////////////////
double AverageAdjCutLength(graph::edge cutEdge) const
	{
		unsigned n = 0;
		double len = 0;
		
		// We look for adjacebt edges on each of the two
		// endpoints of e0, which are added to an array.
		graph::node nv[2] = {source(cutEdge), target(cutEdge)};
		graph::edge e;

		for (int i = 0; i < 2; i++)
		{
			forall_adj_edges(e, nv[i])
			{
				if (e != cutEdge)
				{
					n++;
					len += m_scg.CutLength(e);
				}
			}
		}
		
		return (n > 0) ? len / n : 0;
	}
////////////////////////////////////////////////////////////////////////////

			// If the lenght of the cycle is less than 4 or if the cut
			// maps to an interior edge in the cycle, then all cuts
			// in the cycle are adjacent to the given cut
			if (cy.Size() < 4 || contains(cy.interiorEdges, cv1.cutEdge))
			{
			}

			// The lenght of the cycle must be 4
			if (cy.Size() < 4)
				continue;

			// The varId should not map to an interior edge in the cycle
			// If it is an interior edge, we skip it
			if (contains(cy.interiorEdges, cv1.cutEdge))
				continue;
////////////////////////////////////////////////////////////////////////////
STLInitVector<graph::node> nv(source(e0), target(e0));

double ParallelPr(unsigned varId) const
	{
		const CutVariable& cv1 = m_pModel->GetCutVariable(varId);
		unsigned idx;
		bool isOdd1, isOdd2;
		std::list<unsigned> adjCut;
		std::list<unsigned> oppCut;

		//DBG_PRINT1(cv1.nodes.size())

		// Each node the var belongs to represents a cycle 
		// of length 4 or less and with an optional interior edge.
		for (auto it = cv1.nodes.begin(); it != cv1.nodes.end(); ++it)
		{
			const CVClique& cl = m_pModel->inf(*it);
			
			if (cl.Size() < 4)
				continue;

			idx = cl.Find(varId);

			ASSERT(idx < cl.Size());

			// If it is an interior edge, we skip it
			if (idx >= 4)
				continue;

			isOdd1 = (idx % 2) != 0;

			adjCut.clear();
			oppCut.clear();

			// Only the first 4 variables are exterior edges in SCG
			for (unsigned i = 0; i < 4; ++i)
			{
				if (i == idx)
					continue;

				isOdd2 = (i % 2) != 0;

				if ((isOdd1 && isOdd2) || (!isOdd1 && !isOdd2))
				{
					// i and idx are opposite to each other in the cycle
					oppCut.push_back(cl.variables[i]);
				}
				else
				{
					// i and idx are adjacent in the cycle
					adjCut.push_back(cl.variables[i]);
				}
			}

			ASSERT(oppCut.size() == 1 && adjCut.size() == 2);
			
			const CutVariable& cvOpp = m_pModel->GetCutVariable(oppCut.front());
			const CutVariable& cvAdj = m_pModel->GetCutVariable(adjCut.front());

			graph::node a1 = SharedNode(cv1.cutEdge, cvAdj.cutEdge);
			graph::node b1 = SharedNode(cvOpp.cutEdge, cvAdj.cutEdge);

			ASSERT(a1 != nil && b1 != nil);

			graph::node a2 = opposite(a1, cv1.cutEdge);
			graph::node b2 = opposite(b1, cvOpp.cutEdge);

			// Compute the angle between the segments
			Point A = m_scg.inf(a2).pt - m_scg.inf(a1).pt;
			double normA = A.norm();

			Point B = m_scg.inf(b2).pt - m_scg.inf(b1).pt;
			double normB = B.norm();

			double ang = SignedVectorAngle(A.x, A.y, B.x, B.y, normA, normB);

			DBG_PRINT5(varId, oppCut.front(), ang, normA, normB)
		}

		return 1;
	}
////////////////////////////////////////////////////////////////////////////

//unsigned cliqueId; //!< Id of the clique using during constriction

	//! Iterator of the cycle in SCG inducing this clique
	//std::list<ShapeCutPath>::const_iterator cycleIt; 

//cliqueId  = rhs.cliqueId;
		//cycleIt   = rhs.cycleIt;


//! Returns the node shared by both edges
inline NodePair LinkedNodes(graph::edge e1, graph::edge e2, graph::edge e3)
{
	return std::make_pair(SharedNode(e1, e2), SharedNode(e3, e2));
}

	double CutLength(unsigned varId) const
	{
		graph::edge e = m_ptrModel->GetCutVariable(varId).cutEdge;

		return m_scg.CutLength(e);
	}

return (n > 0) ? len / n : m_scg.CutLength(e0);
void ShapeParsingModel::SortConfigurations()
{
	std::list<unsigned> emptyList; 
	node v;

	// For each clique v
	forall_nodes(v, *this)
	{
		const std::list<unsigned>& S_v = (indeg(v) > 0) ? 
			inf(first_in_edge(v)).sharedVars : emptyList;

		// For each configuration s_v in S_v
		for (auto it = S_v.begin(); it != S_v.end(); ++it)
		{
		}
	}
}
////////////////////////////////////////////////////////////////////////////
	//! Print subtree
	void CollectInstantiation(node r, unsigned s_inst, 
		unsigned k, RVAssignments& inst) const
	{
		node v;

		ChildCandidateTerms terms = inf(r).CollectInstantiation(s_inst, k, inst);

		unsigned childId = 0;

		// Collect variables from child-to-parent message
		forall_adj_nodes(v, r)
		{
			CollectInstantiation(v, terms[childId++], inst);
		}

		
	}

	//! Print forest
	RVAssignments CollectInstantiation(std::ostream& os) const
	{
		RVAssignments inst;
		node v;

		forall_nodes(v, *this)
		{
			if (indeg(v) == 0) // it's a root node
			{
				inst.clear();

				CollectInstantiation(v, inst);
			}
		}

		return inst;
	}
	
	ChildCandidateTerms CollectInstantiation(unsigned s_inst, unsigned k,
		RVAssignments& inst) const
	{
		return m_parentMsg.CollectInstantiation(s_inst, k, inst);
	}

/*!

		Returns descTerms[a]=b, such that the term of the a'th descendant
		used is its b'th candidate.
	*/
	/*ChildCandidateTerms CollectInstantiation(unsigned s_inst, unsigned k,
		RVAssignments& inst) const
	{
		const Candidates& c = m_table[s_inst];
		
		if (k < c.size())
		{
			AddVarAssignment(m_reminder, c[k].remInst);

			return c[k].descTerms;
		}

		return ChildCandidateTerms();
	}*/

	RVAssignments config;

	// Find the var names in the set
	for (unsigned i = 0; i < vars.size(); ++i)
		config.push_back(RVAssignment(vars[i], inst[i]));

// Store the consitent candidates used to compute each element
		// of the list
		for (auto it = prodsList.begin(); it != prodsList.end(); ++it)
			it->candidates = cc;
////////////////////////////////////////////////////////////////////////////
template <typename T> struct STLInitVector : public std::vector<T>
{
	STLInitVector() { }

	STLInitVector(const T& e0) : std::vector<T>(1, e0) { }
	
	STLInitVector(const T& e0, const T& e1) 
		: std::vector<T>(2) 
	{ 
		operator[](0) = e0;
		operator[](1) = e1;
	}

	STLInitVector(const T& e0, const T& e1, const T& e2) 
		: std::vector<T>(3) 
	{ 
		operator[](0) = e0;
		operator[](1) = e1;
		operator[](2) = e2;
	}

	STLInitVector(const T& e0, const T& e1, const T& e2, const T& e3) 
		: std::vector<T>(4) 
	{ 
		operator[](0) = e0;
		operator[](1) = e1;
		operator[](2) = e2;
		operator[](3) = e3;
	}

	STLInitVector(const T& e0, const T& e1, const T& e2, const T& e3, const T& e4) 
		: std::vector<T>(5) 
	{ 
		operator[](0) = e0;
		operator[](1) = e1;
		operator[](2) = e2;
		operator[](3) = e3;
		operator[](4) = e4;
	}

	STLInitVector(const T& e0, const T& e1, const T& e2, const T& e3, const T& e4,
		const T& e5) : std::vector<T>(6) 
	{ 
		operator[](0) = e0;
		operator[](1) = e1;
		operator[](2) = e2;
		operator[](3) = e3;
		operator[](4) = e4;
		operator[](5) = e5;
	}

	STLInitVector(const T& e0, const T& e1, const T& e2, const T& e3, const T& e4,
		const T& e5, const T& e6) : std::vector<T>(7) 
	{ 
		operator[](0) = e0;
		operator[](1) = e1;
		operator[](2) = e2;
		operator[](3) = e3;
		operator[](4) = e4;
		operator[](5) = e5;
		operator[](6) = e6;
	}

	STLInitVector(const T& e0, const T& e1, const T& e2, const T& e3, const T& e4,
		const T& e5, const T& e6, const T& e7) : std::vector<T>(8) 
	{ 
		operator[](0) = e0;
		operator[](1) = e1;
		operator[](2) = e2;
		operator[](3) = e3;
		operator[](4) = e4;
		operator[](5) = e5;
		operator[](6) = e6;
		operator[](7) = e7;
	}
};

////////////////////////////////////////////////////////////////////////////
if (!vec.empty())
	{
		if (width > 0)
		{
			std::ostringstream oss;

			auto it = vec.begin();

			oss << *it;

			for (++it; it != vec.end(); ++it)
				oss << "," << *it;

			// calc size
			os << std::setw(width) << oss.str();
		}
		else
		{
			auto it = vec.begin();

			os << *it;

			for (++it; it != vec.end(); ++it)
				os << "," << *it;
		}
	}
////////////////////////////////////////////////////////////////////////////

//const unsigned m_K;  //!< Maximum number of most probable config needed

template <typename T> void InitContainer(std::vector<T>& c, const T& e0)
{
	c.resize(1, e0);
}

template <typename T> void InitContainer(std::vector<T>& c, const T& e0)
{
	c.resize(1, e0);
}
/*!
	STL vector that can be initialized as

	STLInitVector<T> v = {a, b, ..., z};
*/
template <typename T> struct STLInitVector : public std::vector<T>
{
	/*STLInitVector(std::initializer_list<T> init_list)
		: std::vector<T>(init_list.size())
	{
		auto it0 = begin();
		auto it1 = init_list.begin();

		for (; it1 != init_list.end(); ++it0, it1)
			*it0 = *it1;
	}*/

	STLInitVector(const T& e0) : std::vector<T>(1, e0) { }
};
////////////////////////////////////////////////////////////////////////////
std::list<std::pair<unsigned,bool>> path;
class BeliefPropagationGraph : AttributedGraph<ConditionalProbabilityTable, int>
{
public:

};

/*!
		Init the probabilities using a given conditional probability
		function.
	*/
	void Init(ConditionalProbabilityFunction* pFun)
	{
		for (unsigned i = 0; i < m_table.ni(); ++i)
		{
			BinaryInstance r((int) i);

			for (unsigned j = 0; j < m_table.nj(); ++j)
			{
				BinaryInstance s((int) j);

				ConditionalProbability& cp = m_table(i, j);

				cp.reminder = r;
				cp.separator = s;

				cp.pr = pFun->pr(m_R, r, m_S, s);
			}
		}
	}

	const BottomUpBeliefMessage* CreateParentMessage()
	{
		ConsistentCandidates cc(m_childMsgs.size());

		for (unsigned i = 0; i < m_table.ni(); ++i)
		{
			BinaryInstance r((int) i);

			for (unsigned j = 0; j < m_table.nj(); ++j)
			{
				BinaryInstance s((int) j);

				ConditionalProbability& cp = m_table(i, j);

				unsigned childIdx = 0;
				auto it = m_childMsgs.begin();

				// Get one consistent candidate per child node
				for (; it != m_childMsgs.end(); ++it, ++childIdx)
				{
					cc[childIdx] = (*it)->Get(cp.reminder, cp.separator);
				}

				m_parentMsg.Add(ComputeMaxProducts(cp, cc), 
					cp.reminder, cp.separator);				
			}
		}

		struct ConditionalProbability
{
	double pr;                  //!< Probability p(R=reminder | S=separator)
	BinaryInstance reminder;    //!< Instantiation of the reminder part
	BinaryInstance separator;   //!< Instantiation of the separator part

	//! Path from leaf variables that lead to this conditional probability
	std::list<std::pair<unsigned,bool>> path;

	ConditionalProbability()
	{
		// Set pr to 1 so that it can be multiplied rigth away
		pr = 1; 
	}

	ConditionalProbability(const double& p, const BinaryInstance& r,
		const BinaryInstance& s)
		: pr(p), reminder(r), separator(s)
	{
	}

	void operator=(const ConditionalProbability& rhs)
	{
		pr = rhs.pr;
		reminder = rhs.reminder;
		separator = rhs.separator;
	}

	bool operator>(const ConditionalProbability& rhs) const
	{
		return pr > rhs.pr;
	}
};
////////////////////////////////////////////////////////////////////////////
	struct Product
	{
		double val;
		std::vector<unsigned> terms;
		const ConsistentCandidates* pCandidates;

		Product(const ConsistentCandidates* cc) : pCandidates(cc)
		{
			val = 0; // this is assumed in ComputeMaxProducts
			pCandidates = NULL;
		}

		void operator=(const Product& rhs)
		{
			val = rhs.val;
			terms = rhs.terms;
			pCandidates = rhs.pCandidates;
		}

		bool operator>(const Product& rhs)
		{
			return val > rhs.val;
		}
	};

std::list<MaxProducts> prodsList(1, MaxProducts(1, bestProd));

		// Now find the suboptimal maximum producs
		MaxProducts abs_maxprods;
		MaxProducts cand_maxprods;

		abs_maxprods.reserve(m_K);

		cand_maxprods = ComputeMaxProducts(bestProd, m_K - 1);

		prodsList.push_back(cand_maxprods);

		// The order of the list matters. The first element corresponds
		// to top value, the second to the send best value and so on
		for (auto it = prodsList.begin(); it != prodsList.end(); ++it)
		{
			// Find max product in list
			abs_maxprods.push_back(max(*it));
		}

		for (unsigned k = m_K; k; --k)
		{
			
		}
////////////////////////////////////////////////////////////////////////////
for (auto it = cc.begin(); it != cc.end(); ++it)
const BottomUpBeliefMessage::Candidates& cand = **it;
BottomUpBeliefMessage m_msg;

std::map<BinaryInstance, std::less<BinaryInstance>> m_inst2col;
////////////////////////////////////////////////////////////////////////////
/*VisSysComponent::~VisSysComponent() 
{
	//DBG_PRINT1(Name())
	// Get the derived classes a chance to release 
	// their resources

	std::cerr << Name() << std::endl;


	Clear(); 
}*/
////////////////////////////////////////////////////////////////////////////
Tuple<std::string, DIM, BEGIN_BRACKET, END_BRACKET> aux;

	is >> aux;

	for (unsigned i = 0; i < st.size(); ++i)
		st[i] = atof(aux[i].c_str());
/////////////////////////////////////////////////////////////////////////////
/*!
*/
template <unsigned DIM> struct StringTuple : public Tuple<std::string, DIM>
{
	const std::string& operator[](unsigned i) const
	{
		return BASE_CLASS::operator[](i);
	}

	std::string& operator[](unsigned i)
	{
		return BASE_CLASS::operator[](i);
	}
};

/*!
*/
template <unsigned DIM> struct NumericTuple : public Tuple<double, DIM>
{
	const double& operator[](unsigned i) const
	{
		return BASE_CLASS::operator[](i);
	}

	double& operator[](unsigned i)
	{
		return BASE_CLASS::operator[](i);
	}
};
/////////////////////////////////////////////////////////////////////////////
// Declare the serialization functions for all types of medatada
//DECLARE_BASIC_SERIALIZATION(vpl::VideoParseMetadata)
//DECLARE_BASIC_SERIALIZATION(vpl::FrameParseMetadata)
/////////////////////////////////////////////////////////////////////////////
bool loaded;
for (loaded = modelDB.LoadFirst(parseMeta, ii); loaded; loaded = modelDB.LoadNext(parseMeta, ii))

for (loaded = modelDB.LoadFirst(shapeMeta); loaded; loaded = modelDB.LoadNext(shapeMeta))
/////////////////////////////////////////////////////////////////////////////
//IndexInfo m_lastRead; //!< Index info of last object read

if (!Load(x, ii))
			{
				ii.objId = INVALID_STORAGE_ID;
				return false;
			}

			return true;
/////////////////////////////////////////////////////////////////////////////
		template <typename T> bool LoadNext(T& x, IndexInfo& ii) const
		{
			ASSERT(TypeName(x) == m_lastRead.className);

			SimpleDatabase* pDB = const_cast<SimpleDatabase*>(this);

			++(pDB->m_lastRead.objId);

			return Read(x, *m_lastRead.pDataIndex, m_lastRead.objId);
		}
/////////////////////////////////////////////////////////////////////////////
		//! Returns true if the pointer in NOT null, and false otherwise.
		operator bool() const
		{
			return pts != NULL;
		}
/////////////////////////////////////////////////////////////////////////////
