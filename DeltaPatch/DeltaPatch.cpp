// AutoPatch.cpp : DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//

#include "stdafx.h"
#include "DeltaPatch.h"
#include "PatchUnPack.h"
#include "PatchUtils.h"
#include "PatchHashChecker.h"
#include <codecvt>

//
//const wchar_t * StringToWstring(const char * szStr)
//{
//	typedef std::codecvt_utf8<wchar_t> convert_typeX;
//	std::wstring_convert<convert_typeX, wchar_t> converterX;
//	std::wstring re = converterX.from_bytes(szStr);
//	return re.c_str();
//}
//const char *  WstringToString(const wchar_t * wszStr)
//{
//	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> con;
//	std::string re = con.to_bytes(wszStr);
//	return re.c_str();
//}

 void __stdcall StlStrToStlWStr(const char *  szInput, wchar_t * szOutPut)
{
	std::string strIn(szInput);
	typedef std::codecvt_utf8<wchar_t> convert_typeX;
	std::wstring_convert<convert_typeX, wchar_t> converterX;
	std::wstring wstr = converterX.from_bytes(strIn);

	wmemcpy(szOutPut, wstr.c_str(), wstr.size());
}

//bool __stdcall ApplyDeltaPatch(const DeltaPatchConfig sCofifg, const wchar_t * szDeltaFilePath, const wchar_t * szTargetPath, FuncPatchProcess funcProcess)
//{
//
//	std::wstring wstrPackFile = szDeltaFilePath;
//	std::wstring wstrOrgPath = szTargetPath;
//	return ApplyDeltaPatchWString(sCofifg, wstrPackFile, wstrOrgPath, funcProcess);
//}

bool __stdcall ApplyDeltaPatch(const DeltaPatchConfig* sCofifg, const wchar_t * szDeltaFilePath, const wchar_t * szTargetPath, FuncPatchProcess funcProcess)
{

	std::wstring wstrPackFile = szDeltaFilePath;
	std::wstring wstrOrgPath = szTargetPath;
	return ApplyDeltaPatchWString(*sCofifg, wstrPackFile, wstrOrgPath, funcProcess);
}

int __stdcall FullHashCechk(const wchar_t * strFilePath, const wchar_t * strTargetPath, FuncCrcProcess funcProcess)
{
	PatchHashChecker Checker;
	return Checker.Build(strFilePath, strTargetPath, funcProcess);
}

//bool ApplyDeltaPatchC(const DeltaPatchConfig sCofifg, const char * szDeltaFilePath, const char * szTargetPath, FuncPatchProcess funcProcess)
//{
//	std::string strPackFile = szDeltaFilePath;
//	std::string strOrgPath = szTargetPath;
//
//	return ApplyDeltaPatchStr(sCofifg, strPackFile, strOrgPath, funcProcess);
//}

bool __stdcall ApplyDeltaPatchWString(const DeltaPatchConfig sCofifg, const std::wstring strDeltaFilePath, const std::wstring  strTargetPath, FuncPatchProcess funcProcess)
{
	PatchUnPack PatchApply;

	PatchApply.SetConsolOut(sCofifg.m_bConsolOut);
	PatchApply.SetAutoDelete(sCofifg.m_bAutoDelete);
	PatchApply.SetCheckFileSize(sCofifg.m_bChkFileSize);
	PatchApply.SetCheckHash(sCofifg.m_bChkFileHash);
	PatchApply.SetRollBack(sCofifg.m_bRollback);
	PatchApply.SetStopOnError(sCofifg.m_bErrorStop);
	PatchApply.SetApplyCallBackFunc(funcProcess);
	
	return PatchApply.ApplyPatch(strDeltaFilePath, strTargetPath);
}

//bool ApplyDeltaPatchStr(const DeltaPatchConfig sCofifg, const std::string  strDeltaFilePath, const std::string  strTargetPath, FuncPatchProcess funcProcess)
//{
//	std::wstring wstrPackFile = PatchUtils::StringToWstring(strDeltaFilePath);
//	std::wstring wstrOrgPath = PatchUtils::StringToWstring(strTargetPath);
//	
//	return ApplyDeltaPatchWStr(sCofifg, wstrPackFile, wstrOrgPath, funcProcess);
//}

bool __stdcall ExtracZipFile(const wchar_t * strzipFilePath, const wchar_t * strTargetPath, const  bool bZipdelete)
{
	return PatchUtils::ZipExtractAll(strzipFilePath, strTargetPath, bZipdelete);
}

