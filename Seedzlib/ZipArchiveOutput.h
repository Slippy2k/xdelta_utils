#pragma once


namespace zip
{

	namespace CompressionLevel
	{
		enum Enum
		{
			  NO_COMPRESSION         = 0
			, BEST_SPEED             = 1
			, BEST_COMPRESSION       = 9
			, DEFAULT_COMPRESSION    = -1
		};
	}

	class ZipArchiveOutput
	{
	public:
		ZipArchiveOutput();
		~ZipArchiveOutput();

		bool Open(tstring const& archiveName, bool utf8fileNames, int compressionLevel);
		bool WriteFile(tstring const& fileName, const void* pMemoryBlock, size_t size);
		bool Close();

		tstring ErrorMessage() const;
		void SetPasswd(const std::string strPasswd){ m_strPasswd = strPasswd; }
	private:
		tstring m_archiveName;
		bool m_utf8fileNames;
		
		tstringstream m_errorMessage;
		int err;
		const char* password;
		std::string m_strPasswd;
		int opt_compress_level;

		typedef void* ZipFile_t;
		ZipFile_t zf;
	};
}

