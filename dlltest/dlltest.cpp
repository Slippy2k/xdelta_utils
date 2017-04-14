// dlltest.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "XDeltaPatch.h"
#include <boost/program_options.hpp>

using namespace boost;
using namespace boost::program_options;

#include <iostream>
#include <fstream>
#include <string>

#include <exception>


class ApplySample
{
public:
	ApplySample(){}
	~ApplySample(){}

	void ApplyPatch(const std::string strPatchPack, const std::string strPatchTargetPath)
	{
		XDeltaPatch patch;

		std::wstring wstrPatchPack = patch.StringToWstring(strPatchPack.c_str());
		std::wstring wstrPatchTargetPath = patch.StringToWstring(strPatchTargetPath.c_str());

		DeltaPatchConfig config;
		config.m_bConsolOut = false;
		//config.m_bAutoDelete = true; //패치 패키지 파일이 자동 삭제 된다.
		config.m_bRollback = true;
		patch.SetApplyCofnig(config);

		//FuncPatchProcess funcType(&ApplySample::Apply, this);

		//patch.SetApplyFunction(funcType);
		patch.ApplyPatch(wstrPatchPack.c_str(), wstrPatchTargetPath.c_str(),(FuncPatchProcess) &ApplySample::Apply);
	}

	static void Apply(const char * szProcessFileName, const int nIdx, const  int nMaxCount)
	{
		printf("ApplyPatch : %s Sed : %d , MAX : %d \n", szProcessFileName, nIdx, nMaxCount);
	}

	static void CrcProcess(const char * szProcessFileName, const int nIdx, const  int nMaxCount,bool bEqual)
	{
		printf("Crc : %s Sed : %d , MAX : %d CHECK : %s \n", szProcessFileName, nIdx, nMaxCount, bEqual ? "TRUE" : "FALSE");
	}

	void FullCrcCheck(const std::string strCrcFile, const std::string strPatchTargetPath)
	{
		XDeltaPatch patch;
		std::wstring wstrCrcFile = patch.StringToWstring(strCrcFile.c_str());
		std::wstring wstrPatchTargetPath = patch.StringToWstring(strPatchTargetPath.c_str());

		//FuncCrcProcess funcType(&ApplySample::CrcProcess, this);
		int nRet = patch.CrcCheckStart(wstrCrcFile.c_str(), wstrPatchTargetPath.c_str(), (FuncCrcProcess)&ApplySample::CrcProcess);

		// 리턴 값
		//-1 : strCrcFile 로딩 실패
		
	}
};


bool CommandLineParse(const int argc, _TCHAR* argv[] )
	
{

	std::string  NewFolder;
	std::string  OldFolder;
	std::string  strPackFilePath;
	try
	{
		namespace po = boost::program_options;
		boost::program_options::options_description Options;
		Options.add_options()
			("help", "Patch Test Program")
			("apply", "패치 적용. 아래 옵션을 줘야만 실행된다")
			("packfile", po::value<std::string>(), "패치파일")
			("tfolder", po::value<std::string>(), "패치 적용할 대상경로")

			("crc", "CRC 체크 아래 옵션을 줘야만 실행된다")
			("file", po::value<std::string>(), "crc 정보 파일")
			("folder", po::value<std::string>(), "crc 체크할 대상 폴더")
			;

		po::variables_map vm;

		po::store(boost::program_options::parse_command_line(argc, argv, Options), vm);

		if (vm.count("help") || argc <= 1)
		{
			std::cout << "Basic Command Line Parameter App" << std::endl
				<< Options << std::endl;
			return true;
		}

		if (vm.count("apply"))
		{
			if (vm.count("packfile"))
			{
				std::cout << "packfile : " << vm["packfile"].as<std::string>() << std::endl;
				OldFolder = vm["packfile"].as<std::string>();
			}
			if (vm.count("tfolder"))
			{
				std::cout << "tfolder : " << vm["tfolder"].as<std::string>() << std::endl;
				NewFolder = vm["tfolder"].as<std::string>();
			}

			ApplySample test;
			test.ApplyPatch(OldFolder, NewFolder);

		}

		if (vm.count("crc"))
		{
			if (vm.count("file"))
			{
				std::cout << "file : " << vm["file"].as<std::string>() << std::endl;
				OldFolder = vm["file"].as<std::string>();
			}
			if (vm.count("folder"))
			{
				std::cout << "folder: " << vm["folder"].as<std::string>() << std::endl;
				NewFolder = vm["folder"].as<std::string>();
			}

			ApplySample test;
			test.FullCrcCheck(OldFolder, NewFolder);

		}
		po::notify(vm);

	}
	catch (std::exception& e)
	{
		::printf("Option-Loading Failed : %s\r\n", e.what());
		return false;
	}

	return true;
}


int _tmain(int argc, _TCHAR* argv[])
{

	//ApplySample test;

	//std::wstring strPatchPack = _T("C:\\Users\\kyd\\Documents\\test\\target\\bb.zip");

	//std::wstring strPatchTargetPath = _T("C:\\Users\\kyd\\Documents\\test\\org");

	//XDeltaPatch patch;

	//DeltaPatchConfig config;
	//config.m_bConsolOut = false;
	////config.m_bAutoDelete = true; //패치 패키지 파일이 자동 삭제 된다.
	//patch.SetApplyCofnig(config);


	//FuncPatchProcess dg1(&ApplySample::Apply, &test);

	//patch.SetApplyFunction(dg1);
	//patch.ApplyPatch(strPatchPack, strPatchTargetPath);

	//test.ApplyPatch();
	//test.FullCrcCheck();
	CommandLineParse(argc,argv);
	return 0;
}

