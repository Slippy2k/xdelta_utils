#pragma once

#include "stdAfxZlib.h"
#include "XDeltaFolder.h"
//#include "XDeltaEncoder.h"
#include "RDFAction.h"
#include "ZipArchiveOutput.h"



class PatchPackage
{
public:
	PatchPackage();
	PatchPackage(XDeltaFolder & FolderInfo);
	~PatchPackage();

	bool Build(tstring strPackFileName, const tstring strNewPath, const tstring strOldPath);
	bool Build(std::string strPackFileName, const std::string strNewPath, const std::string strOldPath);
	bool Build(tstring strPackFileName, XDeltaFolder & FolderInfo, const tstring strNewPath, const tstring strOldPath);

private:
	bool BuildCopyFiles(fs::path relativePath, const XDeltaFiles & FileInfos);
	bool BuildCopyFolders(fs::path relativePath, const XDeltaFolders & FolderInfo);
	bool BuildCopy();

	tstring DiffFileName(tstring  const& fileName);
#ifdef XDELTA3
	bool CreateRDFFile(XdeltaFile * fileInfo, fs::path const& fullNew, fs::path const& fullOld, fs::path const& relativeTemp, FileMemory const& newFile, FileMemory const& oldFile);
	bool CreateRDFFileStream(XdeltaFile * fileInfo, fs::path const& fullNew, fs::path const& fullOld, fs::path const& relativeTemp);
	bool BuildDiffFile(fs::path const& relativePath,XdeltaFile * fileInfo);
	bool BuildDiffFileStream(fs::path const& relativePath, XdeltaFile * fileInfo);
#else
	bool BuildDiffFiles(fs::path const& relativePath, XDeltaFiles & FileInfos);
	bool BuildDiffFolders(fs::path  const& relativePath, XDeltaFolders const& folderInfos);
	bool BuildDiff();
#endif

	bool CreateRDFFileCmd(XdeltaFile * fileInfo, fs::path const& fullNew, fs::path const& fullOld, fs::path const& relativeTemp);
	bool BuildDiffFileCmd(fs::path const& relativePath, XdeltaFile * fileInfo);

	//Make File Hash info
	bool MakeHashFiles(fs::path const& relativePath, XDeltaFiles const& fileInfos);
	bool MakeHashFolders(fs::path const& relativePath, XDeltaFolders const& folderInfos);
	bool MakeHash();

	//RDF 파일 인포 저장
	void WriteRDFFiles(XDeltaFiles const& fileInfos, RDFAction::EAction action, fs::path const& relativePath, tstringstream & OutFile);
	void WriteRDFFolders(XDeltaFolders const& folderInfos, fs::path const& relativePath, tstringstream & OutFile);
	bool WriteRDFInfo(const tstring & strPackFileName);

private:
	tostream	&				m_Consol;
	XDeltaFolder				m_RootFolder;
	tstring						m_strNewFolder, m_strOldFolder;
	//XDeltaEncoder				m_XDeltaEncoder;
	zip::ZipArchiveOutput		m_PackFile;

};

