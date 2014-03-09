
#include "qe.h"
Filter::Filter(Iterator* input, const Condition &condition) {
	this->iter=input;
	this->con=condition;
}
RC Filter::getNextTuple(void *data)
{
	RC rc= iter->getNextTuple(data);
	if(rc!=0)
		return rc;
	while(rc==0)
	{
		vector<Attribute> *a=new vector<Attribute>();
		iter->getAttributes(*a);
		int offset=0;
		int result=0;
		for(int i=0;i<(int)a->size();i++)
		{
			if(a->at(i).name==con.lhsAttr)
			{
				int len=0;
				if(a->at(i).type==TypeInt)
				{
					len=sizeof(int);
					int num1,num2;
					memcpy(&num1,(char*)data+offset,len);
					memcpy(&num2,(char*)con.rhsValue.data,len);
					if(con.op==NO_OP)
					{
						result=1;
						break;
					}
					else if(con.op==EQ_OP)
					{
						if(num1==num2)
							result=1;
						break;
					}
					else if(con.op==LE_OP)
					{
						if(num1<=num2)
							result=1;
						break;
					}
					else if(con.op==GE_OP)
					{
						if(num1>=num2)
							result=1;
						break;
					}
					else if(con.op==LT_OP)
					{
						if(num1<num2)
							result=1;
						break;
					}
					else if(con.op==GT_OP)
					{
						if(num1>num2)
							result=1;
						break;
					}
					else
					{
						if(num1!=num2)
							result=1;
						break;
					}
				}
				else if(a->at(i).type==TypeReal)
				{
					len=sizeof(float);
					float num1,num2;
					memcpy(&num1,(char*)data+offset,len);
					memcpy(&num2,(char*)con.rhsValue.data,len);
					if(con.op==NO_OP)
					{
						result=1;
						break;
					}
					else if(con.op==EQ_OP)
					{
						if(num1==num2)
							result=1;
						break;
					}
					else if(con.op==LE_OP)
					{
						if(num1<=num2)
							result=1;
						break;
					}
					else if(con.op==GE_OP)
					{
						if(num1>=num2)
							result=1;
						break;
					}
					else if(con.op==LT_OP)
					{
						if(num1<num2)
							result=1;
						break;
					}
					else if(con.op==GT_OP)
					{
						if(num1>num2)
							result=1;
						break;
					}
					else
					{
						if(num1!=num2)
							result=1;
						break;
					}
				}
				else if(a->at(i).type==TypeVarChar)
				{
					int len_1,len_2;
					void* result_s=(void*)malloc(PAGE_SIZE);
					void* comp_s=(void*)malloc(PAGE_SIZE);
					memcpy(&len_1,(char*)data+offset,sizeof(int));
					memcpy(&len_2,(char*)con.rhsValue.data,sizeof(int));
					result_s=(void*)malloc(len_1+1);
					comp_s=(void*)malloc(len_2+1);
					memset(result_s,'\0',len_1+1);
					memset(comp_s,'\0',len_2+1);
					memcpy((char*)result_s,(((char*)(data))+offset+sizeof(int)),len_1);
					memcpy((char*)comp_s,(((char*)(con.rhsValue.data))+sizeof(int)),len_2);
					int r=strcmp((char*)result_s,(char*)comp_s);
					free(result_s);
					free(comp_s);
					if(con.op==NO_OP)
					{
						result=1;
						break;
					}
					else if(con.op==EQ_OP)
					{
						if(r==0)
							result=1;
						break;
					}
					else if(con.op==LE_OP)
					{
						if(r<=0)
							result=1;
						break;
					}
					else if(con.op==GE_OP)
					{
						if(r>=0)
							result=1;
						break;
					}
					else if(con.op==LT_OP)
					{
						if(r<0)
							result=1;
						break;
					}
					else if(con.op==GT_OP)
					{
						if(r>0)
							result=1;
						break;
					}
					else
					{
						if(r!=0)
							result=1;
						break;
					}
				}
			}
			else
			{
				int len=0;
				if(a->at(i).type==TypeInt)
				{
					len=sizeof(int);
				}
				else if(a->at(i).type==TypeReal)
				{
					len=sizeof(float);
				}
				else if(a->at(i).type==TypeVarChar)
				{
					memcpy(&len,(char*)data+offset,sizeof(int));
					len+=sizeof(int);
				}
				offset+=len;
			}
		}
		a->clear();
		if(result==1)
		{
			break;
		}
		rc= iter->getNextTuple(data);
	}
	return rc;
}
void Filter::getAttributes(vector<Attribute> &attrs) const
{
	iter->getAttributes(attrs);
}
Project::Project(Iterator *input,                            // Iterator of input R
               const vector<string> &attrNames)
{
	this->iter=input;
	this->attrNames=new vector<Attribute>();
	vector<Attribute> *tmp=new vector<Attribute>();
	input->getAttributes(*tmp);
	int j=0;
	for(int i=0;i<(int)tmp->size();i++)
	{
		if(tmp->at(i).name==attrNames.at(j))
		{
			this->attrNames->push_back(tmp->at(i));
			j++;
		}
	}
	tmp->clear();
};
RC Project::getNextTuple(void *data)
{
	void *tmp=(void *)malloc(PAGE_SIZE);
	RC rc=iter->getNextTuple(tmp);
	if(rc!=0)
	{
		free(tmp);
		return rc;
	}
	else
	{
		int offset=0;
		int off=0;
		int j=0;
		vector<Attribute> *a=new vector<Attribute>();
		iter->getAttributes(*a);
		for(int i=0;i<(int)a->size();i++)
		{
			if(a->at(i).name==attrNames->at(j).name)
			{
				int len=0;
				if(a->at(i).type==TypeInt)
				{
					len=sizeof(int);
				}
				else if(a->at(i).type==TypeReal)
				{
					len=sizeof(float);
				}
				else if(a->at(i).type==TypeVarChar)
				{
					memcpy(&len,(char*)tmp+offset,sizeof(int));
					len+=sizeof(int);
				}
				memcpy((char*)data+off,(char*)tmp+offset,len);
				off+=len;
				offset+=len;
				j++;
			}
			else
			{
				int len=0;
				if(a->at(i).type==TypeInt)
				{
					len=sizeof(int);
				}
				else if(a->at(i).type==TypeReal)
				{
					len=sizeof(float);
				}
				else if(a->at(i).type==TypeVarChar)
				{
					memcpy(&len,(char*)tmp+offset,sizeof(int));
					len+=sizeof(int);
				}
				offset+=len;
			}
		}
		a->clear();
		free(tmp);
		return rc;
	}
};
void Project::getAttributes(vector<Attribute> &attrs) const
{
	attrs=*(this->attrNames);
}
NLJoin::NLJoin(Iterator *leftIn,                             // Iterator of input R
               TableScan *rightIn,                           // TableScan Iterator of input S
               const Condition &condition,                   // Join condition
               const unsigned numPages                       // Number of pages can be used to do join (decided by the optimizer)
        )
{
	this->iterl=leftIn;
	this->iterr=rightIn;
	this->con=condition;
	left=true;
	tmp1=(void*)malloc(PAGE_SIZE);
	tmp2=(void*)malloc(PAGE_SIZE);
	result_s=(void*)malloc(PAGE_SIZE);
	comp_s=(void*)malloc(PAGE_SIZE);
	left=true;
	vector<Attribute> *tmp1=new vector<Attribute>();
	iterl->getAttributes(*tmp1);
	for(int i=0;i<(int)tmp1->size();i++)
		attrs.push_back(tmp1->at(i));
	iterr->getAttributes(*tmp1);
	for(int i=0;i<(int)tmp1->size();i++)
		attrs.push_back(tmp1->at(i));
	tmp1->clear();
}
RC NLJoin::getNextTuple(void *data)
{
	RC rc=0;
	vector<Attribute> *a=new vector<Attribute>();
	while(rc==0)
	{
		if(left)
		{
			rc=iterl->getNextTuple(tmp1);
			left=false;
			if(rc!=0)
				break;
			iterr->setIterator();
		}
		rc=iterr->getNextTuple(tmp2);
		if(rc!=0)
		{
			left=true;
			rc=0;
		}
		else
		{
			int off=0;
			int total_1=0;
			int total_2=0;
			iterr->getAttributes(*a);
			for(int i=0;i<(int)a->size();i++)
			{
				if(a->at(i).name==con.rhsAttr)
				{
					break;
				}
				else
				{
					int len=0;
					if(a->at(i).type==TypeInt)
					{
						len=sizeof(int);
					}
					else if(a->at(i).type==TypeReal)
					{
						len=sizeof(float);
					}
					else if(a->at(i).type==TypeVarChar)
					{
						memcpy(&len,(char*)tmp2+off,sizeof(int));
						len+=sizeof(int);
					}
					off+=len;
				}
			}
			for(int i=0;i<(int)a->size();i++)
			{
				int len=0;
				if(a->at(i).type==TypeInt)
				{
					len=sizeof(int);
				}
				else if(a->at(i).type==TypeReal)
				{
					len=sizeof(float);
				}
				else if(a->at(i).type==TypeVarChar)
				{
					memcpy(&len,(char*)tmp2+off,sizeof(int));
					len+=sizeof(int);
				}
				total_2+=len;
			}
			a->clear();
			iterl->getAttributes(*a);
			for(int i=0;i<(int)a->size();i++)
			{
				//cout<<a->at(i).name<<endl;
				int len=0;
				if(a->at(i).type==TypeInt)
				{
					len=sizeof(int);
				}
				else if(a->at(i).type==TypeReal)
				{
					len=sizeof(float);
				}
				else if(a->at(i).type==TypeVarChar)
				{
					memcpy(&len,(char*)tmp2+off,sizeof(int));
					len+=sizeof(int);
				}
				total_1+=len;
			}
			int offset=0;
			int result=0;
			int len_1,len_2;
			for(int i=0;i<(int)a->size();i++)
			{
				if(a->at(i).name==con.lhsAttr)
				{
					int len=0;
					if(a->at(i).type==TypeInt)
					{
						len_1=sizeof(int);
						len_2=sizeof(int);
						len=sizeof(int);
						int num1,num2;
						memcpy(&num1,(char*)tmp1+offset,len);
						memcpy(&num2,(char*)tmp2+off,len);
						if(con.op==NO_OP)
						{
							result=1;
							break;
						}
						else if(con.op==EQ_OP)
						{
							if(num1==num2)
								result=1;
							break;
						}
						else if(con.op==LE_OP)
						{
							if(num1<=num2)
								result=1;
							break;
						}
						else if(con.op==GE_OP)
						{
							if(num1>=num2)
								result=1;
							break;
						}
						else if(con.op==LT_OP)
						{
							if(num1<num2)
								result=1;
							break;
						}
						else if(con.op==GT_OP)
						{
							if(num1>num2)
								result=1;
							break;
						}
						else
						{
							if(num1!=num2)
								result=1;
							break;
						}
					}
					else if(a->at(i).type==TypeReal)
					{
						len=sizeof(float);
						len_1=sizeof(float);
						len_2=sizeof(float);
						float num1,num2;
						memcpy(&num1,(char*)tmp1+offset,len);
						memcpy(&num2,(char*)tmp2+off,len);
						if(con.op==NO_OP)
						{
							result=1;
							break;
						}
						else if(con.op==EQ_OP)
						{
							if(num1==num2)
								result=1;
							break;
						}
						else if(con.op==LE_OP)
						{
							if(num1<=num2)
								result=1;
							break;
						}
						else if(con.op==GE_OP)
						{
							if(num1>=num2)
								result=1;
							break;
						}
						else if(con.op==LT_OP)
						{
							if(num1<num2)
								result=1;
							break;
						}
						else if(con.op==GT_OP)
						{
							if(num1>num2)
								result=1;
							break;
						}
						else
						{
							if(num1!=num2)
								result=1;
							break;
						}
					}
					else if(a->at(i).type==TypeVarChar)
					{
						memcpy(&len_1,(char*)tmp1+offset,sizeof(int));
						memcpy(&len_2,(char*)tmp2+off,sizeof(int));
						result_s=(void*)malloc(len_1+1);
						comp_s=(void*)malloc(len_2+1);
						memset(result_s,'\0',len_1+1);
						memset(comp_s,'\0',len_2+1);
						memcpy((char*)result_s,(((char*)(tmp1))+offset+sizeof(int)),len_1);
						memcpy((char*)comp_s,(((char*)(tmp2))+off+sizeof(int)),len_2);
						int r=strcmp((char*)result_s,(char*)comp_s);
						if(con.op==NO_OP)
						{
							result=1;
							break;
						}
						else if(con.op==EQ_OP)
						{
							if(r==0)
								result=1;
							break;
						}
						else if(con.op==LE_OP)
						{
							if(r<=0)
								result=1;
							break;
						}
						else if(con.op==GE_OP)
						{
							if(r>=0)
								result=1;
							break;
						}
						else if(con.op==LT_OP)
						{
							if(r<0)
								result=1;
							break;
						}
						else if(con.op==GT_OP)
						{
							if(r>0)
								result=1;
							break;
						}
						else
						{
							if(r!=0)
								result=1;
							break;
						}
					}
				}
				else
				{
					int len=0;
					if(a->at(i).type==TypeInt)
					{
						len=sizeof(int);
					}
					else if(a->at(i).type==TypeReal)
					{
						len=sizeof(float);
					}
					else if(a->at(i).type==TypeVarChar)
					{
						memcpy(&len,(char*)data+offset,sizeof(int));
						len+=sizeof(int);
					}
					offset+=len;
				}
			}
			if(result==1)
			{
				memcpy((char*)data,(char*)tmp1,total_1);
				memcpy((char*)data+total_1,(char*)tmp2,total_2);
				break;
			}
		}
	}
	return rc;
}
void NLJoin::getAttributes(vector<Attribute> &attrs) const
{
	attrs=this->attrs;
}
INLJoin::INLJoin(Iterator *leftIn,                               // Iterator of input R
                IndexScan *rightIn,                             // IndexScan Iterator of input S
                const Condition &condition,                     // Join condition
                const unsigned numPages                         // Number of pages can be used to do join (decided by the optimizer)
        )
{
	this->iterl=leftIn;
	this->iterr=rightIn;
	this->con=condition;
	left=true;
	tmp1=(void*)malloc(PAGE_SIZE);
	tmp2=(void*)malloc(PAGE_SIZE);
	result_s=(void*)malloc(PAGE_SIZE);
	comp_s=(void*)malloc(PAGE_SIZE);
	left=true;
	vector<Attribute> *tmp1=new vector<Attribute>();
	iterl->getAttributes(*tmp1);
	for(int i=0;i<(int)tmp1->size();i++)
		attrs.push_back(tmp1->at(i));
	iterr->getAttributes(*tmp1);
	for(int i=0;i<(int)tmp1->size();i++)
		attrs.push_back(tmp1->at(i));
	tmp1->clear();
}
void INLJoin::getAttributes(vector<Attribute> &attrs) const
{
	attrs=this->attrs;
}
RC INLJoin::getNextTuple(void *data)
{
	RC rc=0;
	vector<Attribute> *a=new vector<Attribute>();
	while(rc==0)
	{
		if(left)
		{
			rc=iterl->getNextTuple(tmp1);
			left=false;
			if(rc!=0)
				break;
			iterr->setIterator(NULL,NULL,true,true);
		}
		rc=iterr->getNextTuple(tmp2);
		if(rc!=0)
		{
			left=true;
			rc=0;
		}
		else
		{
			int off=0;
			int total_1=0;
			int total_2=0;
			iterr->getAttributes(*a);
			for(int i=0;i<(int)a->size();i++)
			{
				if(a->at(i).name==con.rhsAttr)
				{
					break;
				}
				else
				{
					int len=0;
					if(a->at(i).type==TypeInt)
					{
						len=sizeof(int);
					}
					else if(a->at(i).type==TypeReal)
					{
						len=sizeof(float);
					}
					else if(a->at(i).type==TypeVarChar)
					{
						memcpy(&len,(char*)tmp2+off,sizeof(int));
						len+=sizeof(int);
					}
					off+=len;
				}
			}
			for(int i=0;i<(int)a->size();i++)
			{
				int len=0;
				if(a->at(i).type==TypeInt)
				{
					len=sizeof(int);
				}
				else if(a->at(i).type==TypeReal)
				{
					len=sizeof(float);
				}
				else if(a->at(i).type==TypeVarChar)
				{
					memcpy(&len,(char*)tmp2+off,sizeof(int));
					len+=sizeof(int);
				}
				total_2+=len;
			}
			a->clear();
			iterl->getAttributes(*a);
			for(int i=0;i<(int)a->size();i++)
			{
				//cout<<a->at(i).name<<endl;
				int len=0;
				if(a->at(i).type==TypeInt)
				{
					len=sizeof(int);
				}
				else if(a->at(i).type==TypeReal)
				{
					len=sizeof(float);
				}
				else if(a->at(i).type==TypeVarChar)
				{
					memcpy(&len,(char*)tmp2+off,sizeof(int));
					len+=sizeof(int);
				}
				total_1+=len;
			}
			int offset=0;
			int result=0;
			int len_1,len_2;
			for(int i=0;i<(int)a->size();i++)
			{
				if(a->at(i).name==con.lhsAttr)
				{
					int len=0;
					if(a->at(i).type==TypeInt)
					{
						len_1=sizeof(int);
						len_2=sizeof(int);
						len=sizeof(int);
						int num1,num2;
						memcpy(&num1,(char*)tmp1+offset,len);
						memcpy(&num2,(char*)tmp2+off,len);
						if(con.op==NO_OP)
						{
							result=1;
							break;
						}
						else if(con.op==EQ_OP)
						{
							if(num1==num2)
								result=1;
							break;
						}
						else if(con.op==LE_OP)
						{
							if(num1<=num2)
								result=1;
							break;
						}
						else if(con.op==GE_OP)
						{
							if(num1>=num2)
								result=1;
							break;
						}
						else if(con.op==LT_OP)
						{
							if(num1<num2)
								result=1;
							break;
						}
						else if(con.op==GT_OP)
						{
							if(num1>num2)
								result=1;
							break;
						}
						else
						{
							if(num1!=num2)
								result=1;
							break;
						}
					}
					else if(a->at(i).type==TypeReal)
					{
						len=sizeof(float);
						len_1=sizeof(float);
						len_2=sizeof(float);
						float num1,num2;
						memcpy(&num1,(char*)tmp1+offset,len);
						memcpy(&num2,(char*)tmp2+off,len);
						if(con.op==NO_OP)
						{
							result=1;
							break;
						}
						else if(con.op==EQ_OP)
						{
							if(num1==num2)
								result=1;
							break;
						}
						else if(con.op==LE_OP)
						{
							if(num1<=num2)
								result=1;
							break;
						}
						else if(con.op==GE_OP)
						{
							if(num1>=num2)
								result=1;
							break;
						}
						else if(con.op==LT_OP)
						{
							if(num1<num2)
								result=1;
							break;
						}
						else if(con.op==GT_OP)
						{
							if(num1>num2)
								result=1;
							break;
						}
						else
						{
							if(num1!=num2)
								result=1;
							break;
						}
					}
					else if(a->at(i).type==TypeVarChar)
					{
						memcpy(&len_1,(char*)tmp1+offset,sizeof(int));
						memcpy(&len_2,(char*)tmp2+off,sizeof(int));
						result_s=(void*)malloc(len_1+1);
						comp_s=(void*)malloc(len_2+1);
						memset(result_s,'\0',len_1+1);
						memset(comp_s,'\0',len_2+1);
						memcpy((char*)result_s,(((char*)(tmp1))+offset+sizeof(int)),len_1);
						memcpy((char*)comp_s,(((char*)(tmp2))+off+sizeof(int)),len_2);
						int r=strcmp((char*)result_s,(char*)comp_s);
						if(con.op==NO_OP)
						{
							result=1;
							break;
						}
						else if(con.op==EQ_OP)
						{
							if(r==0)
								result=1;
							break;
						}
						else if(con.op==LE_OP)
						{
							if(r<=0)
								result=1;
							break;
						}
						else if(con.op==GE_OP)
						{
							if(r>=0)
								result=1;
							break;
						}
						else if(con.op==LT_OP)
						{
							if(r<0)
								result=1;
							break;
						}
						else if(con.op==GT_OP)
						{
							if(r>0)
								result=1;
							break;
						}
						else
						{
							if(r!=0)
								result=1;
							break;
						}
					}
				}
				else
				{
					int len=0;
					if(a->at(i).type==TypeInt)
					{
						len=sizeof(int);
					}
					else if(a->at(i).type==TypeReal)
					{
						len=sizeof(float);
					}
					else if(a->at(i).type==TypeVarChar)
					{
						memcpy(&len,(char*)data+offset,sizeof(int));
						len+=sizeof(int);
					}
					offset+=len;
				}
			}
			if(result==1)
			{
				memcpy((char*)data,(char*)tmp1,total_1);
				memcpy((char*)data+total_1,(char*)tmp2,total_2);
				break;
			}
		}
	}
	return rc;
}
// ... the rest of your implementations go here
