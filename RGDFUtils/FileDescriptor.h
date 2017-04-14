#pragma once
#include "stdAfxZlib.h"
class FileDescriptor
{
public:
	FileDescriptor();
	~FileDescriptor();
	void SetBlockSize(const int nBlocSize = 0x1000){ m_nBlockSize = nBlocSize; }
	int  GetBlockSize() { return m_nBlockSize; }
	bool Open(tstring strFilePath,const char * cszOption );
	int  Read(const int nReadSize,char * szOutBuffer);
	int  Write(const int nWriteSize, const char * szWriteBuffer);
	int  BlockRead(char * szOutBuffer);
	int  BlockWrite(const char * szWriteBuffer);
	int  GetSize();

	int  Seek(const int nPOS = SEEK_SET, const int nOption = 0);
	int GetPos(){ return ftell(m_fpFile); }
	void Close();
	FILE * Get() { return m_fpFile; }
private:
	FILE *	m_fpFile;
	int		m_nBlockSize;
};

