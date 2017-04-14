#include "PatchPackage.h"
#include "FileMemory.h"
#include "PatchUtils.h"
#include "XdeltaCmd.h"
#include <boost/locale.hpp>
namespace loc = boost::locale;


const _TCHAR endl = '\n';
const _TCHAR quote = '\"';



PatchPackage::PatchPackage():
m_Consol(tcout)
{

}
PatchPackage::PatchPackage(XDeltaFolder & FolderInfo):
m_Consol(tcout),
m_RootFolder(FolderInfo)
{
	
}

PatchPackage::~PatchPackage()
{
}

bool PatchPackage::Build(tstring strPackFileName, const tstring strNewPath, const tstring strOldPath)
{
	m_strNewFolder = strNewPath;
	m_strOldFolder = strOldPath;
	m_PackFile.SetPasswd(PatchUtils::cszGamePackPasswd);
	if (!m_PackFile.Open(strPackFileName, true, PatchUtils::PackCompressLevel))
	{
		m_Consol << "Can't open zip archive " << strPackFileName << " zip error: " << m_PackFile.ErrorMessage() << std::endl;
		return false;
	}

	bool bRet = BuildCopy();
	bRet &= BuildDiff();
	bRet &= MakeHash();

	bRet &= WriteRDFInfo(strPackFileName);

	if (!m_PackFile.Close())
	{
		m_Consol << "Can't close zip archive " << strPackFileName << " zip error: " << m_PackFile.ErrorMessage() << std::endl;
		bRet = false;
	}

	if (bRet)
		m_Consol << "Successfully done!" << std::endl;
	else
		m_Consol << "FAILED." << std::endl;


	return bRet;
}

bool PatchPackage::Build(std::string strPackFileName, const std::string strNewPath, const std::string strOldPath)
{
	std::wstring NewFolder = PatchUtils::StringToWstring(strNewPath);
	std::wstring OldFolder = PatchUtils::StringToWstring(strOldPath);
	std::wstring strPackFilePath = PatchUtils::StringToWstring(strPackFileName);
		
	return Build(strPackFilePath, NewFolder, OldFolder);
}

bool PatchPackage::Build(tstring strPackFileName, XDeltaFolder & FolderInfo, const tstring strNewPath, const tstring strOldPath)
{
	m_RootFolder = FolderInfo;
	return Build(strPackFileName, strNewPath, strOldPath);
}

bool PatchPackage::BuildCopyFiles(fs::path relativePath, const XDeltaFiles & FileInfos)
{
	int nErrorCount = 0;
	tstring strTemp = _T("temp");
	for each(auto & cFile in FileInfos)
	{
		fs::path  fullNew = m_strNewFolder / relativePath / cFile->name;
		fs::path relativeTemp = relativePath / cFile->name;
		fs::path fullTemp = strTemp / relativeTemp;

		FileMemory NewFile;
		if (!NewFile.ReadFile(fullNew.c_str()))
		{
			m_Consol << "Can't read file " << PatchUtils::FileString(fullNew) << std::endl;
			nErrorCount++;
			continue;
		}

		SHA1Compute(NewFile.Get(), NewFile.GetSize(), cFile->newSha1);
		cFile->newSize = NewFile.GetSize();

		//Size Limit 설정 하려면 여기서 한다.
		//if ( NewFile.GetSize() > LimitSize ) continue;

		if (!m_PackFile.WriteFile(PatchUtils::GenericString(relativeTemp), NewFile.Get(), NewFile.GetSize()))
		{
			m_Consol << "Can't write file to archive " << PatchUtils::GenericString(relativeTemp) << " size=" << NewFile.GetSize() << std::endl;
			nErrorCount++;
		}

		m_Consol << "Copy file " << PatchUtils::FileString(fullNew) << "size=" << cFile->newSize << std::endl;
	}
	return nErrorCount == 0;;
}

bool PatchPackage::BuildCopyFolders(fs::path relativePath, const XDeltaFolders & FolderInfo)
{
	bool bRet = true;

	for each(auto & cFolder in FolderInfo)
	{
		fs::path nextRelativePath = relativePath / cFolder->GetName();

		bRet &= BuildCopyFiles(nextRelativePath, cFolder->GetNewFiles());

		bRet &= BuildCopyFolders(nextRelativePath, cFolder->GetNewFolder());
		bRet &= BuildCopyFolders(nextRelativePath, cFolder->GetBothFolder());

	}
	return bRet;
}
bool PatchPackage::BuildCopy()
{
	fs::path  relativePath;

	m_Consol << "Building whole file copies (new files)..." << std::endl;

	bool bRet = BuildCopyFiles(relativePath, m_RootFolder.GetNewFiles());
	bRet &= BuildCopyFolders(relativePath, m_RootFolder.GetNewFolder());
	bRet &= BuildCopyFolders(relativePath, m_RootFolder.GetBothFolder());

	return bRet;
}

tstring PatchPackage::DiffFileName(tstring  const& fileName)
{
	return fileName + _T(".") + PatchUtils::PackedEnxtention;
}

#ifdef XDELTA3
bool PatchPackage::CreateRDFFile( XdeltaFile * fileInfo,fs::path const& fullNew, fs::path const& fullOld, fs::path const& relativeTemp,	FileMemory const& newFile, FileMemory const& oldFile)
{
	fs::path fullTemp = relativeTemp.filename();

	m_Consol << "Encoding XDelta diff file " << PatchUtils::GenericString(relativeTemp) << std::endl;

	FileMemory  XDelataFile;
	if (!m_XDeltaEncoder.EncodeDiffMemoryBlock(newFile.GetConst(), newFile.GetSize(), oldFile.GetConst(), oldFile.GetSize(), XDelataFile))
	{
		tstring errorMessage;
		m_XDeltaEncoder.GetErrorMessage(errorMessage);
		m_Consol << "Encoding error: " << errorMessage << std::endl;
		return false;
	}


	if (!m_PackFile.WriteFile(PatchUtils::GenericString(relativeTemp), XDelataFile.Get(), XDelataFile.GetSize()))
	{
		m_Consol << "Can't write file " << PatchUtils::GenericString(relativeTemp) << " to package. Size=" << XDelataFile.GetSize() << std::endl;
		return false;
	}
	return true;
}

bool PatchPackage::CreateRDFFileStream(XdeltaFile * fileInfo, fs::path const& fullNew, fs::path const& fullOld, fs::path const& relativeTemp)
{

	std::wstring wstrOut = fullNew.c_str();
	wstrOut += _T(".dff");

	if (!m_XDeltaEncoder.EncodeDiffStream(fullOld.c_str(), fullNew.c_str(), wstrOut, 0x1000))
	{
		tstring errorMessage;
		m_XDeltaEncoder.GetErrorMessage(errorMessage);
		m_Consol << "Encoding error: " << errorMessage << std::endl;
		return false;
	}

	FileMemory  XDelataFile;
	XDelataFile.ReadFile(wstrOut.c_str());
	if (!m_PackFile.WriteFile(PatchUtils::GenericString(relativeTemp), XDelataFile.Get(), XDelataFile.GetSize()))
	{
		m_Consol << "Can't write file " << PatchUtils::GenericString(relativeTemp) << " to package. Size=" << XDelataFile.GetSize() << std::endl;
		return false;
	}
	PatchUtils::RemoveFile(wstrOut.c_str());
	return true;
}



bool PatchPackage::BuildDiffFile(fs::path const& relativePath,XdeltaFile * fileInfo)
{
	tstring strTemp = _T("temp");
	fs::path fullNew = m_strNewFolder / relativePath / fileInfo->name;
	fs::path fullOld = m_strOldFolder / relativePath / fileInfo->name;
	fs::path relativeTemp = relativePath / DiffFileName(fileInfo->name);

	FileMemory OldFile;
	if (!OldFile.ReadFile(fullOld.wstring()))
	{
		m_Consol << "Can't read file " << PatchUtils::FileString(fullOld) << std::endl;
		return false;
	}

	SHA1Compute(OldFile.Get(), OldFile.GetSize(), fileInfo->oldSha1);
	fileInfo->oldSize = OldFile.GetSize();

	//스킵 파일 있는경우 여기서 처리한다.

	FileMemory NewFile;
	if (!NewFile.ReadFile(fullNew.wstring()))
	{
		m_Consol << "Can't read file " << PatchUtils::FileString(fullNew) << std::endl;
		return false;
	}

	SHA1Compute(NewFile.Get(), NewFile.GetSize(), fileInfo->newSha1);
	fileInfo->newSize = NewFile.GetSize();

	bool bRet = true;

	if (fileInfo->newSha1 != fileInfo->oldSha1)
	{
		fileInfo->isDifferent = true;

		bRet &= CreateRDFFile(fileInfo, fullNew, fullOld, relativeTemp, NewFile, OldFile);
	}
	else
		fileInfo->isDifferent = false;

	m_Consol << "diff file " << PatchUtils::FileString(fullNew) << std::endl;

	return bRet;
}

bool PatchPackage::BuildDiffFileStream(fs::path const& relativePath, XdeltaFile * fileInfo)
{
	tstring strTemp = _T("temp");
	fs::path fullNew = m_strNewFolder / relativePath / fileInfo->name;
	fs::path fullOld = m_strOldFolder / relativePath / fileInfo->name;
	fs::path relativeTemp = relativePath / DiffFileName(fileInfo->name);

	{
		FileMemory OldFile;
		if (!OldFile.ReadFile(fullOld.wstring()))
		{
			m_Consol << "Can't read file " << PatchUtils::FileString(fullOld) << std::endl;
			return false;
		}

		SHA1Compute(OldFile.Get(), OldFile.GetSize(), fileInfo->oldSha1);
		fileInfo->oldSize = OldFile.GetSize();
	}

		//스킵 파일 있는경우 여기서 처리한다.
	{
		FileMemory NewFile;
		if (!NewFile.ReadFile(fullNew.wstring()))
		{
			m_Consol << "Can't read file " << PatchUtils::FileString(fullNew) << std::endl;
			return false;
		}

		SHA1Compute(NewFile.Get(), NewFile.GetSize(), fileInfo->newSha1);
		fileInfo->newSize = NewFile.GetSize();
	}

	if (fileInfo->newSha1 != fileInfo->oldSha1) fileInfo->isDifferent = true;
	else fileInfo->isDifferent = false;

	bool bRet = true;

	if (true == fileInfo->isDifferent )
	{

		bRet &= CreateRDFFileStream(fileInfo, fullNew, fullOld, relativeTemp);
	}

	m_Consol << "diff file " << PatchUtils::FileString(fullNew) << std::endl;

	return bRet;
}
#else 
bool PatchPackage::CreateRDFFileCmd(XdeltaFile * fileInfo, fs::path const& fullNew, fs::path const& fullOld, fs::path const& relativeTemp)
{
	std::wstring wstrOut = fullNew.c_str();
	wstrOut += _T(".dff");

	XdeltaCmd cRunCmd;
	int nRet = cRunCmd.DoEncode(PatchUtils::WstringToString(fullOld.c_str()), PatchUtils::WstringToString(fullNew.c_str()), PatchUtils::WstringToString(wstrOut));

	if (nRet != 0)
	{
		m_Consol << "Encoding error: " << "CreateRDFFileCmd" << std::endl;
		return false;
	}

	FileMemory  XDelataFile;
	XDelataFile.ReadFile(wstrOut.c_str());
	if (!m_PackFile.WriteFile(PatchUtils::GenericString(relativeTemp), XDelataFile.Get(), XDelataFile.GetSize()))
	{
		m_Consol << "Can't write file " << PatchUtils::GenericString(relativeTemp) << " to package. Size=" << XDelataFile.GetSize() << std::endl;
		return false;
	}
	PatchUtils::RemoveFile(wstrOut.c_str());
	return true;
}

//xdelta 3.10으로 빌드 한다.
bool PatchPackage::BuildDiffFileCmd(fs::path const& relativePath, XdeltaFile * fileInfo)
{
	tstring strTemp = _T("temp");
	fs::path fullNew = m_strNewFolder / relativePath / fileInfo->name;
	fs::path fullOld = m_strOldFolder / relativePath / fileInfo->name;
	fs::path relativeTemp = relativePath / DiffFileName(fileInfo->name);

	{
		FileMemory OldFile;
		if (!OldFile.ReadFile(fullOld.wstring()))
		{
			m_Consol << "Can't read file " << PatchUtils::FileString(fullOld) << std::endl;
			return false;
		}

		SHA1Compute(OldFile.Get(), OldFile.GetSize(), fileInfo->oldSha1);
		fileInfo->oldSize = OldFile.GetSize();
	}

	//스킵 파일 있는경우 여기서 처리한다.
	{
		FileMemory NewFile;
		if (!NewFile.ReadFile(fullNew.wstring()))
		{
			m_Consol << "Can't read file " << PatchUtils::FileString(fullNew) << std::endl;
			return false;
		}

		SHA1Compute(NewFile.Get(), NewFile.GetSize(), fileInfo->newSha1);
		fileInfo->newSize = NewFile.GetSize();
	}

	if (fileInfo->newSha1 != fileInfo->oldSha1) fileInfo->isDifferent = true;
	else fileInfo->isDifferent = false;

	bool bRet = true;

	if (true == fileInfo->isDifferent)
	{

		bRet &= CreateRDFFileCmd(fileInfo, fullNew, fullOld, relativeTemp);
	}

	m_Consol << "diff file " << PatchUtils::FileString(fullNew) << std::endl;

	return bRet;
}

#endif

//파일 비교 하여 변경내용 추출
//Build
bool PatchPackage::BuildDiffFiles(fs::path const& relativePath, XDeltaFiles & FileInfos)
{
	bool result = true;

	for each(auto  cFile in FileInfos)
	{

		//result &= BuildDiffFile(relativePath, cFile);
		//result &= BuildDiffFileStream(relativePath, cFile);
		result &= BuildDiffFileCmd(relativePath, cFile);
	}

	return result;
}


bool PatchPackage::BuildDiffFolders(fs::path  const& relativePath, XDeltaFolders const& folderInfos)
{
	bool result = true;

	for each(auto & cFolder in folderInfos)
	{

		fs::path  nextRelativePath = relativePath / cFolder->GetName();

		result &= BuildDiffFiles(nextRelativePath, cFolder->GetBothFiles());
		result &= BuildDiffFolders(nextRelativePath, cFolder->GetBothFolder());
	}

	return result;
}


bool PatchPackage::BuildDiff()
{
	bool bRet = true;

	m_Consol << "Building diff files..." << std::endl;

	fs::path relativePath;

	bRet &= BuildDiffFiles(relativePath, m_RootFolder.GetBothFiles());
	bRet &= BuildDiffFolders(relativePath, m_RootFolder.GetBothFolder());

	return bRet;
}

bool PatchPackage::MakeHashFiles(fs::path const& relativePath, XDeltaFiles const& fileInfos)
{
	int nErrorCount= 0;

	for each(auto cFile in fileInfos)
	{

		fs::path  fullOld = m_strOldFolder / relativePath / cFile->name;

		FileMemory OldFile;
		if (!OldFile.ReadFile(fullOld.wstring()))
		{
			m_Consol << "Can't read file " << PatchUtils::FileString(fullOld) << std::endl;
			nErrorCount++;
			continue;
		}

		SHA1Compute(OldFile.Get(), OldFile.GetSize(), cFile->oldSha1);
		cFile->oldSize = OldFile.GetSize();
	}

	return nErrorCount == 0;
}

bool PatchPackage::MakeHashFolders(fs::path const& relativePath, XDeltaFolders const& folderInfos)
{
	bool bRet = true;

	for each (auto cFolder in folderInfos)
	{
		fs::path nextRelativePath = relativePath / cFolder->GetName();
		bRet &= MakeHashFiles(nextRelativePath, cFolder->GetOldFiles() );
		bRet &= MakeHashFolders(nextRelativePath, cFolder->GetOldFolder());
		bRet &= MakeHashFolders(nextRelativePath, cFolder->GetBothFolder());
	}

	return bRet;
}


bool PatchPackage::MakeHash()
{
	bool bRet = true;

	fs::path relativePath;

	m_Consol << "Gathering SHA1 digests for files in old directory..." << std::endl;
	bRet &= MakeHashFiles(relativePath, m_RootFolder.GetOldFiles());

	bRet &= MakeHashFolders(relativePath, m_RootFolder.GetOldFolder());
	bRet &= MakeHashFolders(relativePath, m_RootFolder.GetBothFolder());

	return bRet;

}


void PatchPackage::WriteRDFFiles(XDeltaFiles const& fileInfos, RDFAction::EAction action, fs::path const& relativePath, tstringstream & OutFile)
{
	for each(auto cFile in fileInfos)
	{
		RDFAction::EAction fileAction = cFile->isDifferent ? RDFAction::EAction::APPLY_DIFF : action;

		//if (fileAction == dung::Action::NEW && config.newFileLimit > 0 && fileInfo.newSize >= config.newFileLimit)
		//	fileAction = dung::Action::NEW_BUT_NOT_INCLUDED;

		//if (fileAction == RDFAction::EAction::NEW && MatchName(config.newOverrideFiles_regex, fileInfo.name))
		//	fileAction = dung::Action::OVERRIDE;

		//if (fileAction == RDFAction::EAction::DELETE && MatchName(config.oldPreserveRemoved_regex, fileInfo.name))
		//	fileAction = dung::Action::NONE;

/*		if (fileAction == dung::Action::APPLY_DIFF && MatchName(config.oldSkipChanged_regex, fileInfo.name))
			fileAction = dung::Action::NONE*/;

		OutFile << _T("file") << endl;
		OutFile << _T("{") << endl;

		OutFile << _T("\t") << _T("action=") << RDFAction::ActionToString(fileAction) << endl;

		if (fileAction != RDFAction::EAction::REMOVE && fileAction != RDFAction::EAction::NONE)
		{
			OutFile << _T("\t") << _T("new_path=") << quote << PatchUtils::GenericString(relativePath / cFile->name) << quote << endl;
			OutFile << _T("\t") << _T("new_size=") << cFile->newSize << endl;
			OutFile << _T("\t") << _T("new_sha1=") << quote << SHA1_TO_TSTRING(cFile->newSha1) << quote << endl;
		}

		if (fileAction != RDFAction::EAction::NEW && fileAction != RDFAction::EAction::OVERRIDE && fileAction != RDFAction::EAction::NEW_BUT_NOT_INCLUDED)
		{
			OutFile << _T("\t") << _T("old_path=") << quote << PatchUtils::GenericString(relativePath / cFile->name) << quote << endl;
			OutFile << _T("\t") << _T("old_size=") << cFile->oldSize << endl;
			OutFile << _T("\t") << _T("old_sha1=") << quote << SHA1_TO_TSTRING(cFile->oldSha1) << quote << endl;
		}

		if (fileAction == RDFAction::EAction::APPLY_DIFF)
		{
			OutFile << _T("\t") << _T("diff_path=") << quote << PatchUtils::GenericString(relativePath / DiffFileName(cFile->name)) << quote << endl;
			OutFile << _T("\t") << _T("diff_method=") << "xdelta" << endl;
		}

		OutFile << _T("}") << endl;
	}
}

void PatchPackage::WriteRDFFolders( XDeltaFolders const& folderInfos, fs::path const& relativePath ,tstringstream & OutFile)
{
	for each (auto cFolder in folderInfos)
	{

		fs::path nextRelativePath = relativePath / cFolder->GetName();

		WriteRDFFiles(cFolder->GetBothFiles(), RDFAction::EAction::NONE, nextRelativePath, OutFile);
		WriteRDFFiles(cFolder->GetNewFiles(), RDFAction::EAction::NEW, nextRelativePath, OutFile);
		WriteRDFFiles(cFolder->GetOldFiles(), RDFAction::EAction::REMOVE, nextRelativePath, OutFile);

		WriteRDFFolders(cFolder->GetBothFolder(), nextRelativePath, OutFile);
		WriteRDFFolders(cFolder->GetNewFolder(), nextRelativePath, OutFile);
		WriteRDFFolders(cFolder->GetOldFolder(), nextRelativePath, OutFile);
	}
}

bool PatchPackage::WriteRDFInfo(const tstring & strPackFileName)
{
	bool bRet = true;

	m_Consol << "Creating registry file..." << std::endl;

	tstringstream stringStream;
	//OutputContext outputContext(stringStream, out);

	//if (!options.newVersion.empty())
	//	outputContext.stream << _T("new_version=") << quote << options.newVersion << quote << endl;

	//if (!options.oldVersion.empty())
	//	outputContext.stream << _T("old_version=") << quote << options.oldVersion << quote << endl;

	XDeltaFolders tmp;
	tmp.push_back(&m_RootFolder);
	fs::path relativePath;
	WriteRDFFolders(tmp, relativePath, stringStream);

	tstring fileContent = stringStream.str();
	std::string fileContentUtf8 = loc::conv::utf_to_utf<char>(fileContent);

	if (!m_PackFile.WriteFile(PatchUtils::REGISTRY_FILENAME, fileContentUtf8.c_str(), fileContentUtf8.size()))
	{
		m_Consol << "Can't write file " << PatchUtils::REGISTRY_FILENAME << " to package" << std::endl;
		return false;
	}

	
	tstring tstrpath = strPackFileName.substr(0,strPackFileName.find_last_of('\\'));
	tstrpath += _T("\\") ;
	tstrpath += PatchUtils::REGISTRY_FILENAME;
	FileMemory UpdateInfo(fileContentUtf8);
	UpdateInfo.WriteFile(tstrpath);
	return true;
}
