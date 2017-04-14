#pragma once
#include "stdAfxZlib.h"
#include "FileMemory.h"
#include "FileDescriptor.h"
struct XDeltaConfig
{
	int compression;

	bool DJW;
	bool FGK;
	bool LZMA;

	bool nodata;
	bool noinst;
	bool noaddr;

	bool adler32;
	bool adler32_nover;

	bool beGreedy;
};

class XDeltaEncoder
{
public:
	XDeltaEncoder();
	~XDeltaEncoder();

	int MakeFlags(XDeltaConfig const& config);
	bool EncodeDiffMemoryBlock(const BYTE* newBlock, size_t newSize, const BYTE* oldBlock, size_t oldSize, FileMemory & rMemBlock);
	bool EncodeDiffMemoryBlock(const BYTE* newBlock, size_t newSize, const BYTE* oldBlock, size_t oldSize, BYTE * pMemBlock, size_t & outSize);
	bool EncodeDiffStream(tstring strOrgFile, tstring strNewFile, tstring strOutFile, int nBuffSize);
	bool EncodeDiffStream(FileDescriptor & rOrgFile, FileDescriptor & rpNewFile, FileDescriptor & rpOutFile, int nBuffSize);
	void GetErrorMessage(tstring & errorMessage) const;
private:

	XDeltaConfig	m_config;
	int				m_errorCode;
};

