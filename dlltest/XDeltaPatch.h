#pragma once

//typedef  void(*FuncPatchProcess)(const char * szProcessFileName, int nIdx, int nMaxCount);
//typedef fd::delegate3 < void, const char *, const int, int > FuncPatchProcess;

typedef bool(*ApplyDeltaPatchType)(const DeltaPatchConfig sCofifg, const wchar_t * szDeltaFilePath, const wchar_t * szTargetPath, FuncPatchProcess funcProcess);
//typedef fd::delegate4 < bool, const DeltaPatchConfig,const wchar_t *, const wchar_t *, FuncPatchProcess > ApplyDeltaPatchType;

typedef int(*PatchHashCheckFuncType)(const wchar_t * strFilePath,const wchar_t * strTargetPath, FuncCrcProcess funcProcess);

typedef char *(*WStrToStr)(const wchar_t * str);
typedef const wchar_t *(*StrToWStr)(const char * str);

typedef void(*WSTR_TO_STR)(const char *  szInput, wchar_t * szOutPut);


typedef bool(*ZipFileExtract)(const wchar_t *, const wchar_t *, const bool);
class XDeltaPatch
{
public:
	XDeltaPatch();
	~XDeltaPatch();

	std::wstring StringToWstring(const std::string & str);
	std::string  WstringToString(const std::wstring &  str);

	bool ApplyPatch(const wchar_t * szDeltaFilePath, const wchar_t * szTargetPath, FuncPatchProcess funcPatchApply = NULL);
	bool ApplyPatch(const char * szDeltaFilePath, const char * szTargetPath, FuncPatchProcess funcPatchApply = NULL);
	bool ApplyPatch(const std::wstring& strDeltaFilePath, const std::wstring & strTargetPath, FuncPatchProcess funcPatchApply = NULL );
	bool ApplyPatch(const std::string  &strDeltaFilePath, const std::string &  strTargetPath, FuncPatchProcess funcPatchApply = NULL);

	void SetApplyCofnig(const DeltaPatchConfig sApplyConfig);
	void SetApplyFunction(FuncPatchProcess funcPatchApply);

	int CrcCheckStart(const std::wstring & strUpdateInfoPath, const std::wstring & strTargetPath, FuncCrcProcess funcPatchApply);
	int CrcCheckStart(const std::string & strUpdateInfoPath, const std::string  & strTargetPath, FuncCrcProcess funcPatchApply);

	bool ExtractZipFile(const std::string & strZipFilePath, const std::string & strTargetPath,bool bZipFileDelete = false);
	bool ExtractZipFile(const std::wstring & strZipFilePath, const std::wstring & strTargetPath, bool bZipFileDelete = false);

private:
	HINSTANCE		 m_DllInst;
	DeltaPatchConfig m_ApplyConfig;
	FuncPatchProcess m_PatchFunc;
	ApplyDeltaPatchType m_ApplyFunction;
	WStrToStr			m_WStringToString;
	StrToWStr			m_StringToWString;
	PatchHashCheckFuncType	m_CrcCheckFunc;
	ZipFileExtract			m_Extract;
	WSTR_TO_STR				m_stlStringToWString;
};

