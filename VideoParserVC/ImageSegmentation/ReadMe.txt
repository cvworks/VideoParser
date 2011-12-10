	// Merge the contour points
	m_contour.reserve(m_contour.size() + rhs.m_contour.size());

	m_contour.insert(m_contour.end(), rhs.m_contour.begin(), 
		rhs.m_contour.end());


	

	// Create the new mask
	cv::Mat_<uchar> newMask(new_bbox.height, new_bbox.width);
	newMask.setTo(0);

	for (int y = 0; y < m_mask.rows; y++)
		for (int x = 0; x < m_mask.cols; x++)
			newMask(x + m_bbox.x - new_bbox.x, y + m_bbox.y - new_bbox.y) = m_mask.at<uchar>(x, y);

	for (int y = 0; y < rhs.m_mask.rows; y++)
		for (int x = 0; x < rhs.m_mask.cols; x++)
			newMask(x + rhs.m_bbox.x - new_bbox.x, y + rhs.m_bbox.y - new_bbox.y) = rhs.m_mask.at<uchar>(x, y);

	// Update the bounding box
	m_bbox = new_bbox;

	// Update the bounding box
	m_mask = newMask;

for (auto it = m_contour.begin(); it != m_contour.end(); ++it)
		{
			mean_x += it->x;
			mean_y += it->y;

			auto mapIt = m_rowIntervals.find(it->y);

			if (mapIt == m_rowIntervals.end())
			{
				m_rowIntervals[it->y] = cv::Vec2i(it->x, it->x);
			}
			else
			{
				if (it->x < mapIt->second[0])
					mapIt->second[0] = it->x;
					
				if (it->x > mapIt->second[1])
					mapIt->second[1] = it->x;
			}
		}


		for (auto it = m_rowIntervals.begin(); it != m_rowIntervals.end(); ++it)
		{
			for (int i = it->second[0]; i < it->second[1]; i++)
				img(i, it->first) = 255;
		}


		for (auto it = m_rowIntervals.begin(); it != m_rowIntervals.end(); ++it)
		{
			for (int i = it->second[0]; i < it->second[1]; i++)
			{
				bin = img(i, it->first) / float(HISTO_SIZE);

				m_color_histogram[(int)floor(bin)]++;
			}
		}

		//std::map<int, cv::Vec2i> m_rowIntervals;

		for (auto it = rhs.m_rowIntervals.begin(); it != rhs.m_rowIntervals.end(); ++it)
		{
			auto mapIt = m_rowIntervals.find(it->first);

			if (mapIt == m_rowIntervals.end())
			{
				m_rowIntervals[it->first] = it->second;
			}
			else
			{
				if (it->second[0] < mapIt->second[0])
					mapIt->second[0] = it->second[0];
					
				if (it->second[1] > mapIt->second[1])
					mapIt->second[1] = it->second[1];
			}
		}

		for (unsigned j = 0; j < m_rowIntervals.size(); j++)
		{
			decltype(m_rowIntervals[j]) range = m_rowIntervals[j];

			if (!range.empty())
			{
				ASSERT(range.size() % 2 == 0);

				auto endIt = range.end();
				auto it1 = range.begin();
				auto it0 = it1++;

				for (; it1 != endIt; ++it1)
				{
					for (int i = *it0; i <= *it1; i++)
						img(i, j + m_bbox.y) = 255;

					it0 = ++it1;
				}
			}
		}

		// Compute the new row intervals
		RowIntervals newRowInt(m_bbox.height);

		for (unsigned j = 0; j < m_rowIntervals.size(); j++)
			move_back(m_rowIntervals[j], newRowInt[j + m_bbox.y - new_bbox.y]);

		for (unsigned j = 0; j < rhs.m_rowIntervals.size(); j++)
			copy_back(rhs.m_rowIntervals[j], newRowInt[j + rhs.m_bbox.y - new_bbox.y]);

		

		// Update the row intervals
		m_rowIntervals = newRowInt;

				img(x + m_bbox.x, y + m_bbox.y) = m_mask(x, y);

		for (unsigned j = 0; j < m_rowIntervals.size(); j++)
		{
			decltype(m_rowIntervals[j]) range = m_rowIntervals[j];

			if (!range.empty())
			{
				ASSERT(range.size() % 2 == 0);

				auto endIt = range.end();
				auto it1 = range.begin();
				auto it0 = it1++;

				for (; it1 != endIt; ++it1)
				{
					for (int i = *it0; i <= *it1; i++)
					{
						bin = img(i, j + m_bbox.y) / float(HISTO_SIZE);

						m_color_histogram[(int)floor(bin)]++;
					}

					it0 = ++it1;
				}
			}
		}
/*

	const int width = img.ni();
	const int height = img.nj();

	image<float> *r = new image<float>(width, height);
	image<float> *g = new image<float>(width, height);
	image<float> *b = new image<float>(width, height);

	// smooth each color channel  
	for (int y = 0; y < height; y++) {
	for (int x = 0; x < width; x++) {
	imRef(r, x, y) = img(x, y).R();
	imRef(g, x, y) = img(x, y).G();
	imRef(b, x, y) = img(x, y).B();
	}
	}

	image<float> *smooth_r, *smooth_g, *smooth_b;

	if (sigma > 0)
	{
	smooth_r = smooth(r, sigma);
	smooth_g = smooth(g, sigma);
	smooth_b = smooth(b, sigma);

	delete r;
	delete g;
	delete b;
	}
	else
	{
	smooth_r = r;
	smooth_g = g;
	smooth_b = b;
	}

	// build graph
	int num = 0;

	for (int y = 0; y < height; y++) {
	for (int x = 0; x < width; x++) {
	if (x < width-1) {
	edges[num].a = y * width + x;
	edges[num].b = y * width + (x+1);
	edges[num].w = diff(smooth_r, smooth_g, smooth_b, x, y, x+1, y);
	num++;
	}

	if (y < height-1) {
	edges[num].a = y * width + x;
	edges[num].b = (y+1) * width + x;
	edges[num].w = diff(smooth_r, smooth_g, smooth_b, x, y, x, y+1);
	num++;
	}

	if ((x < width-1) && (y < height-1)) {
	edges[num].a = y * width + x;
	edges[num].b = (y+1) * width + (x+1);
	edges[num].w = diff(smooth_r, smooth_g, smooth_b, x, y, x+1, y+1);
	num++;
	}

	if ((x < width-1) && (y > 0)) {
	edges[num].a = y * width + x;
	edges[num].b = (y-1) * width + (x+1);
	edges[num].w = diff(smooth_r, smooth_g, smooth_b, x, y, x+1, y-1);
	num++;
	}
	}
	}
	delete smooth_r;
	delete smooth_g;
	delete smooth_b;*/

std::vector<unsigned> m_groupColorMap; //!< Used to color region groups selected by the user

// Save the first region used to create the group
				// in order to color the group
				m_groupColorMap.resize(m_currentGroupId);
				m_groupColorMap[m_currentGroupId] = regId;

bool operator==(const RegionPyramid& rhs) const
	{
		// We should have the same number of levels...
		if (height() != rhs.height())
			return false;

		// The levels must be in the same order...
		// and they should have the same number of regions...
		auto it0 = m_levels.begin();
		auto it1 = rhs.m_levels.begin();
	
		for (; it0 != m_levels.end(); ++it0, ++it1)
		{
			// Compare the number of regions in each level
			if (it0->size() != it1->size())
				return false;

			// The order of the regions (ie, the region ID in a level)  
			// and their boundary should be the same
			for (unsigned i = 0; i < it0->size(); i++)
			{
				if ((*it0)[i].boundaryPts != (*it1)[i].boundaryPts)
					return false;
			}
		}

		// If we made it here, then the sets are equal
		return true;
	}

iterator end() { return m_levels.front().end(); }

	//! Returns an iterator set to the first level
	level_iterator begin() { return m_levels.begin(); }

	//! Returns an iterator set to the dummy last level
	level_iterator end()   { return m_levels.end(); }

	//! Returns an iterator set to the first level
	const_level_iterator begin() const { return m_levels.begin(); }

	//! Returns an iterator set to the dummy last level
	const_level_iterator end() const  { return m_levels.end(); }

	//! Iterates over all the regions
	//Region& operator[](unsigned i)
	//{
	//}

========================================================================
    STATIC LIBRARY : ImageSegmentation Project Overview
========================================================================

AppWizard has created this ImageSegmentation library project for you.

No source files were created as part of your project.


ImageSegmentation.vcxproj
    This is the main project file for VC++ projects generated using an Application Wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

ImageSegmentation.vcxproj.filters
    This is the filters file for VC++ projects generated using an Application Wizard. 
    It contains information about the association between the files in your project 
    and the filters. This association is used in the IDE to show grouping of files with
    similar extensions under a specific node (for e.g. ".cpp" files are associated with the
    "Source Files" filter).

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" comments to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////
