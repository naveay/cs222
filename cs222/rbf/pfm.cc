#include "pfm.h"

PagedFileManager* PagedFileManager::_pf_manager = 0;


PagedFileManager* PagedFileManager::instance()
{
    if(!_pf_manager)
    {
        _pf_manager = new PagedFileManager();
    }
    return _pf_manager;
}


PagedFileManager::PagedFileManager()
{
	refer=new map<const char*,int>();
}


PagedFileManager::~PagedFileManager()
{
}


RC PagedFileManager::createFile(const char *fileName)
{
	FILE * pFile;
	pFile=fopen(fileName,"rb");
	if(pFile!=NULL)
	{
		fclose(pFile);
		return 1;    //file already exist while using createFile
	}
	else
	{
		refer->insert(pair<const char*,int>(fileName,0));
		pFile=fopen(fileName,"wb+");
		fclose(pFile);
		return 0;
	}
}


RC PagedFileManager::destroyFile(const char *fileName)
{
	if(refer->find(fileName)->second!=0)
		return  7;
	if(remove(fileName)==0)
	{
		return 0;
	}
	else
	{
		return 2;  //file not exist while using deleteFile
	}
}


RC PagedFileManager::openFile(const char *fileName, FileHandle &fileHandle)
{
	FILE * pFile;
	pFile=fopen(fileName,"rb+");
	if(pFile==NULL)
	{
		return 3;    //file not exist while using OpenFile
	}
	else
	{
		refer->insert(pair<const char*,int>(fileName,refer->find(fileName)->second+1));
		fileHandle.setFile(fileName);
		return fileHandle.setFile(pFile);
	}
}


RC PagedFileManager::closeFile(FileHandle &fileHandle)
{
	refer->insert(pair<const char*,int>(fileHandle.getFileName(),refer->find(fileHandle.getFileName())->second-1));
    fflush(fileHandle.getFile());
    fclose(fileHandle.getFile());
    return 0;
}


FileHandle::FileHandle()
{
	pagenumber=0;
}


FileHandle::~FileHandle()
{
}


RC FileHandle::readPage(PageNum pageNum, void *data)
{
	//pFile=fopen(pfile,"rb");
	if(pageNum>=pagenumber||pageNum<0)
		return 5;   //pageNum is not exist;
	fseek(pFile,pageNum*PAGE_SIZE,SEEK_SET);
	fread((char*)data,sizeof(char),PAGE_SIZE,pFile);
	//fclose(pFile);
    return 0;
}


RC FileHandle::writePage(PageNum pageNum, const void *data)
{
	if(pageNum==pagenumber)
		appendPage(data);
	else if(pageNum>pagenumber)
	{
		return 5;   //pageNum is not exist;
	}
	else
	{
		//pFile=fopen(pfile,"rb+");
		fseek(pFile,pageNum*PAGE_SIZE,SEEK_SET);
		fputs((char *)data,pFile);
		//fwrite((char*)data,sizeof(char),len,pFile);
		fflush (pFile);
		//fclose(pFile);
	}
	return 0;
}


RC FileHandle::appendPage(const void *data)
{
	//pFile=fopen(pfile,"rb+");
	fseek(pFile,pagenumber*PAGE_SIZE,SEEK_SET);
	fputs((char *)data,pFile);
	//fwrite((char*)data,sizeof(char),len,pFile);
	fflush (pFile);
	//fclose(pFile);
	pagenumber++;
	return 0;
}


unsigned FileHandle::getNumberOfPages()
{
    return pagenumber;
}

RC FileHandle::setFile(FILE *file)
{
	if(pFile!=NULL)
		return 4;
		//fileHandle is already a handle for an open file when it is passed to the OpenFile method
	else
		pFile=file;
	fseek(pFile,0,SEEK_END);
	pagenumber=ftell(pFile)/PAGE_SIZE;
	return 0;
}

RC FileHandle::setFile(const char * pfile)
{
	if(this->pfile!=NULL)
			return 4;
			//fileHandle is already a handle for an open file when it is passed to the OpenFile method
	else
		this->pfile=pfile;
	return 0;
}

FILE * FileHandle::getFile()
{
	return pFile;
}
const char * FileHandle::getFileName()
{
	return this->pfile;
}


