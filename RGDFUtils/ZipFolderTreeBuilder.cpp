#include "ZipFolderTreeBuilder.h"
#include "PatchUtils.h"


ZipFolderTreeBuilder::ZipFolderTreeBuilder()
{
}


ZipFolderTreeBuilder::~ZipFolderTreeBuilder()
{
}



bool ZipFolderTreeBuilder::BuildRecursive(fs::path path, const tstring & strAddPath)
{
	tstring strAdd = strAddPath;
	for (fs::directory_iterator it(path); it != fs::directory_iterator(); ++it)
	{
		fs::file_status status = it->status();
		fs::path fullpath = it->path();
		tstring name = PatchUtils::GenericString(fullpath.filename());

		if (fs::is_regular_file(status))
		{
			if (PatchUtils::TextMatch(m_vecIgnoreFiles, name)) continue;

			tstring zipFileName = m_strTarget + strAddPath + _T("\\") + name + _T(".zip");
			bool bRet = PatchUtils::ZipActionDo(zipFileName, fullpath.c_str());
			tcout << _T("ZipFolderTreeBuilder ZipFile : ") << zipFileName << std::endl;
			if (false == bRet) return false;

		}
		else if (fs::is_directory(status))
		{
			if (PatchUtils::TextMatch(m_vecIgnoreFolders, name)) continue;
			bool bRet = BuildRecursive(fullpath, strAdd + _T("\\") + PatchUtils::GenericString(fullpath.filename()));
			if (false == bRet) 	return false;

		}
	}

	return true;
}

bool ZipFolderTreeBuilder::Build(const tstring strSouceFolder, const tstring strTargetFolder)
{
	m_strSource = strSouceFolder;
	m_strTarget = strTargetFolder;
	return BuildRecursive(strSouceFolder, _T(""));
}

bool ZipFolderTreeBuilder::Build(std::string strSouceFolder, const std::string strTargetFolder)
{
	std::wstring SouceFolder = PatchUtils::StringToWstring(strSouceFolder);
	std::wstring TargetFolder = PatchUtils::StringToWstring(strTargetFolder);

	return Build(SouceFolder, TargetFolder);
}

void ZipFolderTreeBuilder::AddIgnoreFolder(const tstring strFilter)
{
	tregex tFilter(strFilter);
	m_vecIgnoreFolders.push_back(tFilter);
}

void ZipFolderTreeBuilder::AddIgnoreFile(const tstring strFilter)
{
	tregex tFilter(strFilter);
	m_vecIgnoreFiles.push_back(tFilter);
}

void ZipFolderTreeBuilder::AddIgnoreFolder(const std::vector<std::string> & vecFilters)
{
	for each(auto const strFilter in vecFilters)
	{
		std::wstring wStrFilter = PatchUtils::StringToWstring(strFilter);
		AddIgnoreFolder(wStrFilter);
	}
}

void ZipFolderTreeBuilder::AddIgnoreFile(const std::vector<std::string> & vecFilters)
{
	for each(auto const strFilter in vecFilters)
	{
		std::wstring wStrFilter = PatchUtils::StringToWstring(strFilter);
		AddIgnoreFile(wStrFilter);
	}
}