#include "stdAfxZlib.h"
#include "FileDescriptor.h"
#include "PatchUtils.h"
#include "xdelta3.h"


FileDescriptor::FileDescriptor()
{
	SetBlockSize(XD3_ALLOCSIZE);
}


FileDescriptor::~FileDescriptor()
{
	Close();
}

bool FileDescriptor::Open(tstring strFilePath, const char * cszOption)
{
	std::string strPath = PatchUtils::WstringToString(strFilePath);

	m_fpFile = fopen(strPath.c_str(), cszOption);

	return m_fpFile != nullptr;
}

int  FileDescriptor::Read(const int nReadSize, char * szOutBuffer)
{
	return fread((void*)szOutBuffer, 1, nReadSize, m_fpFile);
}
int  FileDescriptor::Write(const int nWriteSize, const char * szWriteBuffer)
{
	return fwrite(szWriteBuffer, 1, nWriteSize, m_fpFile);
}

int  FileDescriptor::Seek(const int nPOS,const int nOption)
{
	return fseek(m_fpFile, nOption, nPOS);
}

void FileDescriptor::Close()
{
	if (m_fpFile) fclose(m_fpFile);
}

int FileDescriptor::BlockRead(char * szOutBuffer)
{
	return Read(m_nBlockSize, szOutBuffer);
}

int  FileDescriptor::BlockWrite(const char * szWriteBuffer)
{
	return Write(m_nBlockSize, szWriteBuffer);
}

int  FileDescriptor::GetSize()
{
	Seek(SEEK_END);
	int nLen = ftell(m_fpFile);
	Seek(SEEK_SET);
	return nLen;
}
