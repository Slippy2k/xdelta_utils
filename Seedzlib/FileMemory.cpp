#include "FileMemory.h"
#include <fstream>
#include <memoryapi.h>


FileMemory::FileMemory(): 
m_pFileMemery()
, m_tSize()

{

}


FileMemory::FileMemory(size_t tsize):
m_pFileMemery()
, m_tSize(tsize)
{
	m_pFileMemery = new BYTE[tsize];
}

FileMemory::FileMemory(void* pFileMemery, size_t tsize)
	: m_pFileMemery((BYTE*)pFileMemery)
	, m_tSize(tsize)
{
}

FileMemory::FileMemory(const FileMemory & rFileMemoery)
{
	m_tSize = rFileMemoery.GetSize();
	New();
	memcpy_s(m_pFileMemery, m_tSize, rFileMemoery.GetConst(), rFileMemoery.GetSize());
}

FileMemory::FileMemory(const std::string & rstrSource)
{
	m_tSize = rstrSource.size();
	New();
	memcpy_s(m_pFileMemery, m_tSize, rstrSource.c_str(), m_tSize);
}

FileMemory::~FileMemory()
{
	if (m_pFileMemery != nullptr)
	{
		//UnmapViewOfFile(m_pFileMemery);
		//CloseHandle(m_hMapHandle);
		delete[] m_pFileMemery;
		//VirtualFree(m_pFileMemery, 0, MEM_RELEASE);
		m_pFileMemery = nullptr;
	}
	m_tSize = 0;
}

void FileMemory::New()
{
	m_pFileMemery = new BYTE[GetSize()];
	//m_pFileMemery = (BYTE *)VirtualAlloc(NULL, GetSize(), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	//m_hMapHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, GetSize(), NULL);

	//if (m_hMapHandle)
	//{
	//	m_pFileMemery = (BYTE *)MapViewOfFile(m_hMapHandle, FILE_MAP_ALL_ACCESS, 0, 0, GetSize());
	//}

}

bool FileMemory::ReadFile(tstring const& fullPath)
{
	std::ifstream file(fullPath, std::ios::in | std::ios::binary | std::ios::ate);
	return ReadFile(file);
}

bool FileMemory::ReadFile(std::ifstream & file)
{
	if (!file.is_open())
		return false;

	SetSize((size_t)file.tellg());

	New();

	file.seekg(0, std::ios::beg);
	file.read((char*)m_pFileMemery, GetSize());
	file.close();

	return true;
}

bool FileMemory::WriteFile(tstring const& fullPath)
{
	std::ofstream file(fullPath, std::ios::out | std::ios::binary | std::ios::trunc);
	return WriteFile(file);
}

bool FileMemory::WriteFile(std::ofstream& file)
{
	if (!file.is_open())
		return false;

	file.write((const char*)Get(), GetSize());
	file.close();
	return true;
}



bool FileMemory::ReadFile(tstring const& fullPath, FileMemory& memoryBlock)
{
	std::ifstream file(fullPath, std::ios::in | std::ios::binary | std::ios::ate);
	return ReadFile(file, memoryBlock);
}



bool FileMemory::ReadFile(std::ifstream & file, FileMemory& memoryBlock)
{
	if (!file.is_open())
		return false;

	memoryBlock.SetSize((size_t)file.tellg() );

	memoryBlock.New();

	file.seekg(0, std::ios::beg);
	file.read((char*)memoryBlock.Get(), memoryBlock.GetSize());
	file.close();

	return true;
}

bool FileMemory::WriteFile(tstring const& fullPath, FileMemory& memoryBlock)
{
	std::ofstream file(fullPath, std::ios::out | std::ios::binary | std::ios::trunc);
	return WriteFile(file, memoryBlock);
}

bool FileMemory::WriteFile(std::ofstream& file, FileMemory& memoryBlock)
{
	if (!file.is_open())
		return false;

	file.write((const char*)memoryBlock.Get(), memoryBlock.GetSize());
	file.close();
	return true;
}

