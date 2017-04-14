#pragma once

#include "stdAfxZlib.h"
#include "FileInfoReader.h"
#include "FileMemory.h"

#include ".\Delegate\delegate.h"

typedef	 void(__stdcall *FuncCrcProcess)(const char * szProcessFileName, const int nIdx, int nMaxCount, bool bResult);
//typedef fd::delegate4 < void, const char *, const int, int, bool > FuncCrcProcess;

class PatchHashChecker
{
public:
	PatchHashChecker();
	~PatchHashChecker();

	int Build(const std::wstring strUpdateInfoTxt, const std::wstring strCrcTargetFolder, FuncCrcProcess  HashProcessFunction = NULL);
	int Build(const std::string strUpdateInfoTxt, const std::string strCrcTargetFolder, FuncCrcProcess  HashProcessFunction = NULL);

private:
	int HashActionDo();
	bool LoadInfoFile();
	bool HashCompare(const FileMemory& oldFile, const SUpdateAction * action);
private:
	tstring				m_UpdateInfoTxt;
	tstring				m_CrcTargetFolder;
	FuncCrcProcess		m_HashProcessFunc;
	SFileInfo			m_UpdateInfo;
};

