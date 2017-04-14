// XDeltaBuilder.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "stdAfxZlib.h"

#include "XDeltaFolder.h"
#include "PatchPackage.h"
#include "PatchUtils.h"
#include "ZipFolderTreeBuilder.h"
#include <boost/program_options.hpp>

using namespace boost;
using namespace boost::program_options;

#include <iostream>
#include <fstream>
#include <string>

#include <exception>

#include "PatchUnPack.h"
#include "PatchHashChecker.h"


using namespace std;




void tokenize(const string& str, vector<string>& tokens, const string& delimiters = ",")
{
	// Skip delimiters at beginning.
	string::size_type lastPos = str.find_first_not_of(delimiters, 0);

	// Find first non-delimiter.
	string::size_type pos = str.find_first_of(delimiters, lastPos);

	while (string::npos != pos || string::npos != lastPos) {
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));

		// Skip delimiters.
		lastPos = str.find_first_not_of(delimiters, pos);

		// Find next non-delimiter.
		pos = str.find_first_of(delimiters, lastPos);
	}
}

bool CommandLineParse(const int argc, _TCHAR* argv[],
						std::string & NewFolder, std::string & OldFolder, std::string & strPackFilePath, 
						std::vector<std::string> & vecIgNoreFiles, std::vector<std::string> & vecIgNoreFolder)
{
	try
	{
		namespace po = boost::program_options;
		boost::program_options::options_description Options;
		Options.add_options()
			("help", "XDeltaBuilder Command Help")
			("patch", "패치 파일 만든다 아래 옵션은 이 옵션을 줘야만 실행된다")
			("oldpath", po::value<std::string>(), "패치 오리지널 경로")
			("newpath", po::value<std::string>(), "패치 할 예정인 대상경로")
			("igfs", po::value<std::string>(), "제외 파일 목록 정규 표현식으로 표현 , 로 여러개를 넣을수 있다 ex :.*\.dll,.*\.log ,test.cgr")
			("igds", po::value<std::string>(), "제외할 폴더 목록 형식은 파일 목록과 같음")
			("packfile", po::value<std::string>(), "생성할 패치 파일명 ( full path)")

			("fzip", "CRC 체크해서 파손된 파일을 복구하기위해 만드는 데이터 아래 옵션 넣어줘야한다.")
			("sfolder", po::value<std::string>(), "데이터 생성 소스 폴더 ex: c:\mytest\source")
			("tfolder", po::value<std::string>(), "데이터 생성 타겟 폴더 ex: c:\mytest\target")
			("zigfs", po::value<std::string>(), "제외 파일 목록 정규 표현식으로 표현 , 로 여러개를 넣을수 있다 ex :.*\.dll,.*\.log ,test.cgr")
			("zigds", po::value<std::string>(), "제외할 폴더 목록 형식은 파일 목록과 같음")

			("apply", "패치 적용. 아래 옵션을 줘야만 실행된다")
			("pkfile", po::value<std::string>(), "패치파일")
			("pfolder", po::value<std::string>(), "패치 적용할 대상경로")

			("crc", "CRC 체크 아래 옵션을 줘야만 실행된다")
			("file", po::value<std::string>(), "crc 정보 파일")
			("folder", po::value<std::string>(), "crc 체크할 대상 폴더")

			("mapply", "수동으로 패치 하는 옵션")
			("start", po::value<std::int32_t>(), "패치 시작 번호")
			("end", po::value<std::int32_t>(), "패치 마지막 번호")
			("pkfolder", po::value<std::string>(), "패치 파일 위치 폴더")
			("targetfolder", po::value<std::string>(), "패치 대상 폴더")
			;
		po::variables_map vm;

		po::store(boost::program_options::parse_command_line(argc, argv, Options), vm);

		if (vm.count("help") || argc <= 1)
		{
			std::cout << "Basic Command Line Parameter App" << std::endl
				<< Options << std::endl;
			return true;
		}

		if (vm.count("patch"))
		{
			if (vm.count("oldpath"))
			{
				std::cout << "Old Path : " << vm["oldpath"].as<std::string>() << std::endl;
				OldFolder = vm["oldpath"].as<std::string>();
			}
			if (vm.count("newpath"))
			{
				std::cout << "New Path : " << vm["newpath"].as<std::string>() << std::endl;
				NewFolder = vm["newpath"].as<std::string>();
			}

			if (vm.count("igfs"))
			{
				std::cout << "New Path : " << vm["igfs"].as<std::string>() << std::endl;

				tokenize(vm["igfs"].as<std::string>(), vecIgNoreFiles);

			}
			if (vm.count("igds"))
			{
				std::cout << "New Path : " << vm["igds"].as<std::string>() << std::endl;
				tokenize(vm["igds"].as<std::string>(), vecIgNoreFolder);
			}
			if (vm.count("packfile"))
			{
				std::cout << "New Path : " << vm["packfile"].as<std::string>() << std::endl;
				strPackFilePath = vm["packfile"].as<std::string>();
			}

			XDeltaFolder XDeltaFolders;
			XDeltaFolders.AddIgnoreFile(vecIgNoreFiles);
			XDeltaFolders.AddIgnoreFolder(vecIgNoreFolder);
			XDeltaFolders.BuildRoot(NewFolder, OldFolder);

			PatchPackage PatchPack(XDeltaFolders);
			PatchPack.Build(strPackFilePath, NewFolder, OldFolder);
		}

		if (vm.count("fzip"))
		{
			if (vm.count("sfolder"))
			{
				std::cout << "sfolder : " << vm["sfolder"].as<std::string>() << std::endl;
				OldFolder = vm["sfolder"].as<std::string>();
			}
			if (vm.count("tfolder"))
			{
				std::cout << "tfolder: " << vm["tfolder"].as<std::string>() << std::endl;
				NewFolder = vm["tfolder"].as<std::string>();
			}

			if (vm.count("zigfs"))
			{
				std::cout << "zigfs : " << vm["zigfs"].as<std::string>() << std::endl;

				tokenize(vm["zigfs"].as<std::string>(), vecIgNoreFiles);

			}
			if (vm.count("zigds"))
			{
				std::cout << "zigds : " << vm["zigds"].as<std::string>() << std::endl;
				tokenize(vm["zigds"].as<std::string>(), vecIgNoreFolder);
			}
			ZipFolderTreeBuilder zipBuiler;
			zipBuiler.AddIgnoreFile(vecIgNoreFiles);
			zipBuiler.AddIgnoreFolder(vecIgNoreFolder);
			zipBuiler.Build(OldFolder, NewFolder);

		}

		if (vm.count("apply"))
		{
			if (vm.count("pkfile"))
			{
				std::cout << "pkfile : " << vm["pkfile"].as<std::string>() << std::endl;
				OldFolder = vm["pkfile"].as<std::string>();
			}
			if (vm.count("pfolder"))
			{
				std::cout << "pfolder : " << vm["pfolder"].as<std::string>() << std::endl;
				NewFolder = vm["pfolder"].as<std::string>();
			}

			PatchUnPack PatchApply;

			PatchApply.SetConsolOut(true);
			PatchApply.SetAutoDelete(false);
			PatchApply.SetCheckFileSize(true);
			PatchApply.SetCheckHash(true);
			PatchApply.SetRollBack(true);
			PatchApply.SetStopOnError(true);

			return PatchApply.ApplyPatch(OldFolder, NewFolder);
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

			PatchHashChecker Checker;
			return Checker.Build(OldFolder, NewFolder);

		}

		if (vm.count("mapply"))
		{
			int nstart, nend;
			if (vm.count("start"))
			{
				std::cout << "file : " << vm["start"].as<std::int32_t>() << std::endl;
				nstart = vm["start"].as<std::int32_t>();
			}
			if (vm.count("end"))
			{
				std::cout << "folder: " << vm["end"].as<std::int32_t>() << std::endl;
				nend = vm["end"].as<std::int32_t>();
			}
			if (vm.count("pkfolder"))
			{
				std::cout << "pkfile : " << vm["pkfolder"].as<std::string>() << std::endl;
				OldFolder = vm["pkfolder"].as<std::string>();
			}
			if (vm.count("targetfolder"))
			{
				std::cout << "pfolder : " << vm["targetfolder"].as<std::string>() << std::endl;
				NewFolder = vm["targetfolder"].as<std::string>();
			}

			for (int i = nstart; i < nend + 1; i++)
			{
				std::cout << "start patch number : " << i << std::endl;
				PatchUnPack PatchApply;

				PatchApply.SetConsolOut(true);
				PatchApply.SetAutoDelete(false);
				PatchApply.SetCheckFileSize(true);
				PatchApply.SetCheckHash(true);
				PatchApply.SetRollBack(true);
				PatchApply.SetStopOnError(true);
				std::string strPatchFile = OldFolder;
				strPatchFile += "\\";
				strPatchFile += boost::lexical_cast<string>(i);
				strPatchFile += ".Patch";

				bool bRet = PatchApply.ApplyPatch(strPatchFile, NewFolder);
				if (bRet == false)
				{
					std::cout << "patch error number : " << i << std::endl;
					break;
				}
			}


			return true;

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
	std::vector<std::string> vecIgNoreFiles;
	std::vector<std::string> vecIgNoreFolder;
	std::string NewFolder, OldFolder,strPackFilePath;

	bool bRet = CommandLineParse(argc, argv, NewFolder, OldFolder, strPackFilePath, vecIgNoreFiles, vecIgNoreFolder);
	if (false == bRet) return -1;
	return 0;
}

