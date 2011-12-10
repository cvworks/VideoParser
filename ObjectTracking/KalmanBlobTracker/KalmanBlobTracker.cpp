/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "KalmanBlobTracker.h"

#include <VideoParser/ImageProcessor.h>
#include <ImageSegmentation/BlobFinder.h>
#include <Tools/Num2StrConverter.h>
#include <Tools/UserArguments.h>
#include <Tools/CvMatView.h>

#define ABSOLUTE_MIN_COST 1.5

/*! 
	Used in target_is_recent(), which is called by predict_occluded_target_position() and
	get_target_cost_matrix().
*/
#define DORMANT_TIME 100u

//! The traces considered for matching or for the history must be longer than this
#define TRANSIENT_TIME 3u

//! Novel targets are created from unassigned traces longer than TRANSIENT_TIME + MATCH_TIME
#define MATCH_TIME 3u  

using namespace vpl;

extern UserArguments g_userArgs;

///////////////////////////////////////////////////////
// begin required functions from parent VisSysComponent
void KalmanBlobTracker::ReadParamsFromUserArguments()
{
	BlobTracker::ReadParamsFromUserArguments();

	g_userArgs.ReadArg(Name(), "minComponentSize", 
		"Minimum size of a connected component that is detected", 
		200, &m_params.minComponentSize);

	g_userArgs.ReadArg(Name(), "maxInactiveTargetTime", 
		"Maximum number of seconds that a target can be inactive before being erased", 
		time_t(60), &m_params.maxInactiveTargetTime);

	g_userArgs.ReadArg(Name(), "maxTargetTime", 
		"Maximum number of seconds that any target can last before being erased", 
		time_t(60 * 3), &m_params.maxTargetTime);

	g_userArgs.ReadArg(Name(), "maxDisplacementCoeff", 
		"A target can match to a trace if the displacement between them is less than "
		"the average displacement times this factor", 
		5.0, &m_params.maxDisplacementCoeff);
}

void KalmanBlobTracker::Initialize(graph::node v)
{
	BlobTracker::Initialize(v);

	cvSetErrMode(CV_ErrModeSilent);

	//#define CV_ErrModeLeaf 0
	//#define CV_ErrModeParent 1
	//#define CV_ErrModeSilent 2
}

void KalmanBlobTracker::Clear()
{
	BlobTracker::Clear(); // it resets the dbm session

	m_targetCount = 0;
	m_traceCount = 0;

	m_history.clear();
	m_traces.clear();
	m_targets.clear();
}

void KalmanBlobTracker::Run()
{
	cvSetErrMode(CV_ErrModeSilent);

	if (!m_pImgProcessor)
	{
		ShowMissingDependencyError(ImageProcessor);
		return;
	}

	/*if (!m_pBackSub)
	{
		ShowMissingDependencyError(BackgroundSubtractor);
		return;
	}*/
	
	if (!m_pBlobFinder)
	{
		ShowMissingDependencyError(BlobFinder);
		return;
	}

	if (GetInputImageInfo().numFramesProcessed % 500 == 0)
	{
		StreamMsg("Memory usage at time " << Time(Timestamp()).str() << 
			" for frame number " << FrameNumber() << ".\n"
			" Targets: " << Target::s_targetCount << 
			" traces: " << Trace::s_traceCount << 
			" blobs: " << Blob::s_blobCount <<
			" too dark: " << m_pImgProcessor->IsTooDark() <<
			" noisy: " << m_pImgProcessor->IsNoisy());
	}

	// Begin by removing all inactive targets
	for (auto it = m_targets.begin(); it != m_targets.end(); )
	{
		if (Timestamp() - (*it)->end_time() > m_params.maxInactiveTargetTime)
			it = m_targets.erase(it);
		else if (Timestamp() - (*it)->start_time() > m_params.maxTargetTime)
			it = m_targets.erase(it);
		else
			++it;
	}

	// Then remove all innactive traces (this removals happen very infrequently)
	for (auto it = m_traces.begin(); it != m_traces.end(); )
	{
		if (!(*it)->is_matched() && Timestamp() - (*it)->end_time() > m_params.maxInactiveTargetTime)
			it = m_traces.erase(it);
		else
			++it;
	}

	if (m_pImgProcessor->IsTooDark())
	{
		ShowStatus("Image is too dark. Tracking is suspended.");
		return;
	}

	if (m_targets.size() > 30 || Trace::s_traceCount > 300)
		SetAntiNoiseMode(true);

	// The traces in 'm_traces' corresponds to all the traces that were matched
	// to a blob in the previous run plus the traces initiated by blobs that
	// were not matched to any trace. However, what we now want is the last traces
	// of any active target and the recent unmatched traces in 'm_traces'.
	/*Traces activeTraces;

	activeTraces.reserve(m_traces.size() + m_targets.size());

	for (auto it = m_traces.begin(); it != m_traces.end(); ++it)
		if (!(*it)->is_matched() && Timestamp() - (*it)->end_time() <= m_params.maxInactiveTargetTime)
			activeTraces.push_back(*it);

	for (auto it = m_targets.begin(); it != m_targets.end(); ++it)
		activeTraces.push_back((*it)->get_last_trace());

	m_traces = activeTraces;*/

	// Add an extra reference to the input image so that
	// it stays around in case we need to draw on it
	m_inputImg = m_pImgProcessor->GetRGBImage();

	m_features = m_pBlobFinder->GetBlobs();
	
	merge_overlapped_components();
	
	match_connected_components_with_traces();
	
#ifdef SMOOTH_TRACES_ONLINE
	perform_kalman_filtering();

	// Diego: the prediction alters the values of the centroids
	// and that might not be a good thing, so it's commented out...
	//predict_occluded_target_position();
#endif
	
	match_traces_with_targets();
	
	if (m_dbm)
	{
		if (!m_dbm.hasSession())
		{
			TrackingSessionInfo tsi(VideoFilename(), m_inputImg.ni(), m_inputImg.nj());

			m_dbm.createSession(tsi);
		}

		// Save any changes made to the targets if we have a session
		if (m_dbm.hasSession())
		{
			for (auto it = m_targets.begin(); it != m_targets.end(); ++it)
				(*it)->SaveChangesToDatabase(m_dbm);
		}
	}
}

void KalmanBlobTracker::GetDisplayInfo(const DisplayInfoIn& dii, 
	DisplayInfoOut& dio) const
{
	RGBImg inputImgCopy;
	
	inputImgCopy.deep_copy(m_inputImg);

	if (dii.outputIdx < 2)
	{
		int scope = (int)dii.params.front();

		if (dii.outputIdx == 0)
			dio.message = draw_traces(inputImgCopy, scope);
		else if (dii.outputIdx == 1)
			dio.message = draw_targets(inputImgCopy, scope);

		dio.imageType = RGB_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(inputImgCopy);
	}
	else // components image
	{
		for (auto it = m_features.begin(); it != m_features.end(); it++)
			(*it)->draw(inputImgCopy);

		dio.imageType = RGB_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(inputImgCopy);
	}
}
///////////////////////////////////////////////////////
// end required functions from parent VisSysComponent
ByteImg KalmanBlobTracker::GetBlobMask() const
{
	return m_pBlobFinder->GetBlobMask();
}

KalmanBlobTracker::Targets KalmanBlobTracker::get_active_targets() const
{
	Targets return_list;

	for (auto iter = m_targets.begin(); iter!=m_targets.end(); iter++)
	{
		if (target_is_being_tracked(*iter)) 
			return_list.push_back(*iter);
	}

	return return_list;
}

//! Returns the distance matrix between traces and blobs
KalmanBlobTracker::Matrix KalmanBlobTracker::get_distance_matrix() const
{
	Matrix M(m_traces.size(), m_features_for_matching.size());
	int i, j;

	M = 0;
	j = 0;

	//setup M matrix
	for (auto comp_iter = m_features_for_matching.begin();
		comp_iter != m_features_for_matching.end(); comp_iter++)
	{
		i = 0;
		for (auto trace_iter = m_traces.begin();trace_iter != m_traces.end();trace_iter++)
		{
			if ((*trace_iter)->is_blob_close(*comp_iter))
			{
				M(i, j) = 1;
			}
			else
			{
				M(i, j) = 0;
			}
			i++;
		}
		j++;
	}

	return M;
}

//returns the cost matrix
KalmanBlobTracker::Matrix KalmanBlobTracker::get_cost_matrix(const Matrix& M) const
{
	Matrix C(m_traces.size(), m_features_for_matching.size());

	int i, j;

	C = 0;
	j = 0;

	//setup M matrix
	for (auto comp_iter = m_features_for_matching.begin();
		comp_iter != m_features_for_matching.end(); ++comp_iter)
	{
		i = 0;

		for (auto trace_iter = m_traces.begin(); trace_iter != m_traces.end(); ++trace_iter)
		{
			if (M(i, j) != 0)
			{
				C(i, j) = (float)cost_value((*trace_iter)->expected_feature(), **comp_iter);
			}
			else
			{
				C(i, j) = -1.0;
			}

			i++;
		}
		j++;
	}

	return C;
}

//given trace #i, returns a set of components that are matched with trace #i
std::vector<BlobPtr> KalmanBlobTracker::get_matched_components(int i, const Matrix& M)
{
	std::vector<BlobPtr> result;
	
	for (int j = 0; j < (int)m_features_for_matching.size(); j++)
		if (M(i, j)) 
			result.push_back(m_features_for_matching[j]);

	return result;
}

//! Takes a set of components as its input and returns the merged version of all the components
BlobPtr KalmanBlobTracker::merge_components(std::vector<BlobPtr>& target_components) const
{
	ASSERT(target_components.size() >= 2);

	BlobPtr newBlob(new Blob(*target_components.front()));
	
	for (size_t i = 0; i < target_components.size(); i++)
		newBlob->merge(*target_components[i]);
		
	return newBlob;
}

//given a trace i, returns the minumum cost assigned to it in matrix C
double KalmanBlobTracker::get_minumum_cost_of_trace(int i, const Matrix& C, const Matrix& M)
{
	double cost = 9999999; //very large number
	
	for (int j = 0; j < (int)m_features_for_matching.size(); j++)
		if (C(i, j) < cost && M(i, j)) 
			cost = C(i, j);

	return cost;
}

/*! 
	Replaces the first component that is matched with trace i with component c 
	and resets all the other matches to 0 in matrix M

	replaces c with the first match in C and resets all the other matches to 0 in M
*/
void KalmanBlobTracker::replace_first_match(int i, BlobPtr c, Matrix& M)
{
	int first_match = -1;

	for (int j = 0; j < (int)m_features_for_matching.size(); j++)
	{
		if (M(i, j) && first_match==-1)
		{
			first_match = j;
			*m_features_for_matching[j] = *c;
		}
		else if (M(i, j) && first_match != -1)
		{
			M(i, j) = 0;
			m_features_for_matching[j].reset();
		}
	}
}

//resets all the non-minimal matches to 0 in matrix M with trace #i
void KalmanBlobTracker::remove_non_minimal(Matrix& M, const Matrix& C, int i, double min_cost)
{
	for (int j=0;j<(int)m_features_for_matching.size();j++)
		if (C(i, j) != min_cost && M(i, j))
		{
			M(i, j) = 0;

			m_features_for_matching[j].reset();
		}
}


/*! 
	@brief Match traces with the best matching component from the component list.

	Matches traces with the best matching component from the component list. If there 
	are no components, the traces that are longer than TRANSIENT_TIME are copied to the
	history and the shorter ones are deleted.

	It assigns the one-to-one feature-to-traces matches found. Each matched feature is 
	used to extend a trace by adding one new node to it.

	In concludes by creating traces with the connected components that were left 
	unmatched.
*/
void KalmanBlobTracker::match_connected_components_with_traces()
{
	Traces matchedTraces;

	m_features_for_matching = m_features;

	// If there's any trace to match the components, do the matching
	if (!m_traces.empty() && !m_features_for_matching.empty())
	{
		// Scan each component and each trace and set up M matrix if they 
		// are in a close distance from each other
		Matrix M = get_distance_matrix();
		
		// Create a cost matrix only for the matches from matrix M
		Matrix C = get_cost_matrix(M);  

		// Modify the matrix M (set the lowest costs to 1 and the rest to 0)
		set_best_matches(M, C); 

		// Assign components to traces. If a *component* is matched to a trace,  
		// removed it from m_features_for_matching.
		for (unsigned i = 0, j; i < m_traces.size(); i++)
		{
			if (get_matched_component(M, i, &j))
			{
				m_traces[i]->add_path_node(m_features_for_matching[j]);
			
				m_features_for_matching[j].reset();

				matchedTraces.push_back(m_traces[i]);
			}
			else
			{
				// The trace i doesn't match to any component. Move it to m_history if 
				// it's long enough. Otherwise, just delete it.
				//if (m_traces[i]->get_length() > TRANSIENT_TIME)
				//	m_history.push_back(m_traces[i]); 
			}
		}
	}
	else if (m_features_for_matching.empty()) // if there's no component but there are traces
	{
		// We are going to delete all unmatched traces. Move long traces to the history.
		//for (auto iter= m_traces.begin(); iter != m_traces.end(); iter++)
		//	if ((*iter)->get_length() > TRANSIENT_TIME)
		//		m_history.push_back(*iter);
	}

	// Remove unmatched traces
	m_traces = matchedTraces;

	// Finally, create new traces from the components that are left
	for (auto iter = m_features_for_matching.begin(); 
		iter != m_features_for_matching.end(); iter++)
	{
		if (*iter)
			add_trace(*iter);
	}

	// Empty the list of features to match
	m_features_for_matching.clear();
}

//finds the index of the component associated to trace i.
//if no component is found, it returns -1
bool KalmanBlobTracker::get_matched_component(const Matrix&M, unsigned i, unsigned* j)
{
	for (*j = 0; *j < m_features_for_matching.size(); (*j)++)
		if (M(int(i), int(*j))) 
			return true;

	return false;
}

/*! 
	select the best(minimum cost) match for each component. set_best_matches() 
	only modifies the M(match) matrix.
*/
void KalmanBlobTracker::set_best_matches(Matrix& M, const Matrix& C)
{
	//Do a "per component" matching
	for (int j = 0; j < (int)m_features_for_matching.size(); j++)
		set_minimum_matches_per_component(M, C, j, get_minimum_cost_of_component(j, C, M));

	//Do a "per trace" matching
	//for (int j=0; j<m_traces.size(); j++)
	//   set_minimum_matches_per_trace(M, C, j, get_minimum_cost_of_trace(j, C, M));

}

//returns the minimum cost of component i
double KalmanBlobTracker::get_minimum_cost_of_component(int j, const Matrix& C, const Matrix& M)
{
	double cost = 1E308; //start with a very large cost
	for (int i=0;i<(int)m_traces.size();i++)
		if (C(i, j) < cost && M(i, j) == 1) 
			cost = C(i, j);

	return cost;
}

double KalmanBlobTracker::get_minimum_cost_of_trace(int i, const Matrix& C, const Matrix& M)
	/*************************************************************************************************
	* By Jeff Lalonde
	* Date: 2009/05/12
	* Description: Returns the minimum cost for trace j
	*************************************************************************************************/
{
	double cost = 1E308; //start with a very large cost
	
	for (int j=0;j<(int)m_features_for_matching.size();j++)
		if (C(i, j) < cost && M(i, j) == 1) 
			cost = C(i, j);

	return cost;
}

//! sets the matrix M where the cost in matrix C is equal to min_cost
void KalmanBlobTracker::set_minimum_matches_per_component(
	Matrix& M, const Matrix& C, int j, double min_cost)
{
	if (min_cost > ABSOLUTE_MIN_COST)
	{
		for (int i=0;i<(int)m_traces.size();i++) 
			M(i, j) = 0;
	}
	else
	{
		for (int i=0;i<(int)m_traces.size();i++)
			if (C(i, j) != min_cost)
				M(i, j) = 0;
			else
				min_cost = -2; // the rest of the elements are going to be set to 0
	}
}

void KalmanBlobTracker::set_minimum_matches_per_trace(
	Matrix& M, const Matrix& C, int i, double min_cost)
	/*************************************************************************************************
	* By Jeff Lalonde
	* Date: 2009/05/12
	* Description: Returns the minimum cost for trace j
	*************************************************************************************************/
{
	if (min_cost > ABSOLUTE_MIN_COST)
	{
		for (int j = 0; j < (int)m_features_for_matching.size(); j++) 
			M(i, j) = 0;
	}
	else
	{
		for (int j=0;j<(int)m_features_for_matching.size();j++)
			if (C(i, j) != min_cost)
				M(i, j) = 0;
			else
				min_cost = -2; // the rest of the elements are going to be set to 0
	}
}

void KalmanBlobTracker::merge_overlapped_components()
{
	int i = 0;
	
	for (auto iter = m_features.begin();iter != m_features.end(); iter++)
	{
		merge_feature(&m_features,i);
		i++;
	}

	//Create merged list
	std::vector<BlobPtr> merged_features;

	for (auto iter = m_features.begin(); iter != m_features.end(); iter++)
	{
		if (*iter) merged_features.push_back(*iter);
	}

	m_features = merged_features;

	//cout << "Number of m_features after = " << m_features.count() << endl;
	//if (m_features.count() > 0 ) cout << m_features.first()->ul().x << "  " << m_features.first()->ul().y << endl;
}


void KalmanBlobTracker::merge_feature(std::vector<BlobPtr>* list_to_merge, int index)
{
	std::vector<BlobPtr> merged_list;
	BlobPtr f = (*list_to_merge)[index];
	if (f)
	{
		for (std::vector<BlobPtr>::iterator iter = list_to_merge->begin(); iter != list_to_merge->end(); iter++)
		{
			BlobPtr g = *iter;
			if (g && f!=g)
			{
				if (features_overlap(f, g, 0.5))
				{
					f->merge(*g);

					(*iter).reset();
				}
			}
		}
	}
}

bool KalmanBlobTracker::features_overlap(BlobPtr f, BlobPtr g, float threshold)
{
	int A1 = (f->br().x - f->br().x)*(f->br().y - f->br().y);
	int A2 = (g->br().x - g->br().x)*(g->br().y - g->br().y);


	int x1,x2;
	if (f->ul().x < g->ul().x)
	{
		x1 = g->ul().x;
		x2 = f->br().x;
	}
	else
	{
		x1 = f->ul().x;
		x2 = g->br().x;
	}

	int Lx = x2 - x1;
	if (Lx < 0) return false;

	int y1,y2;
	if (f->ul().y < g->ul().y)
	{
		y1 = g->ul().y;
		y2 = f->br().y;

	}
	else
	{
		y1 = f->ul().y;
		y2 = g->br().y;
	}

	int Ly = y2-y1;
	if (Ly < 0) return false;

	float A_overlap = (float)Lx*(float)Ly;

	float A_min;
	if (A1 < A2) A_min = (float)A1;
	else A_min = (float)A2;

	if (A_min > 0)
		return ((A_overlap / A_min) > threshold);
	else
		return false;
}

#ifdef SMOOTH_TRACES_ONLINE
void KalmanBlobTracker::perform_kalman_filtering()
{
	//For each trace being tracked, perform kalman filtering
	for (auto iter = m_traces.begin(); iter != m_traces.end(); iter++)
	{
		(*iter)->Perform_Kalman_Filtering();
	}
}

/*!
	Evolves the centroid of each target that is "recent" and NOT "being tracked".

	For each target thought to be occluded (ie, recent but not active), 
	it predicts its position.
*/
void KalmanBlobTracker::predict_occluded_target_position()
{
	TargetPtr T;

	for (auto iter = m_targets.begin(); iter != m_targets.end(); iter++)
	{
		T = *iter;

		if (!target_is_being_tracked(T) && target_is_recent(T))
		{
			T->Evolve();
		}
	}
}
#endif

void KalmanBlobTracker::match_traces_with_targets(const Traces& tr2match, 
	const Targets& tgt2match)
{
	//Get the similarity matrix for the m_traces and m_targets that require a match
	Matrix M = get_target_cost_matrix(tr2match, tgt2match);    
			
	const unsigned num_rows = M.rows;
	unsigned i, j;
	
	// for each trace
	for (i = 0; i < num_rows; i++)          
	{
		// see if the trace has a matching target
		if (trace_matches_a_target(i, &j, M))        
		{
			// Add the trace to the matching target
			// this makes the target "active" since now current
			// frame belong to its last trace
			tgt2match[j]->add_trace(tr2match[i]);
		}
	}
	
}

void KalmanBlobTracker::match_traces_with_targets()
{
	// Get traces for which is_matched == false and length > TRANSIENT_TIME
	Traces tr2match = get_traces_for_matching();  

	if (tr2match.empty())
		return;

	// Get targets that are NOT currently being tracked (ie, are inactive)
	Targets tgt2match = get_targets_for_matching();

	if (!tgt2match.empty())
	{
		match_traces_with_targets(tr2match, tgt2match);
	}
	// Diego: it seems better to remove the cas below
	/*else //there are no targets for matching, so for each trace, create a target.
	{
		// @todo This applies only at the begining, so it's confussing.
		for (auto iter = tr2match.begin(); iter != tr2match.end(); iter++)
			add_target(*iter);
	}*/

	// Create new targets from novel traces, where traces are
	// considered novel if no match is found after a certain amount of frames
	unsigned novel_length = TRANSIENT_TIME + MATCH_TIME;
	TracePtr tr;

	for (auto it = tr2match.begin(); it != tr2match.end(); ++it)
	{
		tr = *it;

		// Diego: added the check to see if the trace is already matched
		if (!tr->is_matched() && tr->get_length() > novel_length)
			add_target(tr);
	}
}

bool KalmanBlobTracker::trace_matches_a_target(const unsigned & trace_index, unsigned* target_index, 
	const Matrix& M)
{
	//returns true if the trace with index 'trace_index' matches a target and
	//returns the matching target's index: 'target_index'
	double row_min;
	double col_min;
	
	get_minimum_in_row(row_min, *target_index, M, trace_index);

	get_minimum_in_col(col_min, M, *target_index);
	
	if (row_min == col_min)
	{
		return (row_min < ABSOLUTE_MIN_COST);
	}
	else
	{
		return false;
	}
}

void KalmanBlobTracker::get_minimum_in_row(double & minimum_in_row, 
	unsigned & min_column_index, const Matrix& M, const unsigned & row_index)
{
	minimum_in_row = M(row_index, 0);

	min_column_index = 0;

	for (unsigned j=1; j<(unsigned)M.cols; j++)
	{
		if (minimum_in_row > M(row_index, j))
		{
			minimum_in_row = M(row_index, j);
			min_column_index = j;
		}
	}
}

void KalmanBlobTracker::get_minimum_in_col(double & minimum_in_col, const Matrix& M, 
	const unsigned & col_index)
{
	minimum_in_col = M(0, col_index);

	for (unsigned i=1; i<(unsigned)M.rows; i++)
	{
		if (minimum_in_col > M(i, col_index))
		{
			minimum_in_col = M(i, col_index);
		}
	}
}

/*!
	The idea is that a target that is not being tracked anymore might match
	a trace that is not associated with a target. Then, the beging of the trace should
	be a natural continuation of the target's last trace, among other things.
*/
KalmanBlobTracker::Matrix KalmanBlobTracker::get_target_cost_matrix(
	const Traces& tr2match, 
	const Targets& tgt2match)
{
	Matrix M(tr2match.size(), tgt2match.size());
	TracePtr F, G;
	int i = 0, j;

#ifndef SMOOTH_TRACES_ONLINE
	for (auto traceIt = tr2match.begin(); traceIt != tr2match.end(); ++traceIt, ++i)
	{
		F = *traceIt;
		j = 0;

		for (auto tgtIt = tgt2match.begin(); tgtIt != tgt2match.end(); ++tgtIt, ++j)
		{
			G = (*tgtIt)->get_last_trace();

			M(i, j) = (trace_motion_is_similar(F, G)) ? 
				(float)cost_value(*F->get_last_feature(), *G->get_last_feature()) 
				: 1E20f;
		}
	}
#else
	TargetPtr T;
	Blob f(0);      //Blob feature associated with trace F
	Blob g(0);      //Blob feature associated with trace G

	for (auto t_iter = tr2match.begin(); t_iter != tr2match.end(); t_iter++)
	{
		F = *t_iter;
		f = F->get_kalman_state_post();

		//For each target, get the similarity to trace tr
		j=0;
		for (auto iter = tgt2match.begin(); iter != tgt2match.end(); iter++)
		{
			T = (*iter);
			G = T->get_last_trace();

			// if the dormant trace has not been dormant long, match using 
			// the expected position and velocity
			if (target_is_recent(T))   
			{
				g = G->get_kalman_state_post();

				if (trace_motion_is_similar(F,G))
				{
					M(i, j) = (float)cost_value(f, g);
				}
				else
				{
					M(i, j) = 1E20f;
				}
			}
			else        // match using the object's appearance only - not robust, needs revision
			{
				// cvmSet(M,i,j,cost_value(&f,&g));
				M(i, j) = 1E20f;         // for the elevator application, do not match m_traces with old targets
			}
			j++;
		}
		i++;
	}
#endif

	return M;
}

KalmanBlobTracker::Traces KalmanBlobTracker::get_traces_for_matching() const
{
	Traces tr2match;
	TracePtr tr;

	for (auto it = m_traces.begin(); it != m_traces.end(); ++it)
	{
		tr = *it;

		// see if the trace is not yet matched to a target and if it's
		// non-transient and the Kalman Filters have been running for a few frames
		// @todo there is not check for how long the KF is been running
		if (!tr->is_matched() && tr->get_length() > TRANSIENT_TIME)
			tr2match.push_back(tr);
	}

	return tr2match;
}

/*!
	It returns the array of dorman targets. That is, all the targets that are
	not currently being tracked.
*/
KalmanBlobTracker::Targets KalmanBlobTracker::get_targets_for_matching() const
{
	Targets tgt2match;

	for (auto iter = m_targets.begin(); iter != m_targets.end(); iter++)
	{
		// if not already being tracked, i.e. the target is dormant
		if (!target_is_being_tracked(*iter))
			tgt2match.push_back(*iter);
	}

	return tgt2match;
}

/*!
	Adds a target and increments by one the target count, which
	counts the total number of targets created at any point in time.
	ie, m_targetCount >= m_targets.size().
*/
void KalmanBlobTracker::add_target(TracePtr tr)
{
	// Create a new target with a unique ID (starting from one)
	TargetPtr T(new Target(++m_targetCount));

	T->add_trace(tr);

	m_targets.push_back(T);
}

/*!
	Adds a trace and increments by one the trace count, which
	counts the total number of traces created at any point in time.
	ie, m_traceCount >= m_traces.size().
*/
void KalmanBlobTracker::add_trace(BlobPtr bf)
{
	TracePtr trace(new Trace(FrameNumber(), ++m_traceCount));

	trace->add_path_node(bf);

	m_traces.push_back(trace);
}

bool KalmanBlobTracker::target_is_being_tracked(TargetPtr T) const
{
	return (T->get_last_frame() == FrameNumber());
}

bool KalmanBlobTracker::target_is_recent(TargetPtr T) const
{
	// get the time since the target was last seen
	fnum_t time_elapsed = FrameNumber() - T->get_last_frame(); 

	return (time_elapsed < DORMANT_TIME);
}

/*!
*/
bool KalmanBlobTracker::trace_motion_is_similar(TracePtr t1, TracePtr t2) const
{
#ifdef SMOOTH_TRACES_ONLINE
	//difference in position
	double del_p = (t1->filtered_feature().centroid.State(0) 
		- t2->filtered_feature().centroid.State(0)).Norm();    

	//difference in velocity
	double del_s = (t1->filtered_feature().centroid.State(1) 
		- t2->filtered_feature().centroid.State(1)).Norm();    
	
	//the speed of trace 1
	double s1 = (t1->filtered_feature().centroid.State(1)).Norm();      

	//the speed of trace 2
	double s2 = (t2->filtered_feature().centroid.State(1)).Norm();     

	double s_max = (s1 > s2) ? s1:s2;

	bool velocity_is_similar;

	if (s_max > 0)
	{
		velocity_is_similar = (del_s/s_max < 1);
	}
	else
	{
		velocity_is_similar = true;
	}

	bool position_is_similar = (del_p < 100);

	return position_is_similar && velocity_is_similar;
#else
	double avgDisp1 = t1->average_displacement();
	double avgDisp2 = t2->average_displacement();

	double avgDisp = MAX(avgDisp1, avgDisp2);

	double del_p = (t1->get_last_feature()->centroid() 
		- t2->get_last_feature()->centroid()).Norm();    

	return (del_p <= avgDisp * m_params.maxDisplacementCoeff);
#endif

}

KalmanBlobTracker::Traces KalmanBlobTracker::get_unmatched_traces() const
{
	Traces unmatched_traces;
	
	for (auto t_iter = m_traces.begin(); t_iter != m_traces.end(); ++t_iter)
	{
		if (!(*t_iter)->is_matched())
			unmatched_traces.push_back(*t_iter);
	}

	return unmatched_traces;
}

std::string KalmanBlobTracker::draw_traces(RGBImg img, int scope) const
{
	XYCoord A, B;
	CvPoint p1, p2;
	TracePtr tr;
	std::string msg;
	Traces traces;

	//Draw every trace
	if (scope == 0)
	{
		traces = get_traces_for_matching();  
		msg = "traces for matching ";
		msg += Num2StrConverter(5).toString((int)traces.size());
	}
	else if (scope == 1)
	{
		traces = m_traces;
		msg = "only active traces";
	}
	else
	{
		traces.assign(m_history.begin(), m_history.end());
		traces.insert(traces.end(), m_traces.begin(), m_traces.end());
		msg = "history and active traces";
	}

	for (auto it = traces.begin(); it != traces.end(); ++it)
	{
		tr = *it;
		tr->draw(img);
		tr->get_matching_region(A,B);
		BlobPtr b = tr->get_last_feature();
		p1 = cvPoint(b->ul().x,b->ul().y);
		p2 = cvPoint(b->br().x, b->br().y);
		//cvRectangle(img, p1, p2, tr->get_color(), 1, 1, 0);
	}

	return msg;
}

std::string KalmanBlobTracker::draw_targets(RGBImg img, int scope) const
{
	XYCoord upLeft, botRight;
	CvPoint p1, p2;
	TracePtr tr;
	TargetPtr tgt;
	Blob f(0);
	std::string msg;

	if (scope == 0)
		msg = "mobile and active targets";
	else if (scope == 1)
		msg = "active and inactive mobiles targets (only active have numbers)";
	else if (scope == 2)
		msg = "active and inactive mobiles targets (all have numbers)";
	else if (scope == 3)
		msg = "all targets (static, mobile, active, inactive, all with numbers)";
	else if (scope == 4)
		msg = "all targets and all unmatched traces (mobile or not)";

	CvMatView mat(img);

	for (auto iter = m_targets.begin(); iter != m_targets.end(); iter++)
	{
		tgt = *iter;

		if (tgt->is_mobile() || scope >= 3)
		{
			// If it's being tracked, draw the bounding box
			if (target_is_being_tracked(*iter) || scope >= 1)
			{
				tr = tgt->get_last_trace();

				tr->get_matching_region(upLeft, botRight);

				p1 = cvPoint(upLeft.x, upLeft.y);
				p2 = cvPoint(botRight.x, botRight.y);

				// Draw rectangle but don't show target ID
				cv::rectangle(mat, p1, p2, tr->get_color(), 2);
			}

			// If it's being tracked or is recent, draw the predicted position
			if (target_is_recent(*iter) || scope >= 2)
			{
				// Draw the target ID
				tgt->draw(img);

				tr = tgt->get_last_trace();

#ifdef SMOOTH_TRACES_ONLINE
				f = tr->get_kalman_state_post();
#else
				f = *tr->get_last_feature();
#endif

				p1 = cvPoint(f.centroid().x ,f.centroid().y);

				// Draw just a point of a line
				cv::line(mat, p1, p1, cv::Scalar(255, 0, 0), 4); //tr->get_color()
			}
		}
	}

	// Get the traces that are around and are not matched to any target. Note that
	// the last only TRANSIENT_TIME + MATCH_TIME. After that, they either: (a) become a novel
	// target, (b) are matched to a target, (c) are moved to the history, (d) are deleted
	Traces unmatched_traces = get_unmatched_traces();

	// Draw the traces that are still active (ie, might be assoc with a target later on)
	for (auto iter = unmatched_traces.begin(); iter != unmatched_traces.end(); iter++)
	{
		tr = *iter;

		if (tr->is_mobile() || scope >= 4)
		{
			tr->draw(img);
			
			tr->get_matching_region(upLeft, botRight);

			p1 = cvPoint(upLeft.x, upLeft.y);
			p2 = cvPoint(botRight.x, botRight.y);

			cv::rectangle(mat, p1, p2, cv::Scalar(255, 0, 0), 10); // tr->get_color()
		}
	}

	return msg;
}

