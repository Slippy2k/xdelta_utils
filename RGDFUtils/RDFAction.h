#pragma once
#include "stdAfxZlib.h"

class RDFAction
{
public:
	RDFAction();
	~RDFAction();
	enum EAction
	{
		NEW = 0
		, REMOVE
		, MOVE
		, APPLY_DIFF
		, NONE
		, NEW_BUT_NOT_INCLUDED
		, OVERRIDE
	};

	static tstring ActionToString(EAction eAction);
	static bool StringToAction(const TCHAR* parse, EAction& eAction);
};

