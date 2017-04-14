#pragma once

#include ".\Delegate\delegate.h"
#ifdef __cplusplus
extern "C" {
#endif
	typedef  void(__stdcall *FuncPatchProcess)(const char * szProcessFileName, int nIdx, int nMaxCount);
	typedef	 void(__stdcall *FuncCrcProcess)(const char * szProcessFileName, const int nIdx, int nMaxCount, bool bResult);
	//typedef fd::delegate3 < void, const char *, const int, int > FuncPatchProcess;
	//typedef fd::delegate4 < void, const char *, const int, int ,bool > FuncCrcProcess;
	struct DeltaPatchConfig
	{
		DeltaPatchConfig()
		{
			m_bConsolOut = false;
			m_bChkFileHash = true;
			m_bChkFileSize = true;
			m_bErrorStop = true;
			m_bRollback = false;
			m_bAutoDelete = false;
		}
		bool	m_bConsolOut;	//콘솔 출력
		bool	m_bChkFileHash; //올드파일 파일 해쉬 체크 
		bool	m_bChkFileSize; //올드파일 싸이즈 체크 
		bool	m_bErrorStop;	//에러서 정지 할것인지
		bool	m_bRollback;	//롤백을 수행 할것인지
		bool	m_bAutoDelete;  //패치 완료후 다운 받은 패치 파일 삭제 할것인지
	};

	//4개 함수는 다 같다 패치 적용 하는 함수 
	//__declspec(dllexport) bool __stdcall ApplyDeltaPatch(const DeltaPatchConfig sCofifg, const wchar_t * szDeltaFilePath, const wchar_t * szTargetPath, FuncPatchProcess funcProcess);
	//bool ApplyDeltaPatchC(const DeltaPatchConfig sCofifg, const char * szDeltaFilePath, const char * szTargetPath, FuncPatchProcess funcProcess);

	__declspec(dllexport) bool __stdcall ApplyDeltaPatch(const DeltaPatchConfig* sCofifg, const wchar_t * szDeltaFilePath, const wchar_t * szTargetPath, FuncPatchProcess funcProcess);


	__declspec(dllexport) bool __stdcall ApplyDeltaPatchWString(const DeltaPatchConfig sCofifg, const std::wstring strDeltaFilePath, const std::wstring  strTargetPath, FuncPatchProcess funcProcess);
	/*bool ApplyDeltaPatchStr(const DeltaPatchConfig sCofifg, const std::string  strDeltaFilePath, const std::string  strTargetPath, FuncPatchProcess funcProcess);*/

	//__declspec(dllexport) const char *  WstringToString(const wchar_t * wszStr);
	//__declspec(dllexport) const wchar_t * StringToWstring(const char * szStr);

	__declspec(dllexport)  void __stdcall  StlStrToStlWStr(const char *  szInput, wchar_t * szOutPut);

	__declspec(dllexport) int __stdcall FullHashCechk(const wchar_t * strFilePath, const wchar_t * strTargetPath, FuncCrcProcess funcProcess);

	__declspec(dllexport) bool __stdcall ExtracZipFile(const wchar_t * strzipFilePath, const wchar_t * strTargetPath, const bool bZipdelete);

#ifdef __cplusplus
};
#endif