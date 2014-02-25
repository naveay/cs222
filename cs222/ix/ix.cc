
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
	void *data = malloc(PAGE_SIZE);
	int index;
	fileHandle.readPage(0,data);
	memcpy(&index,(char*)data,sizeof(int));
	if(index==-1)
	{
		start_new_tree(fileHandle,attribute,key,rid);
		return 0;
	}
	int l=find_leaf(fileHandle,attribute,index,key);
	node leaf;
	readNode(fileHandle,l,leaf);
	//
	if(leaf.freespace>=(int)((attribute.length+sizeof(int)*4)*3))
	{
		insert_into_leaf(fileHandle,l,attribute,key,rid);
		freeNode(leaf);
		return 0;
	}
	insert_into_leaf_after_splitting(fileHandle,l,attribute,key,rid);
	freeNode(leaf);
	return 0;
}
void IndexManager::print(FileHandle &fileHandle,const Attribute &attribute,int root,const void* key)
{
	cout<<endl;
	cout<<endl;
	node r;
	root=3;
	int level=0;
	//do

	readNode(fileHandle,root,r);
	//while(r.isleaf!=1)
	{
		for(int i=0;i<r.num;i++)
		{
			int tmp,a,b;
			memcpy(&tmp,(char*)r.data+r.start[i]+sizeof(int)*2,sizeof(int));
			memcpy(&a,(char*)r.data+r.start[i],sizeof(int));
			memcpy(&b,(char*)r.data+r.start[i]+sizeof(int),sizeof(int));
			cout<<tmp<<" "<<a<<" "<<b<<" ";
		}
		node sys;
		if(r.rightnode!=-1)
		{
			readNode(fileHandle,r.rightnode,sys);

			do
			{
				readNode(fileHandle,sys.rightnode,sys);
				cout<<" || ";
				for(int i=0;i<sys.num;i++)
				{
					int tmp,a,b;
								memcpy(&tmp,(char*)r.data+r.start[i]+sizeof(int)*2,sizeof(int));
								memcpy(&a,(char*)r.data+r.start[i],sizeof(int));
								memcpy(&b,(char*)r.data+r.start[i]+sizeof(int),sizeof(int));
								cout<<tmp<<" "<<a<<" "<<b<<" ";
				}
			}
			while(sys.rightnode!=-1);cout<<" || ";
			for(int i=0;i<sys.num;i++)
			{
				int tmp;
				memcpy(&tmp,(char*)sys.data+sys.start[i]+sizeof(int)*2,sizeof(int));
				cout<<tmp<<" ";
			}
		}
		level++;
		memcpy(&root,(char*)r.data+r.start[0],sizeof(int));
		readNode(fileHandle,root,r);
	}

cout<<endl;
	readNode(fileHandle,1,r);
		//while(r.isleaf!=1)
		{
			for(int i=0;i<r.num;i++)
			{
				int tmp;
				memcpy(&tmp,(char*)r.data+r.start[i]+sizeof(int)*2,sizeof(int));
				cout<<tmp<<" ";
			}
			node sys;
			if(r.rightnode!=-1)
			{
				readNode(fileHandle,r.rightnode,sys);

				do
				{
					readNode(fileHandle,sys.rightnode,sys);
					cout<<" || ";
					for(int i=0;i<sys.num;i++)
					{
						int tmp;
						memcpy(&tmp,(char*)sys.data+sys.start[i]+sizeof(int)*2,sizeof(int));
						cout<<tmp<<" ";
					}
				}
				while(sys.rightnode!=-1);cout<<" || ";
				for(int i=0;i<sys.num;i++)
				{
					int tmp;
					memcpy(&tmp,(char*)sys.data+sys.start[i]+sizeof(int)*2,sizeof(int));
					cout<<tmp<<" ";
				}
			}
			level++;
			memcpy(&root,(char*)r.data+r.start[0],sizeof(int));
			readNode(fileHandle,root,r);
		}
	//while(r.isleaf!=1);


}
int IndexManager::find_leaf(FileHandle &fileHandle,const Attribute &attribute,int root,const void* key)
{
	node r;
	readNode(fileHandle,root,r);
	if(key==NULL)
	{
		while(r.isleaf!=1)
		{
			int page;
			memcpy(&page,(char*)r.data+r.start[0],sizeof(int));
			freeNode(r);
			readNode(fileHandle,page,r);
		}
		int result=r.pagenum;
		freeNode(r);
		return result;
	}
	while(r.isleaf!=1)
	{
		int i=0;
		while(i<r.num)
		{
			if(attribute.type==TypeInt)
			{
				int comp1,comp2;
				memcpy(&comp1,(char*)r.data+r.start[i]+sizeof(int)*2,sizeof(int));
				memcpy(&comp2,(char*)key,sizeof(int));
				if(comp2>=comp1)
				{
					i++;
					continue;
				}
				else
					break;
			}
			else if(attribute.type==TypeReal)
			{
				float comp1,comp2;
				memcpy(&comp1,(char*)r.data+r.start[i]+sizeof(int)*2,sizeof(float));
				memcpy(&comp2,(char*)key,sizeof(float));
				if(comp2>=comp1)
				{
					i++;
					continue;
				}
				else
					break;
			}
			else
			{
				int len_1,len_2;
				memcpy(&len_1,(char*)r.data+r.start[i]+sizeof(int)*2,sizeof(int));
				memcpy(&len_2,(char*)key,sizeof(int));
				void* comp1=(void*)malloc(len_1+1);
				void* comp2=(void*)malloc(len_2+1);
				memset(comp1,'\0',len_1+1);
				memset(comp2,'\0',len_2+1);
				memcpy((char*)comp1,(char*)r.data+r.start[i]+sizeof(int)*3,len_1);
				memcpy((char*)comp2,(char*)key+sizeof(int),len_2);
				int r=strcmp((char*)comp1,(char*)comp2);
				free(comp1);
				free(comp2);
				if(r<=0)
				{
					i++;
					continue;
				}
				else
					break;
			}
		}
		if(i<r.num)
		{
			int page;
			memcpy(&page,(char*)r.data+r.start[i],sizeof(int));
			freeNode(r);
			readNode(fileHandle,page,r);
		}
		else
		{
			int page;
			memcpy(&page,(char*)r.data+r.start[i-1]+sizeof(int),sizeof(int));
			freeNode(r);
			readNode(fileHandle,page,r);
		}
	}
	int result=r.pagenum;
	freeNode(r);
	return result;
}
RC IndexManager::deleteEntry(FileHandle &fileHandle, const Attribute &attribute, const void *key, const RID &rid)
{
	void *data = malloc(PAGE_SIZE);
	int index;
	fileHandle.readPage(0,data);
	memcpy(&index,(char*)data,sizeof(int));
	if(index==-1)
	{
		free(data);
		return -1;
	}
	int l=find_leaf(fileHandle,attribute,index,key);
	node leaf;
	readNode(fileHandle,l,leaf);

	int left_index=0;
	for(left_index=0;left_index<leaf.num;left_index++)
	{
		if(attribute.type==TypeInt)
		{
			int comp1,comp2;
			memcpy(&comp1,(char*)leaf.data+leaf.start[left_index]+sizeof(int)*2,sizeof(int));
			memcpy(&comp2,(char*)key,sizeof(int));
			if(comp1==comp2)
			{
				int page,slot;
				memcpy(&page,(char*)leaf.data+leaf.start[left_index],sizeof(int));
				memcpy(&slot,(char*)leaf.data+leaf.start[left_index]+sizeof(int),sizeof(int));
				if(page==(int)rid.pageNum&&slot==(int)rid.slotNum)
				{
					break;
				}
			}
		}
		else if(attribute.type==TypeReal)
		{
			float comp1,comp2;
			memcpy(&comp1,(char*)leaf.data+leaf.start[left_index]+sizeof(int)*2,sizeof(float));
			memcpy(&comp2,(char*)key,sizeof(float));
			if(comp1==comp2)
			{
				int page,slot;
				memcpy(&page,(char*)leaf.data+leaf.start[left_index],sizeof(int));
				memcpy(&slot,(char*)leaf.data+leaf.start[left_index]+sizeof(int),sizeof(int));
				if(page==(int)rid.pageNum&&slot==(int)rid.slotNum)
				{
					break;
				}
			}
		}
		else
		{
			int len_1,len_2;
			memcpy(&len_1,(char*)leaf.data+leaf.start[left_index]+sizeof(int)*2,sizeof(int));
			memcpy(&len_2,(char*)key,sizeof(int));
			void* comp1=(void*)malloc(len_1+1);
			void* comp2=(void*)malloc(len_2+1);
			memset(comp1,'\0',len_1+1);
			memset(comp2,'\0',len_2+1);
			memcpy((char*)comp1,(char*)leaf.data+leaf.start[left_index]+sizeof(int)*3,len_1);
			memcpy((char*)comp2,(char*)key+sizeof(int),len_2);
			int r=strcmp((char*)comp1,(char*)comp2);
			free(comp1);
			free(comp2);
			if(r==0)
			{
				int page,slot;
				memcpy(&page,(char*)leaf.data+leaf.start[left_index],sizeof(int));
				memcpy(&slot,(char*)leaf.data+leaf.start[left_index]+sizeof(int),sizeof(int));
				if(page==(int)rid.pageNum&&slot==(int)rid.slotNum)
				{
					break;
				}
			}
		}
	}
	if(left_index==leaf.num)
	{
		return -1;
	}
	for(int i=left_index;i<leaf.num-1;i++)
	{
		leaf.start[i]=leaf.start[i+1];
		leaf.length[i]=leaf.length[i+1];
	}
	leaf.num-=1;
	writeNode(fileHandle,leaf);
	freeNode(leaf);
	return 0;
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
	int x=left.pagenum;
	memcpy((char*)root.data+root.nextinsert_pos,&x,sizeof(int));
	x=right.pagenum;
	memcpy((char*)root.data+root.nextinsert_pos+sizeof(int),&x,sizeof(int));
	memcpy((char*)root.data+root.nextinsert_pos+sizeof(int)*2,(char*)key,length);
	void* re=(void*)malloc(PAGE_SIZE);
	x=root.pagenum;
	memcpy((char*)re,&x,sizeof(int));
	fileHandle.writePage(0,re);
	writeNode(fileHandle,root);
	free(re);
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
	unsigned int x=rid.pageNum;
	memcpy((char*)root.data+root.nextinsert_pos,&x,sizeof(int));
	x=rid.slotNum;
	memcpy((char*)root.data+root.nextinsert_pos+sizeof(int),&x,sizeof(int));
	memcpy((char*)root.data+root.nextinsert_pos+sizeof(int)*2,(char*)key,length);
	void* re=(void*)malloc(PAGE_SIZE);
	int y=root.pagenum;
	memcpy((char*)re,&y,sizeof(int));
	fileHandle.writePage(0,re);
	writeNode(fileHandle,root);
	free(re);
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
		int z=res.start[i];
		res.start[i+1]=z;
		z=res.length[i];
		res.length[i+1]=z;
	}
	res.start[insertion_point]=res.nextinsert_pos;
	length+=sizeof(int)*2;
	res.length[insertion_point]=length;
	unsigned int i=rid.pageNum;
	memcpy((char*)res.data+res.nextinsert_pos,&i,sizeof(int));
	i=rid.slotNum;
	memcpy((char*)res.data+res.nextinsert_pos+sizeof(int),&i,sizeof(int));
	memcpy((char*)res.data+res.nextinsert_pos+sizeof(int)*2,(char*)key,length-sizeof(int)*2);
	res.nextinsert_pos=res.nextinsert_pos-length;
	res.freespace-=length+sizeof(int)*2;
	res.num++;
	memcpy((char*)new_leaf.data,(char*)res.data,PAGE_SIZE);
	num_key+=1;
	split=cut(num_key);
	res.num=split;
	new_leaf.num=0;
	for(int i=split,j=0;i<num_key;i++,j++)
	{
		int z=res.start[i];
		new_leaf.start[j]=z;
		z=res.length[i];
		new_leaf.length[j]=z;
		new_leaf.num++;
	}
	void * prime=(void*)malloc(new_leaf.length[0]-sizeof(int)*2);
	memcpy((char*)prime,(char*)new_leaf.data+new_leaf.start[0]+sizeof(int)*2,new_leaf.length[0]-sizeof(int)*2);
	writeNode(fileHandle,res);
	writeNode(fileHandle,new_leaf);
	insert_into_parent(fileHandle,attribute,res,prime,new_leaf);
	writeNode(fileHandle,res);
	writeNode(fileHandle,new_leaf);

	free(prime);
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
	//
	if(p.freespace>=(int(attribute.length+sizeof(int)*4)*3))
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
	if(attribute.type==TypeInt)
	{
		parent.length[left_index]=sizeof(int)+sizeof(int)*2;
	}
	else if(attribute.type==TypeReal)
	{
		parent.length[left_index]=sizeof(float)+sizeof(int)*2;
	}
	else
	{
		int len;
		memcpy(&len,(char*)key,sizeof(int));
		parent.length[left_index]=sizeof(int)+len+sizeof(int)*2;
	}

	int n=right.leftnode;
	memcpy((char*)parent.data+parent.nextinsert_pos,&n,sizeof(int));
	n=right.pagenum;
	memcpy((char*)parent.data+parent.nextinsert_pos+sizeof(int),&n,sizeof(int));
	memcpy((char*)parent.data+parent.nextinsert_pos+sizeof(int)*2,(char*)key,parent.length[left_index]-sizeof(int)*2);
	if(left_index!=parent.num)
	{
		memcpy((char*)parent.data+parent.start[left_index+1],&n,sizeof(int));
	}
	parent.num++;
	right.parent=parent.pagenum;
	//writeNode(fileHandle,parent);

	int num=(int)fileHandle.getNumberOfPages();
	node new_leaf;
	initialIndex(fileHandle,num,new_leaf);
	new_leaf.isleaf=0;
	int split=0;
	new_leaf.parent=parent.parent;
	new_leaf.rightnode=parent.rightnode;
	new_leaf.leftnode=parent.pagenum;
	parent.rightnode=new_leaf.pagenum;

	split=cut(parent.num);
	new_leaf.num=0;
	void * prime=(void*)malloc(parent.length[split-1]-sizeof(int)*2);
	memcpy((char*)prime,(char*)parent.data+parent.start[split-1]+sizeof(int)*2,parent.length[split-1]-sizeof(int)*2);
	memcpy((char*)new_leaf.data,(char*)parent.data,PAGE_SIZE);
	for(int i=split,j=0;i<parent.num;i++,j++)
	{
		new_leaf.start[j]=parent.start[i];
		new_leaf.length[j]=parent.length[i];
		new_leaf.num++;
	}
	parent.num=split-1;
	for(int i=0;i<new_leaf.num;i++)
	{
		int child;
		memcpy(&child,(char*)new_leaf.data+new_leaf.start[i],sizeof(int));
		node p,p2;
		readNode(fileHandle,child,p);
		p.parent=new_leaf.pagenum;
		writeNode(fileHandle,p);
		freeNode(p);

		memcpy(&child,(char*)new_leaf.data+new_leaf.start[i]+sizeof(int),sizeof(int));
		readNode(fileHandle,child,p2);
		p2.parent=new_leaf.pagenum;
		writeNode(fileHandle,p2);
		freeNode(p2);
	}
	writeNode(fileHandle,parent);
	writeNode(fileHandle,new_leaf);
	insert_into_parent(fileHandle,attribute,parent,prime,new_leaf);
	writeNode(fileHandle,parent);
	writeNode(fileHandle,new_leaf);
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
	if(attribute.type==TypeInt)
	{
		parent.length[left_index]=sizeof(int)+sizeof(int)*2;
	}
	else if(attribute.type==TypeReal)
	{
		parent.length[left_index]=sizeof(float)+sizeof(int)*2;
	}
	else
	{
		int len;
		memcpy(&len,(char*)key,sizeof(int));
		parent.length[left_index]=sizeof(int)+len+sizeof(int)*2;
	}

	int n=right.leftnode;
	memcpy((char*)parent.data+parent.nextinsert_pos,&n,sizeof(int));
	n=right.pagenum;
	memcpy((char*)parent.data+parent.nextinsert_pos+sizeof(int),&n,sizeof(int));
	memcpy((char*)parent.data+parent.nextinsert_pos+sizeof(int)*2,(char*)key,parent.length[left_index]-sizeof(int)*2);
	if(left_index!=parent.num)
	{
		memcpy((char*)parent.data+parent.start[left_index+1],&n,sizeof(int));
	}
	parent.num++;
	right.parent=parent.pagenum;
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
	return left_index;
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
		int z=res.start[i];
		res.start[i+1]=z;
		z=res.length[i];
		res.length[i+1]=z;
	}
	res.start[insertion_point]=res.nextinsert_pos;
	length+=sizeof(int)*2;
	res.length[insertion_point]=length;
	unsigned int i=rid.pageNum;
	memcpy((char*)res.data+res.nextinsert_pos,&i,sizeof(int));
	i=rid.slotNum;
	memcpy((char*)res.data+res.nextinsert_pos+sizeof(int),&i,sizeof(int));
	memcpy((char*)res.data+res.nextinsert_pos+sizeof(int)*2,(char*)key,length-sizeof(int)*2);
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
	FileHandle t;
	PagedFileManager *pfm=PagedFileManager::instance();
	RC rc=pfm->openFile(fileHandle.pfilename.c_str(),t);
	if(rc!=0)
		return -1;
	pfm->closeFile(t);
	ix_ScanIterator._index_manager=this;
	ix_ScanIterator.fileHandle=fileHandle;
	ix_ScanIterator.attribute=attribute;
	ix_ScanIterator.lowKey=lowKey;
	ix_ScanIterator.highKey=highKey;
	ix_ScanIterator.lowKeyInclusive=lowKeyInclusive;
	ix_ScanIterator.highKeyInclusive=highKeyInclusive;
	ix_ScanIterator.currentpage=-1;
	ix_ScanIterator.slot=-1;
	return 0;
}

IX_ScanIterator::IX_ScanIterator()
{
}

IX_ScanIterator::~IX_ScanIterator()
{
}

RC IX_ScanIterator::getNextEntry(RID &rid, void *key)
{
	if(currentpage==-1)
	{
		void *data = malloc(PAGE_SIZE);
		int index;
		fileHandle.readPage(0,data);
		memcpy(&index,(char*)data,sizeof(int));
		if(index==-1)
		{
			free(data);
			return -1;
		}
		int l=_index_manager->find_leaf(fileHandle,attribute,index,lowKey);
		_index_manager->readNode(fileHandle,l,leaf);
		currentpage=l;
	}
	int left_index;
	for(left_index=slot+1;left_index<leaf.num;left_index++)
	{
		if(attribute.type==TypeInt)
		{
			int comp1,comp2,comp3;
			memcpy(&comp1,(char*)leaf.data+leaf.start[left_index]+sizeof(int)*2,sizeof(int));
			if(lowKey!=NULL)
				memcpy(&comp2,(char*)lowKey,sizeof(int));
			if(highKey!=NULL)
				memcpy(&comp3,(char*)highKey,sizeof(int));
			if(highKey!=NULL)
			{
				if(comp1>comp3)
					return -1;
				else if(comp1==comp3)
				{
					if(!highKeyInclusive)
						return -1;
				}
			}

			if(lowKey!=NULL)
			{
				if(comp1<comp2)
					continue;
				else if(comp1==comp2)
				{
					if(lowKeyInclusive)
						break;
					else
						continue;
				}
				else
					break;
			}
			else
				break;


		}
		else if(attribute.type==TypeReal)
		{
			float comp1,comp2,comp3;
			memcpy(&comp1,(char*)leaf.data+leaf.start[left_index]+sizeof(int)*2,sizeof(float));
			if(lowKey!=NULL)
				memcpy(&comp2,(char*)lowKey,sizeof(float));
			if(highKey!=NULL)
				memcpy(&comp3,(char*)highKey,sizeof(float));

			if(highKey!=NULL)
			{
				if(comp1>comp3)
					return -1;
				else if(comp1==comp3)
				{
					if(!highKeyInclusive)
						return -1;
				}
			}

			if(lowKey!=NULL)
			{
				if(comp1<comp2)
					continue;
				else if(comp1==comp2)
				{
					if(lowKeyInclusive)
						break;
					else
						continue;
				}
				else
					break;
			}
			else
				break;
		}
		else
		{
			int len_1,len_2,len_3;
			memcpy(&len_1,(char*)leaf.data+leaf.start[left_index]+sizeof(int)*2,sizeof(int));
			void* comp1=(void*)malloc(len_1+1);
			memset(comp1,'\0',len_1+1);
			memcpy((char*)comp1,(char*)leaf.data+leaf.start[left_index]+sizeof(int)*3,len_1);
			int r,r2;
			if(lowKey!=NULL)
			{
				memcpy(&len_2,(char*)lowKey,sizeof(int));
				void* comp2=(void*)malloc(len_2+1);
				memset(comp2,'\0',len_2+1);
				memcpy((char*)comp2,(char*)lowKey+sizeof(int),len_2);
				r=strcmp((char*)comp1,(char*)comp2);
				free(comp2);
			}

			if(highKey!=NULL)
			{
				memcpy(&len_3,(char*)highKey,sizeof(int));
				void* comp3=(void*)malloc(len_3+1);
				memset(comp3,'\0',len_3+1);
				memcpy((char*)comp3,(char*)highKey+sizeof(int),len_3);
				r2=strcmp((char*)comp1,(char*)comp3);
				free(comp3);
			}

			free(comp1);

			if(highKey!=NULL)
			{
				if(r2>0)
					return -1;
				else if(r2==1)
				{
					if(!highKeyInclusive)
						return -1;
				}
			}
			if(lowKey!=NULL)
			{
				if(r<0)
					continue;
				else if(r==0)
				{
					if(lowKeyInclusive)
						break;
					else
						continue;
				}
				else
					break;
			}
			else
				break;

		}
	}
	if(left_index==leaf.num)
	{
		if(leaf.rightnode==-1)
			return -1;
		else
		{
			currentpage=leaf.rightnode;
			slot=-1;
			_index_manager->freeNode(leaf);
			_index_manager->readNode(fileHandle,currentpage,leaf);
			return getNextEntry(rid, key);
		}
	}

	currentpage=leaf.pagenum;
	slot=left_index;
	unsigned int page,slot_1;
	memcpy(&page,(char*)leaf.data+leaf.start[left_index],sizeof(int));
	memcpy(&slot_1,(char*)leaf.data+leaf.start[left_index]+sizeof(int),sizeof(int));
	rid.pageNum=page;
	rid.slotNum=slot_1;
	if(attribute.type==TypeInt)
	{
		memcpy((char*)key,(char*)leaf.data+leaf.start[left_index]+sizeof(int)*2,sizeof(int));
	}
	else if(attribute.type==TypeReal)
	{
		memcpy((char*)key,(char*)leaf.data+leaf.start[left_index]+sizeof(int)*2,sizeof(float));
	}
	else
	{
		int len;
		memcpy(&len,(char*)leaf.data+leaf.start[left_index]+sizeof(int)*2,sizeof(int));
		memcpy((char*)key,(char*)leaf.data+leaf.start[left_index]+sizeof(int)*2,len+sizeof(int));
	}
	return 0;
}

RC IX_ScanIterator::close()
{
	_index_manager->freeNode(leaf);
	return 0;
}

void IX_PrintError (RC rc)
{
}
