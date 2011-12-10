/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "ShapeContextComp.h"
#include <Tools/HelperFunctions.h>
#include <Tools/UserArguments.h>
#include <Tools/LogFile.h>
#include "ShapeContextTestData.h"
#include <GraphTheory/Blossom5BGMatcher.h>
#include <GraphTheory/HungarianBGMatcher.h>
#include <GraphTheory/CircularBGMatcher.h>

//#define IMITATE_MATLAB_CODE

using namespace vpl;

extern UserArguments g_userArgs;

#define DEBUG_SHAPE_CONTEXT_MATCHER 0

#ifdef COMPILE_DBG_STATEMENTS
#if DEBUG_SHAPE_CONTEXT_MATCHER
LogFile g_debugLog("shape_context_matcher_log.m");

//! Debug macro for writing a matrix as a Matlab assignment "L_N = M;" in g_debugLog
#define LOG_MATRIX(M, L, N) DBG_LOG_MATRIX(g_debugLog, M, L, N)

//! Debug macro for writing a vector as a Matlab assignment "L_N = V;" in g_debugLog
#define LOG_VECTOR(V, L, N) DBG_LOG_VECTOR(g_debugLog, V, L, N)

//! Debug macro for writing a scalar as a Matlab assignment "L_N = S;" in g_debugLog
#define LOG_SCALAR(S, L, N) DBG_LOG_SCALAR(g_debugLog, S, L, N)
#else
#define LOG_MATRIX(M, L, N) 
#define LOG_VECTOR(V, L, N) 
#define LOG_SCALAR(S, L, N) 
#endif
#endif

double MultiplyByLog(const double& val)
{
	return val * log(val);
}

double MultiplyByLogWithEps(const double& val)
{
	return val * log(val + vnl_math::eps);
}

/*!
	Bookstein PAMI89
	
	[cx,cy,E,L]=bookstein(M,Q,beta_k); 

	where

	cx = c(:,1);
	cy = c(:,2);
*/
/*static*/
void ShapeContextComp::Bookstein(const Matrix& M, const Matrix& Q, 
									const double& beta_k, Matrix* pCoords, 
									double* pBendingEnergy)
{
	if (M.rows() != Q.rows())
		THROW_BASIC_EXCEPTION("Number of landmarks must be equal");

	Matrix r2;

	// Compute distances between left points
	r2.AssignRowSqDistances(M, M);

	// Add identity matrix to r2, which currently has zeros along the diagonal 
	r2.fill_diagonal(1);

	// Compute K = r2 .* log(r2). K will also have zeros along the diagonal
	Matrix K = r2.apply(MultiplyByLog);

	Matrix P, V, L;
	const unsigned N = M.rows();

	P.Set1x2(Ones(N, 1), M);

	L.Set2x2(K, P, P.transpose(), Zeros(3, 3));

	V.Set1x2(Q.transpose(), Zeros(2, 3));

	// See if regularization parameter is valid and apply it
	if (beta_k > 0)
	{
	   for (unsigned i = 0; i < N; ++i)
		   L(i, i) += beta_k;
	}

	Matrix invL = vnl_matrix_inverse<double>(L);

	*pCoords = invL * V.transpose();

	// Compute bending energy (w/o regularization)
	if (pBendingEnergy)
	{
		Matrix cc = pCoords->Sub(0, N - 1, 0, pCoords->cols() - 1);

		Matrix Q = cc.transpose() * K * cc;

		*pBendingEnergy = Q.MeanAlongDiagonal();
	}
}

/*!
	Computes the cost of matching the points represented
	by two shape contexts. 
	
	The given cost matrix may be padded with dummy rows or cols.
	This function does not set or modify the matching cost 
	associated with dummy points.

	Computes HC=0.5*sum(((tmp1-tmp2).^2)./(tmp1+tmp2+eps),3);
*/
/*static*/
void ShapeContextComp::ComputeCostMatrix(const ShapeContext& sc1, 
	                                     const ShapeContext& sc2,
					                     Matrix& costMat)
{
	unsigned i, j, k;
	double diff;

	const unsigned N1 = sc1.NumPoints();
	const unsigned N2 = sc2.NumPoints();
	const unsigned NBINS = sc1.NumBins();

	DBG_DECLARE_TIMER(timer)

	for (i = 0; i < N1; ++i)
	{
		for (j = 0; j < N2; ++j)
		{
			double& cost = costMat(i, j);

			cost = 0;

			for (k = 0; k < NBINS; ++k)
			{
				const double& b1 = sc1(i, k);
				const double& b2 = sc2(j, k);

				diff = b1 - b2;

				cost += (diff * diff) / (b1 + b2 + vnl_math::eps);
			}

			cost *= 0.5;
		}
	}

	DBG_PRINT_ELAPSED_TIME_IF_GREATER_THAN(timer, 40, 
			"Cost matrix " << sc1.NumPoints() 
			<< ", " << sc2.NumPoints() << ", " << sc2.NumBins())


	/*
		//In the matlab code, there is a computation of approx cost like this
		costMat.RowMin(&a1, 0, model.NumPoints());
		costMat.ColMin(&a2, 0, query.NumPoints());

		// Presumably, in each iteration, sc_cost should decrease
		double sc_cost = MAX(a1.mean(), a2.mean());
	*/
}

/*!
	Compares two shape contexts (SC). 
	
	These two SC are labeled  'model' and 'query' to indicate that the 
	comparison works better if the 'model' shape has no noise.
*/
void ShapeContextComp::MatchSCFast(const ShapeContext& model, 
	const ShapeContext& query)
{
	if (model.NumBins() == 0)
	{
		m_matchCost = query.NilMatchValue();
		return;
	}
	else if (query.NumBins() == 0)
	{
		m_matchCost = model.NilMatchValue();
		return;
	}

	if (model.NumBins() != query.NumBins())
	{
		StreamError("Numbers of bins in model and query shape context differ."
			" There are " << model.NumBins() << " model bins and " 
			<< query.NumBins() << " query bins.");
	}

	ASSERT(model.NumBins() == query.NumBins());

	const unsigned nptsd = MAX(model.NumPoints(), query.NumPoints());

	// Init the cost matrix
	m_costMat.set_size(nptsd, nptsd);

	// Set the fixed cost of matching dummy nodes in the matrix
	if (model.NumBins() != query.NumBins())
		m_costMat.fill(Params().unmatchCost);

	// Compute pairwise cost between all shape contexts
	// The cost of matching dummy nodes is set by fill() above
	ComputeCostMatrix(model, query, m_costMat);

	if (!m_pMatcher)
	{
		m_pMatcher = new Blossom5BGMatcher;
	}

	m_pMatcher->Init(nptsd, nptsd);

	// Run the min cost assignemt algorithm
	m_matchCost = m_pMatcher->SolveMinCost(m_costMat);
}

/*!
	Compares two shape contexts (SC). 
	
	These two SC are labeled  'model' and 'query' to indicate that the 
	comparison works better if the 'model' shape has no noise. It is 
	assumed that the 'query' shape will have some noise, and so there 
	is an iterative procedure that identifies outliers and recomputes 
	the SC using only the inliers.
*/
void ShapeContextComp::MatchSC(ShapeContext model, ShapeContext query)
{
	Matrix M_orig, M_orig_homo, M_new;

#if DEBUG_SHAPE_CONTEXT_MATCHER
	Matrix A;
	double aff_cost, aff_score;
#endif

	// Save the original data points in model, which will be warped
	M_orig = model.Points();

	// Form 3xN matrix of homogeneous model coordinates (Matlab A = [ones(1,nsamp1); X'])
	M_orig_homo.Set2x1(Ones(1, model.NumPoints()), M_orig);

	// We need the original model points as column vectors
	M_orig.inplace_transpose();

	double beta_k;

	for (int k = 1; ; k++)
	{
		// Call MatchSCFast() to set the current 'm_matchCost'
		MatchSCFast(model, query);

		// Get the SC correspondences in order to update the inliers 
		m_pMatcher->GetCorrespondences(m_model2queryMap, m_query2modelMap);

		// Update inliers on each shape context
		// An outlier is a point that maps to a "padding" point
		model.UpdateInliersUsingMapping(m_model2queryMap, query.NumPoints());
		query.UpdateInliersUsingMapping(m_query2modelMap, model.NumPoints());

		// Compute regularization parameter
		beta_k = (model.MeanRadius() * model.MeanRadius()) *
			Params().betaInit * pow(Params().annealingRate, k - 1);

		// Compute the transformation that aligns the model to the query
		// These are always necessary because the params might be requested
		// by the GetTransformationParams() function
		ComputeTransformationParameters(beta_k, model, query, M_orig);

		// We might be able to finish iterating right now...
		if (m_matchCost == 0 || k >= Params().maxNumIterations)
			break;

		// Transform the model coordinates
		if (Params().doTPSWarp)
		{
			M_new = m_M_aff * M_orig_homo + m_M_wrp;
		}
		else // apply the simple affine warp
		{
			M_new = ShapeDescriptor::ApplyHomography(m_M_aff, M_orig);
		}
		
		// The point coordinates of shape 1 changed so we need 
		// to recompute the histogram. It will also use the
		// new set of inliers updated above.
		model.Recompute(M_new);

		// The point coordinates of shape 2 didn't change but 
		// its associated set of inliers might have changed. It 
		// will also use the new set of inliers updated above.
		query.Recompute(model.MeanRadius());
	}
}

//! Compute the transformation that aligns the model to the query
void ShapeContextComp::ComputeTransformationParameters(const double& beta_k,
	const ShapeContext& model, const ShapeContext& query, const Matrix& M_orig)
{
	Matrix M_inliers, Q_inliers, C, U, D2;
	DoubleVector a1, a2;
	unsigned numGoodPts, modelPtIdx;
	//double tps_bending_energy;

#if !defined(IMITATE_MATLAB_CODE)
	numGoodPts = query.InlierCount();
#else
	numGoodPts = query.InlierCount(model.NumPts());
#endif

	// Collect the inlier points of the model and the query
	M_inliers.set_size(numGoodPts, 2);
	Q_inliers.set_size(numGoodPts, 2);

	ASSERT(numGoodPts <= m_query2modelMap.size());

	for (unsigned i = 0, j = 0; j < numGoodPts; ++i)
	{
		if (query.IsInlier(i))
		{
			// Grab the ORIGINAL points from the model
			modelPtIdx = m_query2modelMap[i];

			M_inliers(j,0) = M_orig(modelPtIdx, 0);
			M_inliers(j,1) = M_orig(modelPtIdx, 1);

			query.GetPoint(i, &Q_inliers(j,0), &Q_inliers(j,1));

			++j;
		}
	}

	if (Params().doTPSWarp)
	{
		// Estimate regularized TPS transformation
		Bookstein(M_inliers, Q_inliers, beta_k, &C, NULL/*&tps_bending_energy*/);

		// Calculate affine cost and score for debugging purposes
	#if DEBUG_SHAPE_CONTEXT_MATCHER
		//Compute A= cx(numGoodPts+2:numGoodPts+3,:) cy(numGoodPts+2:numGoodPts+3,:)];
		A.Set1x2(C.Sub(numGoodPts + 1, numGoodPts + 2, 0, 0), 
			C.Sub(numGoodPts + 1, numGoodPts + 2, 1, 1));

		// Compute the singular value decomposition of A
		vnl_svd<double> svd(A);

		// Get the affine cost and score from the singular values
		aff_cost = log(svd.W(0) / svd.W(1));
		aff_score = svd.W(0);

		LOG_SCALAR(aff_cost, "aff_cost", k);
		LOG_SCALAR(aff_score, "aff_score", k);
	#endif
		// Compute d2 = max(dist2(M_inliers, M_orig), 0) 
		// Here, max(.) seems redundant and its only purpose could
		// be to remove NaN. However, we don't expect any NaNs. It could also
		// be that the second parameter was greater than 0 at some point.

		D2.AssignRowSqDistances(M_inliers, M_orig);

		// Just in case, make sure that there are no NaN. Otherwise, we should
		// call D2.ReplaceAllNaN(0);
		ASSERT(!D2.HasNaN());

		// Compute U = d2 .* log(d2 + vnl_math::eps)
		U = D2.apply(MultiplyByLogWithEps);

		m_M_aff.Set2x1(C.Sub(numGoodPts, numGoodPts + 2, 0, 0).transpose(),
					   C.Sub(numGoodPts, numGoodPts + 2, 1, 1).transpose());

		m_M_wrp.Set2x1(C.Sub(0, numGoodPts - 1, 0, 0).transpose() * U,
					   C.Sub(0, numGoodPts - 1, 1, 1).transpose() * U);
	}
	else // do a simpler affine warp
	{
		M_inliers.inplace_transpose();
		Q_inliers.inplace_transpose();

		m_M_aff = ShapeDescriptor::ComputeHomography(M_inliers, Q_inliers);

		m_M_wrp.clear();
	}
}

/*	
	//Compute M_x_aff = cx(numGoodPts+1:numGoodPts+3)' * M_orig_homo
	M_x_aff = m_C.Sub(numGoodPts, numGoodPts + 2, 0, 0).transpose() * M_orig_homo;

	// Compute M_x_wrp = cx(1:numGoodPts)' * U
	M_x_wrp = m_C.Sub(0, numGoodPts - 1, 0, 0).transpose() * m_U;

	// Compute M_y_aff = cy(numGoodPts+1:numGoodPts+3)' * M_orig_homo
	M_y_aff = m_C.Sub(numGoodPts,numGoodPts + 2, 1, 1).transpose() * M_orig_homo;

	// Compute M_y_wrp = cy(1:numGoodPts)' * U
	M_y_wrp = m_C.Sub(0, numGoodPts - 1, 1, 1).transpose() * m_U;

	// Compute M_new=[fx; fy]' (not that ' and ; are redundant), where 
	// fx = M_x_aff + M_x_wrp and fy = M_y_aff + M_y_wrp
	M_new.Set2x1(M_x_aff + M_x_wrp, M_y_aff + M_y_wrp);
*/

void ShapeContextComp::GetTransformationParams(const ShapeDescriptor& sd1, 
	const ShapeDescriptor& sd2,	PointTransform* pPT)
{
	const ShapeContext& model = CastDescriptor<ShapeContext>(sd1);
	const ShapeContext& query = CastDescriptor<ShapeContext>(sd2);

	BipartiteGraphMatcher* pOldMatcher = m_pMatcher;

	m_pMatcher = new CircularBGMatcher;

	MatchSC(model, query);

	delete m_pMatcher;

	m_pMatcher = pOldMatcher;

	pPT->A = m_M_aff;
	pPT->T = m_M_wrp;

	pPT->P.resize(m_model2queryMap.size(), Point(0, 0));

	for (unsigned i = 0; i < m_model2queryMap.size(); i++)
	{
		if (m_model2queryMap[i] != UINT_MAX)
			query.GetPoint(m_model2queryMap[i], &pPT->P[i].x, &pPT->P[i].y);
	}
}

/*!
	Thest the shape context and matching code by using the
	example found online along with the Matlab code.
*/
/*static*/
void ShapeContextComp::Test()
{
	/*PointArray data1 = ModelFish();
	DoubleArray tangents1(data1.size(), 0);

	PointArray data2 = QueryFishWithOutliers();
	DoubleArray tangents2(data2.size(), 0);

	ShapeContext sc1;
	ShapeContext sc2;

	ShapeContext().ReadClassParameters();

	sc1.Create(data1, tangents1);
	sc2.Create(data2, tangents2, sc1.MeanRadius());

	ShapeContextComp matcher;

	matcher.Match(sc1, sc2);*/
}
