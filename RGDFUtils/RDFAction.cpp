#include "RDFAction.h"
#include <assert.h>

struct StringsForAction
{
	RDFAction::EAction action;
	const TCHAR* str;
};

StringsForAction g_stringsForAction[] =
{
	RDFAction::EAction::NEW, _T("new")
	, RDFAction::EAction::REMOVE, _T("delete")
	, RDFAction::EAction::MOVE, _T("move")
	, RDFAction::EAction::APPLY_DIFF, _T("apply_diff")
	, RDFAction::EAction::NONE, _T("none")
	, RDFAction::EAction::NEW_BUT_NOT_INCLUDED, _T("new_but_not_included")
	, RDFAction::EAction::OVERRIDE, _T("override")
};


RDFAction::RDFAction()
{
}


RDFAction::~RDFAction()
{
}

tstring RDFAction::ActionToString(EAction eAction)
{
	assert(g_stringsForAction[eAction].action == eAction);
	return g_stringsForAction[eAction].str;
}

bool RDFAction::StringToAction(const TCHAR* parse, EAction& eAction)
{
	for (int i = 0; i < sizeof(g_stringsForAction) / sizeof(g_stringsForAction[0]); ++i)
	{
		if (tstrcmp(g_stringsForAction[i].str, parse) == 0)
		{
			eAction = g_stringsForAction[i].action;
			return true;
		}
	}
	return false;
}
