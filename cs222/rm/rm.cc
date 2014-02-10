
#include "rm.h"

RelationManager* RelationManager::_rm = 0;

RelationManager* RelationManager::instance()
{
    if(!_rm)
        _rm = new RelationManager();

    return _rm;
}

RelationManager::RelationManager()
{
	catalog="catalog";
	column_name="columnTable";

	Attribute attr;
	attr.name = "TableId";
	attr.type = TypeInt;
	attr.length = (AttrLength)4;
	catalog_v.push_back(attr);
	catalog_s.push_back(attr.name);

	attr.name = "TableName";
	attr.type = TypeVarChar;
	attr.length = (AttrLength)30;
	catalog_v.push_back(attr);
	catalog_s.push_back(attr.name);

	attr.name = "FileName";
	attr.type = TypeVarChar;
	attr.length = (AttrLength)30;
	catalog_v.push_back(attr);
	catalog_s.push_back(attr.name);

	attr.name = "TableId";
	attr.type = TypeInt;
	attr.length = (AttrLength)4;
	column_name_v.push_back(attr);
	column_name_s.push_back(attr.name);

	attr.name = "ColumnName";
	attr.type = TypeVarChar;
	attr.length = (AttrLength)30;
	column_name_v.push_back(attr);
	column_name_s.push_back(attr.name);

	attr.name = "ColumnType";
	attr.type = TypeInt;
	attr.length = (AttrLength)4;
	column_name_v.push_back(attr);
	column_name_s.push_back(attr.name);

	attr.name = "ColumnLength";
	attr.type = TypeInt;
	attr.length = (AttrLength)4;
	column_name_v.push_back(attr);
	column_name_s.push_back(attr.name);

	attr.name = "Position";
	attr.type = TypeInt;
	attr.length = (AttrLength)4;
	column_name_v.push_back(attr);
	column_name_s.push_back(attr.name);
	initial();

}

RelationManager::~RelationManager()
{
}
void RelationManager::initial()
{
	RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
	RC rc;
	FileHandle fileHandle;
	RID rid;

	unsigned int temp;
	unsigned int offset = 0;
	void *cat_data = (void *)malloc(PAGE_SIZE);
	void *col_data = (void *)malloc(PAGE_SIZE);
	rc = rbfm->createFile(catalog);
	if (rc != 0)
	{
		tableId=0;
		RBFM_ScanIterator rbfm_ScanIterator;
		rbfm->openFile(catalog,fileHandle);
		int l=catalog.length();
		memcpy((char*)cat_data,&l,sizeof(int));
		memcpy((char*)cat_data+sizeof(int),catalog.c_str(),l);
		rbfm->scan(fileHandle,catalog_v,"", NO_OP, NULL,catalog_s,rbfm_ScanIterator);
		while(rbfm_ScanIterator.getNextRecord(rid,col_data)!=RM_EOF)
		{
			memcpy(&l,(char*)col_data,sizeof(int));
			if(l>(int)tableId)
				tableId=l;
		}
		rbfm->closeFile(fileHandle);
	}
	else
	{
		tableId=0;
		memcpy((char*)cat_data,&tableId,sizeof(int));
		offset +=sizeof(int);

		unsigned int tableNameLength = (unsigned int)catalog.length();
		memcpy((char*)cat_data + offset,&tableNameLength,sizeof(int));
		offset+=sizeof(int);

		memcpy((char*)cat_data+offset,catalog.c_str(),sizeof(char) * tableNameLength);
		offset += tableNameLength;

		unsigned int fileNameLength = (unsigned int)catalog.length();
		memcpy((char*)cat_data + offset,&fileNameLength,sizeof(int));
		offset +=sizeof(int);

		memcpy((char*)cat_data+offset,catalog.c_str(),sizeof(char) * fileNameLength);
		offset += tableNameLength;
		rc = UpdateCatalogTable(cat_data);


		offset = 0;
		memcpy((char*)col_data,&tableId,sizeof(int));
		offset +=sizeof(int);

		unsigned int numOfCol = (unsigned int)catalog_v.size();
		for (int i=0;i<(int)numOfCol;i++)
		{

			temp = (unsigned int)catalog_v[i].name.length();
			memcpy((char *)col_data+offset,&temp,sizeof(int));
			offset +=sizeof(int);
			memcpy((char *)col_data+offset,&catalog_v[i].name,temp);
			offset +=temp;
			memcpy((char *)col_data+offset,&catalog_v[i].type,sizeof(int));
			offset +=sizeof(int);
			memcpy((char *)col_data+offset,&catalog_v[i].length,sizeof(int));
			offset +=sizeof(int);
			memcpy((char *)col_data+offset,&i,sizeof(int));
			UpdateColumnTable(col_data);
		}
	}
}
RC RelationManager::createTable(const string &tableName, const vector<Attribute> &attrs)
{
    FileHandle fileHandle;
    RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
    RC rc;
    unsigned int temp;
    unsigned int offset = 0;
    char *cat_data = (char *)malloc(PAGE_SIZE);
    char *col_data = (char *)malloc(PAGE_SIZE);
    rc = rbfm->createFile(tableName);
    if (rc != 0)
    {
        cout << "file is already exist" << endl;
        return -1;
    }
    //update catalog table//
    //table Id count up//
    tableId ++;
    memcpy((char*)cat_data,&tableId,sizeof(int));
    offset +=sizeof(int);

    //length of the file name//
    unsigned int tableNameLength = (unsigned int)tableName.length();
    memcpy((char*)cat_data + offset,&tableNameLength,sizeof(int));
    offset+=sizeof(int);

    //add table name to data//
    memcpy((char*)cat_data+offset,tableName.c_str(),sizeof(char) * tableNameLength);
    offset += tableNameLength;

    //length of the file name//
    //NOTE make the file name and the table name the same//
    unsigned int fileNameLength = (unsigned int)tableName.length();
    memcpy((char*)cat_data + offset,&fileNameLength,sizeof(int));
    offset +=sizeof(int);

    //add file name to table//
    memcpy((char*)cat_data+offset,tableName.c_str(),sizeof(char) * fileNameLength);
    offset += tableNameLength;
    rc = UpdateCatalogTable(cat_data);

    //updata Column Table//


    unsigned int numOfCol = (unsigned int)attrs.size();

    for (int i=0;i<(int)numOfCol;i++)
    {
    	offset = 0;
    	memcpy((char*)col_data,&tableId,sizeof(int));
    	offset +=sizeof(int);
        temp = (unsigned int)attrs[i].name.length();
        memcpy((char *)col_data+offset,&temp,sizeof(int));
        offset +=sizeof(int);
        memcpy((char *)col_data+offset,attrs[i].name.c_str(),temp);
        offset +=temp;

        memcpy((char *)col_data+offset,&attrs[i].type,sizeof(int));

        offset +=sizeof(int);
        memcpy((char *)col_data+offset,&attrs[i].length,sizeof(int));

        offset +=sizeof(int);
        memcpy((char *)col_data+offset,&i,sizeof(int));
        UpdateColumnTable(col_data);
    }

    free(col_data);
    free(cat_data);

    return 0;
}

RC RelationManager::deleteTable(const string &tableName)
{
	FileHandle fileHandle;
	RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
	rbfm->destroyFile(tableName);
	RBFM_ScanIterator rbfm_ScanIterator;
	rbfm->openFile(catalog,fileHandle);
	char * returndata=(char*)malloc(PAGE_SIZE);
	char * ret=(char*)malloc(PAGE_SIZE);
	int l=tableName.length();
	memcpy((char*)returndata,&l,sizeof(int));
	memcpy((char*)returndata+sizeof(int),tableName.c_str(),sizeof(char) * l);
	RID rid;
	rbfm->scan(fileHandle,catalog_v,catalog_v[1].name, EQ_OP, returndata,catalog_s,rbfm_ScanIterator);
	if(rbfm_ScanIterator.getNextRecord(rid,ret)!=RM_EOF)
	{
		rbfm->deleteRecord(fileHandle,catalog_v,rid);
	}
	else
	{
		return -1;
	}
	rbfm->closeFile(fileHandle);
	FileHandle fileHandle1;
	rbfm->openFile(column_name,fileHandle1);
	vector<RID> *r=new vector<RID>();
	rbfm->scan(fileHandle1,column_name_v,column_name_v[0].name, EQ_OP, ret,column_name_s,rbfm_ScanIterator);
	while(rbfm_ScanIterator.getNextRecord(rid,ret)!=RM_EOF)
	{
		r->push_back(rid);
	}
	for(int i=0;i<(int)r->size();i++)
	{
		rbfm->deleteRecord(fileHandle1,column_name_v,r->at(i));
	}
	rbfm->closeFile(fileHandle1);
    return 0;
}

RC RelationManager::getAttributes(const string &tableName, vector<Attribute> &attrs)
{
	RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
	FileHandle fileHandle;
	RBFM_ScanIterator rbfm_ScanIterator;

	void * returndata=(void*)malloc(100);
	void * ret=(void*)malloc(100);
	void * re=(void*)malloc(100);
	int l=tableName.length();
	memcpy((char*)returndata,&l,sizeof(int));
	memcpy((char*)returndata+sizeof(int),tableName.c_str(),sizeof(char) * l);
	RID rid;
	rbfm->openFile(catalog,fileHandle);
	rbfm->scan(fileHandle,catalog_v,catalog_v[1].name, EQ_OP, returndata,catalog_s,rbfm_ScanIterator);
	if(rbfm_ScanIterator.getNextRecord(rid,ret)!=RM_EOF)
	{
		rbfm_ScanIterator.close();
		memcpy((char*)re,(char*)ret,100);
	}
	else
	{
		cout<<"adfadf"<<endl;
		return -10;
	}
	rbfm->closeFile(fileHandle);

	FileHandle fileHandle1;
	rbfm = RecordBasedFileManager::instance();
	rbfm->openFile(column_name,fileHandle1);

	map<int,Attribute> *att_pos=new map<int,Attribute>();
	Attribute attr;
	RBFM_ScanIterator rbfm_ScanIterator1;
	rbfm->scan(fileHandle1,column_name_v,column_name_v[0].name, EQ_OP, re,column_name_s,rbfm_ScanIterator1);
	while(rbfm_ScanIterator1.getNextRecord(rid,ret)!=RM_EOF)
	{
			memcpy(&l,(char*)ret,sizeof(int));
			unsigned int offset = 0;
			unsigned int namelenght;
			int intval;
			offset+=sizeof(int);
			memcpy(&namelenght,(char *)ret+offset,sizeof(int));
			offset +=sizeof(int);
			char *name = (char *)malloc(namelenght+1);
			memset(name,'\0',namelenght+1);
			memcpy(name,(char *)ret+offset,namelenght);
			offset+= namelenght;
			attr.name=name;

			memcpy(&intval,(char *)ret+offset,sizeof(int));
			offset+=sizeof(int);
			attr.type=(AttrType)intval;

			memcpy(&intval,(char *)ret+offset,sizeof(int));
			offset+=sizeof(int);
			attr.length=intval;

			memcpy(&intval,(char *)ret+offset,sizeof(int));
			offset+=sizeof(int);

			att_pos->insert(pair<int,Attribute>(intval,attr));
	}
	rbfm_ScanIterator1.close();
	attrs.clear();
	for(int i=0;i<(int)att_pos->size();i++)
	{
		attrs.push_back(att_pos->at(i));
	}
	rbfm->closeFile(fileHandle1);

	free(returndata);
	free(ret);
	free(re);
	return 0;
}

RC RelationManager::insertTuple(const string &tableName, const void *data, RID &rid)
{
    RC rc;
    vector<Attribute> attrs;
    rc = getAttributes(tableName,attrs);
    if (rc != 0)
    {
    	cout << rc<<"  Error in insert record" << endl;
        return -1;
    }
    FileHandle fileHandle;
    RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
    rc = rbfm->openFile(tableName, fileHandle);
    rc = rbfm->insertRecord(fileHandle, attrs, data, rid);
    rbfm->closeFile(fileHandle);
    return 0;

}
RC RelationManager::readTuple(const string &tableName, const RID &rid, void *data)
{
	RC rc;
	vector<Attribute> attrs;
	rc = getAttributes(tableName,attrs);
	if (rc != 0)
	{
		cout << rc<<"  Error in getting record" << endl;
		return -1;
	}

    FileHandle fileHandle;
    RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
	rc = rbfm->openFile(tableName, fileHandle);

	rc=rbfm->readRecord(fileHandle,attrs,rid,data);
	rbfm->closeFile(fileHandle);

	return rc;
}
RC RelationManager::deleteTuples(const string &tableName)
{
    return -1;
}

RC RelationManager::deleteTuple(const string &tableName, const RID &rid)
{
	RC rc;
	vector<Attribute> attrs;
	rc = getAttributes(tableName,attrs);
	if (rc != 0)
	{
		cout << rc<<"  Error in getting record" << endl;
		return -1;
	}

	FileHandle fileHandle;
	RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
	rc = rbfm->openFile(tableName, fileHandle);

	rc=rbfm->deleteRecord(fileHandle,attrs,rid);
	rbfm->closeFile(fileHandle);
    return 0;
}

RC RelationManager::updateTuple(const string &tableName, const void *data, const RID &rid)
{
	RC rc;
	vector<Attribute> attrs;
	rc = getAttributes(tableName,attrs);
	if (rc != 0)
	{
		cout << rc<<"  Error in insert record" << endl;
		return -1;
	}
	FileHandle fileHandle;
	RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
	rc = rbfm->openFile(tableName, fileHandle);
	rc = rbfm->updateRecord(fileHandle, attrs, data, rid);
	rbfm->closeFile(fileHandle);
	return 0;
}

RC RelationManager::readAttribute(const string &tableName, const RID &rid, const string &attributeName, void *data)
{
	RC rc;
	vector<Attribute> attrs;
	rc = getAttributes(tableName,attrs);
	if (rc != 0)
	{
		cout << rc<<"  Error in getting record" << endl;
		return -1;
	}

	FileHandle fileHandle;
	RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
	rc = rbfm->openFile(tableName, fileHandle);
	rc=rbfm->readAttribute(fileHandle,attrs,rid,attributeName,data);
	rbfm->closeFile(fileHandle);

	return rc;
}

RC RelationManager::reorganizePage(const string &tableName, const unsigned pageNumber)
{
    return -1;
}

RC RelationManager::scan(const string &tableName,
      const string &conditionAttribute,
      const CompOp compOp,                  
      const void *value,                    
      const vector<string> &attributeNames,
      RM_ScanIterator &rm_ScanIterator)
{
    return -1;
}

// Extra credit
RC RelationManager::dropAttribute(const string &tableName, const string &attributeName)
{
    return -1;
}

// Extra credit
RC RelationManager::addAttribute(const string &tableName, const Attribute &attr)
{
    return -1;
}

// Extra credit
RC RelationManager::reorganizeTable(const string &tableName)
{
    return -1;
}

RC RelationManager::UpdateCatalogTable(const void *data)
{
    FileHandle fileHandle;
    RID rid;
    RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
    RC rc;

    //open the file//
    rc = rbfm->openFile(catalog, fileHandle);

    if (rc != 0)
    {
        rbfm->createFile(catalog);
        rc = rbfm->openFile(catalog, fileHandle);
    }
    //update the table
    rc = rbfm->insertRecord(fileHandle, catalog_v, data, rid);
    rc = rbfm->closeFile(fileHandle);
    return rc;
}
RC RelationManager::UpdateColumnTable(const void *data)
{
    FileHandle fileHandle;
    RID rid;
    RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
    RC rc;
    //open the file//
    rc = rbfm->openFile(column_name, fileHandle);
    if (rc != 0)
    {
    	rbfm->createFile(column_name);
    	rc = rbfm->openFile(column_name, fileHandle);
    }
    rc = rbfm->insertRecord(fileHandle, column_name_v, data, rid);
    rc = rbfm->closeFile(fileHandle);

    return rc;
}
