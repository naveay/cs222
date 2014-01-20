
#include "rbfm.h"
RecordBasedFileManager* RecordBasedFileManager::_rbf_manager = 0;

RecordBasedFileManager* RecordBasedFileManager::instance()
{
    if(!_rbf_manager)
        _rbf_manager = new RecordBasedFileManager();
    return _rbf_manager;
}

RecordBasedFileManager::RecordBasedFileManager()
{
    pfm=PagedFileManager::instance();
}

RecordBasedFileManager::~RecordBasedFileManager()
{
}

RC RecordBasedFileManager::createFile(const string &fileName) {
	return pfm->createFile(fileName.c_str());
}

RC RecordBasedFileManager::destroyFile(const string &fileName) {
	return pfm->destroyFile(fileName.c_str());
}

RC RecordBasedFileManager::openFile(const string &fileName, FileHandle &fileHandle) {
	RC result=pfm->openFile(fileName.c_str(),fileHandle);
	if(fileHandle.getNumberOfPages()==0)
		initialDirectory(fileHandle,0);
	return result;
}

RC RecordBasedFileManager::closeFile(FileHandle &fileHandle) {
	return pfm->closeFile(fileHandle);
}

RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid) {
    unsigned int len=0;int offset=0;
    if(recordDescriptor.size()==0)
    	return 9;
    //----------
	for(int i=0;i<recordDescriptor.size();i++)
	{
		if(recordDescriptor[i].type==TypeInt)
		{
			len+=sizeof(int);
		}
		else if(recordDescriptor[i].type==TypeReal)
		{
			len+=sizeof(float);
		}
		else
		{
			memcpy(&offset,(char*)data+len,sizeof(int));
			len+=sizeof(int)+offset;
		}
	}
	//void* result=malloc(PAGE_SIZE);
	//len=changeData(recordDescriptor,data,result);
	managePage(fileHandle,len,data,rid);
	return 0;
}

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
	unsigned int len=0;int offset=0;
	if(recordDescriptor.size()==0)
	    return 9;
	void* result=malloc(PAGE_SIZE);
	fileHandle.readPage(rid.pageNum,result);
	memcpy(&offset,(char*)result+rid.slotNum,sizeof(int));
	for(int i=0;i<recordDescriptor.size();i++)
	{
		if(recordDescriptor[i].type==TypeInt)
		{
			memcpy((char*)data+len,(char*)result+offset,sizeof(int));
			len+=sizeof(int);
			offset+=sizeof(int);
		}
		else if(recordDescriptor[i].type==TypeReal)
		{
			memcpy((char*)data+len,(char*)result+offset,sizeof(float));
			len+=sizeof(float);
			offset+=sizeof(float);
		}
		else
		{
			int l=0;
			memcpy(&l,(char*)result+offset,sizeof(int));
			l+=sizeof(int);
			memcpy((char*)data+len,(char*)result+offset,l);
			len+=l;
			offset+=l;
		}
	}

	return 0;
}

RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor, const void *data) {
    return -1;
}
/* Maintain a list of page with their free space
 * Directory of file. Starting from the first page.
 * the first 4 byte is a pointer which points to the index of
 * the next page of directory. Default index is 0;
 * the next 4 byte is the number of page included in this page
 * of directory. For each page, we store the page number and
 * free space.
 *
 * We will have several helper function
 *
 */
int RecordBasedFileManager::changeData(const vector<Attribute> &recordDescriptor, const void *data,void* result)
{
	return -1;
}
int RecordBasedFileManager::insert(FileHandle &fileHandle,unsigned int page,const void* data,int length)
{
	void * d=malloc(PAGE_SIZE);
	fileHandle.readPage(page,d);
	unsigned int free_space=0;
	unsigned int offset=PAGE_SIZE-sizeof(int);
	memcpy(&free_space,(char*)d+offset,sizeof(int));
	int slot;
	offset-=sizeof(int);
	memcpy((char*)d+free_space,data,length);
	int num=0;
	memcpy(&num,(char*)d+offset,sizeof(int));
	num++;
	memcpy((char*)d+offset,&num,sizeof(int));
	offset-=sizeof(int)*num;
	slot=offset;
	memcpy((char*)d+offset,&free_space,sizeof(int));
	offset=PAGE_SIZE-sizeof(int);
	free_space+=length;
	memcpy((char*)d+offset,&free_space,sizeof(int));
	fileHandle.writePage(page,d);
	free(d);
	return slot;
}
RC RecordBasedFileManager::initialPage(FileHandle &fileHandle,unsigned int page)
{
	void *data = malloc(PAGE_SIZE);
	unsigned int free_space=0;
	unsigned int offset=PAGE_SIZE-sizeof(int);
	memcpy((char*)data+offset,&free_space,sizeof(int));
	offset-=sizeof(int);
	int num=0;
	memcpy((char*)data+offset,&num,sizeof(int));
	fileHandle.writePage(page,data);
	free(data);
	return 0;
}
//called after openfile;
RC RecordBasedFileManager::initialDirectory(FileHandle &fileHandle, unsigned int page)
{
	void *data = malloc(PAGE_SIZE);
	unsigned int index=0;
	memcpy((char*)data,&index,sizeof(int));
	memcpy((char*)data+sizeof(int),&index,sizeof(int));
	fileHandle.writePage(page,data);
	free(data);
	return 0;
}
RC RecordBasedFileManager::managePage(FileHandle & fileHandle, unsigned int recordsize,const void* records,RID &rid)
{
	unsigned int index=0;int offset=0;
	unsigned int pagenum=0;
	void *data=malloc(PAGE_SIZE);
	do{
		offset=0;
		fileHandle.readPage(index,data);
		memcpy(&index,(char*)data+offset,sizeof(int));
		offset+=sizeof(int);
		memcpy(&pagenum,(char*)data+offset,sizeof(int));
		offset+=sizeof(int);
		for(unsigned int i=0;i<pagenum;i++)
		{
			unsigned int page;
			memcpy(&page,(char*)data+offset,sizeof(int));
			offset+=sizeof(int);
			int freespace;
			memcpy(&freespace,(char*)data+offset,sizeof(int));
			offset+=sizeof(int);
			if(freespace>=recordsize+sizeof(int))
			{
				int diff=freespace-recordsize-sizeof(int);
				memcpy((char*)data+offset-sizeof(int),&diff,sizeof(int));
				fileHandle.writePage(index,data);
				free(data);
				rid.pageNum=page;
				rid.slotNum=insert(fileHandle,page,records,recordsize);

				//write record to page;
				return 0;
			}
		}
	}while(index>0);
	if(offset<(PAGE_SIZE-2*sizeof(int)))
	{
		pagenum++;
		memcpy((char*)data+sizeof(int),&pagenum,sizeof(int));
		unsigned int page=fileHandle.getNumberOfPages();
		//cout<<page;
		memcpy((char*)data+offset,&page,sizeof(int));
		offset+=sizeof(int);

		int freespace=PAGE_SIZE-recordsize-sizeof(int)*3;
		memcpy((char*)data+offset,&freespace,sizeof(int));
		offset+=sizeof(int);
		fileHandle.writePage(index,data);
		free(data);
		initialPage(fileHandle,page);
		rid.pageNum=page;
		rid.slotNum=insert(fileHandle,page,records,recordsize);
		//write record to page;
		return 0;
	}
	else
	{
		index=fileHandle.getNumberOfPages();
		memcpy((char*)data,&index,sizeof(int));
		fileHandle.writePage(index,data);
		initialDirectory(fileHandle,index);
		free(data);
		data=malloc(PAGE_SIZE);

		fileHandle.readPage(index,data);
		offset=sizeof(int);
		index=1;
		memcpy((char*)data+offset,&index,sizeof(int));
		offset+=sizeof(int);

		int page=fileHandle.getNumberOfPages();
		memcpy((char*)data+offset,&page,sizeof(int));
		offset+=sizeof(int);
		int freespace=PAGE_SIZE-recordsize-sizeof(int)*3;
		memcpy((char*)data+offset,&freespace,sizeof(int));
		offset+=sizeof(int);
		fileHandle.writePage(index,data);
		free(data);
		initialPage(fileHandle,page);
		rid.pageNum=page;
		rid.slotNum=insert(fileHandle,page,records,recordsize);
		//write record to page;
		return 0;
	}
}

