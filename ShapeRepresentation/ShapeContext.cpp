/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "ShapeContext.h"
#include <Tools/HelperFunctions.h>
#include <Tools/UserArguments.h>
#include <VideoParserGUI/DrawingUtils.h>
#include <Tools/NamedColor.h>

using namespace vpl;

extern UserArguments g_userArgs;

#define DEBUG_SHAPE_CONTEXT 0

#if DEBUG_SHAPE_CONTEXT
LogFile g_debugLog("shape_context_log.m");
#endif

ShapeContext::Params ShapeContext::s_params;

/*!
	Reads parameters from global user's arguments
*/
void ShapeContext::Params::ReadFromUserArguments()
{
	// Creation params
	g_userArgs.ReadArg("ShapeContext", "minPointsForPolygon", 
		"Creation: Min number of points for a polygon", 3, &minPointsForPolygon);

	g_userArgs.CheckMinValues("ShapeContext", "minPointsForPolygon", 3);

	g_userArgs.ReadArg("ShapeContext", "minRadius", 
		"Creation: Minimum value used to bin the radius distances", 0.125, &minRadius);

	g_userArgs.ReadArg("ShapeContext", "maxRadius", 
		"Creation: Maximum value used to bin the radius distances", 2.0, &maxRadius);

	g_userArgs.ReadArg("ShapeContext", "numThetaBins", 
		"Creation: Number of bins used when quantizing angles", 8, &numThetaBins);

	g_userArgs.ReadArg("ShapeContext", "numRadiusBins", 
		"Creation: Number of bins used when quantizing point distances", 8, &numRadiusBins);

	// State that we want to serialize all the CREATION fields
	g_userArgs.AddSerializableFieldKey("ShapeContext");

	// NOTE: the field key of the matching params must be different, otherwise they
	// will be serialized too, whci is not what we want

	// Matching params
	g_userArgs.ReadArg("ShapeContextComp", "maxNumIterations", 
		"Matching: Max number of alignment iterations during matching", 1, &maxNumIterations);

	g_userArgs.ReadArg("ShapeContextComp", "annealingRate", 
		"Matching: Rate of annealing used to compute the regularization parameter", 
		1.0, &annealingRate);

	g_userArgs.ReadArg("ShapeContextComp", "betaInit", 
		"Matching: Initial regularization parameter (normalized)", 1.0, &betaInit);

	g_userArgs.ReadArg("ShapeContextComp", "unmatchCost", 
		"Matching: Cost of matching a data point with a dummy point", 0.15, &unmatchCost);

	g_userArgs.ReadBoolArg("ShapeContextComp", "fastMatch", 
		"Matching: Use a faster function for minimizing costs?", true, &fastMatch);

	g_userArgs.ReadBoolArg("ShapeContextComp", "doTPSWarp", 
		"Do regularized Thin Plate Spline or Affine warp?", true, &doTPSWarp);
}

/*!
	Computes the average distance using the given inliersMap map
*/
void ShapeContext::ComputeMeanRadius()
{
	m_meanRadius = 0;

	// Note: the radii matrix is symmetric, so we can save
	// some computations. Plus, the diagonal is all zeros
	for (unsigned i = 0; i < m_radii.rows(); ++i)
		for (unsigned j = i + 1; j < m_radii.cols(); ++j)
			if (m_inliersMap[i] && m_inliersMap[j])
				m_meanRadius += m_radii(i, j); 

	// Multiply the sum by 2, since we summed upper diagonals only
	m_meanRadius *= 2;

	// Normalize distance by the number of distances in the matrix
	m_meanRadius /= (m_numInliers * m_numInliers);

	// Ensure that the current average distance is valid
	if (m_meanRadius == 0)
		m_meanRadius = 1;
}

/*!
	Creates joint (r,theta) histogram by binning r_array and theta_array.
	
	The histogram has 's_params.numThetaBins' angle bins and 
	's_params.numRadiusBins' radius bins.

	@param meanRadius if negative, the mean radius is computed using the
		sample data points. Otherwise, the radius values are quantizes
		using the given mean radius.
*/
void ShapeContext::CreateHistogram(const double& meanRadius)
{
	// Compute a matrix of all angles for each data point
	SetAngleMatrix();

	// Creat a histogram of angles
	QuantizeAngles();

	// The binning of angles doesn't depend on which points are inliersMap, 
	// and so we can recompute a SC (if required) without the angles
//#if DEBUG_SHAPE_CONTEXT == 0
//	m_angles.clear();  // clear the data unless we are debugging
//#endif

	// Compute all possible distances centered at each sample point
	SetRadiusMatrix();

	// Bin the radii into a histogram
	QuantizeRadii(meanRadius);

	// The radii depend on the inlier points used and so must be kept around
	// in order to update the shape context when the set of inliersMap changes
	
	// Create joint (theta,r) histogram by mapping the radius histogram 
	// and the angles histogram of EACH sample point onto a 0-1 matrix.
	SetJointHistogram(); 
}

/*!
	Sets the angle matrix, which has all angles from each point to all the others.
*/
void ShapeContext::SetAngleMatrix()
{
	double dx, dy;
	const double twoPI = 2 * M_PI;

	m_angles.set_size(NumSamples(), NumSamples());

	// Compute the angles of each ray cannecting a pair
	// of contour points
	for (unsigned i = 0; i < m_angles.rows(); ++i)
	{
		for (unsigned j = 0; j < m_angles.cols(); ++j)
		{
			// Get the angle wrt the standard xy-coordinate axis
			dx = X(j) - X(i);
			dy = Y(j) - Y(i);

			m_angles(i, j) = atan2(dy, dx);

			// Express the angle wrt to the given tangent
			// Needs proper tangent in order to be rotation invariant
			m_angles(i, j) -= m_tangents[i];

			// Put all angles in [0,2pi) range
			m_angles(i, j) = fmod(fmod(m_angles(i, j), twoPI) + twoPI, twoPI);
		}
	}
}

/*!
	Quantizes to a fixed set of angles. Bin edges lie on 0,(2*pi)/k,...2*pi
*/
void ShapeContext::QuantizeAngles()
{
	const double normFactor = (2.0 * M_PI) / s_params.numThetaBins;

	m_angleBins.set_size(m_angles.rows(), m_angles.cols());

	// Note: in the matlab implementation, the 'floor' value is incremented
	// by one because matlab indices aren't zero based like in c/c++
	for (unsigned i = 0; i < m_angles.rows(); i++)
	{
		for (unsigned j = 0; j < m_angles.cols(); j++)
		{
			m_angleBins(i, j) = (int) floor(m_angles(i, j) / normFactor);

			// Note: due to rounding errors, we might go outside bins
			// so, this must be corrected
			if (m_angleBins(i, j) >= s_params.numThetaBins)
				m_angleBins(i, j) = 0;
		}
	}
}

/*!
	Sets the radius matrix, which has all distances from each point to all the others.
*/
void ShapeContext::SetRadiusMatrix()
{
	double dx, dy, euclideDist;

	m_radii.set_size(NumSamples(), NumSamples());

	// Compute point distance
	for (unsigned i = 0; i < m_radii.rows(); ++i)
	{
		for (unsigned j = i; j < m_radii.cols(); ++j)
		{
			// Compute the distance between the points
			dx = X(i) - X(j);
			dy = Y(i) - Y(j);

			euclideDist = sqrt(dx * dx + dy * dy);

			// Distance matrix is symmetric
			m_radii(i, j) = euclideDist;
			m_radii(j, i) = euclideDist;
		}
	}
}

/*!
	Quantize to a fix set if distances using a log. scale. The bin edges 
	lie on [log10(s_params.minRadius), log10(s_params.maxRadius)]
*/
void ShapeContext::QuantizeRadii(const double& givenMeanRadius)
{
	DoubleArray binEdges(s_params.numRadiusBins); // sets the num of bins here
	unsigned i, j, k;
	double val;

	// Use a logarithmic scale for binning the radius distances
	ComputeLogSpace(binEdges, log10(s_params.minRadius), log10(s_params.maxRadius));

	// Let the 'm_meanRadius' be either the given one of valid or the one
	// compute using the sample points
	if (givenMeanRadius > 0)
		m_meanRadius = givenMeanRadius;
	else
		ComputeMeanRadius();

	// Init the bins
	m_radiusBins.set_size(m_radii.rows(), m_radii.cols());

	m_radiusBins.fill(-1);

	// Find the corresponding bin of each point
	for (i = 0; i < m_radii.rows(); ++i)
	{
		for (j = 0; j < m_radii.cols(); ++j)
		{
			// Normalize radius distance (scale invariance)
			val = m_radii(i, j) / m_meanRadius;

			for (k = 0; k < binEdges.size(); ++k)
			{
				if (val < binEdges[k])
					++m_radiusBins(i, j);
			}
		}
	}
}

/*!
	Creates joint (theta,r) histogram by mapping the radius histogram 
	and the angles histogram of EACH sample point onto a 0-1 matrix, 
	such that (i,j) == 1 iff points falls in Theta-bin i and R-bin j.

	The histogram are normalized right here. This is different from
	the Matlab code in which the histogram is normalized when computing
	the cost matrix. It seems more reazonable to do it her, so that
	when a shape context is saved to disk and read several times for
	matching, it will be already normalized.
*/
void ShapeContext::SetJointHistogram()
{
	unsigned i, j;
	int idx;

	m_histogram.resize(NumSamples());

	for (i = 0; i < m_histogram.size(); ++i)
	{
		// Populate the histogram that represents the context of point 'i' 
		DoubleVector& h = m_histogram[i]; 
		
		h.set_size(s_params.numThetaBins * s_params.numRadiusBins);

		h.fill(0);

		for (j = 0; j < m_angleBins.cols(); j++)
		{
			// Only consider inliers points whose radii frequencies are valid
			if (m_inliersMap[j] && m_radiusBins(i, j) >= 0)
			{
				idx = m_radiusBins(i, j) * s_params.numThetaBins +
					m_angleBins(i, j);

				ASSERT(idx >= 0 && idx < (int) h.size());
					
				++h(idx);
			}
		}
	}

#if DEBUG_SHAPE_CONTEXT
	Print(g_debugLog);
#endif

	// Normalize histogram
	for (i = 0; i < m_histogram.size(); i++)
		m_histogram[i] /= m_histogram[i].sum() + vnl_math::eps;

#if DEBUG_SHAPE_CONTEXT
	PrintAll(g_debugLog);
#endif
}

void ShapeContext::Print(vcl_ostream& os) const
{
	os << "\n" << "Histogram" << ":\n[\n";

	for (unsigned k = 0; k < m_histogram.size(); k++)
		PrintVector(os, m_histogram[k]);

	os << "]\n";
}

void ShapeContext::PrintAll(vcl_ostream& os) const
{
	os << "\nShape context with " << NumSamples() << " points\n";

	PrintMatrix(os, m_dataPts, "Data Points");

	os << "\nMean radius: " << m_meanRadius << "\n";

	PrintMatrix(os, m_angles, "Angles");

	PrintMatrix(os, m_angleBins, "Angle Bins");

	PrintMatrix(os, m_radii, "Radii");
	PrintMatrix(os, m_radiusBins, "Radius Bins");

	Print(os);
}

void ShapeContext::Serialize(OutputStream& os) const
{
	::Serialize(os, m_histogram);
	::Serialize(os, m_meanRadius);

	m_angleBins.Serialize(os);
	m_radiusBins.Serialize(os);

	m_dataPts.Serialize(os);
	::Serialize(os, m_tangents);

	m_angles.Serialize(os);
	m_radii.Serialize(os);

	//::Serialize(os, m_inliersMap);
	//::Serialize(os, m_numInliers);
}
		
void ShapeContext::Deserialize(InputStream& is)
{
	::Deserialize(is, m_histogram);
	::Deserialize(is, m_meanRadius);

	m_angleBins.Deserialize(is);
	m_radiusBins.Deserialize(is);

	m_dataPts.Deserialize(is);
	::Deserialize(is, m_tangents);

	m_angles.Deserialize(is);
	m_radii.Deserialize(is);

	//::Deserialize(is, m_inliersMap);
	//::Deserialize(is, m_numInliers);
	m_inliersMap.resize(NumSamples(), true);
	m_numInliers = NumSamples();
}

/*!
	Creates a new shape descriptor that is the result of transforming
	this descriptor using 'params'. If the transformation is not
	possible, a NULL pointer is returned.
*/
ShapeDescPtr ShapeContext::Transform(const PointTransform& params) const
{
	if (NumPoints() == 0 || params.A.empty())
		return ShapeDescPtr();

	// Transform the point coordinates
	Matrix newPts;

	if (s_params.doTPSWarp)
	{
		Matrix homoPts;

		// Form 3xN matrix of homogeneous coordinates 
		// like (Matlab A = [ones(1,nsamp1); X'])
		homoPts.Set2x1(Ones(1, NumPoints()), Points());

		newPts = params.A * homoPts + params.T;
	}
	else // apply the simple affine warp
	{
		ASSERT(params.T.empty());

		newPts = ShapeDescriptor::ApplyHomography(params.A, Points());
	}
	
	ShapeContext* pSC = new ShapeContext;

	pSC->SetDataPoints(newPts, m_tangents);

	return ShapeDescPtr(pSC);
}

void ShapeContext::Draw(const RGBColor& color) const
{
	PointArray::const_iterator it;

	SetDrawingColor(color);

	for (unsigned i = 0; i < m_dataPts.cols(); ++i)
	{
		DrawDisk(Point(m_dataPts(0, i), m_dataPts(1, i)), 1);
	}

	SetDefaultDrawingColor();

	//Print(std::cout);
}
