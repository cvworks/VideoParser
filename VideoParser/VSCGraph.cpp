/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "VSCGraph.h"
#include "ImageProcessor.h"
#include "VisSysComponentCreator.h"

#include <queue>
#include <exception>

#include <vil/algo/vil_tile_images.h>
#include <vul/vul_timer.h>

#include <Tools/UserArguments.h>
#include <Tools/Num2StrConverter.h>
#include <Tools/Exceptions.h>

//#include <cxcore.h>

using namespace vpl;

extern UserArguments g_userArgs;

//! Global map of component labels
//CompLblMap g_compLblMap;
VisSysComponentCreator* VSCGraph::s_compCreator = new VisSysComponentCreator();

struct GraphNodeInfo
{
	VSCGraph& g;
	graph::node v;

	GraphNodeInfo(VSCGraph& _g, graph::node _v) : g(_g), v(_v) { };
};


//! Adds a vision system component to the graph
VSCGraph::Node VSCGraph::AddComponent(const std::string& name)
{
	// Create the appropriate object
	VisSysComponent* pComp = s_compCreator->NewVisSysComponent(name);

	// When the component class is 'none', there no component created
	if (!pComp)
		return nil;

	// Let the component know the object that can help it 
	// store/retrieve its data from a database if it needs to.
	pComp->SetDataSerializer(m_pDataSerializer);

	// Let the component know the object that can help it 
	// interact with an SQL database.
	pComp->SetSQLDatabase(m_pSQLDatabase);

	return NewNode(VSCPtr(pComp));
}

/*!
	Initializes the graph.

	@taskName Name of the task to be performed by the network of
	          vision system components.

	@param pDataSerializer Optional parameter that specifies an object
	       that stores/retrieves the data of a VSC component. It *must*
		   be NULL if storage/retrieval operations are not desired.
*/
bool VSCGraph::Initialize(RUNNING_MODE runningMode, const std::string& taskName, 
	VSCDataSerializer* pDataSerializer, SQLDatabase* pDB)
{
	std::string compName;
	struct NodeInfo { Node node; bool init; StrArray deps; };
	std::map<std::string, NodeInfo> compInfoMap;
	std::queue<NodeInfo*> compQueue;

	ShowStatus("\nInitializing components...");

	m_runningMode = runningMode;
	m_taskName = taskName;
	m_pDataSerializer = pDataSerializer;
	m_pSQLDatabase = pDB;

	m_saveResults = false; // by default, the results are not saved

	// Recover the settings for every possible component (and target task)
	for (auto it = s_compCreator->begin(); it != s_compCreator->end(); ++it)
	{
		compName = s_compCreator->ReadUserSelection(it, taskName);

		// Map the GENERIC component name *it to the
		// info of its selected specialized component 
		NodeInfo& ni = compInfoMap[it->first]; 

		ni.node = AddComponent(compName);

		if (ni.node != nil)
		{
			ni.init = false;
			ni.deps = Component(ni.node)->Dependencies();
			compQueue.push(&ni);
		}
		else
		{
			// Set to init (otherwise it'll always be false)
			ni.init = true; 
		}	
	}

	// Add edges using the components' dependency information
	std::map<std::string, NodeInfo>::iterator compIt;
	unsigned int i;

	for (compIt = compInfoMap.begin(); compIt != compInfoMap.end(); ++compIt)
	{
		Node& v = compIt->second.node;
		StrArray& deps = compIt->second.deps;

		for (i = 0; i < deps.size(); i++)
		{
			Node& u = compInfoMap[deps[i]].node;

			if (u != nil)
				NewEdge(u, v);

			// Some parent components are optional. So, let the dependants complain
			// if a mandatory parent is missing
			//else 
			//	StreamError("Missing dependent component " << deps[i] 
			//		<< " of " << compIt->first);
		}
	}

	// Initialize all components in the order defined by their dependencies
	NodeInfo* pNI;

	// Get num comps from compQueue, which has only non-null nodes
	m_sortedNodes.reserve(compQueue.size());

	while (!compQueue.empty())
	{
		pNI = compQueue.front();
		compQueue.pop();
		
		// See if all the dependencies of the node are initialized
		for (i = 0; i < pNI->deps.size(); i++)
		{
			if (!compInfoMap[pNI->deps[i]].init)
			{
				compQueue.push(pNI); // put component back in the queue
				break;
			}
		}

		// If all dependencies were seen
		if (i == pNI->deps.size())
		{
			m_sortedNodes.push_back(pNI->node);

			ShowStatus1("\tinitializing", Component(pNI->node)->Name());
			
			Component(pNI->node)->Initialize(pNI->node);
			
			pNI->init = true;
		}
	}

	// Init output image info

	InitOutputImageInfo();

	return true;
}

//! Calls VisSysComponent::Clear() for all nodes of the graph
void VSCGraph::Reset()
{
	node v;

	forall_nodes(v, *this)
		Component(v)->Clear();
}

/*!
	It queries each components about the output images that
	they produces and stores they answer into a convenient data structure.
*/
void VSCGraph::InitOutputImageInfo()
{
	// Find out how many output images we have
	Node v;
	int numImgs = 0;

	// Add up the output image count off all components
	forall_nodes(v, *this)
	{
		numImgs += Component(v)->NumOutputImages();
	}
	
	// Set the size of the image info vector
	m_outImgInfo.resize(numImgs);

	// Init the info of each output image
	VSCPtr comp;
	int n = 0;

	// Add component info following their dependency order
	// so that they are displayed correctly
	for (unsigned int i = 0; i < m_sortedNodes.size(); i++)
	{
		v = m_sortedNodes[i];

		comp = Component(v);

		for (int i = 0; i < comp->NumOutputImages(); i++)
		{
			OutputImageInfo& oii = m_outImgInfo[n++];

			oii.component = comp;
			oii.index = i;
			oii.label = comp->GetOutputImageLabel(i);
		}
	}
}

/*!
	Gets the labels of the output images of each component.
*/
void VSCGraph::GetOutputImageLabels(std::vector<std::string>& lbls) const
{
	std::string str;

	lbls.clear();

	for (unsigned int i = 0; i < m_outImgInfo.size(); i++)
	{
		str = m_outImgInfo[i].component->Name();
		str += "," + m_outImgInfo[i].label;

		lbls.push_back(str);
	}
}

/*!
	Runs only the first component on each frame.
*/
void VSCGraph::ProcessFrameForPlayback(const InputImageInfo& imgInfo)
{
	// Store the current frame data
	m_inImgInfo = imgInfo;

	// Set the timestamp in the base class
	SetTimestamp(m_inImgInfo.timestamp);

	// Call the ImageProcessor component, which should be the only 
	// component with no dependencies. ie, the first component in the
	// sorted list of components

	Component(m_sortedNodes.front())->RunWithMutex(GetRunningMode());
}

/*!
	Runs all components without any video information.
*/
void VSCGraph::ProcessDataOffline()
{
	// Run each component in the order determined by their
	// dependencies
	m_inImgInfo.Clear();

	for (unsigned int i = 0; i < m_sortedNodes.size(); i++)
		Component(m_sortedNodes[i])->RunWithMutex(GetRunningMode());
}

//! Globals function used to create new video parsing threads
void* RunComponent(void* pParams)
{
	/*GraphNodeInfo* gni = (GraphNodeInfo*) pParams;
	graph::node u;

	std::vector<Mutex::HANDLE> muts;
	
	muts.reserve(gni->g.indeg(gni->v));

	forall_in_nodes(u, gni->v)
		muts.push_back(gni->g.Component(u)->GetMutexHandle());

	DWORD returnCode = WaitForMultipleObject(m_hMutex, INFINITE);

	if (returnCode)
		gni->g.Component(gni->v)->RunWithMutex();*/

	return 0;
}

void VSCGraph::MultithreadComponentRun()
{
	enum STATE {WAITING, RUNNING, FINISHED};
	std::vector<STATE> states(m_sortedNodes.size(), WAITING);
	Node v;

	/*for (unsigned int i = 0; i < m_sortedNodes.size(); i++)
	{
		v = m_sortedNodes[i];

		forall_parents(u, v)

		

		Component(v)->RunWithMutex();
	}*/
}

/*!
	Runs all components on each frame while respecting their dependencies.
*/
void VSCGraph::ProcessNewFrame(const InputImageInfo& imgInfo)
{
	std::cout << "Processing frame " << imgInfo.frameNumber << std::endl;
	// Store the current frame data
	m_inImgInfo = imgInfo;

	// Set the timestamp in the base class
	SetTimestamp(m_inImgInfo.timestamp);

	// Prepare a container to save the output images (if they are requested)
	std::list<RGBImg> outImgs;

	// Run each component in the order determined by their
	// dependencies
	Node v;
	vul_timer tic;

	for (unsigned int i = 0; i < m_sortedNodes.size(); i++)
	{
		v = m_sortedNodes[i];

		StreamStatus("Running " << Component(v)->Name() << "...");

		tic.mark();

		try {
			Component(v)->RunWithMutex(GetRunningMode());
		} 
		catch (std::exception e)
		{
			ShowError(e.what());
			ASSERT(false);
			break;
		}
		catch (...)
		{
			ShowError("A vision system component has crashed");
			ASSERT(false);
			break;
		}

		StreamStatus(" (" << tic.real() << " milliseconds)");

		// Save the results if requested
		if (m_saveResults && Component(v)->HasOutputImagesToSave())
		{
			ShowStatus1("Saving results for", Component(v)->Name());

			Component(v)->GetOutputImagesToSave(&outImgs);
		}
	}

	if (!outImgs.empty())
	{
		vcl_vector<RGBImg> patches(outImgs.begin(), outImgs.end());
		RGBImg bigImage;

		vil_tile_images(bigImage, patches);

		if (!m_videoWriter.IsOpen())
		{
			m_videoWriter.Open(m_outVideoFilename, 
				bigImage.ni(), bigImage.nj());
		}

		if (m_videoWriter.IsOpen())
			m_videoWriter.WriteFrame(bigImage);
		else
			ShowError("Cannot open output video");
	}
}

void VSCGraph::StartRecordingResults(const std::string& filename)
{
	ASSERT(!m_videoWriter.IsOpen());

	m_outVideoFilename = filename;

	m_saveResults = true;
}

void VSCGraph::StopRecordingResults()
{
	if (m_videoWriter.IsOpen())
	{
		m_videoWriter.Close();
	}

	m_saveResults = false;
}

//void VSCGraph::SavePartialResults(int nType /*=0*/)
//{
//	static const char* movieTypes[] = {"ImageList", "AVI", "MPEG"};
//	static const char* fileNames[] = {"Results/segment", "Results/motEst"};
//	
//	if (!g_userArgs.bSaveResults)
//		return;
//	
//	ShowStatus("Saving partial results...");
//	
//	ASSERT(nType >= 0 && nType <= 2);
//	
//	vidl_movie_sptr movies[2];
//	
//	vidl_clip_sptr segClip = new vidl_clip(m_segmentationResults);
//	movies[0] = new vidl_movie(segClip);
//	
//	vidl_clip_sptr motEstClip = new vidl_clip(m_motionEstimResults);
//	movies[1] = new vidl_movie(motEstClip);
//	
//	for (int i = 0; i < 2; i++)
//		if (!vidl_io::save(movies[i], fileNames[i], movieTypes[nType]))
//			ShowError1("Failed to save movie", fileNames[i]);
//		else	
//			ShowStatus1(fileNames[i], "was saved");
//	
//	// Show supported video types
//	/*vcl_list<vcl_string> strList = vidl_io::supported_types();
//	vcl_list<vcl_string>::iterator lit;
//	
//	for (lit = strList.begin(); lit != strList.end(); ++lit)
//		std::cerr << *lit << ", " << std::endl;*/
//}
//
//void StorePartialResults()
//{
//	if (!g_userArgs.bSaveResults)
//		return;
//	
//	ShowStatus("Storing partial results...");
//	
//	/*char szFileName[1024];
//	FloatImg outImg1 = pMotionEst->GetMotionMap(0, MotionMaps::OUTLIERS);
//	ByteImg outImg2;
//	
//	vil_convert_stretch_range(outImg1, outImg2);
//	
//	sprintf(szFileName, "Clip03/Outliers%d.pgm", nFrame);
//	vil_save(outImg2, szFileName);
//	
//	RGBImg outImg3 = CurrentRGBSegmentation();
//	sprintf(szFileName, "Clip03/Segmentation%d.ppm", nFrame);	
//	vil_save(outImg3, szFileName);*/
//	
//	FloatImg outImg1 = m_pMotionEst->GetMotionMap(0, MotionMaps::OUTLIERS);
//	ByteImg outImg2;				
//	vil_convert_stretch_range(outImg1, outImg2);
//	m_motionEstimResults.push_back(vil_new_image_resource_of_view(outImg2));
//	
//	//segmentationResults.push_back(vil_new_image_resource_of_view(CurrentRGBSegmentation()));
//	ByteImg outImg3;
//	vil_convert_stretch_range(CurrentRegion(), outImg3);
//	m_segmentationResults.push_back(vil_new_image_resource_of_view(outImg3));
//}