/**************************************************************************
File:                DivergenceSkeletonMaker.h

Author(s):           Pavel Dimitrov

Created:             27 Jun 2002

Last Revision:       Date: 2009/10/14 by Diego Macrini
**************************************************************************/
#ifndef DIVERGENCESKELETONMAKER_H
#define DIVERGENCESKELETONMAKER_H

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "DivergenceMap.h"
#include "DDSGraph.h"
#include "DivArr.h"

#include <vul/vul_timer.h>

namespace sg {

	void colour_skeleton_array(DivArr &da);
	DivArr* create_shape_DivArr(DivergenceMap& dm, const double& step = 1.0);
	void thin_div_arr(DivArr &da, double thresh);
	void buildDDSGraph(DivArr &da, DDSGraph* skeleton);

	/*!
		The main skeleton algorithm. It computes a divergence array, 
		thins the shock points into an 8-connected 1-pixel thin skeleton, 
		and	builds a graph of endpoints (nodes) and skeletal branches (edges).
	*/
	inline void computeDDSkeleton(DDSGraph* dds,
		DivergenceMap& dm, const double& step, const double& thresh)
	{
		vul_timer tic;

		tic.mark();
		DivArr* da = create_shape_DivArr(dm, step);
		ShowStatus2("Divergence array created", tic.real(), "milliseconds");

		tic.mark();
		thin_div_arr(*da, thresh);
		ShowStatus2("Div array thinning done", tic.real(), "milliseconds");

		tic.mark();
		colour_skeleton_array(*da);
		ShowStatus2("Div array colouring done", tic.real(), "milliseconds");

		tic.mark();
		buildDDSGraph(*da, dds);
		ShowStatus2("DDS graph built", tic.real(), "milliseconds");

		delete da; // destroy the divergence array
	}
}

#endif  /* DIVERGENCESKELETONMAKER_H */
