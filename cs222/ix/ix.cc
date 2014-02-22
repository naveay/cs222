
#include "ix.h"

IndexManager* IndexManager::_index_manager = 0;
#define ORDER 400;
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
			initialDirectory(fileHandle);
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
RC IndexManager::initialDirectory(FileHandle &fileHandle)
{
	void *data = malloc(PAGE_SIZE);
	int index=-1;
	memcpy((char*)data,&index,sizeof(int));
	fileHandle.writePage(0,data);
	free(data);
	return 0;
}
int IndexManager::cut(int length)
{
	if(length%2==0)
		return length/2;
	else
		return length/2+1;
}
RC IndexManager::insert_into_new_root(FileHandle &fileHandle,const Attribute &attribute,node &left,const void* key,node &right)
{
	node root;
	int num=(int)fileHandle.getNumberOfPages();
	initialIndex(fileHandle,num,root);
	root.isleaf=0;
	left.parent=root.pagenum;
	right.parent=root.pagenum;
	root.num++;
	int length=0;
	if(attribute.type==TypeInt)
	{
		length=sizeof(int);
	}
	else if(attribute.type==TypeReal)
	{
		length=sizeof(float);
	}
	else
	{
		int len;
		memcpy(&len,(char*)key,sizeof(int));
		length=sizeof(int)+len;
	}
	root.start[0]=root.nextinsert_pos;
	root.length[0]=length+sizeof(int)*2;
	memcpy((char*)root.data+root.nextinsert_pos,&left.num,sizeof(int));
	memcpy((char*)root.data+root.nextinsert_pos+sizeof(int),&right.num,sizeof(int));
	memcpy((char*)root.data+root.nextinsert_pos+sizeof(int)*2,key,length);
	void* read=(void*)malloc(PAGE_SIZE);
	memcpy((char*)read,&root.num,sizeof(int));
	fileHandle.writePage(0,read);
	writeNode(fileHandle,root);
	free(read);
	freeNode(root);
	return 0;
}
RC IndexManager::start_new_tree(FileHandle &fileHandle,const Attribute &attribute,const void* key,const RID &rid)
{
	node root;
	int num=(int)fileHandle.getNumberOfPages();
	initialIndex(fileHandle,num,root);
	root.isleaf=1;
	root.num++;
	int length=0;
	if(attribute.type==TypeInt)
	{
		length=sizeof(int);
	}
	else if(attribute.type==TypeReal)
	{
		length=sizeof(float);
	}
	else
	{
		int len;
		memcpy(&len,(char*)key,sizeof(int));
		length=sizeof(int)+len;
	}
	root.start[0]=root.nextinsert_pos;
	root.length[0]=length+sizeof(int)*2;
	memcpy((char*)root.data+root.nextinsert_pos,&rid.pageNum,sizeof(int));
	memcpy((char*)root.data+root.nextinsert_pos+sizeof(int),&rid.slotNum,sizeof(int));
	memcpy((char*)root.data+root.nextinsert_pos+sizeof(int)*2,key,length);
	void* read=(void*)malloc(PAGE_SIZE);
	memcpy((char*)read,&root.num,sizeof(int));
	fileHandle.writePage(0,read);
	writeNode(fileHandle,root);
	free(read);
	freeNode(root);
	return 0;
}
RC IndexManager::insert_into_leaf_after_splitting(FileHandle &fileHandle, int page, const Attribute &attribute, const void *key, const RID &rid)
{
	int num=(int)fileHandle.getNumberOfPages();
	node new_leaf;
	initialIndex(fileHandle,num,new_leaf);
	node res;
	new_leaf.isleaf=1;
	int insertion_point=0,split=0;
	readNode(fileHandle,page,res);
	new_leaf.parent=res.parent;
	new_leaf.rightnode=res.rightnode;
	new_leaf.leftnode=res.pagenum;
	res.rightnode=new_leaf.pagenum;

	int num_key=res.num;
	int length=0;
	for(insertion_point=0;insertion_point<num_key;insertion_point++)
	{
		int start=res.start[insertion_point];
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

	memcpy((char*)new_leaf.data,(char*)res.data,res.nextinsert_pos);
	split=cut(num_key);
	res.num=split;
	new_leaf.num=0;
	for(int i=split,j=0;i<num_key;i++,j++)
	{
		new_leaf.start[j]=res.start[i];
		new_leaf.length[j]=res.length[i];
		new_leaf.num++;
	}
	new_leaf.leftnode=res.num;
	new_leaf.rightnode=res.rightnode;
	res.rightnode=new_leaf.num;
	insert_into_parent(fileHandle,attribute,res,key,new_leaf);
	writeNode(fileHandle,res);
	writeNode(fileHandle,new_leaf);
	freeNode(new_leaf);
	freeNode(res);
	return 0;
}
RC IndexManager::insert_into_parent(FileHandle &fileHandle,const Attribute &attribute,node &left,const void* key,node &right)
{
	int left_index;
	int parent=left.parent;
	node p;

	if(parent==-1)
	{
		insert_into_new_root(fileHandle,attribute,left,key,right);
		return 0;
	}
	readNode(fileHandle,parent,p);
	left_index=get_left_index(p,left);
	if(p.freespace>=((attribute.length+sizeof(int)*4)*3))
	{
		insert_into_node(fileHandle,attribute,key,p,left_index,right);
		freeNode(p);
		return 0;
	}
	insert_into_node_after_splitting(fileHandle,attribute,key,p,left_index,right);
	freeNode(p);
	return 0;
}
RC IndexManager::insert_into_node_after_splitting(FileHandle &fileHandle, const Attribute &attribute, const void *key,node &parent,int left_index, node &right)
{
	int i;
	for(i=parent.num;i>left_index;i--)
	{
		parent.start[i]=parent.start[i-1];
		parent.length[i]=parent.length[i-1];
	}
	parent.start[left_index]=parent.nextinsert_pos;
	parent.length[left_index]=right.length[0];
	int n=right.leftnode;
	memcpy((char*)parent.data+parent.nextinsert_pos,&n,sizeof(int));
	n=right.pagenum;
	memcpy((char*)parent.data+parent.nextinsert_pos+sizeof(int),&n,sizeof(int));
	memcpy((char*)parent.data+parent.nextinsert_pos+sizeof(int)*2,(char*)key,right.length[0]-sizeof(int)*2);
	if(!left_index==parent.num)
	{
		memcpy((char*)parent.data+parent.start[left_index+1],&n,sizeof(int));
	}
	parent.num++;
	//writeNode(fileHandle,parent);

	int num=(int)fileHandle.getNumberOfPages();
	node new_leaf;
	initialIndex(fileHandle,num,new_leaf);
	node res;
	new_leaf.isleaf=1;
	int insertion_point=0,split=0;
	new_leaf.parent=parent.parent;
	new_leaf.rightnode=parent.rightnode;
	new_leaf.leftnode=parent.pagenum;
	parent.rightnode=new_leaf.pagenum;

	//_______
	return 0;
}
RC IndexManager::insert_into_node(FileHandle &fileHandle, const Attribute &attribute, const void *key,node &parent,int left_index, node &right)
{
	int i;
	for(i=parent.num;i>left_index;i--)
	{
		parent.start[i]=parent.start[i-1];
		parent.length[i]=parent.length[i-1];
	}
	parent.start[left_index]=parent.nextinsert_pos;
	parent.length[left_index]=right.length[0];
	int n=right.leftnode;
	memcpy((char*)parent.data+parent.nextinsert_pos,&n,sizeof(int));
	n=right.pagenum;
	memcpy((char*)parent.data+parent.nextinsert_pos+sizeof(int),&n,sizeof(int));
	memcpy((char*)parent.data+parent.nextinsert_pos+sizeof(int)*2,(char*)key,right.length[0]-sizeof(int)*2);
	if(!left_index==parent.num)
	{
		memcpy((char*)parent.data+parent.start[left_index+1],&n,sizeof(int));
	}
	parent.num++;
	writeNode(fileHandle,parent);
	return 0;
}
int IndexManager::get_left_index(node &parent, node& left)
{
	int left_index=0;
	for(left_index=0;left_index<parent.num;left_index++)
	{
		int start=parent.start[left_index];
		int pointer;
		memcpy(&pointer,(char*)parent.data+start,sizeof(int));
		if(pointer==left.pagenum)
		{
			return left_index;
		}
	}
	return left_index+1;
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
	void * tmp=(void*)malloc(PAGE_SIZE);
	int total=0;
	for(int i=0;i<res.num;i++)
	{
		int start,length;
		start=res.start[i];
		length=res.length[i];
		memcpy((char*)tmp+total,(char*)res.data+start,length);
		res.start[i]=total;
		total+=length;
	}
	res.nextinsert_pos=total;
	memcpy((char*)res.data,(char*)tmp,total);
	free(tmp);

	res.freespace=PAGE_SIZE-res.nextinsert_pos-sizeof(int)*7-sizeof(int)*2*res.num;
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
RC IndexManager::initialIndex(FileHandle &fileHandle,unsigned int page,node &res)
{
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
