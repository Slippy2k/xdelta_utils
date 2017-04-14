#include "XDeltaEncoder.h"
#include "xdelta3.h"
#include <sys/stat.h>
//#include "xdelta3.c"
#include "PatchUtils.h"

XDeltaEncoder::XDeltaEncoder()
{
	//m_config.compression = 9;
}


XDeltaEncoder::~XDeltaEncoder()
{
}

int XDeltaEncoder::MakeFlags(XDeltaConfig const& config)
{
	int flags = 0;

	if (config.DJW) flags |= XD3_SEC_DJW;
	if (config.FGK) flags |= XD3_SEC_FGK;
	if (config.LZMA) flags |= XD3_SEC_LZMA;

	if (config.nodata) flags |= XD3_SEC_NODATA;
	if (config.noinst) flags |= XD3_SEC_NOINST;
	if (config.noaddr) flags |= XD3_SEC_NOADDR;

	if (config.adler32) flags |= XD3_ADLER32;
	if (config.adler32_nover) flags |= XD3_ADLER32_NOVER;

	if (config.beGreedy) flags |= XD3_BEGREEDY;

	if (config.compression != 0) flags |= ((config.compression << XD3_COMPLEVEL_SHIFT) & XD3_COMPLEVEL_MASK);

	return flags;
}

bool XDeltaEncoder::EncodeDiffMemoryBlock(const BYTE* newBlock, size_t newSize, const BYTE* oldBlock, size_t oldSize, FileMemory & rMemBlock)
{
	const size_t reservedSize = max(newSize, 1024);

	rMemBlock.SetSize(reservedSize);
	rMemBlock.New();
	//diffBlock = new BYTE[reservedSize];
	usize_t diffSize = 0;

	int flags = MakeFlags(m_config);
	m_errorCode = xd3_encode_memory(newBlock, newSize, oldBlock, oldSize, rMemBlock.Get(), &diffSize, reservedSize, flags);

	rMemBlock.SetSize(diffSize);

	return m_errorCode == 0;
}


bool XDeltaEncoder::EncodeDiffMemoryBlock(const BYTE* newBlock, size_t newSize, const BYTE* oldBlock, size_t oldSize, BYTE * pMemBlock, size_t & outSize)
{
	usize_t diffSize = 0;
	int flags = MakeFlags(m_config);
	m_errorCode = xd3_encode_memory(newBlock, newSize, oldBlock, oldSize, pMemBlock, &diffSize, outSize, flags);

	outSize = diffSize;

	return m_errorCode == 0;

}

bool XDeltaEncoder::EncodeDiffStream(tstring strOrgFile, tstring strNewFile, tstring strOutFile, int nBuffSize)
{
	bool bRet = true;
	FileDescriptor fdOrg, fdNew, fdOut;

	fdOrg.Open(strOrgFile, "rb");
	fdNew.Open(strNewFile, "rb");
	fdOut.Open(strOutFile, "wb");

	fdOrg.SetBlockSize(XD3_ALLOCSIZE);
	fdNew.SetBlockSize(XD3_ALLOCSIZE);
	fdOut.SetBlockSize(XD3_ALLOCSIZE);


	if (!EncodeDiffStream(fdOrg, fdNew, fdOut, nBuffSize))
	{
		bRet = false;
	}

	return bRet;
}

bool XDeltaEncoder::EncodeDiffStream(FileDescriptor & rOrgFile, FileDescriptor & rpNewFile, FileDescriptor & rpOutFile, int nBuffSize)
{
	int r, ret;
	struct stat statbuf;
	xd3_stream stream;
	xd3_config config;
	xd3_source source;
	void* Input_Buf;
	int Input_Buf_Read;

	if (nBuffSize < XD3_ALLOCSIZE) 	nBuffSize = XD3_ALLOCSIZE;

	memset(&stream, 0, sizeof(stream));
	memset(&source, 0, sizeof(source));

	xd3_init_config(&config, XD3_ADLER32);
	config.winsize = nBuffSize;
	xd3_config_stream(&stream, &config);

	if (rOrgFile.Get())
	{
		r = fstat(_fileno(rOrgFile.Get()), &statbuf);
		if (r)
		{
			m_errorCode = r;
			return false;
		}

		source.blksize = nBuffSize;
		source.curblk = (const uint8_t *)malloc(source.blksize);

		/* Load 1st block of stream. */
		r = rOrgFile.Seek();
		if (r)
		{
			m_errorCode = r;
			return false;
		}
		//source.onblk = fread((void*)source.curblk, 1, source.blksize, fpOrgFile);
		source.onblk = rOrgFile.BlockRead((char*)source.curblk);
		source.curblkno = 0;
		/* Set the stream. */
		xd3_set_source(&stream, &source);
	}

	Input_Buf = malloc(nBuffSize);

	rpNewFile.Seek();
	do
	{
		//Input_Buf_Read = fread(Input_Buf, 1, nBuffSize, fpNewFile);
		Input_Buf_Read = rpNewFile.BlockRead((char *)Input_Buf);
		//fprintf(stderr, "Read Size: %d , BlockSize : %d\n", Input_Buf_Read, nBuffSize);
		if (Input_Buf_Read < nBuffSize)
		{
			xd3_set_flags(&stream, XD3_FLUSH | stream.flags);
		}
		xd3_avail_input(&stream,(const uint8_t*) Input_Buf, Input_Buf_Read);

	process:
		ret = xd3_encode_input(&stream);

		switch (ret)
		{
		case XD3_INPUT:
		{
			//fprintf(stderr, "XD3_INPUT\n");
			continue;
		}

		case XD3_OUTPUT:
		{
			//fprintf(stderr, "XD3_OUTPUT\n");
			//r = fwrite(stream.next_out, 1, stream.avail_out, fpOutFile);
			r = rpOutFile.Write(stream.avail_out,(char *) stream.next_out);
			if (r != (int)stream.avail_out)
			{
				m_errorCode = r;
				return false;
			}
			xd3_consume_output(&stream);
			goto process;
		}

		case XD3_GETSRCBLK:
		{
			//fprintf(stderr, "XD3_GETSRCBLK %qd\n", source.getblkno);
			if (rOrgFile.Get())
			{
				//r = fseek(fpOrgFile, source.blksize * source.getblkno, SEEK_SET);
				r = rOrgFile.Seek(SEEK_SET, source.blksize * source.getblkno);
				if (r)
				{
					m_errorCode = r;
					return false;
				}
				//source.onblk = fread((void*)source.curblk, 1,source.blksize, fpOrgFile);
				source.onblk = rOrgFile.BlockRead((char*)source.curblk);
				source.curblkno = source.getblkno;
			}
			goto process;
		}

		case XD3_GOTHEADER:
		{
			//fprintf(stderr, "XD3_GOTHEADER\n");
			goto process;
		}

		case XD3_WINSTART:
		{
			//fprintf(stderr, "XD3_WINSTART\n");
			goto process;
		}

		case XD3_WINFINISH:
		{
			//fprintf(stderr, "XD3_WINFINISH\n");
			goto process;
		}

		default:
		{
			//fprintf(stderr, "!!! INVALID %s %d !!!\n",
			//	stream.msg, ret);
			m_errorCode = ret;
			return false;
		}

		}

	} while (Input_Buf_Read == nBuffSize);

	free(Input_Buf);

	free((void*)source.curblk);
	xd3_close_stream(&stream);
	xd3_free_stream(&stream);

	return true;
}

void XDeltaEncoder::GetErrorMessage(tstring & errorMessage) const
{
	if (m_errorCode == 0)
		return;

	errorMessage = _T("Can't encode memory with xdelta. Error code: ");
	errorMessage += m_errorCode;

}

