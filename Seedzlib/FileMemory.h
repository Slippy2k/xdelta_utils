#pragma once

#include "stdAfxZlib.h"

class FileMemory
{
public:
	FileMemory();
	~FileMemory();
	FileMemory(size_t tsize);
	FileMemory(void* pBlock, size_t tsize);
	FileMemory(const FileMemory & rFileMemoery);
	FileMemory(const std::string & rstrSource);

	bool ReadFile(tstring const& fullPath);
	bool ReadFile(std::ifstream & file);

	bool WriteFile(tstring const& fullPath);
	bool WriteFile(std::ofstream& out);


	bool ReadFile(tstring const& fullPath, FileMemory& memoryBlock);
	bool ReadFile(std::ifstream & file, FileMemory& memoryBlock);

	bool WriteFile(tstring const& fullPath, FileMemory& memoryBlock);
	bool WriteFile(std::ofstream& out, FileMemory& memoryBlock);


	void SetSize(const size_t tSize) { m_tSize = tSize; }
	size_t GetSize()const  { return m_tSize; } 

	BYTE * Get(){ return m_pFileMemery; }
	void   Set(BYTE * pMemory){ m_pFileMemery = pMemory; }
	BYTE * GetConst()const{ return m_pFileMemery; }
	void New();

private :
	BYTE *			m_pFileMemery;
	size_t			m_tSize;

	//HANDLE			m_hMapHandle;

};

