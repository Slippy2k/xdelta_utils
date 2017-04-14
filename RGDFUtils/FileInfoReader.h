#pragma once

#include "stdAfxZlib.h"
#include "RDFAction.h"
#include "./base/sha1.h"
#include "TextTokenParser.h"


struct SUpdateAction
{
	SUpdateAction():
		action(RDFAction::EAction::NONE)
		, newSize(-1)
		, oldSize(-1)
	{
	}
	RDFAction::EAction action;
	Sha1 newSha1, oldSha1;
	int newSize, oldSize;

	tstring diff_path;
	tstring new_path;
	tstring old_path;
	tstring diff_method; // xdelta, deltamax etc
};

struct SFileInfo
{
	typedef std::vector< SUpdateAction* > VectAction;
	SFileInfo(){}
	~SFileInfo()
	{
		for (VectAction::iterator i = actions.begin(); i != actions.end(); ++i)
			delete *i;
		actions.clear();
	}
	tstring newVersion;
	tstring oldVersion;

	VectAction actions;

};

class FileInfoReader
{
public:
	FileInfoReader();
	~FileInfoReader();

	void Open(const char * szText, size_t tTextSize);
	void Close();

	bool Build(SFileInfo & rUpdateInfo);
	tstring ErrorString() const;

private:
	bool ParseWordValue(tstring& value);
	bool ParseStringValue(tstring& value);
	bool ParseNumValue(tstring& value);

	bool ParseFile(SFileInfo & rUpdateInfo, SUpdateAction & action);
	bool SkipEndLines();
private:
	TextTokenParser		m_tokenizer;
	tstringstream		m_errorMsg;
};

