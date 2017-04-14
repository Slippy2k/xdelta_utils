#include "XDeltaDecoder.h"
#include <assert.h>
//#include "xdelta3.c"
#include "PatchUtils.h"
#include <sys/stat.h>


XDeltaDecoder::XDeltaDecoder()
{
}


XDeltaDecoder::~XDeltaDecoder()
{
}

bool XDeltaDecoder::DecodeDiffMemoryBlock(const BYTE* oldBlock, size_t oldSize, const BYTE* diffBlock, size_t diffSize, FileMemory & rMemBlock)
{

	size_t reservedSize = rMemBlock.GetSize();
	//newBlock = new BYTE[reservedSize];
	size_t newSize = rMemBlock.GetSize();

	rMemBlock.New();

	m_errorCode = xd3_decode_memory(diffBlock, diffSize, oldBlock, oldSize, rMemBlock.Get(), (usize_t *)&newSize, reservedSize, 0);

	assert(reservedSize == newSize);

	return m_errorCode == 0;

}

bool XDeltaDecoder::DecodeDiffStream(tstring strDiffFile, tstring strOrgFile, tstring strOutFile, int & nOutSize)
{
	FileDescriptor fdOrg, fdDiff, fdOut;
	bool bRet = true;

	//FileMemory OrgFile;
	//OrgFile.ReadFile(strOrgFile);
	if (false == fdOrg.Open(strOrgFile, "rb")) return false;
	if (false == fdDiff.Open(strDiffFile, "rb")) return false;
	if (false == fdOut.Open(strOutFile, "wb")) return false;

	fdOrg.SetBlockSize(XD3_ALLOCSIZE);
	fdDiff.SetBlockSize(XD3_ALLOCSIZE);
	fdOut.SetBlockSize(XD3_ALLOCSIZE);
	

	if (!DecodeDiffStream(fdDiff, fdOrg, fdOut, nOutSize))
	//if (!DecodeDiffStream(fdDiff, OrgFile, fdOut))
	{
		bRet = false;
	}

	return bRet;
}

//bool XDeltaDecoder::DecodeDiffStream(FileDescriptor & rDiff, const FileMemory & rOrgMemBlock, FileDescriptor & rOut)
bool XDeltaDecoder::DecodeDiffStream(FileDescriptor & rDiff, FileDescriptor & rOrg, FileDescriptor & rOut, int & nOutSize)
{
	int ret;
	const int  block_size = rDiff.GetBlockSize();
	//int nSourceSize = rOrgMemBlock.GetSize();
	int nSourceSize = rOrg.GetSize();
	xd3_stream stream;
	xd3_config config;
	xd3_source source;
	
	char * input_buffer = (char *)_malloca(block_size);
	int nread;
	
	int input_pos;
	
	memset(&stream, 0, sizeof(stream));
	memset(&config, 0, sizeof(config));
	memset(&source, 0, sizeof(source));
	
	xd3_init_config(&config, XD3_ADLER32);
	config.winsize = block_size;
	ret = xd3_config_stream(&stream, &config);
	
	if (ret != 0)
	{
		fprintf(stderr, "xd3_config_stream failed: %d\n", ret);
		return false;
	}
	
	source.ioh = rOrg.Get();
	source.blksize = block_size;
	//source.size = nSourceSize;
	//source.blksize = nSourceSize;
	
	//source.curblk = rOrgMemBlock.GetConst();
	source.curblk = (const uint8_t *)malloc(source.blksize);
	source.onblk = rOrg.BlockRead((char*)source.curblk);
	source.curblkno = 0;
	
	//ret = xd3_set_source_and_size(&stream, &source, nSourceSize);
	ret = xd3_set_source(&stream, &source);
	
	if (ret != 0)
	{
		fprintf(stderr, "xd3_set_source failed: %d\n", ret);
		return false;
	}
	
	do
	{
		input_pos = rDiff.GetPos();
	
		//nread = fread(input_buffer, 1, block_size, fpOrgwFile);
		nread = rDiff.BlockRead(input_buffer);
		if (nread == -1)
		{
			fprintf(stderr, "fread failed (input)\n");
			ret = -1;
			break;
		}
		else
		{
			fprintf(stderr, "read %d bytes from input at byte %d\n", nread, input_pos);
		}
		if (nread < block_size)
		{
			xd3_set_flags(&stream, XD3_FLUSH | stream.flags);
		}

		if ((ret = DoStreamProcess(&stream, input_buffer, nread, rDiff, rOut, nOutSize)))
			break;
	} while (nread == block_size);
	
	if (ret == 0)
		ret = xd3_close_stream(&stream);
	else
		xd3_abort_stream(&stream);
	
	xd3_free_stream(&stream);
	
	return ret == 0;
}


void XDeltaDecoder::GetErrorMessage(tstring& errorMessage) const
{
	if (m_errorCode == 0)
		return;

	errorMessage = _T("Can't decode memory with xdelta. Error code: ");
	errorMessage += m_errorCode;
}



int XDeltaDecoder::DoStreamProcess(xd3_stream *stream, char * szBuffer, int nInputSize, FileDescriptor & rDiff, FileDescriptor & rOut, int & nOutPutSize)
{
	int ret;
	int block_size = rDiff.GetBlockSize();
	xd3_source *source;
	int nread, nwrite;

	xd3_avail_input(stream, (unsigned char *)szBuffer, nInputSize);

	while (1)
	{
		ret = xd3_decode_input(stream);

		if (ret == XD3_INPUT)
			return 0;

		if (ret == XD3_OUTPUT)
		{
			nwrite = rOut.Write(stream->avail_out,(char *) stream->next_out);
			if (nwrite == -1)
			{
				fprintf(stderr, "fwrite failed\n");
				return -1;
			}
			else
			{
				xd3_consume_output(stream);
				nOutPutSize += nwrite;
				fprintf(stderr, "wrote %d bytes\n", nwrite);
			}
		}
		else if (ret == XD3_GETSRCBLK)
		{
			fprintf(stderr, "xd3_decode_input XD3_GETSRCBLK: %d\n", ret);
			source = stream->src;

			fprintf(stderr, "XD3_GETSRCBLK %qd\n", source->getblkno);
			if (source)
			{
				int r = fseek((FILE *)source->ioh, source->blksize * source->getblkno, SEEK_SET);
				if (r)
					return r;
				source->onblk = fread((void*)source->curblk, 1,
					source->blksize, (FILE *)source->ioh);
				source->curblkno = source->getblkno;
			}

		}
		else if (ret != XD3_GOTHEADER && ret != XD3_WINSTART && ret != XD3_WINFINISH)
		{
			fprintf(stderr, "xd3_decode_input failed: %d\n", ret);
			return -1;
		}
	}
}