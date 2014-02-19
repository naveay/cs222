
#include "rbfm.h"
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <stdio.h>
#define mem 4096
using namespace std;
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
    unsigned int len=0;
    if(recordDescriptor.size()==0)
    	return 9;
    void* result=malloc(mem);
    len=changeData(recordDescriptor,data,result);
	managePage(fileHandle,len,result,rid);
    free(result);
	return 0;
}
RC RecordBasedFileManager::insertRecord_tmp(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid) {
    unsigned int len=0;
    if(recordDescriptor.size()==0)
    	return 9;
    void* result=malloc(mem);
    len=changeData(recordDescriptor,data,result);
	managePage_tmp(fileHandle,len,result,rid);
    free(result);
	return 0;
}
RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
	int len=0;int offset=0;
	if(recordDescriptor.size()==0)
	    return 9;
	void* result=malloc(PAGE_SIZE);
	int index=0;
	bool out=false;
	do{
		offset=0;
		int pagenum=0;
		fileHandle.readPage(index,result);
		offset+=sizeof(int);
		memcpy(&pagenum,(char*)result+offset,sizeof(int));
		offset+=sizeof(int);
		for(int i=0;i<pagenum;i++)
		{
			unsigned int page;
			memcpy(&page,(char*)result+offset,sizeof(int));
			offset+=sizeof(int);
			if(page==rid.pageNum)
			{
				out=true;
			}
			offset+=sizeof(int);
		}
		if(out)
			break;
	}while(index>0);
	if(!out)
		return -1;
	fileHandle.readPage(rid.pageNum,result);
	memcpy(&offset,(char*)result+PAGE_SIZE-sizeof(int),sizeof(int));
	if(offset==0)
		return 10;
	memcpy(&offset,(char*)result+rid.slotNum,sizeof(int));
	if(offset==-1)
		return 10;
	memcpy(&len,(char*)result+rid.slotNum-sizeof(int),sizeof(int));
	if(len==-PAGE_SIZE&&offset>=0)
	{
			int l;
			RID newID;
			memcpy(&l,(char*)result+offset,sizeof(int));
			newID.pageNum=l;
			memcpy(&l,(char*)result+offset+sizeof(int),sizeof(int));
			newID.slotNum=l;
			free(result);
			return readRecord(fileHandle, recordDescriptor, newID, data);

	}
	else if(len<0&&offset<=0)
	{
		reverse_changeData(recordDescriptor,(char *)result-offset,data);
		free(result);
		return 0;
	}
	else if(offset==-1&&len>=0)
		return 10;
	reverse_changeData(recordDescriptor,(char *)result+offset,data);
	free(result);
	return 0;
}
RC RBFM_ScanIterator::readRecord_Att(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
	int len=0;int offset=0;
	if(recordDescriptor.size()==0)
	    return 9;
	void* result=malloc(PAGE_SIZE);
	fileHandle.readPage(rid.pageNum,result);
	memcpy(&offset,(char*)result+rid.slotNum,sizeof(int));
	if(offset==-1)
		return 10;
	memcpy(&len,(char*)result+rid.slotNum-sizeof(int),sizeof(int));
	if(len==-PAGE_SIZE&&offset>=0)
	{
			int l;
			RID newID;
			memcpy(&l,(char*)result+offset,sizeof(int));
			newID.pageNum=l;
			memcpy(&l,(char*)result+offset+sizeof(int),sizeof(int));
			newID.slotNum=l;
			free(result);
			return readRecord_Att(fileHandle, recordDescriptor, newID, data);

	}
	else if(len<0&&offset<=0)
	{
		reverse_changeData_Att(recordDescriptor,(char *)result-offset,data);
		free(result);
		return 0;
	}
	else if(offset==-1&&len>=0)
		return 10;
	reverse_changeData_Att(recordDescriptor,(char *)result+offset,data);
	free(result);
	return 0;
}
RC RBFM_ScanIterator::readRecordMem(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
	int len=0;int offset=0;
	memcpy(&offset,(char*)memory+rid.slotNum,sizeof(int));
	if(offset==-1)
		return 10;
	memcpy(&len,(char*)memory+rid.slotNum-sizeof(int),sizeof(int));
	if(len==-PAGE_SIZE&&offset>=0)
	{
			int l;
			RID newID;
			memcpy(&l,(char*)memory+offset,sizeof(int));
			newID.pageNum=l;
			memcpy(&l,(char*)memory+offset+sizeof(int),sizeof(int));
			newID.slotNum=l;
			return readRecord_Att(fileHandle, recordDescriptor, newID, data);

	}
	else if(len<0&&offset<=0)
	{
		reverse_changeData_Att(recordDescriptor,(char *)memory-offset,data);
		return 0;
	}
	else if(offset==-1&&len>=0)
		return 10;
	reverse_changeData_Att(recordDescriptor,(char *)memory+offset,data);
	return 0;
}

RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor, const void *data) {
    
    
    
    
    
    Attribute attr;
    unsigned int offset = 0;
    unsigned int namelenght,nrofelement,i;
    int intval;
    float floatval;
    nrofelement = (unsigned int)recordDescriptor.size();
    
    //------------------------//
    for (i= 0;i<nrofelement;i++)
    {
        switch (recordDescriptor[i].type)
        {
            case 0:
                memcpy(&intval,(char *)data+offset,sizeof(int));
                cout << recordDescriptor[i].name << ": " << intval << endl;
                offset+=sizeof(int);
                break;
            case 1:
                memcpy(&floatval,(char *)data+offset,sizeof(float));
                cout << recordDescriptor[i].name << ": " << floatval << endl;
                offset+=sizeof(float);
                break;
            case 2://string case
                //read name lengh//
            	memcpy(&namelenght,(char *)data+offset,sizeof(int));
                //namelenght = *(int *)data + offset;
                offset +=sizeof(int);
                //read the name//
                char *name = (char *)malloc(namelenght+1);
                memset(name,'\0',namelenght+1);
                memcpy(name,(char *)data+offset,namelenght);
                cout << recordDescriptor[i].name << ": "<< name << endl;
                offset+= namelenght;
                break;
        }
    }
    
    return 0;
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

    unsigned int nrofelements,i;
    int namelength;
    int offset = 0;
    int offsetdata = 0;
    nrofelements = (unsigned int)recordDescriptor.size();
    //cout << "vectro size is: " << nrofelements << endl;

    for (i=0;i<nrofelements;i++)
    {
        switch (recordDescriptor[i].type)
        {
            case 0://int case
                offset+=sizeof(int);
                break;
            case 1://float case
                offset+=sizeof(float);
                break;
            case 2://string case
            	memcpy(&namelength,(char*)data+offset,sizeof(int));
                offset +=sizeof(int);
                offset+= namelength;
                break;
        }
    }
    //cout << "offset is: " << offset << endl;
    //copy data to new location in the result by the number of offset byte//
    memcpy((char *)result+(sizeof(int) * (1+nrofelements)), data, offset);
    //initialize the second offset value//
    offsetdata = (1+nrofelements) * (sizeof(int));
    //put the offsetdata in the first location//
    for (i = 0;i<nrofelements;i++)
    {
        memcpy((char *)result + (i * sizeof(int)),&offsetdata,sizeof(int));
        switch (recordDescriptor[i].type)
        {
            case 0://int
                offsetdata += sizeof(int);
                break;
            case 1://float
                offsetdata += sizeof(float);
                break;
            case 2://string case//
            	memcpy(&namelength,(char*)result+offsetdata,sizeof(int));
                offsetdata += (namelength + sizeof(int));//number of byte jump//
                break;
        }
    }
    memcpy((char *)result + (nrofelements  * sizeof(int)),&offsetdata,sizeof(int));
    return offsetdata;
}
int RecordBasedFileManager::reverse_changeData(const vector<Attribute> &recordDescriptor, const void *data,void* result)
{
    unsigned int nrofelements;
    int namelength;
    int offset = 0;
    nrofelements = (unsigned int)recordDescriptor.size();
    //cout << "vectro size is: " << nrofelements << endl;
    offset=nrofelements*sizeof(int);
    memcpy(&namelength,(char*)data+offset,sizeof(int));
    offset+=sizeof(int);
    namelength=namelength-offset;
    memcpy((char*)result,(char*)data+offset,namelength);
    return namelength;
}

int RBFM_ScanIterator::reverse_changeData_Att(const vector<Attribute> &recordDescriptor, const void *data,void* result)
{
    int namelength=0;
    int offset = 0;
    for(int m=0;m<(int)attributeNames.size();m++)
    {
        for(int i=0;i<(int)recordDescriptor.size();i++)
        {
        	if(attributeNames[m]==recordDescriptor[i].name)
        	{
        		offset=i*sizeof(int);
        		int start,end;
        		memcpy(&start,(char*)data+offset,sizeof(int));
        		memcpy(&end,(char*)data+offset+sizeof(int),sizeof(int));
        		memcpy((char*)result+namelength,(char*)data+start,end-start);
        		namelength=namelength+end-start;
        	}
        }
    }
    return namelength;
}

int RecordBasedFileManager::insert(FileHandle &fileHandle,unsigned int page,const void* data,int length)
{
	void * d=malloc(PAGE_SIZE);
	fileHandle.readPage(page,d);
	unsigned int free_space=0;
	int offset=PAGE_SIZE-sizeof(int);
	memcpy(&free_space,(char*)d+offset,sizeof(int));
	int slot;
	offset-=sizeof(int);
	memcpy((char*)d+free_space,data,length);
	int num=0;
	memcpy(&num,(char*)d+offset,sizeof(int));
	num++;
	memcpy((char*)d+offset,&num,sizeof(int));
	for(int i=1;i<num-1;i++)
	{
		int tmp_offset=offset-sizeof(int)*i*2+sizeof(int);
		int index=0;
		int le=0;
		memcpy(&index,(char*)d+tmp_offset,sizeof(int));
		memcpy(&le,(char*)d+tmp_offset-sizeof(int),sizeof(int));
		//cout<<"index"<<index<<" "<<le<<endl;
		if(index==-1)
		{
			{
				offset=tmp_offset;
				slot=offset;
				memcpy((char*)d+offset,&free_space,sizeof(int));
				offset-=sizeof(int);
				memcpy((char*)d+offset,&length,sizeof(int));
				offset=PAGE_SIZE-sizeof(int);
				free_space+=length;
				memcpy((char*)d+offset,&free_space,sizeof(int));
				fileHandle.writePage(page,d);
				free(d);
				return slot;
			}
		}
	}
	offset-=sizeof(int)*num*2-sizeof(int);
	slot=offset;
	memcpy((char*)d+offset,&free_space,sizeof(int));
	offset-=sizeof(int);
	memcpy((char*)d+offset,&length,sizeof(int));
	//cout<<"insert"<<free_space<<" "<<length<<endl;
	offset=PAGE_SIZE-sizeof(int);
	free_space+=length;
	memcpy((char*)d+offset,&free_space,sizeof(int));
	fileHandle.writePage(page,d);
	free(d);
	return slot;
}
int RecordBasedFileManager::insert_tmp(FileHandle &fileHandle,unsigned int page,const void* data,int length)
{
	void * d=malloc(PAGE_SIZE);
	fileHandle.readPage(page,d);
	unsigned int free_space=0;
	int offset=PAGE_SIZE-sizeof(int);
	memcpy(&free_space,(char*)d+offset,sizeof(int));
	int slot;
	offset-=sizeof(int);
	memcpy((char*)d+free_space,data,length);
	int num=0;
	memcpy(&num,(char*)d+offset,sizeof(int));
	num++;
	memcpy((char*)d+offset,&num,sizeof(int));
	for(int i=1;i<num;i++)
	{
		int tmp_offset=offset-sizeof(int)*i*2+sizeof(int);
		int index=0;
		int le=0;
		memcpy(&index,(char*)d+tmp_offset,sizeof(int));
		memcpy(&le,(char*)d+tmp_offset-sizeof(int),sizeof(int));
		if(index==-1)
		{
			{
				offset=tmp_offset;
				slot=offset;
				int neg=-((int)free_space);
				memcpy((char*)d+offset,&neg,sizeof(int));
				offset-=sizeof(int);
				length=-length;
				memcpy((char*)d+offset,&length,sizeof(int));
				length=-length;

				//cout<<"space"<<neg<<" "<<-length<<endl;
				offset=PAGE_SIZE-sizeof(int);
				free_space+=length;
				memcpy((char*)d+offset,&free_space,sizeof(int));
				fileHandle.writePage(page,d);
				free(d);
				return slot;
			}
		}
	}
	offset-=sizeof(int)*num*2-sizeof(int);
	slot=offset;
	int neg=-((int)free_space);
	memcpy((char*)d+offset,&neg,sizeof(int));
	offset-=sizeof(int);
	length=-length;
	memcpy((char*)d+offset,&length,sizeof(int));
	length=-length;

	//cout<<"insert"<<neg<<" "<<-length<<endl;
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
	unsigned int current=0;
	void *data=malloc(PAGE_SIZE);
	do{
		offset=0;
		fileHandle.readPage(index,data);
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
			if((unsigned int)freespace>=recordsize+sizeof(int)*2)
			{
				int diff=freespace-recordsize-sizeof(int)*2;
				memcpy((char*)data+offset-sizeof(int),&diff,sizeof(int));
				fileHandle.writePage(index,data);
				free(data);
				rid.pageNum=page;
				rid.slotNum=insert(fileHandle,page,records,recordsize);

				//write record to page;
				return 0;
			}
		}
		memcpy(&index,(char*)data,sizeof(int));
		if(index>0)
			current=index;
	}while(index>0);
	if((unsigned int)offset<(PAGE_SIZE-2*sizeof(int)))
	{
		pagenum++;
		memcpy((char*)data+sizeof(int),&pagenum,sizeof(int));
		unsigned int page=fileHandle.getNumberOfPages();
		//cout<<page;
		memcpy((char*)data+offset,&page,sizeof(int));
		offset+=sizeof(int);

		int freespace=PAGE_SIZE-recordsize-sizeof(int)*4;
		memcpy((char*)data+offset,&freespace,sizeof(int));
		offset+=sizeof(int);
		fileHandle.writePage(current,data);
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
		fileHandle.writePage(current,data);
		initialDirectory(fileHandle,index);
		free(data);
		data=malloc(PAGE_SIZE);

		fileHandle.readPage(index,data);
		offset=sizeof(int);
		int start=1;
		memcpy((char*)data+offset,&start,sizeof(int));
		offset+=sizeof(int);

		int page=fileHandle.getNumberOfPages();
		memcpy((char*)data+offset,&page,sizeof(int));
		offset+=sizeof(int);
		int freespace=PAGE_SIZE-recordsize-sizeof(int)*4;
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
RC RecordBasedFileManager::managePage_tmp(FileHandle & fileHandle, unsigned int recordsize,const void* records,RID &rid)
{
	unsigned int index=0;int offset=0;
	unsigned int pagenum=0;
	unsigned int current=0;
	void *data=malloc(PAGE_SIZE);
	/*
	do{
		offset=0;
		fileHandle.readPage(index,data);
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
			if((unsigned int)freespace>=recordsize+sizeof(int)*2)
			{
				int diff=freespace-recordsize-sizeof(int)*2;
				memcpy((char*)data+offset-sizeof(int),&diff,sizeof(int));
				fileHandle.writePage(index,data);
				free(data);
				rid.pageNum=page;
				rid.slotNum=insert_tmp(fileHandle,page,records,recordsize);

				//write record to page;
				return 0;
			}
		}
		memcpy(&index,(char*)data,sizeof(int));
		if(index>0)
			current=index;
	}while(index>0);
	*/
	offset=0;
	fileHandle.readPage(index,data);
	memcpy(&index,(char*)data,sizeof(int));
	while(index>0)
	{
		fileHandle.readPage(index,data);
		memcpy(&index,(char*)data,sizeof(int));
		if(index!=0)
			current=index;
	}
	offset+=sizeof(int);
	memcpy(&pagenum,(char*)data+offset,sizeof(int));
	offset+=sizeof(int);
	offset+=sizeof(int)*(pagenum-1)*2;
	unsigned int page;
	memcpy(&page,(char*)data+offset,sizeof(int));
	offset+=sizeof(int);
	int freespace;
	memcpy(&freespace,(char*)data+offset,sizeof(int));
	offset+=sizeof(int);
	if((unsigned int)freespace>=recordsize+sizeof(int)*2)
	{
		int diff=freespace-recordsize-sizeof(int)*2;
		memcpy((char*)data+offset-sizeof(int),&diff,sizeof(int));
		fileHandle.writePage(index,data);
		free(data);
		rid.pageNum=page;
		rid.slotNum=insert_tmp(fileHandle,page,records,recordsize);

					//write record to page;
		return 0;
	}


	if((unsigned int)offset<(PAGE_SIZE-2*sizeof(int)))
	{
		pagenum++;
		memcpy((char*)data+sizeof(int),&pagenum,sizeof(int));
		unsigned int page=fileHandle.getNumberOfPages();
		//cout<<page;
		memcpy((char*)data+offset,&page,sizeof(int));
		offset+=sizeof(int);

		int freespace=PAGE_SIZE-recordsize-sizeof(int)*4;
		memcpy((char*)data+offset,&freespace,sizeof(int));
		offset+=sizeof(int);
		fileHandle.writePage(current,data);
		free(data);
		initialPage(fileHandle,page);
		rid.pageNum=page;
		rid.slotNum=insert_tmp(fileHandle,page,records,recordsize);
		//write record to page;
		return 0;
	}
	else
	{
		index=fileHandle.getNumberOfPages();
		memcpy((char*)data,&index,sizeof(int));
		fileHandle.writePage(current,data);
		initialDirectory(fileHandle,index);
		free(data);
		data=malloc(PAGE_SIZE);

		fileHandle.readPage(index,data);
		offset=sizeof(int);
		int start=1;
		memcpy((char*)data+offset,&start,sizeof(int));
		offset+=sizeof(int);

		int page=fileHandle.getNumberOfPages();
		memcpy((char*)data+offset,&page,sizeof(int));
		offset+=sizeof(int);
		int freespace=PAGE_SIZE-recordsize-sizeof(int)*4;
		memcpy((char*)data+offset,&freespace,sizeof(int));
		offset+=sizeof(int);
		fileHandle.writePage(index,data);
		free(data);
		initialPage(fileHandle,page);
		rid.pageNum=page;
		rid.slotNum=insert_tmp(fileHandle,page,records,recordsize);
		//write record to page;
		return 0;
	}
}
RC RecordBasedFileManager::deleteRecords(FileHandle &fileHandle)
{
	unsigned int index=0;int offset=0;
	unsigned int pagenum=0;
	void *data=malloc(PAGE_SIZE);
	do{
		offset=0;
		fileHandle.readPage(index,data);
		offset+=sizeof(int);
		memcpy(&pagenum,(char*)data+offset,sizeof(int));
		offset+=sizeof(int);
		for(unsigned int i=0;i<pagenum;i++)
		{
			unsigned int page;
			memcpy(&page,(char*)data+offset,sizeof(int));
			offset+=sizeof(int);
			int freespace=PAGE_SIZE-sizeof(int)*2;
			memcpy((char*)data+offset,&freespace,sizeof(int));
			offset+=sizeof(int);
			initialPage(fileHandle,page);
		}
		fileHandle.writePage(index,data);
		memcpy(&index,(char*)data,sizeof(int));
	}while(index>0);
	free(data);
	return 0;
}

RC RecordBasedFileManager::deleteRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid)
{
	if(recordDescriptor.size()==0)
	    return 9;
	void* result=malloc(PAGE_SIZE);

	unsigned int index=0;int offset=0;

	fileHandle.readPage(rid.pageNum,result);
	//------------
	offset=PAGE_SIZE-sizeof(int);
	offset-=sizeof(int);
	int num=0;
	memcpy(&num,(char*)result+offset,sizeof(int));
	num--;
	memcpy((char*)result+offset,&num,sizeof(int));
	int length=0;
	int of=0;
	memcpy(&of,(char*)result+rid.slotNum,sizeof(int));
	memcpy(&length,(char*)result+rid.slotNum-sizeof(int),sizeof(int));
	if(length>=0&&of>=0)
	{
		num=-1;
		memcpy((char*)result+rid.slotNum,&num,sizeof(int));
		fileHandle.writePage(rid.pageNum,result);
	}
	else if(length<0&&of<=0&&length!=-PAGE_SIZE)
	{
		num=-1;
		memcpy((char*)result+rid.slotNum,&num,sizeof(int));
		num=1;
		memcpy((char*)result+rid.slotNum-sizeof(int),&num,sizeof(int));
		fileHandle.writePage(rid.pageNum,result);
	}
	else
	{
		length=sizeof(int)*2;
		memcpy((char*)result+rid.slotNum-sizeof(int),&length,sizeof(int));
		memcpy(&num,(char*)result+rid.slotNum,sizeof(int));
		RID newid;
		int tmp=0;
		memcpy(&tmp,(char*)result+num,sizeof(int));
		newid.pageNum=tmp;
		memcpy(&tmp,(char*)result+num+sizeof(int),sizeof(int));
		newid.slotNum=tmp;
		num=-1;
		memcpy((char*)result+rid.slotNum,&num,sizeof(int));
		fileHandle.writePage(rid.pageNum,result);
		deleteRecord(fileHandle,recordDescriptor,newid);
	}
	bool out=true;
	unsigned int pagenum=0;
		do{
			offset=0;
			fileHandle.readPage(index,result);
			offset+=sizeof(int);
			memcpy(&pagenum,(char*)result+offset,sizeof(int));
			offset+=sizeof(int);
			for(unsigned int i=0;i<pagenum;i++)
			{
				unsigned int page;
				memcpy(&page,(char*)result+offset,sizeof(int));
				offset+=sizeof(int);
				if(page==rid.pageNum)
				{
					int freespace=0;
					memcpy(&freespace,(char*)result+offset,sizeof(int));
					freespace=freespace+length+sizeof(int)*2;
					memcpy((char*)result+offset,&freespace,sizeof(int));
					out=true;
					break;
				}
				offset+=sizeof(int);
			}
			fileHandle.writePage(index,result);
			memcpy(&index,(char*)result,sizeof(int));
			if(out)
				break;
		}while(index>0);
	free(result);
	return 0;
}

RC RecordBasedFileManager::updateRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, const RID &rid)
{
	int len=0;int offset=0;
	if(recordDescriptor.size()==0)
		return 9;
	void* result=malloc(PAGE_SIZE);
	void* tmp=malloc(PAGE_SIZE);
	fileHandle.readPage(rid.pageNum,result);
	memcpy(&offset,(char*)result+rid.slotNum,sizeof(int));
	memcpy(&len,(char*)result+rid.slotNum-sizeof(int),sizeof(int));
	if(len==-PAGE_SIZE&&offset>=0)
	{
		//-----------------
		RID newID;
		memcpy(&len,(char*)result+offset,sizeof(int));
		newID.pageNum=len;
		memcpy(&len,(char*)result+offset+sizeof(int),sizeof(int));
		newID.slotNum=len;
		free(result);
		free(tmp);
		return updateRecord(fileHandle,recordDescriptor,data,newID);

	}
	else if(len<0&&offset<=0)
	{
		int length=changeData(recordDescriptor,data,tmp);
			int index=0;
			if(length<=-len)
			{
				length=-length;
				memcpy((char*)result+rid.slotNum-sizeof(int),&length,sizeof(int));
				length=-length;
				len=-len;
				memcpy((char*)result-offset,(char*)tmp,length);
				fileHandle.writePage(rid.pageNum,result);
				bool out=false;
				unsigned int pagenum=0;
						do{
							offset=0;
							fileHandle.readPage(index,result);
							offset+=sizeof(int);
							memcpy(&pagenum,(char*)result+offset,sizeof(int));
							offset+=sizeof(int);
							for(unsigned int i=0;i<pagenum;i++)
							{
								unsigned int page;
								memcpy(&page,(char*)result+offset,sizeof(int));
								offset+=sizeof(int);
								if(page==rid.pageNum)
								{
									int freespace=0;
									memcpy(&freespace,(char*)result+offset,sizeof(int));
									freespace=freespace-length+len;
									memcpy((char*)result+offset,&freespace,sizeof(int));
									out=true;
									break;
								}
								offset+=sizeof(int);
							}
							fileHandle.writePage(index,result);
							memcpy(&index,(char*)result,sizeof(int));
							if(out)
								break;
						}while(index>0);

			}
			else
			{
				RID newID;
				insertRecord_tmp(fileHandle,recordDescriptor,data,newID);
				fileHandle.readPage(rid.pageNum,result);
				length=-PAGE_SIZE;
				memcpy((char*)result+rid.slotNum-sizeof(int),&length,sizeof(int));
				offset=-offset;
				memcpy((char*)result+rid.slotNum,&offset,sizeof(int));
				length=newID.pageNum;
				memcpy((char*)result+offset,&length,sizeof(int));
				length=newID.slotNum;
				memcpy((char*)result+offset+sizeof(int),&length,sizeof(int));
				fileHandle.writePage(rid.pageNum,result);
				len=-len;
				bool out=false;
				unsigned int pagenum=0;
						do{
							offset=0;
							fileHandle.readPage(index,result);
							offset+=sizeof(int);
							memcpy(&pagenum,(char*)result+offset,sizeof(int));
							offset+=sizeof(int);
							for(unsigned int i=0;i<pagenum;i++)
							{
								unsigned int page;
								memcpy(&page,(char*)result+offset,sizeof(int));
								offset+=sizeof(int);
								if(page==rid.pageNum)
								{
									int freespace=0;
									memcpy(&freespace,(char*)result+offset,sizeof(int));
									freespace=freespace+len-sizeof(int)*2;
									memcpy((char*)result+offset,&freespace,sizeof(int));
									out=true;
									break;
								}
								offset+=sizeof(int);
							}
							fileHandle.writePage(index,result);
							memcpy(&index,(char*)result,sizeof(int));
							if(out)
								break;
						}while(index>0);
				//--
			}
			free(result);
					free(tmp);
			return 0;
	}
	int length=changeData(recordDescriptor,data,tmp);
	int index=0;
	if(length<=len)
	{
		memcpy((char*)result+rid.slotNum-sizeof(int),&length,sizeof(int));
		memcpy((char*)result+offset,(char*)tmp,length);
		fileHandle.writePage(rid.pageNum,result);
		unsigned int pagenum=0;
		bool out=false;
				do{
					offset=0;
					fileHandle.readPage(index,result);
					offset+=sizeof(int);
					memcpy(&pagenum,(char*)result+offset,sizeof(int));
					offset+=sizeof(int);
					for(unsigned int i=0;i<pagenum;i++)
					{
						unsigned int page;
						memcpy(&page,(char*)result+offset,sizeof(int));
						offset+=sizeof(int);
						if(page==rid.pageNum)
						{
							int freespace=0;
							memcpy(&freespace,(char*)result+offset,sizeof(int));
							freespace=freespace-length+len;
							memcpy((char*)result+offset,&freespace,sizeof(int));
							out=true;
							break;
						}
						offset+=sizeof(int);
					}
					fileHandle.writePage(index,result);
					memcpy(&index,(char*)result,sizeof(int));
					if(out)
						break;
				}while(index>0);

	}
	else
	{
		RID newID;
		insertRecord_tmp(fileHandle,recordDescriptor,data,newID);
		fileHandle.readPage(rid.pageNum,result);
		length=-PAGE_SIZE;
		memcpy((char*)result+rid.slotNum-sizeof(int),&length,sizeof(int));
		length=newID.pageNum;
		memcpy((char*)result+offset,&length,sizeof(int));
		length=newID.slotNum;
		memcpy((char*)result+offset+sizeof(int),&length,sizeof(int));
		fileHandle.writePage(rid.pageNum,result);
		bool out=false;
		unsigned int pagenum=0;
				do{
					offset=0;
					fileHandle.readPage(index,result);
					offset+=sizeof(int);
					memcpy(&pagenum,(char*)result+offset,sizeof(int));
					offset+=sizeof(int);
					for(unsigned int i=0;i<pagenum;i++)
					{
						unsigned int page;
						memcpy(&page,(char*)result+offset,sizeof(int));
						offset+=sizeof(int);
						if(page==rid.pageNum)
						{
							int freespace=0;
							memcpy(&freespace,(char*)result+offset,sizeof(int));
							freespace=freespace+len-sizeof(int)*2;
							memcpy((char*)result+offset,&freespace,sizeof(int));
							out=true;
							break;
						}
						offset+=sizeof(int);
					}
					fileHandle.writePage(index,result);
					memcpy(&index,(char*)result,sizeof(int));
					if(out)
						break;
				}while(index>0);
		//--
	}
	free(result);
	free(tmp);
	return 0;
}

RC RecordBasedFileManager::readAttribute(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, const string attributeName, void *data)
{
	int len=0;int offset=0;
	if(recordDescriptor.size()==0)
	    return 9;
	void* result=malloc(PAGE_SIZE);
	fileHandle.readPage(rid.pageNum,result);
	memcpy(&offset,(char*)result+rid.slotNum,sizeof(int));
	memcpy(&len,(char*)result+rid.slotNum-sizeof(int),sizeof(int));
	if(len==-PAGE_SIZE)
	{
		//-----------------
		int l;
		RID newID;
		memcpy(&l,(char*)result+offset,sizeof(int));
		newID.pageNum=l;
		memcpy(&l,(char*)result+offset+sizeof(int),sizeof(int));
		newID.slotNum=l;
		free(result);
		return readAttribute(fileHandle, recordDescriptor, newID,attributeName, data);
	}
	if(offset<=0&&len<=0)
		offset=-offset;
	//memcpy((char*)data,(char*)result+offset,len)
	for(int i=0;i<(int)recordDescriptor.size();i++)
	{
		if(recordDescriptor[i].name==attributeName)
		{
			int start,end;
			memcpy(&start,(char*)result+offset+sizeof(int)*i,sizeof(int));
			memcpy(&end,(char*)result+offset+sizeof(int)+sizeof(int)*i,sizeof(int));
			memcpy((char*)data,(char*)result+offset+start,end-start);
			free(result);
			return 0;
		}
	}
	free(result);
	return 11;
}

RC RecordBasedFileManager::reorganizePage(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const unsigned pageNumber)
{
	void * d=malloc(PAGE_SIZE);
	fileHandle.readPage(pageNumber,d);
	unsigned int free_space=0;
	int offset=PAGE_SIZE-sizeof(int);
	memcpy(&free_space,(char*)d+offset,sizeof(int));
	offset-=sizeof(int);
	int num=0;
	memcpy(&num,(char*)d+offset,sizeof(int));
	map<int,int> offset_length;
	map<int,int> offset_index;
	for(int i=1;i<=num;i++)
	{
		int tmp_offset=offset-sizeof(int)*i*2+sizeof(int);
		int index=0;
		int le=0;
		memcpy(&index,(char*)d+tmp_offset,sizeof(int));
		memcpy(&le,(char*)d+tmp_offset-sizeof(int),sizeof(int));
		if(index==-1&&le>=0)
		{
			num++;
		}
		else if(index>=0)
		{
			offset_length.insert(pair<int,int>(index,le));
			offset_index.insert(pair<int,int>(index,i));
		}
		else
		{
			offset_length.insert(pair<int,int>(-index,le));
			offset_index.insert(pair<int,int>(-index,i));
		}
	}
	int current=0;
	map<int,int>::iterator iter_1=offset_length.begin();
	map<int,int>::iterator iter_2=offset_index.begin();
	for(int i=0;i<(int)offset_length.size();i++)
	{
		if(current==(iter_1->first))
		{
			if((iter_1->second)>=0)
			{
				current+=(iter_1->second);
			}
			else if((iter_1->second)==-PAGE_SIZE)
				current+=sizeof(int)*2;
			else
				current-=(iter_1->second);
		}
		else
		{
			if((iter_1->second)>=0)
			{
				memcpy((char*)d+current,(char*)d+(iter_1->first),iter_1->second);
				int t=PAGE_SIZE-sizeof(int)*2-sizeof(int)*(iter_2->second)*2+sizeof(int);
				memcpy((char*)d+t,&current,sizeof(int));
				current+=(iter_1->second);
			}
			else if((iter_1->second)==-PAGE_SIZE)
			{
				memcpy((char*)d+current,(char*)d+(iter_1->first),sizeof(int)*2);
				int t=PAGE_SIZE-sizeof(int)*2-sizeof(int)*(iter_2->second)*2+sizeof(int);
				memcpy((char*)d+t,&current,sizeof(int));
				current+=sizeof(int)*2;
			}
			else
			{
				memcpy((char*)d+current,(char*)d+(iter_1->first),-(iter_1->second));
				current=-current;
				int t=PAGE_SIZE-sizeof(int)*2+sizeof(int)*(iter_2->second)*2+sizeof(int);
				memcpy((char*)d+t,&current,sizeof(int));
				current=-current;
				current-=(iter_1->second);
			}
		}
		iter_1++;
		iter_2++;
	}
	memcpy((char*)d+PAGE_SIZE-sizeof(int),&current,sizeof(int));
	fileHandle.writePage(pageNumber,d);
	free(d);
	return 0;
}
RC RecordBasedFileManager::scan(FileHandle &fileHandle,
     const vector<Attribute> &recordDescriptor,
     const string &conditionAttribute,
     const CompOp compOp,                  // comparision type such as "<" and "="
     const void *value,                    // used in the comparison
     const vector<string> &attributeNames, // a list of projected attributes
     RBFM_ScanIterator &rbfm_ScanIterator)
{
	rbfm_ScanIterator.fileHandle=fileHandle;
	rbfm_ScanIterator.recordDescriptor=recordDescriptor;
	rbfm_ScanIterator.conditionAttribute=conditionAttribute;
	rbfm_ScanIterator.compOp=compOp;
	rbfm_ScanIterator.value=value;
	rbfm_ScanIterator.attributeNames=attributeNames;
	rbfm_ScanIterator.currentid.pageNum=0;
	rbfm_ScanIterator.currentpage=-1;
	rbfm_ScanIterator.readintoMem=true;
	rbfm_ScanIterator.slot_num=-1;
	rbfm_ScanIterator.rbfm=this;
	rbfm_ScanIterator.memory=malloc(PAGE_SIZE);
	return 0;
}
RC RBFM_ScanIterator::getNextRecord(RID &rid, void *data)
{
	unsigned int pagenum=0;
	int offset=0,index=0;
	void* m=malloc(mem);
	if(readintoMem==true)
	{
		bool out=false;
		do{
			offset=0;
			fileHandle.readPage(index,memory);
			offset+=sizeof(int);
			memcpy(&pagenum,(char*)memory+offset,sizeof(int));

			offset+=sizeof(int);
			if(currentpage==-1)
			{
				int page;
				memcpy(&page,(char*)memory+offset,sizeof(int));

				currentid.pageNum=page;
				currentpage=page;
				out=true;
				break;
			}
			for(unsigned int i=0;i<pagenum;i++)
			{
				int page;
				memcpy(&page,(char*)memory+offset,sizeof(int));
				offset+=sizeof(int);
				if(i==pagenum-1)
				{
					currentpage=-1;
					continue;
				}
				if(page==currentpage)
				{
					memcpy(&page,(char*)memory+offset+sizeof(int),sizeof(int));
					currentid.pageNum=page;
					currentpage=page;
					out=true;
					break;
				}
				offset+=sizeof(int);
			}
			if(out)
				break;
			memcpy(&index,(char*)memory,sizeof(int));
		}while(index>0);
	}
	if(index==0&&currentpage==-1)
	{
		free(m);
		return RBFM_EOF;
	}
	if(readintoMem==true)
	{
		fileHandle.readPage(currentpage,memory);
		slot_num=-1;
		readintoMem=false;
	}
	offset=PAGE_SIZE-sizeof(int);
	offset-=sizeof(int);
	int num=0;
	memcpy(&num,(char*)memory+offset,sizeof(int));
	if(num==slot_num)
	{
		slot_num=-1;
		readintoMem=true;
		free(m);
		return getNextRecord(rid,data);
	}
	int rec=0;
	if(slot_num==-1)
	{
		for(int i=1;i<=num;i++)
		{
			int tmp_offset=offset-sizeof(int)*i*2+sizeof(int);
			int in=0;
			int le=0;
			memcpy(&in,(char*)memory+tmp_offset,sizeof(int));
			memcpy(&le,(char*)memory+tmp_offset-sizeof(int),sizeof(int));
			if(in==-1&&le>=0)
			{
				num++;
			}
			else if(in>=0&&le>=0)
			{
				rec++;
				if(condition(((char*)memory)+in))
				{
					slot_num=rec;
					currentid.slotNum=tmp_offset;
					rid.pageNum=currentid.pageNum;
					rid.slotNum=currentid.slotNum;
					readRecordMem(fileHandle,recordDescriptor,currentid,data);
					break;
				}
			}
			else if(in>=0&&le==-PAGE_SIZE)
			{
				rec++;
				RID n;
				n.pageNum=currentid.pageNum;
				n.slotNum=tmp_offset;
				rbfm->readRecord(fileHandle,recordDescriptor,n, m);
				if(condition((char*)m))
				{
					slot_num=rec;
					currentid.slotNum=tmp_offset;
					rid.pageNum=currentid.pageNum;
					rid.slotNum=currentid.slotNum;
					free(m);
					return readRecord_Att(fileHandle,recordDescriptor,currentid,data);
				}
			}
			else
			{
				rec++;
			}
		}
	}
	else
	{
		for(int i=1;i<=num;i++)
		{
			int tmp_offset=offset-sizeof(int)*i*2+sizeof(int);
			int in=0;
			int le=0;
			memcpy(&in,(char*)memory+tmp_offset,sizeof(int));
			memcpy(&le,(char*)memory+tmp_offset-sizeof(int),sizeof(int));
			if(in==-1&&le>=0)
			{
				num++;
			}
			else if(in>=0&&le>=0)
			{
				rec++;
				if(rec>=slot_num+1)
				{
					if(condition(((char*)memory)+in))
					{
						slot_num=rec;
						currentid.slotNum=tmp_offset;
						rid.pageNum=currentid.pageNum;
						rid.slotNum=currentid.slotNum;
						readRecordMem(fileHandle,recordDescriptor,currentid,data);
						break;
					}
				}
			}
			else if(in>=0&&le==-PAGE_SIZE)
			{
				rec++;
				if(rec>=slot_num+1)
				{
					RID n;
					n.pageNum=currentid.pageNum;
					n.slotNum=tmp_offset;
					rbfm->readRecord(fileHandle,recordDescriptor,n, m);
					if(condition((char*)m))
					{
						free(m);
						slot_num=rec;
						currentid.slotNum=tmp_offset;
						rid.pageNum=currentid.pageNum;
						rid.slotNum=currentid.slotNum;
						return readRecord_Att(fileHandle,recordDescriptor,currentid,data);
					}
				}
			}
			else
			{
				rec++;
			}

		}
	}
	free(m);
	if(slot_num==-1||rec!=slot_num)
	{
		readintoMem=true;
		return getNextRecord(rid,data);
	}

	//cout<<currentpage<<"   "<<currentid.slotNum<<endl;
	return 0;
}
bool RBFM_ScanIterator::condition(const char * data)
{
	void *result_s=(void*)malloc(mem);
	void *comp_s=(void*)malloc(mem);
	if(compOp==6)
	{

		free(result_s);
		free(comp_s);
		return true;
	}
	for(int i=0;i<(int)recordDescriptor.size();i++)
	{
		if(recordDescriptor[i].name==conditionAttribute)
		{

			int start,end;
			int r;
			memcpy(&start,(char*)data+sizeof(int)*i,sizeof(int));
			memcpy(&end,(char*)data+sizeof(int)+sizeof(int)*i,sizeof(int));
			switch(recordDescriptor[i].type)
			{
			case 0:
				free(result_s);
				free(comp_s);
				int result,comp;
				memcpy(&result,(char*)data+start,sizeof(int));
				memcpy(&comp,(char*)value,sizeof(int));
				switch(compOp)
				{
				case 0:
					return result==comp;
					break;
				case 1:
					return result<comp;
					break;
				case 2:
					return result>comp;
					break;
				case 3:
					return result<=comp;
					break;
				case 4:
					return result>=comp;
					break;
				case 5:
					return result!=comp;
					break;
				case 6:
					return true;
				default:
					return false;
				}
				break;
			case 1:
				free(result_s);
				free(comp_s);
				float result_f,comp_f;
				memcpy(&result_f,(char*)data+start,sizeof(float));
				memcpy(&comp_f,(char*)value,sizeof(float));
				switch(compOp)
				{
				case 0:
					return result_f==comp_f;
					break;
				case 1:
					return result_f<comp_f;
					break;
				case 2:
					return result_f>comp_f;
					break;
				case 3:
					return result_f<=comp_f;
					break;
				case 4:
					return result_f>=comp_f;
					break;
				case 5:
					return result_f!=comp_f;
					break;
				case 6:
					return true;
				default:
					return false;
				}
				break;
			case 2:
				int len_1,len_2;
				memcpy(&len_1,(char*)data+start,sizeof(int));
				memcpy(&len_2,(char*)value,sizeof(int));
				free(result_s);
				free(comp_s);
				result_s=(void*)malloc(len_1+1);
				comp_s=(void*)malloc(len_2+1);
				memset(result_s,'\0',len_1+1);
				memset(comp_s,'\0',len_2+1);
				memcpy((char*)result_s,(((char*)(data))+start+sizeof(int)),len_1);
				memcpy((char*)comp_s,(((char*)(value))+sizeof(int)),len_2);
				//cout<<result_s+'\0'<< "  "<<comp_s+'\0';
				r=strcmp((char*)result_s,(char*)comp_s);
				free(result_s);
				free(comp_s);
				switch(compOp)
				{
				case 0:
					return r==0;
					break;
				case 1:
					return r<0;
					break;
				case 2:
					return r>0;
					break;
				case 3:
					return r<=0;
					break;
				case 4:
					return r>=0;
					break;
				case 5:
					return r!=0;
					break;
				case 6:
					return true;
				default:
					return false;
				}
				break;
			default:
				return -1;
			}
			free(result_s);
			free(comp_s);
			return 0;
		}
	}
	return 11;
}
