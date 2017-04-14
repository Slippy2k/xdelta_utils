#include "PatchHashChecker.h"
#include "PatchUtils.h"


PatchHashChecker::PatchHashChecker()
{
}


PatchHashChecker::~PatchHashChecker()
{
}

bool PatchHashChecker::LoadInfoFile()
{
	FileMemory UpdateInfoMemory;
	UpdateInfoMemory.ReadFile(m_UpdateInfoTxt);

	FileInfoReader parser;
	parser.Open((const char*)UpdateInfoMemory.Get(), UpdateInfoMemory.GetSize());
	if (!parser.Build(m_UpdateInfo))
	{
		return false;
	}
	parser.Close();

	return true;
}

int PatchHashChecker::HashActionDo()
{

	int nProcessCount = 0;
	int nMaxCount = m_UpdateInfo.actions.size();

	for each (auto const rFileInfo  in m_UpdateInfo.actions )
	{
		tstring tpath = rFileInfo->old_path == _T("") ? rFileInfo->new_path : rFileInfo->old_path;

		//액션이 삭제인 경우 파일이 존재하지 않아 CRC를 할필요가 없다.
		if (rFileInfo->action == RDFAction::EAction::REMOVE) continue;

		FileMemory oldFile;
		tstring fullPathOld = PatchUtils::FullPath(m_CrcTargetFolder, tpath);
		bool bCheck = false;
		if (oldFile.ReadFile(fullPathOld))
		{
			bCheck = HashCompare(oldFile, rFileInfo);
		}

#ifdef _UNICODE
		std::string strProcess = PatchUtils::WstringToString(tpath);
#else
		std::string strProcess = tpath;
#endif
		m_HashProcessFunc ? m_HashProcessFunc(strProcess.c_str(), ++nProcessCount, nMaxCount, bCheck) : NULL;

	}
	return 0;
}


bool PatchHashChecker::HashCompare(const FileMemory& oldFile, const SUpdateAction * action)
{
	int nCheckSize = action->newSize == -1 ? action->oldSize : action->newSize;
	Sha1 ChkSha1 = action->newSize == -1 ? action->oldSha1 : action->newSha1;
	if (oldFile.GetSize() != nCheckSize)
	{
		return false;
	}

	Sha1 oldSha1;
	SHA1Compute(oldFile.GetConst(), oldFile.GetSize(), oldSha1);
	if (oldSha1 != ChkSha1)
	{
		return false;
	}
	return true;
}

int PatchHashChecker::Build(const std::wstring strUpdateInfoTxt, const std::wstring strCrcTargetFolder, FuncCrcProcess  HashProcessFunction)
{
	m_UpdateInfoTxt = strUpdateInfoTxt;
	m_CrcTargetFolder = strCrcTargetFolder;
	m_HashProcessFunc = HashProcessFunction;

	if (!LoadInfoFile()) return -1;

	return HashActionDo();
}

int PatchHashChecker::Build(const std::string strUpdateInfoTxt, const std::string strCrcTargetFolder, FuncCrcProcess  HashProcessFunction)
{
	std::wstring txt = PatchUtils::StringToWstring(strUpdateInfoTxt);
	std::wstring Target = PatchUtils::StringToWstring(strCrcTargetFolder);


	return Build(txt, Target);
}
