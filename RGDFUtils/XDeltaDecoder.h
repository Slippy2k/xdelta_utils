#pragma once

#include "stdAfxZlib.h"
#include "xdelta3.h"
#include "FileMemory.h"
#include "FileDescriptor.h"


class XDeltaDecoder
{
public:
	XDeltaDecoder();
	~XDeltaDecoder();
	bool DecodeDiffMemoryBlock(const BYTE* oldBlock, size_t oldSize, const BYTE* diffBlock, size_t diffSize, FileMemory & rMemBlock);
	bool DecodeDiffStream(tstring strDiffFile, tstring strOrgFile, tstring strOutFile, int & nOutSize);
	//bool DecodeDiffStream(FileDescriptor & rDiff, const FileMemory & rOrgMemBlock, FileDescriptor & rOut);
	bool DecodeDiffStream(FileDescriptor & rDiff, FileDescriptor & rOrg, FileDescriptor & rOut,int & nOutSize);
	void GetErrorMessage(tstring& errorMessage) const;
private:
	int DoStreamProcess(xd3_stream *stream, char * szBuffer, int nInputSize, FileDescriptor & rDiff, FileDescriptor & rOut, int & nOutPutSize);

private:

	int m_errorCode;

};

