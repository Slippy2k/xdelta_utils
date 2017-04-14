#include "PatchUnPack.h"
#include "RDFAction.h"
#include <assert.h>
#include "PatchUtils.h"
#include "FileInfoReader.h"
#include "XdeltaCmd.h"

PatchUnPack::PatchUnPack():
m_Consol(tcout),
m_bConsolOut(true),
m_bChkFileHash(true),
m_bChkFileSize(true),
m_bErrorStop(true),
m_bRollback(false),
m_bAutoDelete(false)
{
}


PatchUnPack::~PatchUnPack()
{
}


bool PatchUnPack::CreateNewFile(SUpdateAction const& action, bool overrideFile)
{
	tstring fullPathNew = PatchUtils::FullPath(m_strOldFolder, action.new_path);

	if (!overrideFile)
	{
		size_t existFileSize;
		if (PatchUtils::FileSize(fullPathNew.c_str(), existFileSize))
		{
			m_Consol << "Can't create new file " << action.new_path << ". File already exists." << std::endl;
			return false;
		}
	}

	FileMemory memoryBlock;
	if (!m_PackFile.ReadFile(action.new_path, memoryBlock))
	{
		m_Consol << "Can't unzip file " << action.new_path << ". zip error: " << m_PackFile.ErrorMessage() << std::endl;
		return false;
	}

	tstring dirPath;
	if (!PatchUtils::ParentPath(fullPathNew, dirPath))
	{
		m_Consol << "Can't extract parent path from " << fullPathNew << std::endl;
		return false;
	}

	if (!PatchUtils::CreateDirectories(dirPath.c_str()))
	{
		m_Consol << "Can't create directory " << dirPath << std::endl;
		return false;
	}

	if (!memoryBlock.WriteFile(fullPathNew))
	{
		m_Consol << "Can't write file " << fullPathNew << std::endl;
		return false;
	}

	return true;
}
bool PatchUnPack::DeleteOldFile(SUpdateAction const& action)
{
	tstring fullPath = PatchUtils::FullPath(m_strOldFolder, action.old_path);
	bool result = PatchUtils::RemoveFile(fullPath.c_str());
	if (!result)
		m_Consol << "Can't delete file " << fullPath << std::endl;

	return result;
}

bool PatchUnPack::CheckOldFileData(FileMemory	const& oldFile, SUpdateAction const& action)
{
	if (m_bChkFileSize && oldFile.GetSize() != action.oldSize)
	{
		m_Consol << _T("Old file has wrong size. ") << action.old_path << _T(" Real size=") << oldFile.GetSize() << _T(", registry size=") << action.oldSize << std::endl;
		return false;
	}

	if (m_bChkFileHash)
	{
		Sha1 oldSha1;
		SHA1Compute(oldFile.GetConst(), oldFile.GetSize(), oldSha1);
		if (oldSha1 != action.oldSha1)
		{
			m_Consol << _T("Old file has wrong SHA1. ") << action.old_path << _T(" Real SHA1=") << SHA1_TO_TSTRING(oldSha1) << _T(", registry SHA1=") << SHA1_TO_TSTRING(action.oldSha1) << std::endl;
			return false;
		}
	}

	return true;
}
#ifdef XDELTA3
bool PatchUnPack::ApplyRDF(SUpdateAction const& action)
{
	FileMemory diffBlock;
	if (!m_PackFile.ReadFile(action.diff_path, diffBlock))
	{
		m_Consol << "Can't unzip file " << action.diff_path << ". zip error: " << m_PackFile.ErrorMessage() << std::endl;
		return false;
	}

	FileMemory oldFile;
	tstring fullPathOld = PatchUtils::FullPath(m_strOldFolder, action.old_path);
	if (!oldFile.ReadFile(fullPathOld))
	{
		m_Consol << "Can't read file " << fullPathOld << std::endl;
		return false;
	}

	if (!CheckOldFileData(oldFile, action))
		return false;

	FileMemory newFile;
	newFile.SetSize(action.newSize);

	bool result = m_XdeltaDecoder.DecodeDiffMemoryBlock(oldFile.GetConst(), oldFile.GetSize(), diffBlock.GetConst(), diffBlock.GetSize(), newFile);
	if (newFile.GetSize() != action.newSize)
		result = false;
	if (!result)
	{
		tstring errorMessage;
		m_XdeltaDecoder.GetErrorMessage(errorMessage);
		m_Consol << "Can't decode with " << action.diff_method << " for file " << action.diff_path << " " << errorMessage << std::endl;
	}
	else
	{
		if (!newFile.WriteFile(fullPathOld))
		{
			m_Consol << "Can't write file " << fullPathOld << std::endl;
			result = false;
		}
	}

	return result;
}

bool PatchUnPack::ApplyRDFStream(SUpdateAction const& action)
{
	tstring fullPathOld = PatchUtils::FullPath(m_strOldFolder, action.old_path);
	{ //이전 파일과 동인한지 파일 체크
		FileMemory oldFile;
		if (!oldFile.ReadFile(fullPathOld))
		{
			m_Consol << "Can't read file " << fullPathOld << std::endl;
			return false;
		}

		if (!CheckOldFileData(oldFile, action))
		{
			return false;
		}
	}


	tstring fullPathDiff = PatchUtils::FullPath(m_strOldFolder, action.diff_path);
	// 비교 대상 파일 임시파일로 저장
	{
		FileMemory diffBlock;
		if (!m_PackFile.ReadFile(action.diff_path, diffBlock))
		{
			m_Consol << "Can't unzip file " << action.diff_path << ". zip error: " << m_PackFile.ErrorMessage() << std::endl;
			return false;
		}
		diffBlock.WriteFile(fullPathDiff);
	}


	tstring text = PatchUtils::ROLLBACK_EXT;
	tstring rollbackfile = fullPathOld.c_str() + text;

	
	int nOutFileSize = 0;
	//bool result = m_XdeltaDecoder.DecodeDiffStream(fullPathDiff, fullPathOld, fullPathOld + _T(".new"));
	bool result = m_XdeltaDecoder.DecodeDiffStream(fullPathDiff, rollbackfile, fullPathOld, nOutFileSize);

	if (nOutFileSize != action.newSize)
		result = false;
	if (!result)
	{
		tstring errorMessage;
		m_XdeltaDecoder.GetErrorMessage(errorMessage);
		m_Consol << "Can't decode with " << action.diff_method << " for file " << action.diff_path << " " << errorMessage << std::endl;
	}
	else
	{
		//if (!newFile.WriteFile(fullPathOld))
		//{
		//	m_Consol << "Can't write file " << fullPathOld << std::endl;
		//	result = false;
		//}
	}
	PatchUtils::RemoveFile(fullPathDiff.c_str());

	return result;
}
#else

bool PatchUnPack::ApplyRDFCmd(SUpdateAction const& action)
{
	tstring fullPathOld = PatchUtils::FullPath(m_strOldFolder, action.old_path);
	{ //이전 파일과 동인한지 파일 체크
		FileMemory oldFile;
		if (!oldFile.ReadFile(fullPathOld))
		{
			m_Consol << "Can't read file " << fullPathOld << std::endl;
			return false;
		}

		if (!CheckOldFileData(oldFile, action))
		{
			return false;
		}
	}


	tstring fullPathDiff = PatchUtils::FullPath(m_strOldFolder, action.diff_path);
	// 비교 대상 파일 임시파일로 저장
	{
		FileMemory diffBlock;
		if (!m_PackFile.ReadFile(action.diff_path, diffBlock))
		{
			m_Consol << "Can't unzip file " << action.diff_path << ". zip error: " << m_PackFile.ErrorMessage() << std::endl;
			return false;
		}
		diffBlock.WriteFile(fullPathDiff);
	}


	tstring text = PatchUtils::ROLLBACK_EXT;
	tstring rollbackfile = fullPathOld.c_str() + text;


	size_t nOutFileSize = 0;

	XdeltaCmd cRunCmd;
	bool result = true;
	int nRet = cRunCmd.DoDecode(PatchUtils::WstringToString(rollbackfile.c_str()), PatchUtils::WstringToString(fullPathDiff.c_str()), PatchUtils::WstringToString(fullPathOld));

	if (nRet != 0)
	{
		m_Consol << "Encoding error: " << "ApplyRDFCmd" << std::endl;
		result = false;
	}

	PatchUtils::FileSize(fullPathOld.c_str(),nOutFileSize);

	if (nOutFileSize != action.newSize)
	{
		m_Consol << "size check error action : " << action.newSize <<"OutFileSize"<< fullPathOld <<" : " << nOutFileSize <<std::endl;
		result = false;
	}

	//bool result = m_XdeltaDecoder.DecodeDiffStream(fullPathDiff, rollbackfile, fullPathOld, nOutFileSize);

	//if (!result)
	//{
	//	tstring errorMessage;
	//	m_XdeltaDecoder.GetErrorMessage(errorMessage);
	//	m_Consol << "Can't decode with " << action.diff_method << " for file " << action.diff_path << " " << errorMessage << std::endl;
	//}
	//else
	//{
	//	//if (!newFile.WriteFile(fullPathOld))
	//	//{
	//	//	m_Consol << "Can't write file " << fullPathOld << std::endl;
	//	//	result = false;
	//	//}
	//}
	PatchUtils::RemoveFile(fullPathDiff.c_str());

	return result;
}
#endif


bool PatchUnPack::PatchApplyAction(SUpdateAction const& action)
{
	tstring fullPathNew = PatchUtils::FullPath(m_strOldFolder, action.new_path);
	tstring text = PatchUtils::ROLLBACK_EXT;
	tstring rollbackfile = fullPathNew.c_str() + text;

	switch (action.action)
	{
	case RDFAction::EAction::NEW:
		if (m_bConsolOut)
			m_Consol << "Creating new file " << action.new_path << std::endl;
		if (!CreateNewFile(action, true)) // Override it anyway.
			return false;
		break;

	case RDFAction::EAction::OVERRIDE:
		if (m_bConsolOut)
			m_Consol << "Overriding with new file " << action.new_path << std::endl;
		if (!PatchUtils::filecopy(fullPathNew, rollbackfile))
		{
			m_Consol << "PatchUtils::filecopy error file : " << action.new_path << std::endl;
		}
		if (!CreateNewFile(action, true))
			return false;
		break;

	case RDFAction::EAction::REMOVE:
		if (m_bConsolOut)
			m_Consol << "Deleting old file " << action.old_path << std::endl;
		if ( !PatchUtils::filecopy(fullPathNew, rollbackfile) )
		{
			m_Consol << "PatchUtils::filecopy error file : " << action.new_path << std::endl;
		}

		if (!DeleteOldFile(action))
			return false;
		break;

	case RDFAction::EAction::MOVE:
		//TODO: implement
		break;

	case RDFAction::EAction::APPLY_DIFF:
		if (m_bConsolOut)
			m_Consol << "Applying difference " << action.diff_path << " to " << action.old_path << std::endl;
		if (!PatchUtils::filecopy(fullPathNew, rollbackfile) )
		{
			m_Consol << "PatchUtils::filecopy error file : " << action.new_path << std::endl;
		}
		//if (!ApplyRDF(action))
		//if (!ApplyRDFStream(action))
		if (!ApplyRDFCmd(action))
			return false;
		break;

	case RDFAction::EAction::NONE:
		if (m_bConsolOut)
			m_Consol << "Skipping old file " << action.old_path << std::endl;
		break;

	case RDFAction::EAction::NEW_BUT_NOT_INCLUDED:
		if (m_bConsolOut)
			m_Consol << "Skipping not included file " << action.new_path << std::endl;
		break;

	default:
		assert(false && "unknown action type");
	}

	return true;
}

bool PatchUnPack::RollBackAction(SUpdateAction const& action)
{
	tstring fullPathNew = PatchUtils::FullPath(m_strOldFolder, action.new_path);
	tstring text = PatchUtils::ROLLBACK_EXT;
	tstring rollbackfile = fullPathNew.c_str() + text;

	switch (action.action)
	{
	case RDFAction::EAction::NEW:
		if (m_bConsolOut)
			m_Consol << "RollBack Creating new file " << action.new_path << std::endl;
		PatchUtils::RemoveFile(fullPathNew.c_str());
		break;
	case RDFAction::EAction::OVERRIDE:
		if (m_bConsolOut)
			m_Consol << "RollBack Overriding with new file " << action.new_path << std::endl;

		PatchUtils::movefile(rollbackfile.c_str(), fullPathNew.c_str());
		break;

	case RDFAction::EAction::REMOVE:
		if (m_bConsolOut)
			m_Consol << " RollBack Deleting old file " << action.old_path << std::endl;
		PatchUtils::movefile(rollbackfile.c_str(), fullPathNew.c_str());
		break;

	case RDFAction::EAction::MOVE:
		//TODO: implement
		break;

	case RDFAction::EAction::APPLY_DIFF:
		if (m_bConsolOut)
			m_Consol << " RollBack Applying difference " << action.diff_path << " to " << action.old_path << std::endl;
		PatchUtils::movefile(rollbackfile.c_str(), fullPathNew.c_str());
		break;

	case RDFAction::EAction::NONE:
		if (m_bConsolOut)
			m_Consol << "Skipping old file " << action.old_path << std::endl;
		break;

	case RDFAction::EAction::NEW_BUT_NOT_INCLUDED:
		if (m_bConsolOut)
			m_Consol << "Skipping not included file " << action.new_path << std::endl;
		break;

	default:
		assert(false && "unknown action type");
	}

	return true;
}

bool PatchUnPack::ApplyPatchActions(SFileInfo  const& rUpdateInfo, SFileInfo::VectAction & rActions)
{
	int numErrors  = 0;
	int nProcessCount = 0;
	int nMaxCount = rUpdateInfo.actions.size();

	for (SFileInfo::VectAction::const_iterator i = rUpdateInfo.actions.begin(); i != rUpdateInfo.actions.end(); ++i)
	{
		SUpdateAction const& action = **i;

		if (true == m_bRollback)	rActions.push_back(*i);

		tstring tpath = action.old_path == _T("") ? action.new_path : action.old_path;
#ifdef _UNICODE
		std::string strProcess = PatchUtils::WstringToString(tpath);
#else
		std::string strProcess = tpath;
#endif
		m_funcProcess ? m_funcProcess(strProcess.c_str(), ++nProcessCount, nMaxCount) : NULL;

		if (!PatchApplyAction(action))
		{
			if (m_bErrorStop)
				return false;
			else
				++numErrors;
		}
		
	}

	if (numErrors == 0)
		return true;

	m_Consol << "FAILED. " << numErrors << " errors occured." << std::endl;

	return false;
}

void PatchUnPack::RollBack(const SFileInfo::VectAction & rActions, const int nMaxCount, bool bSuccess)
{

	int  nProcessCount = rActions.size();
	for each (auto Action in rActions)
	{
		if (true == bSuccess)
		{
			tstring fullPathNew = PatchUtils::FullPath(m_strOldFolder, Action->new_path);
			tstring text = PatchUtils::ROLLBACK_EXT;
			tstring rollbackfile = fullPathNew.c_str() + text;
			PatchUtils::RemoveFile(rollbackfile.c_str());
			continue;
		}

		RollBackAction(*Action);

		tstring tpath = Action->old_path == _T("") ? Action->new_path : Action->old_path;
#ifdef _UNICODE
		std::string strProcess = PatchUtils::WstringToString(tpath);
#else
		std::string strProcess = tpath;
#endif
		m_funcProcess ? m_funcProcess(strProcess.c_str(), --nProcessCount, nMaxCount) : NULL;

	}
}

bool PatchUnPack::ApplyPatch(const std::string strPackFile, const std::string strOrgPath)
{
	std::wstring wstrPackFile = PatchUtils::StringToWstring(strPackFile);
	std::wstring wstrOrgPath = PatchUtils::StringToWstring(strOrgPath);
	return ApplyPatch(wstrPackFile, wstrOrgPath);
}

bool PatchUnPack::ApplyPatch(const std::wstring strPackFile, const std::wstring strOrgPath)
{
	m_strOldFolder = strOrgPath;

	m_PackFile.SetPasswd(PatchUtils::cszGamePackPasswd);
	if (!m_PackFile.Open(strPackFile, false))
	{
		m_Consol << "Can't open zip archive " << strPackFile << ". Zip error: " << m_PackFile.ErrorMessage() << std::endl;
		return false;
	}

	m_Consol << "Opened " << strPackFile << std::endl;

	FileMemory UpdateInfoMemory;
	if (!m_PackFile.ReadFile(PatchUtils::REGISTRY_FILENAME, UpdateInfoMemory))
	{
		m_Consol << "Can't read file " << PatchUtils::REGISTRY_FILENAME << " from zip. Zip error: " << m_PackFile.ErrorMessage() << std::endl;
		return false;
	}

	SFileInfo UpdateInfo;
	{
		FileInfoReader parser;
		parser.Open((const char*)UpdateInfoMemory.Get(), UpdateInfoMemory.GetSize());
		if (!parser.Build(UpdateInfo))
		{
			m_Consol << "Can't parse file " << PatchUtils::REGISTRY_FILENAME << ". parse error: " << m_PackFile.ErrorMessage() << std::endl;
			return false;
		}
		parser.Close();
	}

	m_Consol << "Parsed " << PatchUtils::REGISTRY_FILENAME << std::endl;
	//if (options.verbose)
	m_Consol << UpdateInfo.actions.size() << " actions in total." << std::endl;


	SFileInfo::VectAction rRollBack;
	bool result = false;
	try
	{
		result = ApplyPatchActions(UpdateInfo, rRollBack);
	}
	catch(const wchar_t * szMsg ){
		MessageBox(NULL, szMsg, L"PatchError", MB_OK);
	}


	if (result)
		m_Consol << "Successfully done!" << std::endl;

	try
	{
		RollBack(rRollBack, UpdateInfo.actions.size(), result);
	}
	catch (const wchar_t * szMsg) {
		MessageBox(NULL, szMsg, L"RollBackError", MB_OK);
	}

	m_PackFile.Close();

	if (m_bAutoDelete) PatchUtils::RemoveFile(strPackFile.c_str());

	return result;
}