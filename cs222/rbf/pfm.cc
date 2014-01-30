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
	refer=new map<string,int>();
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
		string name;
		name.assign(fileName);
		refer->insert(pair<string,int>(name,0));
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
		string name;
		name.assign(fileName);
		refer->erase(name);
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
		string name;
		name.assign(fileName);
		refer->insert(pair<string,int>(name,refer->find(name)->second+1));
		fileHandle.setFileName(fileName);
		return fileHandle.setFile(pFile);
	}
}


RC PagedFileManager::closeFile(FileHandle &fileHandle)
{
	string name;
	name.assign(fileHandle.getFileName());
	refer->insert(pair<string,int>(name,refer->find(name)->second-1));
	fflush(fileHandle.getFile());
    fclose(fileHandle.getFile());
    return 0;
}


FileHandle::FileHandle()
{
	pFile=NULL;
	pagenumber=0;
	pfilename="";
}


FileHandle::~FileHandle()
{
}


RC FileHandle::readPage(PageNum pageNum, void *data)
{
	if(pageNum>=pagenumber)
		return 5;   //pageNum is not exist;
	fflush(pFile);
	fseek(pFile,pageNum*PAGE_SIZE,SEEK_SET);
	fread((char*)data,sizeof(char),PAGE_SIZE,pFile);
	fflush(pFile);
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
		fseek(pFile,pageNum*PAGE_SIZE,SEEK_SET);
		fwrite((char*)data,sizeof(char),PAGE_SIZE,pFile);
		fflush (pFile);
	}
	return 0;
}


RC FileHandle::appendPage(const void *data)
{
	//pFile=fopen(pfile,"rb+");
	fseek(pFile,pagenumber*PAGE_SIZE,SEEK_SET);
	//fputs((char *)data,pFile);
	fwrite((char*)data,sizeof(char),PAGE_SIZE,pFile);
	fflush (pFile);
	pagenumber++;
	return 0;
}


unsigned int FileHandle::getNumberOfPages()
{
	fseek(pFile,0,SEEK_END);
	pagenumber=(unsigned)(ftell(pFile)/PAGE_SIZE);
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
	pagenumber=(unsigned)(ftell(pFile)/PAGE_SIZE);
	return 0;
}

RC FileHandle::setFileName(const char * pfile)
{
	if(pfilename!="")
			return 4;
			//fileHandle is already a handle for an open file when it is passed to the OpenFile method
	else
		pfilename.assign(pfile);
	return 0;
}

FILE * FileHandle::getFile()
{
	return pFile;
}
const char * FileHandle::getFileName()
{
	return pfilename.c_str();
}


