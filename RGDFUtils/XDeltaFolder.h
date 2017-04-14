#pragma once
#include "stdAfxZlib.h"
#include "./Base/sha1.h"
#include <unordered_set>



typedef std::unordered_set< tstring >	tstringset;
typedef std::vector< tstring >			tstringvector;



struct XdeltaFile
{
	XdeltaFile()
		: newSize()
		, oldSize()
		, isDifferent(false){}

	tstring  name;

	Sha1 newSha1, oldSha1;
	size_t newSize, oldSize;

	bool isDifferent;
};
class XDeltaFolder;
typedef std::vector< XDeltaFolder* > XDeltaFolders;
typedef std::vector< XdeltaFile* > XDeltaFiles;

class XDeltaFolder
{
public:
	XDeltaFolder();
	~XDeltaFolder();

	const tstring GetName() { return m_strName; }
	void SetName(const tstring strName){ m_strName = strName; }
	XDeltaFolders & GetNewFolder(){ return m_NewFolder; }
	XDeltaFolders & GetOldFolder(){ return m_OldFolder; }
	XDeltaFolders & GetBothFolder(){ return m_BothFolder; }

	XDeltaFiles & GetNewFiles(){ return m_NewFiles; }
	XDeltaFiles & GetOldFiles(){ return m_OldFiles; }
	XDeltaFiles & GetBothFiles(){ return m_BothFiles; }

	void BuildRoot(const std::string strNewPath, const std::string strOldPath);
	void BuildRoot( const tstring strNewPath, const tstring strOldPath );

	void AddIgnoreFolder(const tstring strFilter);
	void AddIgnoreFile(const tstring strFilter);
	void AddIgnoreFolder(const std::vector<std::string> & vecFilters);
	void AddIgnoreFile(const std::vector<std::string> &  vecFilters);
private:

	void BuildSub(const fs::path strNewPath, const fs::path strOldPath, XDeltaFolder * pFolder);
	void BuildPath(fs::path const& path, tstringset & files, tstringset & folders, bool newPath);
	void BuildTreeSet(tstringset& newSet, tstringset& oldSet, tstringvector& existInBoth, tstringvector& newOnly, tstringvector& oldOnly);
	void CreateFiles(tstringvector const& names, XDeltaFiles & fileInfos);
	void CreateFolders(tstringvector const& names, XDeltaFolders & folderInfos);
	void BuildRecursive(fs::path const& pathToNew, fs::path const& pathToOld, XDeltaFolders const& folderInfos);
private:
	tstring			m_strName;
	XDeltaFolders	m_NewFolder;
	XDeltaFolders	m_OldFolder;
	XDeltaFolders	m_BothFolder;

	XDeltaFiles		m_NewFiles;
	XDeltaFiles		m_OldFiles;
	XDeltaFiles		m_BothFiles;

	vecRegex		m_vecIgnoreFolders;
	vecRegex		m_vecIgnoreFiles;

};

