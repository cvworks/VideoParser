/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <list>
#include <vil/vil_new.h>
#include "Mutex.h"
#include "Exceptions.h"
#include "VisSysUtils.h"

#define FindParentComponent(T) std::dynamic_pointer_cast<const T>(ParentComponent(#T))

#define FindSubordinateComponent(T) std::dynamic_pointer_cast<const T>(SubordinateComponent(#T))

#define ShowMissingDependencyError(T) StreamError("Component " << Name() << \
	" cannot run because it depends on the missing component " << #T)

namespace vpl {

class VSCDataSerializer;
class SQLDatabase;

/*!
	Abstract vision system component.

	A derived vision system component (VSC or visys comp) is required to

	1. Specify if it has data to show after each call to run
	2. Define all pure virtual functions
	3. If necesary, write a specialize drawing function
	4. Save and retrieve any intermediate data that can speed up
	   the processing of the same video multiple times.

	Function call specs for Initialize() and Clear():

	Initialize(): called right after the component is created and after 
	all the corresponding Initialize() functions of the parents were called.

	Clear(): It is called by Initialize(), before initializing the component. 
	It is also called by VSCGraph::Reset() before a new video is processed. 
	Thus, member variables that must persist across videos should not be 
	"cleared" with this function.
*/
class VisSysComponent
{
private:
	Mutex* m_pProcDrawMutex; //!< Mutex used to sync processing and drawing operations
	int m_validRunningModes; //!< Bitwise modes in which the Run() function should be executed

	// Declare without a body to eliminate the possibility of calling the function
	VisSysComponent(const VisSysComponent& rhs);

	// Declare without a body to eliminate the possibility of calling the function
	void operator=(const VisSysComponent& rhs);

protected:
	/*!
	*/
	struct UserCommand : public UserCommandInfo
	{
		void (VisSysComponent::*cmdFn)(void* param) const; //!< Command function

		//! Compare a (outIdx, keyCode) pair against the appror fields
		bool operator==(int k) const
		{
			return (keyCode == k);
		}
	};

	typedef std::list<UserCommand> UserCommands;

protected:
	graph::node m_containerNode;

	bool m_saveOutputImages; //!< Whether to save the out imgs of the component or not
	bool m_verbose;     //!< Run in verbose mode or not

	std::list<OutputImageParam> m_outImgsParams;

	std::string m_superiorComponentClassName;
	std::vector<VSCPtr> m_subordinateComponents;

	/*! 
		Data serializer that takes care of associating each
		piece of data saved/loaded with the current frame and video.
	*/
	VSCDataSerializer* m_pDataSerializer; 

	//! Optional SQL database 
	SQLDatabase* m_pSQLDatabase;

	//! Map from output indices to user commands
	std::map<int, UserCommands> m_userCommands;

protected:

	void AddSubordinateComponent(VSCPtr subComp)
	{
		subComp->m_superiorComponentClassName = ClassName();

		m_subordinateComponents.push_back(subComp);
	}

	/*!
		Helper function to flip the value of a boolean parameter
	*/
	void SwitchParamOnOff(void* param) const
	{
		*((bool*) param) = !*((bool*) param);
	}

	/*!
		Registers user command. The list of registered user commands
		is emptied each time the component is initialized.

		@see ExecuteUserCommand
	*/
	void RegisterUserCommand(int outputIdx, const UserCommand& cmd)
	{
		m_userCommands[outputIdx].push_back(cmd);
	}

	/*!
		Registers user command. The list of registered user commands
		is emptied each time the component is initialized.

		Note: copy strings so that 'const char*' are valid params

		@see ExecuteUserCommand
	*/
	void RegisterUserCommand(int outputIdx, std::string label, std::string tooltip, 
		int keyCode, void* param, void (VisSysComponent::*cmdFn)(void*) const)
	{
		UserCommand cmd;

		cmd.Set(label, tooltip, keyCode, param);

		cmd.cmdFn = cmdFn;
		
		RegisterUserCommand(outputIdx, cmd);
	}

	/*!
		Registers a list of simple user arguments acting as switches.
	*/
	void RegisterUserSwitchCommands(int outputIdx, const std::list<UserCommandInfo>& cmds)
	{
		UserCommand cmd;

		cmd.cmdFn = &vpl::VisSysComponent::SwitchParamOnOff;

		for (auto it = cmds.begin(); it != cmds.end(); ++it)
		{
			static_cast<UserCommandInfo&>(cmd) = *it;

			RegisterUserCommand(outputIdx, cmd);
		}
	}

	/*!
		Returns the VSC Graph that contains the component.
	*/
	const GenericVSCGraph* ContainerGraph() const
	{
		return static_cast<const GenericVSCGraph*>(graph_of(m_containerNode));
	}

	void SetAntiNoiseMode(bool mode) const
	{
		ContainerGraph()->SetAntiNoiseMode(mode);
	}

	bool AntiNoiseMode() const
	{
		return ContainerGraph()->GetAntiNoiseMode();
	}

	const InputImageInfo& GetInputImageInfo() const
	{
		return ContainerGraph()->GetInputImageInfo();
	}

	const std::string& VideoFilename() const
	{
		return ContainerGraph()->GetVideoFilename();
	}

	fnum_t FrameNumber() const
	{
		return ContainerGraph()->GetFrameNumber();
	}

	/*!
		Returns the task that the system of vision components
		aims to perform.
	*/
	std::string TaskName() const
	{
		return ContainerGraph()->TaskName();
	}

	/*const VisSysComponent* ParentComponentByIndex(unsigned int i)
	{
		const GenericVSCGraph* pG = ContainerGraph();

		graph::edge e = pG->in_edge(m_containerNode, i);

		ASSERT(!pG->IsNil(e));

		return (const VisSysComponent*) pG->inf(source(e));
	}*/

	/*!
		If the node has a parent component with name 'genericName', 
		it returns a pointer to it. Otherwise, it returns NULL.
	*/
	ConstVSCPtr ParentComponent(const char* genericName)
	{
		const GenericVSCGraph* pG = ContainerGraph();

		graph::edge e;
		ConstVSCPtr pComp;

		forall_edges(e, *pG)
		{
			pComp = std::const_pointer_cast<const VisSysComponent>(pG->inf(source(e)));

			if (pComp->GenericName() == genericName)
				return pComp;
		}

		return pComp;
	}

	/*!
		If the node has a parent component with name 'genericName', 
		it returns a pointer to it. Otherwise, it returns NULL.
	*/
	ConstVSCPtr SubordinateComponent(const char* genericName)
	{
		for (size_t i = 0; i < m_subordinateComponents.size(); i++)
			if (m_subordinateComponents[i]->GenericName() == genericName)
				return m_subordinateComponents[i];

		return ConstVSCPtr();
	}

	/*!
		Helper function to resize 3 arrays and initialize
		their elements.
	*/
	static void InitArrays(unsigned sz, 
		DoubleArray* a0, DoubleArray* a1, DoubleArray* a2, 
		const double& v0, const double& v1, const double& v2)
	{
		a0->resize(sz, v0);
		a1->resize(sz, v1);
		a2->resize(sz, v2);
	}

	/*!
		Helper function to subtract one to every non-zero element
		of an array. Useful for the array of maximum values
		in GetParameterInfo().
	*/
	static void SubtractOneFromNonZeroValues(DoubleArray* a)
	{
		for (auto it = a->begin(); it != a->end(); ++it)
			if (*it > 0)
				(*it)--;
	}

	/*!
		Sets whether a component should be
		run when the running mode is "online" or "offline".

		By default, a component runs in "online" mode only. That is,
		by default, a component does NOT run in "offline" mode.
	*/
	void SetRunningMode(int mode)
	{
		m_validRunningModes = mode;
	}

	/*!
		Gets the bitwise combination of all valid running
		modes of teh component.
	*/
	int GetRunningMode() const
	{
		return m_validRunningModes;
	}

	/*! 
		@brief Draws the output of the component when it can't be 
		displayed simply by an image.

		By default, a component doesn't draw anything. 

		Note that Draw() is normally executed in a different thread than the 
		other VisComponent functions, such as Run(). To avoid conflicts, the
		mutex *m_pProcDrawMutex must be used to control access to any resource
		shared by the processing and drawing threads.

		HasDrawFunction() is true only if m_pProcDrawMutex is not NULL.
		
		A call to Draw() must be surrounded by calls to LockDrawingMutex()
		and UnlockDrawingMutex(). Alternatively, the function 
		DrawWithMutex() can be called instead to accomplish this. 
	*/
	virtual void Draw(const DisplayInfoIn& dii) const
	{
		// nothing to do	
	}

	/*!	
		Returns the basic information specifying the output of this component.
		It must provide an image, its type, and a text message. All of this parameters 
		are optional. For example, if there is no output image, the image type
		can be set to VOID_IMAGE. 
	
		Note that GetDisplayInfo() is normally executed in a different thread than the 
		other VisComponent functions, such as Run(). To avoid conflicts, the
		mutex *m_pProcDrawMutex must be used to control access to any resource
		shared by the processing and drawing threads.
		
		A call to GetDisplayInfo() must be surrounded by calls to LockDrawingMutex()
		and UnlockDrawingMutex(). Alternatively, the function 
		GetDisplayInfoWithMutex() can be called instead to accomplish this.
	*/
	virtual void GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const = 0;

	/*!
		This is the main processing function of the component.

		Note that Run() is normally executed in a different thread than Draw() 
		and GetDisplayInfo(). To avoid conflicts, the mutex *m_pProcDrawMutex 
		must be used to control access to any resource shared by the processing 
		and drawing threads.
		
		A call to Run() must be surrounded by calls to LockDrawingMutex()
		and UnlockDrawingMutex(). Alternatively, the function 
		RunWithMutex() can be called instead to accomplish this.
	*/
	virtual void Run() = 0;

	/*! 
		Called by Initialize() if the list subordinate components is empty.

		Note: if this function is overwritten, remeber to call the base version
		of it, because parent components might be adding their own subordinate
		components.
	*/
	virtual void AddSubordinateComponents()
	{
		// nothing to do by default
	}

public:
	VisSysComponent()
	{ 
		m_pDataSerializer = NULL;
		m_pSQLDatabase = NULL;
		m_pProcDrawMutex = NULL;
		m_verbose = false;
		m_saveOutputImages = false;
		
		// By default, a component runs only in ONLINE mode.
		m_validRunningModes = ONLINE_RUNNING_MODE;

		// don't call Clear() because derived classes are
		// not constructed yet and that function is virtual
	}

	virtual ~VisSysComponent() 
	{
		delete m_pProcDrawMutex;
	}

	//! Creates the mutex used to sync processing and drawing operations
	virtual void CreateDrawingMutex()
	{
		if (!m_pProcDrawMutex)
			m_pProcDrawMutex = new Mutex();
	}
	
	//! Locks the resources shared by the processing and drawing threads
	virtual void LockDrawingMutex() const
	{
		m_pProcDrawMutex->Lock();
	}

	//! Releases the lock on shared resources
	virtual void UnlockDrawingMutex() const
	{
		m_pProcDrawMutex->Release();
	}

	/*! 
		The parameter pDataSerializer can be set 
		to NULL if no data serialization is required.
	*/
	void SetDataSerializer(VSCDataSerializer* pDataSerializer) 
	{
		m_pDataSerializer = pDataSerializer;
	}

	/*! 
		The parameter pDB can be set to NULL if no SQL database is used.
	*/
	void SetSQLDatabase(SQLDatabase* pDB) 
	{
		m_pSQLDatabase = pDB;
	}

	/*!
		Returns the current timestamp, which is the same for all
		component in the same container graph.
	*/
	time_t Timestamp() const
	{
		return ContainerGraph()->GetTimestamp();
	}

	/*!
		Reads common parameters for all components. If specialized,
		the based version of the function should also be called.
	*/
	virtual void ReadParamsFromUserArguments();

	/*!
		Calls Clear(), sets the container node for the component, 
		and reads all user-defined parameters.
	*/
	virtual void Initialize(graph::node v)
	{
		// See of there are subordinate components to add
		if (m_subordinateComponents.empty())
			AddSubordinateComponents();

		// Initialize the subordinate components first
		for (size_t i = 0; i < m_subordinateComponents.size(); i++)
			m_subordinateComponents[i]->Initialize(v);

		Clear();

		m_containerNode = v;

		m_userCommands.clear();

		ReadParamsFromUserArguments();

		CreateDrawingMutex();
	}

	/*!
		This function is rarely needed. It is called right
		after a whole video is processed.
	*/
	virtual void PostProcessSequence()
	{
		// nothing to do
	}

	/*!
		Clears all member variables dedicated to holding
		"temporal" information needed to process data from 
		a video frame.

		This function is called by void VSCGraph::Reset()
		before a new video is processed.

		Note 1: member variables that must persist across videos
		should not be "cleared" with this function.

		Note 2: if this function is overwritten, remeber to call the base version
		of it, so that the subordinate components are also cleared.
	*/
	virtual void Clear()
	{
		// Clear the subordinate components
		for (size_t i = 0; i < m_subordinateComponents.size(); i++)
			m_subordinateComponents[i]->Clear();
	}

	bool HasDrawFunction() const
	{
		return (m_pProcDrawMutex != NULL);
	}

	bool HasOutputImagesToSave() const
	{
		return m_saveOutputImages;
	}

	bool Verbose() const
	{
		return m_verbose;
	}

	/*!
		Executes a user command. Return true if the given keycode
		has an associated command and false otherwise.
	*/
	bool ExecuteUserCommand(int outputIdx, int keyCode) const
	{
		std::map<int, UserCommands>::const_iterator mapIt;

		mapIt = m_userCommands.find(outputIdx);

		if (mapIt != m_userCommands.end())
		{
			UserCommands::const_iterator it;

			it = std::find(mapIt->second.begin(), mapIt->second.end(), keyCode);

			if (it != mapIt->second.end())
			{
				((*this).*(it->cmdFn))(it->pParam);
				return true;
			}
		}

		return false;
	}

	std::list<UserCommandInfo> GetUserCommands(int outputIdx) const
	{
		std::map<int, UserCommands>::const_iterator mapIt;
		std::list<UserCommandInfo> cmds;

		mapIt = m_userCommands.find(outputIdx);

		if (mapIt != m_userCommands.end())
			cmds.assign(mapIt->second.begin(), mapIt->second.end());

		return cmds;
	}

	RGBImg GetRGBOutputImage(const DisplayInfoIn& dii) const;

	void GetOutputImagesToSave(std::list<RGBImg>* pOutImgs) const;

	/*!
		Updates the drawing parameters for the current object or
		for all the objects of the derived class. Ie, it might call 
		static member functions that affect the display of all objects.

		@param keyCode is provided by user (alpha chars a always lower case)
		@return true if a redraw is needed, and false otherwise.
	*/
	virtual bool UpdateDrawingState(const DisplayInfoIn& dii, int keyCode) const
	{
		return ExecuteUserCommand(dii.outputIdx, keyCode);
	}

	/*!
		Returns the min, max and step for each param associated
		with the output index 'i'.
	*/
	virtual void GetParameterInfo(int i, DoubleArray* pMinVals, 
		DoubleArray* pMaxVals, DoubleArray* pSteps) const
	{
		// For any i, set all the arrays to a sigle param equal to zero
		InitArrays(1, pMinVals, pMaxVals, pSteps, 0, 0, 0);
	}

	//! By defautl, it assumes that the component has no dependencies
	virtual StrArray Dependencies() const
	{
		return StrArray(); // empty array
	}

	/*!
		Generic name of the component. That is, ClassName() is
		the name of a specialized component, while GenericName() is
		the "category" name of the component.

		By default, ClassName() and GenericName() are the same.

		Pure abstract classes that derive from VisSysComponent
		must define only their generic name, while the concrete classes must
		define only their class name. That is, in general, the generic name of a
		component is the class name of the closes ancestor that is an abstract class.
	*/
	virtual std::string GenericName() const
	{
		return ClassName();
	}

	/*!
		Non-virtual function that determines the Name of a component based on
		whether the component is a subordinate or another or not. That is,
		the name of a component is its ClassName() if it has no superior component, 
		or SuperiorClassName::ClassName() if it does.
	*/
	std::string Name() const
	{
		return (m_superiorComponentClassName.empty()) ? ClassName() :
			m_superiorComponentClassName + "::" + ClassName();
	}

	/*!
		This function is called whenever there is a GUI event that the
		"active" component might want to know about. 

		This is the const version of the function, so that it can 
		be called from const components. The actual computation should 
		be done by the non-const version of OnGUIEvent().

		@return false if the event was not dealt with and true otherwise.
	*/
	bool OnGUIEvent(const UserEventInfo& uei) const
	{
		return const_cast<VisSysComponent*>(this)->OnGUIEvent(uei);
	}

	/*!
		This function is called whenever there is a GUI event that the
		"active" component might want to know about. 

		@return false if the event was not dealt with and true otherwise.
	*/
	virtual bool OnGUIEvent(const UserEventInfo& uei)
	{
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// Pure Abstract Functions in addition to Run() and GetDisplayInfo(...)
	
	virtual int NumOutputImages() const = 0;
	virtual std::string GetOutputImageLabel(int i) const  = 0;
	virtual std::string ClassName() const = 0;

	///////////////////////////////////////////////////////////////////////////
	// Thread safe wrapper functions

	/*!
		Surrounds the call to Run() with calls to LockDrawingMutex()
		and UnlockDrawingMutex().
	*/
	void RunWithMutex(RUNNING_MODE mode)
	{
		if (mode & m_validRunningModes)
		{
			// Run the subordinate components first
			for (size_t i = 0; i < m_subordinateComponents.size(); i++)
				m_subordinateComponents[i]->RunWithMutex(mode);

			LockDrawingMutex();
			Run();
			UnlockDrawingMutex();
		}
	}

	/*!
		Surrounds the call to Draw(dii) with calls to LockDrawingMutex()
		and UnlockDrawingMutex().
	*/
	void DrawWithMutex(const DisplayInfoIn& dii) const
	{
		LockDrawingMutex();
		Draw(dii);
		UnlockDrawingMutex();
	}

	/*!
		Surrounds the call to Draw(dii) with calls to LockDrawingMutex()
		and UnlockDrawingMutex().
	*/
	void GetDisplayInfoWithMutex(const DisplayInfoIn& dii, DisplayInfoOut& dio) const
	{
		LockDrawingMutex();
		GetDisplayInfo(dii, dio);
		UnlockDrawingMutex();
	}
};

} // namespace vpl

