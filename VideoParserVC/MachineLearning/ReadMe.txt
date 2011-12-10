SpatialPyramid computeMerge(const SpatialPyramid& rhs) const
	{
		ASSERT(rhs.imageWidth() == imageWidth() && 
			   rhs.imageHeight() == imageHeight() &&
			   rhs.size() == size());

		SpatialPyramid sp(imageWidth(), imageHeight(), size());

		int l0, l1;

		for (unsigned k = 0; k < sp.size(); k++)
		{
			const SpatialPyramid::SpatialLevel& img0 = get(k);
			const SpatialPyramid::SpatialLevel& img1 = rhs.get(k);
			SpatialPyramid::SpatialLevel& img2 = sp.get(k);

			SpatialStats::LABEL lbl;

			for (unsigned i = 0; i < img2.ni(); i++)
			{
				for (unsigned j = 0; j < img2.nj(); j++)
				{
					l0 = img0(i, j).label;
					l1 = img1(i, j).label;

					if ((l0 <= SpatialStats::SOMETIMES && l1 >= SpatialStats::SOMETIMES) ||
						(l0 >= SpatialStats::SOMETIMES && l1 <= SpatialStats::SOMETIMES))
					{
						lbl = SpatialStats::SOMETIMES;
					}
					else if (l0 == SpatialStats::ALWAYS && l1 == SpatialStats::ALWAYS)
					{
						lbl = SpatialStats::ALWAYS;
					}
					else if (l0 == SpatialStats::NEVER && l1 == SpatialStats::NEVER)
					{
						lbl = SpatialStats::NEVER;
					}
					else if (l0 < SpatialStats::SOMETIMES && l1 < SpatialStats::SOMETIMES)
					{
						lbl = SpatialStats::VERY_LIKELY;
					}
					else if (l0 > SpatialStats::SOMETIMES && l1 > SpatialStats::SOMETIMES)
					{
						lbl = SpatialStats::VERY_UNLIKELY;
					}
					else
					{
						ASSERT(false);
					}

					img2(i, j).label = lbl;
					sp.incrementLabelCounter(k, lbl);
				}
			}
		}

void set(const XYCoord coord)
		{
			pt = coord;
		}

		void set(const Point& p)
		{
			pt.x = (int)p.x;
			pt.y = (int)p.y;
		}

		void set(int x, int y)
		{
			pt.x = x;
			pt.y = y;
		}

static char buffer[MAX_PATH_SIZE];


//m_pBoostMapModule->BmNew(m_params.data_directory.c_str(), 
		//	m_params.number_of_triples, m_params.use_pdistances);

// Copy the output filename to the modifiable buffer
		std::size_t n = m_params.output_filename.size();
		m_params.output_filename._Copy_s(buffer, MAX_PATH_SIZE, n);
		buffer[n] = '\0';

		m_pBoostMapModule->BmSteps(buffer, m_params.steps);

/////////////////////////////////////////////////////////////////////////////
