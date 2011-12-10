
	g_userArgs.ReadBoolArg("ObjectLearner", "learnModelViews", 
		"Whether to add new model object views to the database", 
		false, &m_params.learnModelViews);

/*if (ShapeContext::GetParams().doTPSWarp)
				{
					ptp.A.Row(0) << 0 << 1 << 0;
					ptp.A.Row(1) << 0 << 0 << 1;

					ptp.T = Zeros(ptp.T.rows(), ptp.T.cols());
				}
				else
				{
					ptp.A.Row(0) << 1 << 0 << 0;
					ptp.A.Row(1) << 0 << 1 << 0;
					ptp.A.Row(2) << 0 << 0 << 1;
				}*/				
				
				/*if (v != nil)
				{
					spc.GetTransformationParams(u, v, &ptp);

					PointArray wrpPts = query_spg->inf(u).ptrDescriptor->WarpAffine(ptp.P);
					LineList ll;

					for (unsigned i = 0 ; i < wrpPts.size(); i++)
					{
						ll.push_back(std::make_pair(wrpPts[i], ptp.P[i]));
					}

					DrawLines(ll, 1.0);
				}*/

========================================================================
    STATIC LIBRARY : ObjectRecognition Project Overview
========================================================================

AppWizard has created this ObjectRecognition library project for you.

No source files were created as part of your project.


ObjectRecognition.vcxproj
    This is the main project file for VC++ projects generated using an Application Wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

ObjectRecognition.vcxproj.filters
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
