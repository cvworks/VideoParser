/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <map>
#include <Tools/STLUtils.h>

namespace vpl {

class VisSysComponent;

class VisSysComponentCreator
{
public:
	typedef std::map<std::string, StrArray> CompLblMap;

	typedef CompLblMap::const_iterator const_iterator;

protected:
	CompLblMap m_compLblMap;
	StrArray m_taskLbls;

	static char* s_defaultNoneArgument;

	virtual void InitVisSysComponentLabels();
	virtual void InitVisSysTaskLabels();

public:

	VisSysComponentCreator()
	{
		InitVisSysTaskLabels();
		InitVisSysComponentLabels();
	}

	const_iterator begin() const
	{ 
		return m_compLblMap.begin();
	}

	const_iterator end() const
	{
		return m_compLblMap.end();
	}

	StrArray ValidTaskLabels() const
	{
		return m_taskLbls;
	}

	virtual std::string ReadUserSelection(const_iterator compLblIt, 
		const std::string& task) const;
	
	virtual VisSysComponent* NewVisSysComponent(
		const std::string& name) const;
};

} // namespace vpl
