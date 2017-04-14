// patchtest.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"


#include "XDeltaFolder.h"
#include "PatchPackage.h"
#include "FileMemory.h"
#include "PatchUnPack.h"
#include "PatchUtils.h"
#include "ZipArchiveOutput.h"
#include "ZipFolderTreeBuilder.h"


//#pragma comment( lib,"Seedzlib.lib")
//#pragma comment( lib,"xdelta.lib")
//#pragma  comment(lib, "RGDFUtils.lib")

void ApplyPatch(const char * szProcessFileName, int nIdx, int nMaxCount)
{
	printf("ApplyPatch : %s Sed : %d , MAX : %d \n", szProcessFileName, nIdx, nMaxCount);
}

int _tmain(int argc, _TCHAR* argv[])
{



	tstring NewFolder = _T("C:\\Users\\kyd\\Documents\\Table");
	tstring OldFolder = _T("C:\\Users\\kyd\\Documents\\Table_org");
	tstring Target = _T("C:\\Users\\kyd\\Documents\\test\\target\\bb.zip");

	tstring PatchOrgPath = _T("C:\\Users\\kyd\\Documents\\test\\org");

	tstring strExtract = _T("C:\\Users\\kyd\\Documents\\test\\seed");

	tstring strzip = _T("C:\\Users\\kyd\\Documents\\test\\seed\\t.zip");
	tstring strs = _T("C:\\Users\\kyd\\Documents\\test\\target\\FileUpdateInfo.txt");



	//tstring s1 = _T("C:\\Users\\kyd\\Documents\\test\\target\\1.zip");
	//tstring t1 = _T("C:\\Users\\kyd\\Documents\\test\\target\\1.zip.roll");

	//PatchUtils::filecopy(s1, t1);

	//tstring NewFolder = _T("C:\\Program Files (x86)\\RisingGames\\SeedWar_Dev\\Seedwar");
	//tstring OldFolder = _T("C:\\Users\\kyd\\Documents\\test\\org");
	//tstring Target = _T("C:\\Users\\kyd\\Documents\\test\\target\\SeedWar.zip");

	//tstring PatchOrgPath = _T("C:\\Users\\kyd\\Documents\\test\\org");


	//"C:\Users\kyd\Documents\Table" --old C:\Users\kyd\Documents\test\org --package C:\Users\kyd\Documents\test\target\1.zip

	tstring strTFolder = _T("C:\\Users\\kyd\\Documents\\test\\seed");
	tstring strSFolder = _T("C:\\Users\\kyd\\Documents\\test\\org");;

	ZipFolderTreeBuilder zipBuiler;
	zipBuiler.Build(strSFolder, strTFolder);

	return 0;
	bool bet = PatchUtils::ZipActionDo(strzip, strs);

	return 0;

	bool bret = PatchUtils::ZipExtractAll(Target, strExtract);

	return 0;

	XDeltaFolder XDeltaFolders;
	//XDeltaFolders.AddIgnoreFile(_T(".*\.xlsx"));
	XDeltaFolders.AddIgnoreFolder(_T("\.svn"));
	XDeltaFolders.BuildRoot(NewFolder, OldFolder);
	
	PatchPackage PatchPack(XDeltaFolders);
	PatchPack.Build(Target, NewFolder, OldFolder);

	return 0;
	PatchUnPack PatchApply;

	PatchApply.SetConsolOut(false);
	PatchApply.SetAutoDelete(true);
	PatchApply.SetApplyCallBackFunc(ApplyPatch);
	PatchApply.ApplyPatch(Target, PatchOrgPath);

	int n = 0;

	return 0;
}

