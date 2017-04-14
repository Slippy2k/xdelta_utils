#pragma once

#include "stdAfxZlib.h"
//#include "XDeltaDecoder.h"
#include "miniunzip.h"
#include "FileInfoReader.h"
#include ".\Delegate\delegate.h"

typedef  void(__stdcall *PatchCallBack)(const char * szProcessFileName, int nIdx, int nMaxCount);
//typedef  void(*PatchCallBack)(const char * szProcessFileName, int nIdx, int nMaxCount);
//typedef fd::delegate3 < void, const char * ,const int , int > PatchCallBack;

class PatchUnPack
{
public:
	PatchUnPack();
	~PatchUnPack();

	bool ApplyPatch(const std::string strPackFile, const std::string strOrgPath);
	bool ApplyPatch(const std::wstring strPackFile, const std::wstring strOrgPath);
	void SetRollBack(const bool bRollback = true) { m_bRollback = bRollback; }
	void SetApplyCallBackFunc(PatchCallBack funcProcess){ m_funcProcess = funcProcess; }
	void SetConsolOut(const bool bShow){ m_bConsolOut = bShow; }
	void SetAutoDelete(const bool bAutoDelete){ m_bAutoDelete = bAutoDelete; }
	void SetCheckHash(const bool bCheckHash){ m_bChkFileHash = bCheckHash; }
	void SetCheckFileSize(const bool bCheckFileSize){ m_bChkFileSize = bCheckFileSize; }
	void SetStopOnError(const bool bErrorStop){ m_bErrorStop = bErrorStop; }
private:
	bool ApplyPatchActions(SFileInfo  const& rUpdateInfo, SFileInfo::VectAction & rActions);
	void RollBack(const SFileInfo::VectAction & rActions,const int nMaxCount,bool bSuccess);
	bool CreateNewFile(SUpdateAction const& action, bool overrideFile);
	bool DeleteOldFile(SUpdateAction const& action);
	bool CheckOldFileData(FileMemory	const& oldFile, SUpdateAction const& action);
#ifdef XDELTA3
	bool ApplyRDF(SUpdateAction const& action);
	bool ApplyRDFStream(SUpdateAction const& action);
#else
	bool ApplyRDFCmd(SUpdateAction const& action);
#endif
	bool PatchApplyAction(SUpdateAction const& action);
	bool RollBackAction(SUpdateAction const& action);


private:
	//XDeltaDecoder			m_XdeltaDecoder;
	tostream	&			m_Consol;
	zip::ZipArchiveInput	m_PackFile;
	bool					m_bConsolOut;
	bool					m_bChkFileHash;
	bool					m_bChkFileSize;
	bool					m_bErrorStop;
	bool					m_bRollback;
	bool					m_bAutoDelete;
	tstring					m_strOldFolder;
	PatchCallBack			m_funcProcess;
};

