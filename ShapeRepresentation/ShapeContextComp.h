/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <queue>
#include "ShapeDescriptorComp.h"
#include "ShapeContext.h"
#include <GraphTheory/BipartiteGraphMatcher.h>

namespace vpl
{
class ShapeContextComp : public ShapeDescriptorComp
{
private:
	Matrix m_M_aff, m_M_wrp;
	Matrix m_costMat;
	double m_matchCost;

	BipartiteGraphMatcher* m_pMatcher;

protected:
	UIntVector m_model2queryMap; //!< rowMap(r) = c means that col c is assigned to row r
	UIntVector m_query2modelMap; //!< colMap(c) = r means that row r is assigned to column c

	static void Bookstein(const Matrix& M, const Matrix& Q, 
		const double& beta_k, Matrix* pCoords, double* pBendingEnergy);

	static void ComputeCostMatrix(const ShapeContext& sc1, 
		const ShapeContext& sc2, Matrix& costMat);

	static const ShapeContext::Params& Params() 
	{
		return ShapeContext::GetParams();
	}

	void FindMinCostMatching(const ShapeContext& sc1, const ShapeContext& sc2);

	//void UpdateInliers(ShapeContext& model, ShapeContext& query);

	void ComputeTransformationParameters(const double& beta_k,
		const ShapeContext& model, const ShapeContext& query,
		const Matrix& M_orig);

	//void TransformModelPoints(unsigned numGoodPts);

public:
	ShapeContextComp()
	{
		m_pMatcher = NULL;
	}

	~ShapeContextComp()
	{
		delete m_pMatcher;
	}

	void MatchSC(ShapeContext model, ShapeContext query);
	void MatchSCFast(const ShapeContext& model, const ShapeContext& query);

	virtual double Match(const ShapeDescriptor& sd1, const ShapeDescriptor& sd2)
	{
		if (ShapeContext::GetParams().fastMatch)
			MatchSCFast(CastDescriptor<ShapeContext>(sd1), CastDescriptor<ShapeContext>(sd2));
		else
			MatchSC(CastDescriptor<ShapeContext>(sd1), CastDescriptor<ShapeContext>(sd2));
			
		return m_matchCost;
	}

	void GetTransformationParams(const ShapeDescriptor& sd1, 
		const ShapeDescriptor& sd2,	PointTransform* pPT);

	static void Test();
};

} // namespace vpl

