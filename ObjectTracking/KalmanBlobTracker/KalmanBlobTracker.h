/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "Target.h"
#include "../BlobTracker.h"

//class ConnectedComponents;
//struct CvMat;

namespace vpl {

/*!
	Glossary of terms used in this code.

	History <- has old traces
	Traces <- each has all active traces (might be associated with targets or not)
	Targets <- each has one or more traces.

	Trace and target "Mobility":
	- A trace is "mobile" if it centroid has shifted by more than DISPLACEMENT_THRESHOLD
	at any point in time.
	- A target is "mobile" if any of its traces is mobile.
	
	"being tracked", "dormant", "recent", and "active" targets:

	- A target is "being tracked" if the last frame of its last trace is the current frame.
	- A target is "dormant" if it is not "being tracked".
	- A target is "recent" if the last frame number of its last trace is is smaller than  
	  DORMANT_TIME frames from the current frame.
    - A target is "active" if it is "being tracked". Ie, "active" is a synonym of "being tracked".

	Note:
	Traces are extended by matching them against the blobs in the current frame. If a trace cannot
	be extended, it is moved to the history (or simply deleted if its too short).

	The next position of each trace is predicted using a Kalman filter based on previous positions. 
	The predicted position is a feature of the trace, and is stored in "expected_feature".

	The "Targets" are lists of traces. It is basically the stitching of traces that is necessary to
	deal with occlusions (and similar problems).

	General idea:

	1. Connected components are matched to all traces in m_traces. If a trace has no match, it is
	removed from m_traces. If a component has no match, it is used to create a new trace.

	2. The traces in m_traces are matched to the targets in m_targets as long as: (a) the trace
	is not matched to a target already, and (b) is non transient (ie, is long enough).

	3. The unmatched traces that are longer than TRANSIENT_TIME + MATCH_TIME are used to 
	create new targets.

	Important properties:

	A. At time t, if a trace is not extended, it is removed from future consideration.
	B. A target is matched to traces at time t only if it's last trace was not extended.
	C. If a target is extended with a new trace at time t, it is because it's last trace 
	   was not extended at time t. In that case, it last frame is also removed from m_traces 
	   and won't have a chance to be extended in the future.
	D. This means that the only traces that can grow are those unmatched to any target or
	   last in the list of some target's traces.

	Main functions:

	Run(): Performs the following steps...
			- retrieve_image_components(); // does connected component on foreground pixels
			
			- match_connected_components_with_traces(); // match components to traces
			
			- perform_kalman_filtering(); // Predicts a new position and bbox for each trace

			- predict_occluded_target_position(); // "Evolve" the centroid of each target that is
			                                      // "recent" and NOT "being tracked".

			- match_traces_with_targets();  // matches unassigned traces to dormant targets. If the
			                                // targets are recent, their centroids are evolved.
											// Otherwise, they will be static wrt the last "evolution"
											// performed on them.

	match_connected_components_with_traces(): matches traces with connected components. The traces that
	                           are not matched to a component are moved to the history if they are
							   long enough, or deleted if they are too short. Each matched component is 
							   used to extend a trace by adding one new node to it. The connected 
							   components that are left unmatched are used to create new traces.

    get_traces_for_matching(): gets the traces that are longer than TRANSIENT_TIME.

	get_targets_for_matching(): gets the targets that are NOT "being tracked". That is,
	                            all the targets that are dormant.

	match_traces_with_targets(): it matches all unassigned traces to all dormant targets. Next,
	                            the unassigned traces that are longer than 
								TRANSIENT_TIME + MATCH_TIME are used to create new targets.
*/
class KalmanBlobTracker : public BlobTracker
{
public:
	struct Params {
		int minComponentSize;
		time_t maxInactiveTargetTime;
		time_t maxTargetTime;
		double maxDisplacementCoeff;
	};

	typedef cv::Mat_<float> Matrix;
	typedef std::vector<TargetPtr> Targets;
	typedef std::vector<TracePtr> Traces;

private:
	Params m_params;

    std::vector<BlobPtr> m_features;
    
    std::vector<BlobPtr> m_features_for_matching;      //!< A temporary list of features used for the matching with traces
    Traces m_traces;
    Targets m_targets;

	std::list<TracePtr> m_history;

	unsigned m_targetCount; //!< Total number of targets created at any point in time
	unsigned m_traceCount; //!< Total number of traces created at any point in time

	RGBImg m_inputImg; //!< Shared pointer to the current input image for drawing purposes

    void match_connected_components_with_traces();

    /* dealing with duplicated traces*/
    void set_best_matches(Matrix& M, const Matrix& C);  //checks for each component
    double get_minimum_cost_of_component(int j, const Matrix& C, const Matrix& M); //of component j
    double get_minimum_cost_of_trace(int i, const Matrix& C, const Matrix& M); //of trace i
    void set_minimum_matches_per_component(Matrix& M, const Matrix& C, int j, double min_cost); //of component j
    void set_minimum_matches_per_trace(Matrix& M, const Matrix& C, int i, double min_cost); //of trace i

    /*dealing with separated components*/
    std::vector<BlobPtr> get_matched_components(int i, const Matrix& M);

	BlobPtr merge_components(std::vector<BlobPtr>& target_components) const;
    double get_minumum_cost_of_trace(int i, const Matrix& C, const Matrix& M);
    
	void replace_first_match(int i, BlobPtr c, Matrix& M);

    void remove_non_minimal(Matrix& M, const Matrix& C, int i, double min_cost);

    void merge_overlapped_components();
    void merge_feature(std::vector<BlobPtr>* list_to_merge, int index);
    bool features_overlap(BlobPtr f, BlobPtr g, float threshold);


    // dealing with matching
    void assign_traces_to_components(const Matrix& M);
    bool get_matched_component(const Matrix& M, unsigned i, unsigned* j);
    void update_traces_in_history();
    void match_traces_with_history();
    void match_this_trace_with_history(TargetPtr T);

    bool trace_motion_is_similar(TracePtr t1, TracePtr t2) const;
    bool target_is_being_tracked(TargetPtr T) const;
    bool target_is_recent(TargetPtr T) const;
    Traces get_unmatched_traces() const;

#ifdef SMOOTH_TRACES_ONLINE
    void perform_kalman_filtering();
    void predict_occluded_target_position();
#endif

    void add_target(TracePtr tr);
	void add_trace(BlobPtr bf);

    Matrix get_target_cost_matrix(const Traces& traces, 
		const Targets& targets);

    Traces get_traces_for_matching() const;
    Targets get_targets_for_matching() const;

	void match_traces_with_targets(const Traces& traces, 
		const Targets& targets);

    void match_traces_with_targets();
    
	void get_minimum_in_row(double & minimum_in_row, unsigned & min_column_index, 
		const Matrix& M, const unsigned & row_index);

    void get_minimum_in_col(double & minimum_in_col, const Matrix& M, const unsigned & col_index);
    
	bool trace_matches_a_target(const unsigned & trace_index, unsigned* target_index, const Matrix& M);

    // Matrix initalizers
    Matrix get_distance_matrix() const;
    Matrix get_cost_matrix(const Matrix& M) const;

public:
	KalmanBlobTracker()
	{
		m_targetCount = 0;
		m_traceCount = 0;
	}

	//! Returns all the traces that are processed
    std::list<TracePtr> get_history() const
	{
		return m_history;
	}

	//! Returns all the traces that are being processed.
    Traces get_active_traces() const
	{
		return m_traces;
	}

    Targets get_targets() const
	{
		return m_targets;
	}

    Targets get_active_targets() const;

    std::vector<BlobPtr> get_features() const
	{
		return m_features;
	}

    std::string draw_traces(RGBImg img, int scope) const;
    std::string draw_targets(RGBImg img, int scope) const;

public: // Required functions from parent VisSysComponent
	virtual void ReadParamsFromUserArguments();

	virtual void Initialize(graph::node v);

	virtual void Clear();

	virtual std::string ClassName() const
	{
		return "KalmanBlobTracker";
	}
	
	virtual ByteImg GetBlobMask() const;

	virtual void Run();

	virtual void GetDisplayInfo(const DisplayInfoIn& dii, 
		DisplayInfoOut& dio) const;

	virtual void GetParameterInfo(int i, DoubleArray* pMinVals, 
		DoubleArray* pMaxVals, DoubleArray* pSteps) const
	{
		InitArrays(1, pMinVals, pMaxVals, pSteps, 0, 4, 1);
	}
	
	virtual int NumOutputImages() const 
	{ 
		return 3; 
	}

	virtual std::string GetOutputImageLabel(int i) const
	{
		ASSERT(i >= 0 && i <3);

		switch (i)
		{
		case 0: return "Traces";
		case 1: return "Targets";
		case 2: return "Components";
		}

		return "error";
	}
};

} // namespace vpl
