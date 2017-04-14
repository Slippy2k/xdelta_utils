#include "stdafx.h"
#include "XDeltaPatch.h"




XDeltaPatch::XDeltaPatch()
{
	m_DllInst = LoadLibrary(TEXT("DeltaPatch.dll"));
	if (m_DllInst)
	{
		m_ApplyFunction = (ApplyDeltaPatchType)GetProcAddress(m_DllInst, "ApplyDeltaPatch");

		m_WStringToString = (WStrToStr)GetProcAddress(m_DllInst, "WstringToString");
		m_StringToWString = (StrToWStr)GetProcAddress(m_DllInst, "StringToWstring");
		m_CrcCheckFunc = (PatchHashCheckFuncType)GetProcAddress(m_DllInst, "FullHashCechk");

		m_Extract = (ZipFileExtract)GetProcAddress(m_DllInst, "ExtracZipFile");

		m_stlStringToWString = (WSTR_TO_STR)GetProcAddress(m_DllInst, "StlStrToStlWStr");
	}
}

XDeltaPatch::~XDeltaPatch()
{
	if (m_DllInst)
	{
		FreeLibrary(m_DllInst);
	}
}

bool XDeltaPatch::ApplyPatch(const wchar_t * szDeltaFilePath, const wchar_t * szTargetPath, FuncPatchProcess funcPatchApply)
{
	if (NULL != funcPatchApply) m_PatchFunc = funcPatchApply;
	return m_ApplyFunction(m_ApplyConfig, szDeltaFilePath, szTargetPath, m_PatchFunc);
}

bool XDeltaPatch::ApplyPatch(const char * szDeltaFilePath, const char * szTargetPath, FuncPatchProcess funcPatchApply)
{
	if (NULL != funcPatchApply) m_PatchFunc = funcPatchApply;
	std::string strPackFile = szDeltaFilePath;
	std::string strOrgPath = szTargetPath;
	return ApplyPatch(strPackFile, strOrgPath);
}

bool XDeltaPatch::ApplyPatch(const std::wstring & strDeltaFilePath, const std::wstring & strTargetPath, FuncPatchProcess funcPatchApply)
{
	if (NULL != funcPatchApply) m_PatchFunc = funcPatchApply;
	return m_ApplyFunction(m_ApplyConfig, strDeltaFilePath.c_str(), strTargetPath.c_str(), m_PatchFunc);;
}

bool XDeltaPatch::ApplyPatch(const std::string &  strDeltaFilePath, const std::string  & strTargetPath, FuncPatchProcess funcPatchApply)
{
	if (NULL != funcPatchApply) m_PatchFunc = funcPatchApply;
	std::wstring wstrPackFile = m_StringToWString(strDeltaFilePath.c_str());
	std::wstring wstrOrgPath = m_StringToWString(strTargetPath.c_str());
	return ApplyPatch(wstrPackFile, wstrOrgPath);
}


void XDeltaPatch::SetApplyCofnig(const DeltaPatchConfig sApplyConfig)
{
	m_ApplyConfig = sApplyConfig;
}

void XDeltaPatch::SetApplyFunction(FuncPatchProcess funcPatchApply)
{
	m_PatchFunc = funcPatchApply;
}

int XDeltaPatch::CrcCheckStart(const std::wstring & strUpdateInfoPath, const std::wstring & strTargetPath, FuncCrcProcess funcPatchApply)
{
	return m_CrcCheckFunc(strUpdateInfoPath.c_str(), strTargetPath.c_str(), funcPatchApply);
}

int XDeltaPatch::CrcCheckStart(const std::string & strUpdateInfoPath, const std::string &  strTargetPath, FuncCrcProcess funcPatchApply)
{
	std::wstring wstrUpdateInfoPath = m_StringToWString(strUpdateInfoPath.c_str());
	std::wstring wstrTargetPath = m_StringToWString(strTargetPath.c_str());
	return CrcCheckStart(wstrUpdateInfoPath, wstrTargetPath, funcPatchApply);
}

bool XDeltaPatch::ExtractZipFile(const std::string & strZipFilePath, const std::string & strTargetPath, bool bZipFileDelete)
{
	std::wstring wstrZipFilePath = m_StringToWString(strZipFilePath.c_str());
	std::wstring wstrTargetPath = m_StringToWString(strTargetPath.c_str());

	return ExtractZipFile(wstrZipFilePath, wstrTargetPath, bZipFileDelete);
}

bool XDeltaPatch::ExtractZipFile(const std::wstring & strZipFilePath, const std::wstring & strTargetPath, bool bZipFileDelete)
{
	return m_Extract(strZipFilePath.c_str(), strTargetPath.c_str(), bZipFileDelete);
}

std::wstring XDeltaPatch::StringToWstring(const std::string & str)
{
	wchar_t out[256] = {0x00,};
	m_stlStringToWString(str.c_str(), out);
	return std::wstring(out);
}

std::string  XDeltaPatch::WstringToString(const std::wstring & str)
{
	std::string strRet = m_WStringToString(str.c_str());
	return strRet;
}

