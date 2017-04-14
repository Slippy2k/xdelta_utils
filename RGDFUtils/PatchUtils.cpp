#include "PatchUtils.h"
#include <direct.h>
#include <codecvt>
#include "FileMemory.h"
#include "miniunzip.h"
#include "ZipArchiveOutput.h"

namespace PatchUtils
{

	tstring GenericString(fs::path const& p)
	{
#ifdef _UNICODE
		return p.generic_wstring();
#else
		return p.generic_string();
#endif
	}
	tstring FileString(fs::path const& p)
	{
#ifdef _UNICODE
		return p.wstring();
#else
		return p.string();
#endif
	}

	bool Compare(const char* t1, const char* t2)
	{
		return strcmp(t1, t2) == 0;
	}

	bool FileSize(const char* path, size_t& size)
	{
		FILE* fp = fopen(path, "rb");
		if (fp == NULL)
			return false;

		fseek(fp, 0L, SEEK_END);
		size = ftell(fp);
		fclose(fp);
		return true;
	}

	bool FileSize(const wchar_t* path, size_t& size)
	{
		FILE* fp = _wfopen(path, L"rb");
		if (fp == NULL)
			return false;

		fseek(fp, 0L, SEEK_END);
		size = ftell(fp);
		fclose(fp);
		return true;
	}

	tstring FullPath(tstring pathToOld, tstring const& relativePath)
	{
		return pathToOld + _T("/") + relativePath;
	}

	bool ParentPath(tstring const& path, tstring& parentPath)
	{
		size_t lastSlash = 0;
		for (size_t i = 0; i < path.size(); ++i)
		{
			if (path[i] == '\\' || path[i] == '/')
				lastSlash = i;
		}

		if (lastSlash == 0)
			return false;

		parentPath = path.substr(0, lastSlash);
		return true;
	}

	int CreateDirectory(const char* dirname)
	{
		int ret = 0;
#ifdef WIN32
		ret = _mkdir(dirname);
#else
		ret = mkdir(dirname, 0775);
#endif
		return ret;
	}

	int CreateDirectory(const wchar_t* dirname)
	{
		int ret = 0;
#ifdef WIN32
		ret = _wmkdir(dirname);
#else
		ret = mkdir(dirname, 0775);
#endif
		return ret;
	}

	bool CreateDirectories(const _TCHAR *newdir)
	{
		size_t len = StrLen(newdir);
		if (len <= 0)
			return false;

		tstring buffer(newdir);

		if (buffer[len - 1] == _T('/') || buffer[len - 1] == _T('\\'))
			buffer.resize(len - 1);

		if (CreateDirectory(buffer.c_str()) == 0)
			return true;

		for (size_t p = 1; p<len; ++p)
		{
			while (p<len && buffer[p] != _T('\\') && buffer[p] != _T('/'))
				p++;

			tstring subDir = buffer.substr(0, p);
			if (CreateDirectory(subDir.c_str()) == -1 && errno == ENOENT)
				return false;
		}

		return true;
	}

	size_t StrLen(const char* str)
	{
		return strlen(str);
	}

	size_t StrLen(const wchar_t* str)
	{
		return wcslen(str);
	}

	bool RemoveFile(const char* path)
	{
		fs::path SourcePath(path);
		boost::filesystem::file_status  fStatus = boost::filesystem::status(SourcePath);

		if (boost::filesystem::directory_file == fStatus.type()) return true;

		if (fStatus.type() == boost::filesystem::status_unknown || fStatus.type() == boost::filesystem::file_not_found) return false;
		return !::remove(path);
	}

	bool RemoveFile(const wchar_t* path)
	{
		fs::path SourcePath(path);
		boost::filesystem::file_status  fStatus = boost::filesystem::status(SourcePath);

		if (boost::filesystem::directory_file == fStatus.type()) return true;

		if (fStatus.type() == boost::filesystem::status_unknown || fStatus.type() == boost::filesystem::file_not_found) return false;

		return !::_wremove(path);
	}

	std::wstring StringToWstring(const std::string str)
	{
		typedef std::codecvt_utf8<wchar_t> convert_typeX;
		std::wstring_convert<convert_typeX, wchar_t> converterX;

		return converterX.from_bytes(str);
	}
	std::string  WstringToString(const std::wstring str)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> con;
		std::string re = con.to_bytes(str);
		return re;
	}

	bool filecopy(const tstring strSource, const tstring strTarget)
	{
		return block_filecopy(strSource, strTarget);

		FileMemory rSouce;
		if (false == rSouce.ReadFile(strSource)) return false;

		FileMemory rTarget(rSouce);
		rTarget.WriteFile(strTarget);

		return true;
	}

	bool block_filecopy(const tstring strSource, const tstring strTarget)
	{
		fs::path SourcePath(strSource);
		boost::filesystem::file_status  fStatus =  boost::filesystem::status(SourcePath);

		if (boost::filesystem::directory_file ==  fStatus.type()  ) return false;

		if (fStatus.type() == boost::filesystem::status_unknown
			|| fStatus.type() == boost::filesystem::file_not_found
			|| fStatus.type() == boost::filesystem::directory_file
			) return false;
		const int cReadFileSize = (BUFSIZ * 2) * 1024 * 100;
		char * buf = new char[cReadFileSize];
		size_t size;

		FILE* fpSource = _wfopen(strSource.c_str(), L"rb");
		FILE* fpTarget = _wfopen(strTarget.c_str(), L"wb");

		while (size = fread(buf, 1, cReadFileSize, fpSource)) {
			fwrite(buf, 1, size, fpTarget);
		}
		fclose(fpSource);
		fclose(fpTarget);
		
		delete[] buf;

		return true;
	}


	bool movefile(const char* szOrgpath, const char* szTargetpath)
	{
		std::wstring wOrg = StringToWstring(szOrgpath);
		std::wstring wTarget = StringToWstring(szTargetpath);
		return movefile(wOrg.c_str(), wTarget.c_str());
	}

	bool movefile(const wchar_t* szOrgpath, const wchar_t* szTargetpath)
	{
		bool bRet = block_filecopy(szOrgpath, szTargetpath);
		if (bRet) bRet = RemoveFile(szOrgpath);
		
		return bRet;
	}

	bool ZipExtractAll(const tstring strZipFilePath, const tstring strExtractFolder,bool bAutoDelete)
	{
		zip::ZipArchiveInput zip;

		if (!zip.Open(strZipFilePath, false))
		{
			return false;
		}
		if (!zip.ExtractAll(strExtractFolder, [&strExtractFolder](const tstring strFileName, FileMemory & rMemory)->bool{
			tstring strfullPath = strExtractFolder +_T("\\") + strFileName;

			tstring dirPath;
			if (!PatchUtils::ParentPath(strfullPath, dirPath))
			{
				return false;
			}

			if (!PatchUtils::CreateDirectories(dirPath.c_str()))
			{
				return false;
			}
			return rMemory.WriteFile(strfullPath);
		}) )
		{
			zip.Close();
			return false;
		}


		//if (false == WriteFile.WriteFile(strFilePath))
		//{
		//	return false;
		//}

		zip.Close();

		if (bAutoDelete ) RemoveFile(strZipFilePath.c_str());

		return true;
	}

	bool ZipActionDo(const tstring strZipFilePath, const tstring strSourceFile)
	{
		zip::ZipArchiveOutput zip;

		tstring dirPath;
		if (!PatchUtils::ParentPath(strZipFilePath, dirPath))
		{
			return false;
		}

		if (!PatchUtils::CreateDirectories(dirPath.c_str()))
		{
			return false;
		}

		if (!zip.Open(strZipFilePath, true, PackCompressLevel))
		{
			return false;
		}

		FileMemory WriteFile;

		if (!WriteFile.ReadFile(strSourceFile))
		{
			return false;
		}



		boost::filesystem::path p(strSourceFile);

		if (!zip.WriteFile(GenericString(p.filename()), WriteFile.Get(), WriteFile.GetSize()))
		{
			return false;
		}
		return zip.Close();
	}

	bool TextMatch(vecRegex TextRegex, tstring  const& strSearch)
	{
		for each (tregex aFilter in TextRegex)
		{
			if (std::regex_match(strSearch, aFilter))
			{
				return true;
			}
		}
		return false;
	}

};

