/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/

template <class SEGMENT> 
double PiecewiseApprox<SEGMENT>::FindMaxYDiff(int seg_num, int s, int e) const
{
	const SEGMENT *pFirstSeg, *pLastSeg;
	double  dMaxYDiff;
	
	FindMaxYDiff(seg_num, s, e, pFirstSeg, pLastSeg, dMaxYDiff);
	
	return dMaxYDiff;
}

template <class SEGMENT> 
void PiecewiseApprox<SEGMENT>::FindMaxYDiff(int seg_num, int s, int e, 
											const SEGMENT*& pFirstSeg, 
											const SEGMENT*& pLastSeg, 
											double& dMaxYDiff) const
{
	const MEMDATA& d = m_minerrors[seg_num][s][e];
	int j = d.GetIdx();
	
	if (seg_num > 1)
	{
		const SEGMENT *pFirstLeft, *pLastLeft, *pFirstRight, *pLastRight;
		double dMaxLeftYDiff, dMaxRightYDiff;
		
		FindMaxYDiff(seg_num / 2, s, j, pFirstLeft, pLastLeft, dMaxLeftYDiff);
		FindMaxYDiff(seg_num / 2 + seg_num % 2, j, e, pFirstRight, pLastRight, dMaxRightYDiff);
		
		double diff = fabs(pLastLeft->Y1() - pFirstRight->Y0()) / MAX(pLastLeft->Y1(), pFirstRight->Y0());
		
		if (diff > dMaxLeftYDiff && diff > dMaxRightYDiff)
			dMaxYDiff = diff;
		else if (dMaxLeftYDiff > dMaxRightYDiff)
			dMaxYDiff = dMaxLeftYDiff;
		else
			dMaxYDiff = dMaxRightYDiff;
		
		pFirstSeg = pFirstLeft;
		pLastSeg = pLastRight;
	}
	else if (seg_num == 1)
	{
		pFirstSeg = &m_segments[s][e];
		pLastSeg = &m_segments[s][e];
		dMaxYDiff = 0;
	}
}

/*!
	@brief Fits a piecewise SEGMENT function to the data points

	Note: The MaxYDiff is only checked when m_dMaxYDiff > 0
*/
template <class SEGMENT> 
void PiecewiseApprox<SEGMENT>::Fit(const POINTS pts)
{
	int seg_num, n = pts.GetSize();
	MEMDATA d;
	double dMaxYDiff = m_dMaxYDiff; // init val allows us to ignore check (see <= below)
	
	ASSERT(n > 1);
	
	m_points = pts;
	m_segments.Resize(n, n);
	m_minerrors.Resize(n);
	m_knots.Clear();
	
	//SetYMinMax(pts, n);
	
	for (seg_num = 1; seg_num <= n; seg_num++)
	{
		//std::cerr << "Trying with " << seg_num << "segments" << std::endl;
		d = Min(seg_num, 0, n - 1);
		
		if (m_dMaxYDiff > 0)
			dMaxYDiff = FindMaxYDiff(seg_num, 0, n - 1);

		if ((d.GetMinError() <= m_dMinError && dMaxYDiff <= m_dMaxYDiff) || seg_num >= m_nMaxSegments)
			break;
	}
	//std::cerr << "Done!" << std::endl;
	
	//WARNING(seg_num == m_nMaxSegments, "Max num segments reached. Consider changing this threshold.");
	
	if (seg_num > n)
		seg_num--;
		
	AddKnots(seg_num, 0, n - 1);
	
	if (m_bDbgMode)
		PlotKnots(seg_num);
}

template <class SEGMENT> 
MEMDATA PiecewiseApprox<SEGMENT>::Min(int seg_num, int s, int e)
{	
	if (m_minerrors[seg_num].GetSize() == 0)
		m_minerrors[seg_num].Resize(m_points.GetSize(), m_points.GetSize());
		
	if (m_minerrors[seg_num][s][e].IsEmpty())
	{
		double minerror = 99999;
		int minpt = 0;
		
		if (seg_num == 1)
		{
			minerror = LeastSquares(m_points, s, e, m_segments[s][e]);
			minpt = s;
		}
		else
		{	
			double m;
			int n_left = seg_num / 2;
			int n_right = seg_num / 2 + seg_num % 2;
		
			for (int i = s + 1; i <= e - 1 ; i++)
			{
				m = Min(n_left, s, i) + Min(n_right, i, e);
			
				if (m < minerror)
				{
					minerror = m;
					minpt = i;
				}
			}
		}
		
		m_minerrors[seg_num][s][e].Set(minerror, minpt);
	}
	
	return m_minerrors[seg_num][s][e];
}

template <class SEGMENT> 
int PiecewiseApprox<SEGMENT>::AddKnots(int seg_num, int s, int e)
{
	const MEMDATA& d = m_minerrors[seg_num][s][e];
	int j = d.GetIdx();
	
	if (seg_num > 1)
	{
		/*int i = */AddKnots(seg_num / 2, s, j);
		int k = AddKnots(seg_num / 2 + seg_num % 2, j, e);
		return k;
	}
	else if (seg_num == 1)
	{
		KNOT knot(e, m_segments[s][e], d.GetMinError());
		SEGMENT& s = knot.seg;
		
		knot.dir = GetSegmentDirection(s);
		m_knots.AddTail(knot);
	}
	
	return j;
}

template <class SEGMENT>
void PiecewiseApprox<SEGMENT>::SetYMinMax()
{
	double y;
	
	m_ymin = -1;
	m_ymax = -1;
	
	for (int i = 0; i < m_points.GetSize(); i++)
	{
		y = m_points[i].y;
		
		if (m_ymin == -1 || y < m_ymin)
			m_ymin = y;
			
		if (m_ymax == -1 || y > m_ymax)
			m_ymax = y;
	}
}

/*!
	@brief Finds the acute angle between the two line segmets
*/
template <class SEGMENT>
double PiecewiseApprox<SEGMENT>::CompAcuteAngle(int seg1, int seg2) const
{
	ASSERT(seg1 < seg2);
	
	const SEGMENT& s1 = m_knots[seg1].seg;
	const SEGMENT& s2 = m_knots[seg2].seg;
	
	double s = (s2.p1.x - s1.p0.x) / (m_ymax - m_ymin);
	
	std::cout << "disp('scaling: " << m_ymin << "," << m_ymax << "," << s << "');\n";	
	
	//find vectors
	Point v1(s1.p0.x - s1.p1.x, (s1.p0.y - s1.p1.y) * s);
	Point v2(s2.p1.x - s2.p0.x, (s2.p1.y - s2.p0.y) * s);
	
	std::cout << "disp('v1: " << v1 << " v2: " << v2 << "');\n";	
	
	// compute angle
	return acos(v1.Dot(v2) / (v1.L2() * v2.L2())) * (180 / 3.14159265);
	
	/*const SEGMENT& s1 = m_knots[seg1].seg;
	const SEGMENT& s2 = m_knots[seg2].seg;
		
	double d = (s2.m > s1.m) ? s2.m - s1.m:s1.m - s2.m;
	
	return atan(d / (1 + s1.m * s2.m)) * (180 / PI);*/
}

/*!
	@brief Finds the obtuse angle between the two line segmets
*/
template <class SEGMENT>
double PiecewiseApprox<SEGMENT>::CompObtuseAngle(int seg1, int seg2) const
{
	return 180 - CompAcuteAngle(seg1, seg2);
}

/*!
	@brief Finds the index of the segment that corresponds to the 
	vertex point with index i
*/
template <class SEGMENT>
int PiecewiseApprox<SEGMENT>::GetPointSegmentIndex(int i) const
{
	for (int j = 0; j < m_knots.GetSize(); j++)
		if (i <= m_knots[j].nIndex)
			return j;

	ASSERT(false);

	// We must return something in release mode...
	return -1;
}

/*!
	@brief Finds the segment that corresponds to the vertex point with index i
*/
template <class SEGMENT>
const SEGMENT& PiecewiseApprox<SEGMENT>::GetPointSegment(int i) const
{
	for (int j = 0; j < m_knots.GetSize(); j++)
		if (i <= m_knots[j].nIndex)
			return m_knots[j].seg;

	ASSERT(false);

	// We must return something in release mode...
	return m_knots[m_knots.GetSize()-1].seg;
}
