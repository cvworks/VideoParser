/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/VisSysComponent.h> // defines GenericVSCGraph
#include <Tools/ParamFile.h>
#include <VideoRepresentation/Video.h>
#include <VideoRepresentation/VideoWriter.h>

namespace vpl {

class VisSysComponentCreator;

/*!
	Graph of Vision System Components (VSC).

	It allows for the storage and retrival of video data
	computed by the visys components.
*/
class VSCGraph : public GenericVSCGraph
{
public:
	#ifdef USE_LEDA_GRAPH_CLASS
		typedef leda::node Node;
		typedef leda::edge Edge;
	#else
		typedef GenericVSCGraph::node Node;
		typedef GenericVSCGraph::edge Edge;
	#endif

protected:
	std::vector<Node> m_sortedNodes;

	std::vector<OutputImageInfo> m_outImgInfo;

	ParamFile m_params; //!< The parameters define the components and their dependencies

	VSCDataSerializer* m_pDataSerializer; //!< Optional loader/saver of VSC's data

	SQLDatabase* m_pSQLDatabase; //!< General SQL database

	// Member variables that hold partial results
	bool m_saveResults;
	VideoWriter m_videoWriter;
	std::string m_outVideoFilename;

	static VisSysComponentCreator* s_compCreator;

protected:
	Node NewNode(VSCPtr ptr)
	{ 
		return GenericVSCGraph::new_node(ptr); 
	}

	Edge NewEdge(Node u, Node v, const double& w = 1) 
	{
		return GenericVSCGraph::new_edge(u, v, w); 
	}

	Node AddComponent(const std::string& name);
	
public:
	// There is no constructor because Initialize() takes care of everything!

	~VSCGraph()
	{
		clear();
	}

	//! Gets the global component creator
	static VisSysComponentCreator* GetComponentCreator()
	{
		return s_compCreator;
	}

	bool Initialize(RUNNING_MODE runningMode, const std::string& taskName, 
		VSCDataSerializer* pDataSerializer, SQLDatabase* pDB);

	/*!
		Gets the current data serializer. It is NULL if 
		there is no associated serializer.
	*/
	VSCDataSerializer* GetDataSerializer()
	{
		return m_pDataSerializer;
	}

	/*! 
		Empties all the contents of the graph. It call the function Clear() of 
		each vision system component before deleting it.
	*/
	void clear()
	{
		node v;

		forall_nodes(v, *this)
		{
			attribute(v)->Clear();
		}

		// Call the respective function in the base class
		GenericVSCGraph::clear();
	}

	/*!
		Calls the OnGUIEvent() function of each component. It is useful
		for doing postprocessing of each frame.

		This function is called when there is a GUI event that some
		component might want to know about. It is delivered to all
		components in their dependency-sorted order.
	*/
	void OnGUIEvent(int id, int value) const
	{
		UserEventInfo uei(id, value);

		for (auto it = m_sortedNodes.begin(); it != m_sortedNodes.end(); ++it)
			Component(*it)->OnGUIEvent(uei);
	}

	/*!
		Calls the PostProcessSequence() function of each component. It is useful
		for doing postprocessing at the end of a video.
	*/
	void PostProcessSequence()
	{
		for (auto it = m_sortedNodes.begin(); it != m_sortedNodes.end(); ++it)
			Component(*it)->PostProcessSequence();
	}

	void Reset();

	void StartRecordingResults(const std::string& filename);

	void StopRecordingResults();

	void ProcessFrameForPlayback(const InputImageInfo& imgInfo);

	void ProcessNewFrame(const InputImageInfo& imgInfo);

	void ProcessDataOffline();

	void MultithreadComponentRun();

	const VSCPtr Component(Node v) const
	{ 
		return inf(v); 
	}

	VSCPtr& Component(Node v)
	{ 
		return operator[](v); 
	}

	void InitOutputImageInfo();
	void GetOutputImageLabels(std::vector<std::string>& lbls) const;

	void GetParameterInfo(int idx, DoubleArray* pMinVals, 
		DoubleArray* pMaxVals, DoubleArray* pSteps) const
	{
		ASSERT(idx < (int)m_outImgInfo.size());

		const OutputImageInfo& oii = m_outImgInfo[idx];

		oii.component->GetParameterInfo(oii.index, pMinVals, pMaxVals, pSteps);
	}

	std::list<UserCommandInfo> GetUserCommands(int idx) const
	{
		ASSERT(idx < (int)m_outImgInfo.size());

		const OutputImageInfo& oii = m_outImgInfo[idx];

		return oii.component->GetUserCommands(oii.index);
	}

	const OutputImageInfo& GetOutputImageInfo(int idx) const
	{
		ASSERT(idx < (int)m_outImgInfo.size());

		return m_outImgInfo[idx];
	}

	/*!
		Retrieves the image info.

		It also sets the appropriate outputIdx
	*/
	void GetDisplayInfo(int idx, DisplayInfoIn& dii, DisplayInfoOut& dio) const
	{
		const OutputImageInfo& oii = GetOutputImageInfo(idx);

		dii.outputIdx = oii.index;

		oii.component->GetDisplayInfoWithMutex(dii, dio);
	}
};

} // namespace vpl
