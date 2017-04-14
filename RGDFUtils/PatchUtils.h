#pragma once
#include "stdAfxZlib.h"
#include "./base/sha1.h"



#ifdef _UNICODE
#	define SHA1_TO_TSTRING SHA1ToWString
#else
#	define SHA1_TO_TSTRING SHA1ToString
#endif

namespace PatchUtils
{
	static const TCHAR REGISTRY_FILENAME[] = _T("FileUpdateInfo.txt");
	static const TCHAR ROLLBACK_EXT[] = _T(".rollback");
	static const char cszGamePackPasswd[] = "risinggames_firstgame";

	static const int		PackCompressLevel = 9;
	const tstring	PackedEnxtention = _T("RDF");


	tstring GenericString(fs::path const& p);

	tstring FileString(fs::path const& p);

	bool Compare(const char* t1, const char* t2);

	bool FileSize(const char* path, size_t& size);

	bool FileSize(const wchar_t* path, size_t& size);

	tstring FullPath(tstring pathToOld, tstring const& relativePath);

	bool ParentPath(tstring const& path, tstring& parentPath);

	int CreateDirectory(const char* dirname);
	int CreateDirectory(const wchar_t* dirname);
	bool CreateDirectories(const TCHAR *newdir);

	size_t StrLen(const wchar_t* str);
	size_t StrLen(const char* str);

	bool RemoveFile(const char* path);
	bool RemoveFile(const wchar_t* path);

	std::wstring StringToWstring(const std::string str);
	std::string  WstringToString(const std::wstring str);

	bool filecopy(const tstring strSource, const tstring strTarget);
	bool block_filecopy(const tstring strSource, const tstring strTarget);

	bool movefile(const char* szOrgpath, const char* szTargetpath);
	bool movefile(const wchar_t* szOrgpath, const wchar_t* szTargetpath);

	bool ZipExtractAll(const tstring strZipFilePath, const tstring strExtractFolder, bool bAutoDelete = false);

	bool ZipActionDo(const tstring strZipFilePath, const tstring strSourceFile);

	bool TextMatch(vecRegex TextRegex, tstring  const& strSearch);
};

