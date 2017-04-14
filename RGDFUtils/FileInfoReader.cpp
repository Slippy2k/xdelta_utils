#include "FileInfoReader.h"
#include "PatchUtils.h"


FileInfoReader::FileInfoReader()
{
}


FileInfoReader::~FileInfoReader()
{
}

void FileInfoReader::Open(const char* text, size_t size)
{
	m_tokenizer.SetDefaultCharsets();
	m_tokenizer.SetCharacterRule('=', PARSING_RULE_SYMBOL);
	m_tokenizer.SetCharacterRule('{', PARSING_RULE_SYMBOL);
	m_tokenizer.SetCharacterRule('}', PARSING_RULE_SYMBOL);

	m_tokenizer.Open(text, size);
}

void FileInfoReader::Close()
{
	m_tokenizer.Close();
}

bool FileInfoReader::SkipEndLines()
{
	do
	{
		if (!m_tokenizer.Next())
			return false;
	} while (m_tokenizer.IsSymbol() && m_tokenizer.GetSymbol() == '\n');

	return true;
}

bool FileInfoReader::Build(SFileInfo & rUpdateInfo)
{
	if (!SkipEndLines())
		return false;

	bool process = true;
	do
	{
		if (m_tokenizer.IsWord())
		{
			const char* key = m_tokenizer.GetWord();
			if (PatchUtils::Compare("new_version", key))
			{
				if (!ParseStringValue(rUpdateInfo.newVersion))
				{
					m_errorMsg << "for new_version";
					return false;
				}
			}
			else if (PatchUtils::Compare("old_version", key))
			{
				if (!ParseStringValue(rUpdateInfo.oldVersion))
				{
					m_errorMsg << "for old_version";
					return false;
				}
			}
			else if (PatchUtils::Compare("file", key))
			{
				if (!SkipEndLines() || !m_tokenizer.IsSymbol() || m_tokenizer.GetSymbol() != '{')
				{
					m_errorMsg << "'{' expected after 'file'";
					return false;
				}

				SUpdateAction* pAction = new SUpdateAction;
				rUpdateInfo.actions.push_back(pAction);

				if (!ParseFile(rUpdateInfo, *pAction))
					return false;
			}
		}

		if (!m_tokenizer.Next())
			process = false;
	} while (process);

	return true;
}

bool FileInfoReader::ParseWordValue(tstring& value)
{
	if (!m_tokenizer.Next() || !m_tokenizer.IsSymbol() || m_tokenizer.GetSymbol() != '=')
	{
		m_errorMsg << "=[Word identifier] expected ";
		return false;
	}

	if (!m_tokenizer.Next() || !m_tokenizer.IsWord())
	{
		m_errorMsg << "Word identifier expected ";
		return false;
	}

#ifdef _UNICODE
	value = utf_convert::as_wide(m_tokenizer.GetWord());
#else
	value = m_tokenizer.GetWord();
#endif
	return true;
}

bool FileInfoReader::ParseStringValue(tstring & value)
{
	if (!m_tokenizer.Next() || !m_tokenizer.IsSymbol() || m_tokenizer.GetSymbol() != '=')
	{
		m_errorMsg << "=[Quoted string] expected ";
		return false;
	}

	if (!m_tokenizer.Next() || !m_tokenizer.IsString())
	{
		m_errorMsg << "Quoted string expected ";
		return false;
	}

#ifdef _UNICODE
	value = utf_convert::as_wide(m_tokenizer.GetString());
#else
	value = m_tokenizer.GetString();
#endif
	return true;
}

bool FileInfoReader::ParseNumValue(tstring & value)
{
	if (!m_tokenizer.Next() || !m_tokenizer.IsSymbol() || m_tokenizer.GetSymbol() != '=')
	{
		m_errorMsg << "=[Number] expected ";
		return false;
	}

	if (!m_tokenizer.Next() || !m_tokenizer.IsNumber())
	{
		m_errorMsg << "Number expected ";
		return false;
	}

#ifdef _UNICODE
	value = utf_convert::as_wide(m_tokenizer.GetNumber());
#else
	value = m_tokenizer.GetNumber();
#endif
	return true;
}

bool FileInfoReader::ParseFile(SFileInfo & rUpdateInfo, SUpdateAction & action)
{
	if (!SkipEndLines())
	{
		m_errorMsg << "Parameters expected after 'file {'";
		return false;
	}

	bool process = true;
	do
	{
		if (m_tokenizer.IsWord())
		{
			const char* key = m_tokenizer.GetWord();
			if (PatchUtils::Compare("action", key))
			{
				tstring value;
				if (!ParseWordValue(value))
				{
					m_errorMsg << "for action";
					return false;
				}

				if (!RDFAction::StringToAction(value.c_str(), action.action))
				{
					m_errorMsg << _T("Unknown action ") << value;
					return false;
				}
			}
			else if (PatchUtils::Compare("diff_path", key))
			{
				if (!ParseStringValue(action.diff_path))
				{
					m_errorMsg << _T("for diff_path");
					return false;
				}
			}
			else if (PatchUtils::Compare("old_path", key))
			{
				if (!ParseStringValue(action.old_path))
				{
					m_errorMsg << _T("for old_path");
					return false;
				}
			}
			else if (PatchUtils::Compare("new_path", key))
			{
				if (!ParseStringValue(action.new_path))
				{
					m_errorMsg << "for new_path";
					return false;
				}
			}
			else if (PatchUtils::Compare("diff_method", key))
			{
				if (!ParseWordValue(action.diff_method))
				{
					m_errorMsg << "for diff_method";
					return false;
				}
			}
			else if (PatchUtils::Compare("old_sha1", key))
			{
				tstring value;
				if (!ParseStringValue(value))
				{
					m_errorMsg << "for old_sha1";
					return false;
				}

				if (!StringToSHA1(value.c_str(), action.oldSha1))
				{
					m_errorMsg << "Can't convert " << value << " to old_sha1";
					return false;
				}

				assert(SHA1_TO_TSTRING(action.oldSha1) == value);
			}
			else if (PatchUtils::Compare("new_sha1", key))
			{
				tstring value;
				if (!ParseStringValue(value))
				{
					m_errorMsg << "for new_sha1";
					return false;
				}

				if (!StringToSHA1(value.c_str(), action.newSha1))
				{
					m_errorMsg << "Can't convert " << value << " to new_sha1";
					return false;
				}

				assert(SHA1_TO_TSTRING(action.newSha1) == value);
			}
			else if (PatchUtils::Compare("old_size", key))
			{
				tstring value;
				if (!ParseNumValue(value))
				{
					m_errorMsg << "for old_size";
					return false;
				}
				action.oldSize = tchar_to_long(value.c_str());
			}
			else if (PatchUtils::Compare("new_size", key))
			{
				tstring value;
				if (!ParseNumValue(value))
				{
					m_errorMsg << "for new_size";
					return false;
				}
				action.newSize = tchar_to_long(value.c_str());
			}
		}
		else if (m_tokenizer.IsSymbol() && m_tokenizer.GetSymbol() == '}')
			return true;

		if (!m_tokenizer.Next())
		{
			m_errorMsg << "'}' expected for 'file'";
			process = false;
		}
	} while (process);

	return false;
}

tstring  FileInfoReader::ErrorString() const
{
	return m_errorMsg.str();
}