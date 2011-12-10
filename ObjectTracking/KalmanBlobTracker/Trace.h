/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <iostream>
#include <list>
#include <vector>
#include "../BlobTrackerDBManager.h"
//#include <ImageSegmentation/Blob.h>
#include "filteredpoint.h"

#define TIME_STEP 0.1

#define KALMAN_START_LENGTH 2u
#define DISPLACEMENT_THRESHOLD 50.0

namespace vpl {

#ifdef SMOOTH_TRACES_ONLINE
struct FilteredFeature {
	FilteredPoint centroid;
	CvKalman* size;
	CvKalman* width;
	CvKalman* height;
	CvKalman* color_histogram[Blob::HISTO_SIZE];
};
#endif

struct Range {
	int min;
	int max;
};

class Trace
{
public:
	enum SMOOTHING_METHOD {NO_SMOOTH, ONLINE_KALMAN, 
		OFFLINE_POLYLINE, OFFLINE_BEZIER};

	static int s_traceCount;

protected:
	std::vector<BlobPtr> m_nodes;
	double m_displacement;

#ifdef SMOOTH_TRACES_ONLINE
	std::vector<BlobPtr> m_filtered_nodes;
	FilteredFeature m_filtFeature;
	BlobPtr m_expected_feature;

	bool m_kalman_filter_is_initialized;
	void Initialize_Kalman_Filter();
#endif

	bool m_is_matched;

	unsigned m_id;
	fnum_t m_start_frame;

	CvScalar m_color;
	
	Range m_x_range, m_y_range;

	// To do with database storage
	int m_dbID; //!< ID of the trace in the database
	unsigned m_numFeaturesSaved; //!< Number of feature already saved to the database

public:
	Trace(fnum_t start_frame, unsigned unique_identifier);

	Trace(const BlobTrace& blobs)
	{
		s_traceCount++;

		m_nodes.reserve(blobs.size());
		m_nodes.assign(blobs.begin(), blobs.end());

		m_id = 0;
		m_color = cvScalar(0);
		m_displacement = 0;
		m_is_matched = false;
	}

	~Trace();

	const Blob& expected_feature() const
	{
#ifdef SMOOTH_TRACES_ONLINE
		return *m_expected_feature;
#else
		ASSERT(!m_nodes.empty());

		return *m_nodes.back();
#endif
	}

	double average_displacement() const
	{
		return (m_nodes.size() > 1) ? m_displacement / (m_nodes.size() - 1) : 0; 
	}

	void add_path_node(BlobPtr feature);

	bool is_blob_close(BlobPtr blob);

	unsigned get_length() const   { return m_nodes.size(); }

	int get_id() const            { return m_id; }

	fnum_t get_first_frame() const { return m_start_frame; }
	
	fnum_t get_last_frame() const 
	{ 
		return (get_first_frame() + get_length()) - 1; 
	}

	time_t start_time() const { return m_nodes.empty() ? 0 : m_nodes.front()->timestamp(); }
	time_t end_time() const   { return m_nodes.empty() ? 0 : m_nodes.back()->timestamp(); }

	void set_color(CvScalar new_color);
	
	CvScalar get_color() const { return m_color; }

	void draw(RGBImg image, SMOOTHING_METHOD smoothMethod = NO_SMOOTH);
	void output_nodes(std::ostream& ostr);
	void get_matching_region(XYCoord &A, XYCoord &B);

	BlobPtr get_last_feature()
	{
		ASSERT(!m_nodes.empty());

		return m_nodes.back();
	}

#ifdef SMOOTH_TRACES_ONLINE
	//To do with Kalman Filtering
	void Perform_Kalman_Filtering();
	void Evolve();
	void Predict_Kalman_Filter();
	void Correct_Kalman_Filter(BlobPtr m);

	const FilteredFeature& filtered_feature() const
	{
		return m_filtFeature;
	}

	//void Combine(const TracePtr& T);
	Blob get_kalman_state_post();
	Blob get_kalman_state_pre();
#endif

	bool is_mobile() const;
	BlobPtr get_first_feature();
	inline XYCoord get_velocity();
	
	bool is_matched() const
	{
		return m_is_matched;
	}

	void set_matched(bool state)
	{
		m_is_matched = state;
	}

	void SaveChangesToDatabase(vpl::BlobTrackerDBManager& dbm, int target_id);
};

#ifdef SMOOTH_TRACES_ONLINE
inline XYCoord Trace::get_velocity()
{
	return m_filtFeature.centroid.State(1);
}
#endif

// Functions involving traces and blob features

double cost_value(const Blob& f, const Blob& g);
bool blob_features_are_equal(BlobPtr f, BlobPtr g);
bool f_contains_g(BlobPtr f, BlobPtr g);

} // namespace vpl
