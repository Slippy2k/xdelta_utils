#pragma once

#include "FileMemory.h"
namespace zip
{

	class ZipArchiveInput
	{
	public:
		ZipArchiveInput();
		~ZipArchiveInput();

		bool Open(tstring const& archiveName, bool caseSensitive);
		bool Close();

		bool LocateAndReadFile(tstring const& fileName, BYTE*& pMemoryBlock, size_t& size);
		bool ReadFile(tstring const& fileName, BYTE*& pMemoryBlock, size_t& size);
		bool ReadFile(tstring const& fileName, FileMemory & rMemory);
		bool ExtractAll(const tstring strTargetpath, std::function<bool(const tstring, FileMemory &)> CallBack);
		tstring ErrorMessage() const;
		void SetPasswd(const std::string strPasswd){ m_strPasswd = strPasswd; }
	private:
		bool Index();
		bool ReadCurrentFile(tstring const& fileName, BYTE*& pMemoryBlock, size_t& size);
		bool ReadCurrentFile(tstring const& fileName, FileMemory & rMemory);

		struct ZipEntry
		{
			unsigned long pos_in_zip_directory;   // offset in zip file directory
			unsigned long num_of_file;            // # of file
			tstring	 strFileName;
		};

		typedef std::unordered_map< tstring, ZipEntry > NameToEntry_t;
		NameToEntry_t m_nameToEntry;

		tstring m_archiveName;
		const char* password;
		std::string m_strPasswd;
		typedef void* UnzipFile_t;
		UnzipFile_t uf;

		tstringstream m_errorMessage;
		bool m_caseSensitive;
	};

	int ZipCreateDirectory( const char* path );
	bool ZipCreateDirectories( const char* path );
}
