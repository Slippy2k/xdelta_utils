#include "XDeltaFolder.h"
#include "PatchUtils.h"

XDeltaFolder::XDeltaFolder()
{
	//std::wregex s (_T("\.svn"));
	//m_vecIgnoreFolders.push_back(s);
	//std::wregex f(_T(".*\.xlsx"));
	//m_vecIgnoreFiles.push_back(f);

}


XDeltaFolder::~XDeltaFolder()
{
}


void XDeltaFolder::BuildPath(fs::path const& path, tstringset & files, tstringset & folders, bool newPath)
{
	for (fs::directory_iterator it(path); it != fs::directory_iterator(); ++it)
	{
		fs::file_status status = it->status();
		fs::path fullpath = it->path();
		tstring name = PatchUtils::GenericString(fullpath.filename());

		vecRegex TextRegex;

		if (fs::is_regular_file(status))
		{
			if (!PatchUtils::TextMatch(m_vecIgnoreFiles, name))
			{
				if (!newPath || !PatchUtils::TextMatch(TextRegex, name)) //파일명 필터링 
					files.insert(PatchUtils::FileString(fullpath.filename()));
			}
		}
		else if (fs::is_directory(status))
		{
			//if ((config.includeFolders_regex.empty() || MatchName(config.includeFolders_regex, name))
			//	&& !MatchName(config.ignoreFolders_regex, name))
			if (!PatchUtils::TextMatch(m_vecIgnoreFolders, name))
			{
				if (!newPath || !PatchUtils::TextMatch(TextRegex, name))//폴더 필터링
					folders.insert(PatchUtils::FileString(fullpath.filename()));
			}
		}
	}
}

void XDeltaFolder::BuildTreeSet(tstringset& newSet, tstringset& oldSet, tstringvector& existInBoth, tstringvector& newOnly, tstringvector& oldOnly)
{
	existInBoth.clear();
	newOnly.clear();
	oldOnly.clear();

	for (tstringset::iterator i = newSet.begin(); i != newSet.end(); ++i)
	{
		const tstring & name = *i;

		bool overrideFile = false;//MatchName(config.newOverrideFiles_regex, name);

		tstringset::iterator f = oldSet.find(name);
		if (f != oldSet.end())
		{
			if (overrideFile)
				newOnly.push_back(name);
			else
				existInBoth.push_back(name);
			oldSet.erase(f);
		}
		else
		{
			newOnly.push_back(name);
		}
	}

	oldOnly.assign(oldSet.begin(), oldSet.end());

	oldSet.clear();
	newSet.clear();
}


void XDeltaFolder::CreateFiles(tstringvector const& names, XDeltaFiles & fileInfos)
{
	for each(auto rName in names)
	{
		XdeltaFile* pFileInfo = new  XdeltaFile();
		pFileInfo->name = rName;
		fileInfos.push_back(pFileInfo);
	}
}

void XDeltaFolder::CreateFolders(tstringvector const& names, XDeltaFolders & folderInfos)
{
	for each (auto rName in names)
	{
		XDeltaFolder * pFolderInfo = new XDeltaFolder();
		pFolderInfo->SetName(rName);
		folderInfos.push_back(pFolderInfo);
	}
}

void XDeltaFolder::BuildRecursive(fs::path const& pathToNew, fs::path const& pathToOld, XDeltaFolders const& folderInfos)
{
	for each ( auto rFolder  in folderInfos)
	{
		XDeltaFolder & folderInfo = *rFolder;
		BuildSub(pathToNew / folderInfo.GetName(), pathToOld / folderInfo.GetName(), &folderInfo);
	}
}

void XDeltaFolder::BuildSub(const fs::path strNewPath, const fs::path strOldPath, XDeltaFolder * pFolder)
{
	tstringset newFiles, OldFiles, NewFolders, OldFolders;

	if (fs::exists(strOldPath))
	{
		BuildPath(strOldPath, OldFiles, OldFolders, false);
	}

	if (fs::exists(strNewPath))
	{
		BuildPath(strNewPath, newFiles, NewFolders, true);
	}

	//파일
	{
		tstringvector existInBoth, newOnly, oldOnly;
		BuildTreeSet(newFiles, OldFiles, existInBoth, newOnly, oldOnly);

		CreateFiles(existInBoth, pFolder->GetBothFiles());
		CreateFiles(newOnly, pFolder->GetNewFiles());
		CreateFiles(oldOnly, pFolder->GetOldFiles());
	}
	//폴더
	{
		tstringvector existInBoth, newOnly, oldOnly;
		BuildTreeSet(NewFolders, OldFolders, existInBoth, newOnly, oldOnly);

		CreateFolders(existInBoth, pFolder->GetBothFolder());
		CreateFolders(newOnly, pFolder->GetNewFolder());
		CreateFolders(oldOnly, pFolder->GetOldFolder());
	}

	BuildRecursive(strNewPath, strOldPath, pFolder->GetBothFolder());
	BuildRecursive(strNewPath, strOldPath, pFolder->GetNewFolder());
	BuildRecursive(strNewPath, strOldPath, pFolder->GetOldFolder());

}

void XDeltaFolder::BuildRoot(const std::string strNewPath, const std::string strOldPath)
{
	BuildSub(strNewPath, strOldPath, this);
}

void XDeltaFolder::BuildRoot(const tstring strNewPath, const tstring strOldPath )
{
	BuildSub(strNewPath, strOldPath,this);
}

void XDeltaFolder::AddIgnoreFolder(const tstring strFilter)
{
	tregex tFilter(strFilter);
	m_vecIgnoreFolders.push_back(tFilter);
}

void XDeltaFolder::AddIgnoreFile(const tstring strFilter)
{
	tregex tFilter(strFilter);
	m_vecIgnoreFiles.push_back(tFilter);
}

void XDeltaFolder::AddIgnoreFolder(const std::vector<std::string> & vecFilters)
{
	for each(auto const strFilter in vecFilters)
	{
		std::wstring wStrFilter = PatchUtils::StringToWstring(strFilter);
		AddIgnoreFolder(wStrFilter);
	}
}

void XDeltaFolder::AddIgnoreFile(const std::vector<std::string> & vecFilters)
{
	for each(auto const strFilter in vecFilters)
	{
		std::wstring wStrFilter = PatchUtils::StringToWstring(strFilter);
		AddIgnoreFile(wStrFilter);
	}
}

