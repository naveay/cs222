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
	pFile=fopen(fileName,"rb");
	if(pFile!=NULL)
	{
		return 1;    //file already exist while using createFile
	}
	else
	{
		pFile=fopen(fileName,"ab");
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
	pFile=fopen(fileName,"rb");
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
	pFile=fopen(pfile,"rb");
	if(pageNum>pagenumber||pageNum<=0)
		return 5;   //pageNum is not exist;
	fseek(pFile,(pageNum-1)*PAGE_SIZE,SEEK_SET);
	if(fgets((char*)data,PAGE_SIZE,pFile)==NULL)
	{
		return 5;   //pageNum is not exist;
	}
	fclose(pFile);
    return 0;
}


RC FileHandle::WritePage(PageNum pageNum, const void *data)
{
	int len=sizeof(data);
	if(len>PAGE_SIZE)
	{
		return 6;     //data is too long;
	}
	if(pageNum==pagenumber+1)
		AppendPage(data);
	else if(pageNum>pagenumber+1)
	{
		return 5;   //pageNum is not exist;
	}
	else
	{
		pFile=fopen(pfile,"rb+");
		fseek(pFile,(pageNum-1)*PAGE_SIZE,SEEK_SET);
		fputs((char *)data,pFile);
		fclose(pFile);
	}
	return 0;
}


RC FileHandle::AppendPage(const void *data)
{
	int len=sizeof(data);
	if(len>PAGE_SIZE)
	{
		return 6;     //data is too long;
	}
	pFile=fopen(pfile,"rb+");
	fseek(pFile,pagenumber*PAGE_SIZE,SEEK_SET);
	fputs((char *)data,pFile);
	fclose(pFile);
	pagenumber++;
	return 0;
}


unsigned FileHandle::GetNumberOfPages()
{
    return pagenumber;
}

RC FileHandle::setFile(const char *file)
{
	if(pfile!=NULL)
		return 4;
		//fileHandle is already a handle for an open file when it is passed to the OpenFile method
	else
		pfile=file;

	pFile=fopen(pfile,"rb");
	fseek(pFile,0,SEEK_END);
	pagenumber=ftell(pFile)/PAGE_SIZE;
	fclose(pFile);
	return 0;
}
const char * FileHandle::getFile()
{
	return pfile;
}


