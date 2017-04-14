#pragma once

#include "stdAfxZlib.h"

class ZipFolderTreeBuilder
{
public:
	ZipFolderTreeBuilder();
	~ZipFolderTreeBuilder();
	bool Build(const tstring strSouceFolder, const tstring strTargetFolder);
	bool Build(std::string strSouceFolder, const std::string strTargetFolder);
	void AddIgnoreFolder(const std::vector<std::string> & vecFilters);
	void AddIgnoreFile(const std::vector<std::string> &  vecFilters);
	void AddIgnoreFolder(const tstring strFilter);
	void AddIgnoreFile(const tstring strFilter);
private:
	bool BuildRecursive(fs::path path, const tstring & strAddPath);

private:

	tstring m_strSource, m_strTarget;

	vecRegex		m_vecIgnoreFolders;
	vecRegex		m_vecIgnoreFiles;

};

