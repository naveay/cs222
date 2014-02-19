
#include "ix.h"

IndexManager* IndexManager::_index_manager = 0;

IndexManager* IndexManager::instance()
{
    if(!_index_manager)
        _index_manager = new IndexManager();

    return _index_manager;
}

IndexManager::IndexManager()
{
	pfm=PagedFileManager::instance();
}

IndexManager::~IndexManager()
{
}

RC IndexManager::createFile(const string &fileName)
{
	return pfm->createFile(fileName.c_str());
}

RC IndexManager::destroyFile(const string &fileName)
{
	return pfm->destroyFile(fileName.c_str());
}

RC IndexManager::openFile(const string &fileName, FileHandle &fileHandle)
{
	RC result=pfm->openFile(fileName.c_str(),fileHandle);
		if(fileHandle.getNumberOfPages()==0)
			initialDirectory(fileHandle,0);
		return result;
}

RC IndexManager::closeFile(FileHandle &fileHandle)
{
	return pfm->closeFile(fileHandle);
}

RC IndexManager::insertEntry(FileHandle &fileHandle, const Attribute &attribute, const void *key, const RID &rid)
{
	return -1;
}

RC IndexManager::deleteEntry(FileHandle &fileHandle, const Attribute &attribute, const void *key, const RID &rid)
{
	return -1;
}

RC IndexManager::initialDirectory(FileHandle &fileHandle, unsigned int page)
{
	void *data = malloc(PAGE_SIZE);
	unsigned int index=0;
	memcpy((char*)data,&index,sizeof(int));
	memcpy((char*)data+sizeof(int),&index,sizeof(int));
	fileHandle.writePage(page,data);
	free(data);
	return 0;
}
RC IndexManager::insert_into_leaf(FileHandle &fileHandle, int page, const Attribute &attribute, const void *key, const RID &rid)
{
	node res;
	readNode(fileHandle,page,res);
	int num_key=res.num;
	int insertion_point=0;
	int length=0;
	for(insertion_point=0;insertion_point<num_key;insertion_point++)
	{
		int start=res.start[insertion_point];
		int len=res.length[insertion_point];
		if(attribute.type==TypeInt)
		{
			length=sizeof(int);
			int comp1,comp2;
			memcpy(&comp1,(char*)res.data+start+sizeof(int)*2,sizeof(int));
			memcpy(&comp2,(char*)key,sizeof(int));
			if(comp1<comp2)
				continue;
			else
				break;
		}
		else if(attribute.type==TypeReal)
		{
			length=sizeof(float);
			float comp1,comp2;
			memcpy(&comp1,(char*)res.data+start+sizeof(int)*2,sizeof(float));
			memcpy(&comp2,(char*)key,sizeof(float));
			if(comp1<comp2)
				continue;
			else
				break;
		}
		else
		{
			int len_1,len_2;
			memcpy(&len_1,(char*)res.data+start+sizeof(int)*2,sizeof(int));
			memcpy(&len_2,(char*)key,sizeof(int));
			length=sizeof(int)+len_2;
			void* comp1=(void*)malloc(len_1+1);
			void* comp2=(void*)malloc(len_2+1);
			memset(comp1,'\0',len_1+1);
			memset(comp2,'\0',len_2+1);
			memcpy((char*)comp1,(char*)res.data+start+sizeof(int)*3,len_1);
			memcpy((char*)comp2,(char*)key+sizeof(int),len_2);
			int r=strcmp((char*)comp1,(char*)comp2);
			free(comp1);
			free(comp2);
			if(r<0)
				continue;
			else
				break;
		}

	}
	for(int i=num_key-1;i>=insertion_point;i--)
	{
		res.start[i+1]=res.start[i];
		res.length[i+1]=res.length[i];
	}
	res.start[insertion_point]=res.nextinsert_pos;
	length+=sizeof(int)*2;
	res.length[insertion_point]=length;
	unsigned int i=rid.pageNum;
	memcpy((char*)res.data+res.nextinsert_pos,&i,sizeof(int));
	i=rid.slotNum;
	memcpy((char*)res.data+res.nextinsert_pos+sizeof(int),&i,sizeof(int));
	memcpy((char*)res.data+res.nextinsert_pos+sizeof(int)*2,key,length-sizeof(int)*2);
	res.nextinsert_pos=res.nextinsert_pos-length;
	res.freespace-=length+sizeof(int)*2;
	res.num++;
	writeNode(fileHandle,res);
	freeNode(res);
	return 0;
}

RC IndexManager::readNode(FileHandle &fileHandle,unsigned int page,node &res)
{
	res.data=(void*)malloc(PAGE_SIZE);
	fileHandle.readPage(page,res.data);
	res.pagenum=page;
	unsigned int free_space=0;
	int offset=PAGE_SIZE-sizeof(int);
	memcpy(&free_space,(char*)res.data+offset,sizeof(int));
	res.nextinsert_pos=free_space;
	offset-=sizeof(int);
	memcpy(&free_space,(char*)res.data+offset,sizeof(int));
	res.freespace=free_space;

	offset-=sizeof(int);
	int isleaf=0;
	memcpy(&isleaf,(char*)res.data+offset,sizeof(int));
	res.isleaf=isleaf;
	offset-=sizeof(int);

	memcpy(&isleaf,(char*)res.data+offset,sizeof(int));   //parent node
	res.parent=isleaf;
	offset-=sizeof(int);

	memcpy(&isleaf,(char*)res.data+offset,sizeof(int));  //left node
	res.leftnode=isleaf;
	offset-=sizeof(int);

	memcpy(&isleaf,(char*)res.data+offset,sizeof(int));   //right node
	res.rightnode=isleaf;
	offset-=sizeof(int);

	memcpy(&isleaf,(char*)res.data+offset,sizeof(int));	//number of node;
	res.num=isleaf;
	offset-=sizeof(int);
	res.start=new int[PAGE_SIZE];
	res.length=new int[PAGE_SIZE];

	for(int i=0;i<isleaf;i++)
	{
		int start,length;
		memcpy(&start,(char*)res.data+offset,sizeof(int));
		res.start[i]=start;
		offset-=sizeof(int);

		memcpy(&length,(char*)res.data+offset,sizeof(int));
		res.length[i]=length;
		offset-=sizeof(int);
	}
	return 0;
}

RC IndexManager::writeNode(FileHandle &fileHandle,node &res)
{
	unsigned int free_space=res.nextinsert_pos;
	int offset=PAGE_SIZE-sizeof(int);
	memcpy((char*)res.data+offset,&free_space,sizeof(int));
	offset-=sizeof(int);
	free_space=res.freespace;
	memcpy((char*)res.data+offset,&free_space,sizeof(int));
	offset-=sizeof(int);

	int isleaf=res.isleaf;
	memcpy((char*)res.data+offset,&isleaf,sizeof(int));
	offset-=sizeof(int);

	isleaf=res.parent;
	memcpy((char*)res.data+offset,&isleaf,sizeof(int));   //parent node
	offset-=sizeof(int);

	isleaf=res.leftnode;
	memcpy((char*)res.data+offset,&isleaf,sizeof(int));  //left node
	offset-=sizeof(int);

	isleaf=res.rightnode;
	memcpy((char*)res.data+offset,&isleaf,sizeof(int));   //right node
	offset-=sizeof(int);

	isleaf=res.num;
	memcpy((char*)res.data+offset,&isleaf,sizeof(int));	//number of node;
	offset-=sizeof(int);

	for(int i=0;i<isleaf;i++)
	{
		int start,length;
		start=res.start[i];
		memcpy((char*)res.data+offset,&start,sizeof(int));
		offset-=sizeof(int);

		length=res.length[i];
		memcpy((char*)res.data+offset,&length,sizeof(int));
		offset-=sizeof(int);
	}
	fileHandle.writePage(res.pagenum,res.data);
	return 0;
}
RC IndexManager::initialIndex(FileHandle &fileHandle,unsigned int page)
{
	node res;
	res.pagenum=page;
	res.nextinsert_pos=0;
	res.freespace=PAGE_SIZE-sizeof(int)*7;
	res.isleaf=0;
	res.parent=-1;
	res.leftnode=-1;
	res.rightnode=-1;
	res.num=0;
	res.data=(void*)malloc(PAGE_SIZE);
	res.start=new int[PAGE_SIZE];
	res.length=new int[PAGE_SIZE];
	writeNode(fileHandle,res);
	freeNode(res);
	return 0;
}
RC IndexManager::freeNode(node &res)
{
	free(res.start);
	free(res.length);
	free(res.data);
	return 0;
}
RC IndexManager::scan(FileHandle &fileHandle,
    const Attribute &attribute,
    const void      *lowKey,
    const void      *highKey,
    bool			lowKeyInclusive,
    bool        	highKeyInclusive,
    IX_ScanIterator &ix_ScanIterator)
{
	return -1;
}

IX_ScanIterator::IX_ScanIterator()
{
}

IX_ScanIterator::~IX_ScanIterator()
{
}

RC IX_ScanIterator::getNextEntry(RID &rid, void *key)
{
	return -1;
}

RC IX_ScanIterator::close()
{
	return -1;
}

void IX_PrintError (RC rc)
{
}
