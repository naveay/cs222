#include "pfm.h"

PagedFileManager* PagedFileManager::_pf_manager = 0;


PagedFileManager* PagedFileManager::Instance()
{
    if(!_pf_manager)
    {
        _pf_manager = new PagedFileManager();
    }
    return _pf_manager;
}


PagedFileManager::PagedFileManager()
{
}


PagedFileManager::~PagedFileManager()
{
}


RC PagedFileManager::CreateFile(const char *fileName)
{
	FILE * pFile;
	pFile=fopen(fileName,"r");
	if(pFile!=NULL)
	{
		return 1;    //file already exist while using createFile
	}
	else
	{
		pFile=fopen(fileName,"a");
		fclose(pFile);
		return 0;
	}
}


RC PagedFileManager::DestroyFile(const char *fileName)
{
	if(remove(fileName)==0)
	{
		return 0;
	}
	else
	{
		return 2;  //file not exist while using deleteFile
	}
}


RC PagedFileManager::OpenFile(const char *fileName, FileHandle &fileHandle)
{
	FILE * pFile;
	pFile=fopen(fileName,"r");
	if(pFile==NULL)
	{
		return 3;    //file not exist while using OpenFile
	}
	else
	{
		fclose(pFile);
		return fileHandle.setFile(fileName);
	}
}


RC PagedFileManager::CloseFile(FileHandle &fileHandle)
{
    return -1;
}


FileHandle::FileHandle()
{
	pagenumber=0;
}


FileHandle::~FileHandle()
{
}


RC FileHandle::ReadPage(PageNum pageNum, void *data)
{
    return -1;
}


RC FileHandle::WritePage(PageNum pageNum, const void *data)
{
    return -1;
}


RC FileHandle::AppendPage(const void *data)
{
    return -1;
}


unsigned FileHandle::GetNumberOfPages()
{
    return -1;
}

RC FileHandle::setFile(const char *file)
{
	if(pfile!=NULL)
		return 4;
		//fileHandle is already a handle for an open file when it is passed to the OpenFile method
	else
		pfile=file;
	return 0;
}
const char * FileHandle::getFile()
{
	return pfile;
}


