ASSERT(FrameCount() > 0);

	double frameRatio = frameIndex / double(FrameCount() - 1);
	
	DBG_PRINT2(frameRatio, FrameCount())

	if (Set(CV_CAP_PROP_POS_FRAMES, frameIndex)) //CV_CAP_PROP_POS_AVI_RATIO

// Save training data
	/*if (m_trainImages.size() < m_params.numBurnInFrames)
	{
		m_trainImages.push_back(m_queryImage);
		m_trainKeypoints.push_back(m_queryKeypoints);
		// Note, a clone of the decriptors is needed because the matrix
		// might be reused in next iteration
		m_trainDescriptors.push_back(m_queryDescriptors.clone());	

		AddNewDescriptorsToBag();

		// If we've seen all the train images, select the keypoints and 
		// descriptors that we want to keep
		if (m_trainImages.size() == m_params.numBurnInFrames)
			FinalizeBackgroundModel();
	}
	else
	{
		cv::Mat mask = ConstrainPossibleMatches(m_queryKeypoints, m_goodKeypoints);

		std::vector<std::vector<cv::DMatch>> knnMatches;

		m_matcher->knnMatch(m_queryDescriptors, m_goodDescriptors, knnMatches, 
			m_params.maxNumMatchesPerFeature, mask);

		// Resize mask and init all elements
		m_goodFeaturesMatched.assign(m_goodDescriptors.rows, false);
		m_queryFeaturesMatched.assign(m_queryDescriptors.rows, false);

		// Resize mask and init all elements
		m_bestMatchesMask.assign(knnMatches.size(), 0);

		// Make sure that the best matches array has the right size
		m_bestMatches.resize(knnMatches.size());

		for (size_t i = 0; i < knnMatches.size(); i++)
		{
			const std::vector<cv::DMatch>& matches = knnMatches[i];

			for (size_t k = 0; k < matches.size(); k++)
			{
				const cv::DMatch& dm = matches[k];

				if (!m_goodFeaturesMatched[dm.trainIdx] &&
					IsGoodMatch(*m_goodDInfo[dm.trainIdx], dm.distance))
				{
					m_goodFeaturesMatched[dm.trainIdx] = true;
					m_queryFeaturesMatched[dm.queryIdx] = true;

					m_bestMatchesMask[i] = 1;
					m_bestMatches[i] = dm;

					break;
				}
			}
		}

		ComputeMatchStats();
		ComputeUnmatchedFeaturesMask();
	}*/

	fnum_t oldFrameNum = m_currentFrameNumber;

	// Get 0-based index of the frame to be decoded/captured next
	m_currentFrameNumber = (fnum_t) cvGetCaptureProperty(m_cap, 
		CV_CAP_PROP_POS_FRAMES);

	if (m_currentFrameNumber < 0)
		m_currentFrameNumber = oldFrameNum + 1;


		{
			for (fnum_t i = m_currentFrameNumber; i < frameIndex; i++)
				cvGrabFrame(m_cap);
		}
		
		// Make sure that we have a valid frame number
		m_currentFrameNumber = (fnum_t) cvGetCaptureProperty(m_cap, 
			CV_CAP_PROP_POS_FRAMES);

		if (m_currentFrameNumber < 0)
			m_currentFrameNumber = frameIndex;

		// Reads the current frame in m_cap
		ReadNextFrame();

	/*void CvVideo::ReadFrame(int i)
{
	ASSERT(m_currentFrameNumber != INVALID_FRAME);

	// NOTE: must always go to the requested frame, even if 
	// m_currentFrameNumber is equal to i, because m_currentFrameNumber may not be correct

	cvSetCaptureProperty(m_cap, CV_CAP_PROP_POS_FRAMES, i);
	ReadCurrentFrame();
	m_currentFrameNumber = i;
}*/

void CvVideo::ReadFrame(fnum_t frameIndex)
{
	ShowStatus2("Seeking frame number ", frameIndex, "...");

	// See if we can move using an opencv function

	/*cvSetCaptureProperty(m_cap, CV_CAP_PROP_POS_FRAMES, (int) frameIndex);

	fnum_t nf = (fnum_t) cvGetCaptureProperty(m_cap, CV_CAP_PROP_POS_FRAMES);

	if (nf == frameIndex)
	{
		m_currentFrameNumber = nf;
		
		// Reads the current frame in m_cap
		ReadNextFrame();

		return;
	}*/

	// Ineficient method to deal with previous frames. ie,
	// just go to the beginning
	if (m_currentFrameNumber < 0 || frameIndex < m_currentFrameNumber)
	{
		ReadFirstFrame(); // it also sets m_currentFrameNumber = 0
	}

	// See if we need to move to a later frame or not
	if (frameIndex > m_currentFrameNumber)
	{
		// Note that m_cap already points to the next frame, so
		// we might be there already
		if (frameIndex > m_currentFrameNumber + 1)
		{
			for (fnum_t i = m_currentFrameNumber; i < frameIndex; i++)
				cvGrabFrame(m_cap);
		}
		
		// Make sure that we have a valid frame number
		m_currentFrameNumber = (fnum_t) cvGetCaptureProperty(m_cap, 
			CV_CAP_PROP_POS_FRAMES);

		if (m_currentFrameNumber < 0)
			m_currentFrameNumber = frameIndex;

		// Reads the current frame in m_cap
		ReadNextFrame();
	}
}

/*bool LoadNextVideo()
	{
		ASSERT(m_vidIt != m_vids.end());

		if (++m_vidIt == m_vids.end())
			return false;

		m_pVideo = ConstructVideoObject(m_vidIt->filename);

		return (m_pVideo && m_pVideo->Load(m_vidIt->filename));
	}

	bool LoadFirstVideo()
	{
		m_vidIt = m_vids.begin();

		if (m_vidIt != m_vids.end())
			return false;

		m_pVideo = ConstructVideoObject(m_vidIt->filename);

		return (m_pVideo && m_pVideo->Load(m_vidIt->filename));
	}*/

bool LoadNextVideo()
	{
		ASSERT(m_vidIt != m_vids.end());

		// We should already have the next video
		m_pVideo = m_pNextVideo;

		// Pre-lead the next video, if there is one
		if (++m_vidIt == m_vids.end())
		{
			m_pNextVideo.reset();

			return true; // sucess because the "trivial" pre-loading worked
		}
		else
		{
			m_pNextVideo = ConstructVideoObject(m_vidIt->filename);

			if (m_pNextVideo && m_pNextVideo->Load(m_vidIt->filename))
			{
				return true;
			}
			else
			{
				m_pNextVideo.reset();
				return false;
			}
		}
	}

	bool LoadFirstVideo()
	{
		m_vidIt = m_vids.begin();

		if (m_vidIt != m_vids.end())
			return false;

		// Load the first video as the "next" video. Then, make
		// this next video the "current" one and pre-load the 
		// actual next one
		m_pNextVideo = ConstructVideoObject(m_vidIt->filename);

		if (m_pNextVideo && m_pNextVideo->Load(m_vidIt->filename))
			return LoadNextVideo();
		else
			return false;
	}
========================================================================
    STATIC LIBRARY : VideoRepresentation Project Overview
========================================================================

AppWizard has created this VideoRepresentation library project for you.

No source files were created as part of your project.


VideoRepresentation.vcxproj
    This is the main project file for VC++ projects generated using an Application Wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

VideoRepresentation.vcxproj.filters
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
