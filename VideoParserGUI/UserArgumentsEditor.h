/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _USER_ARGUMENTS_EDITOR_
#define _USER_ARGUMENTS_EDITOR_

#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_ask.H>
#include <Tools/UserArguments.h>

class UserArgumentsEditor : public Fl_Text_Editor
{
	const vpl::UserArguments* m_pUserArgs;
	std::string m_filename;
	Fl_Text_Buffer m_buffer;
	std::string m_textCopy;
	bool m_bModified;

public:
	UserArgumentsEditor(int x, int y, int w, int h, const char* l = 0)
		: Fl_Text_Editor(x, y, w, h, l)
	{
		buffer(&m_buffer);
		m_pUserArgs = NULL;

		SetModified(false);
	}

	void SetModified(bool status)
	{
		m_bModified = status;
	}

	bool IsModified() const
	{
		return m_bModified;
	}

	void SetText(const char* szArgTxt)
	{
		m_buffer.text(szArgTxt);
	}

	void SetUserArguments(const vpl::UserArguments* pUserArgs)
	{
		m_pUserArgs = pUserArgs;
	}

	//! Copies the current text in the buffer
	void CopyText()
	{
		m_textCopy = m_buffer.text();
	}

	//! Pastes the previously copied text
	void PasteText()
	{
		SetText(m_textCopy.c_str());
	}

	const std::string& GetFilename() const
	{
		return m_filename;
	}

	bool LoadFile()
	{
		ASSERT(m_pUserArgs);

		std::string fname = m_pUserArgs->GetInputFilename();

		if (!LoadFile(fname.c_str()))
			return false;

		m_filename = fname;

		return true;
	}

	bool LoadFile(const char* filename)
	{
		int errNum = m_buffer.loadfile(filename);

		if (errNum != 0)
		{
			fl_choice("Cannot read file %s because:\n  '%s'", 
				"OK", NULL, NULL, filename, strerror(errNum));

			return false;
		}

		// There is new content, so text isn't modified
		SetModified(false);

		return true;
	}

	bool SaveBufferToFile()
	{
		return SaveBufferToFile(m_filename.c_str());
	}

	bool SaveBufferToFile(const char* filename)
	{
		// Ensure that there is a new line at the end of the text before saving
		if (m_buffer.length() > 0 && 
			m_buffer.character(m_buffer.length() - 1) != '\n')
		{
			m_buffer.append("\n");
		}

		// Save the text in the buffer to a file
		int errNum = m_buffer.outputfile(filename, 0, m_buffer.length());

		if (errNum != 0)
		{
			fl_choice("Cannot save file %s because:\n  '%s'", 
				"OK", NULL, NULL, filename, strerror(errNum));

			return false;
		}

		return true;
	}

	void ShowFieldsAndvalues()
	{
		SetText(m_pUserArgs->GetAllFieldsAndValues().c_str());
	}

	void ShowDefaultValues()
	{
		SetText(m_pUserArgs->GetAllDefaultValues().c_str());
	}

	void ShowValueOptions()
	{
		SetText(m_pUserArgs->GetAllValueOptions().c_str());
	}
};

#endif // _USER_ARGUMENTS_EDITOR_