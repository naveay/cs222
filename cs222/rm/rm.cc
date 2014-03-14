
#include "rm.h"
#include "string.h"
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
	cache=new map<string,vector<Attribute> >();
	index_cache=new map<string,int >();
	index_ca=new map<string,Attribute >();
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



		unsigned int numOfCol = (unsigned int)catalog_v.size();
		for (int i=0;i<(int)numOfCol;i++)
		{
            offset = 0;
            memcpy((char*)col_data,&tableId,sizeof(int));
            offset +=sizeof(int);
			temp = (unsigned int)catalog_v[i].name.length();
			memcpy((char *)col_data+offset,&temp,sizeof(int));
			offset +=sizeof(int);
			memcpy((char *)col_data+offset,catalog_v[i].name.c_str(),temp);
			offset +=temp;
			memcpy((char *)col_data+offset,&catalog_v[i].type,sizeof(int));
			offset +=sizeof(int);
			memcpy((char *)col_data+offset,&catalog_v[i].length,sizeof(int));
			offset +=sizeof(int);
			memcpy((char *)col_data+offset,&i,sizeof(int));
			UpdateColumnTable(col_data);
		}
	}
	free(cat_data);
	free(col_data);
}
RC RelationManager::createTable(const string &tableName, const vector<Attribute> &attrs)
{
    FileHandle fileHandle;
    RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
    RC rc;
    //cache->insert(pair<string,vector<Attribute> >(tableName,attrs));
    unsigned int temp;
    unsigned int offset = 0;
    char *cat_data = (char *)malloc(PAGE_SIZE);
    char *col_data = (char *)malloc(PAGE_SIZE);
    rc = rbfm->createFile(tableName);
    if (rc != 0)
    {
        cout << "file is already exist" << endl;
        free(cat_data);
        free(col_data);
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
	vector<Attribute> at;
	getAttributes(tableName,at);
	for(int i =0;i<(int)at.size();i++)
	{
		deleteTable(tableName+"_index_"+at.at(i).name);
		if(index_cache->find(tableName+"_index_"+at.at(i).name)!=index_cache->end())
			index_cache->erase(tableName+"_index_"+at.at(i).name);
		if(index_ca->find(tableName+"_index_"+at.at(i).name)!=index_ca->end())
			index_ca->erase(tableName+"_index_"+at.at(i).name);
	}
	if(cache->find(tableName)!=cache->end())
	{
		cache->erase(tableName);
	}
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
	free(returndata);
	free(ret);
    return 0;
}

RC RelationManager::getAttributes(const string &tableName, vector<Attribute> &attrs)
{

	if(cache->find(tableName)!=cache->end())
	{
		attrs=cache->at(tableName);
		return 0;
	}
	RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
	FileHandle fileHandle;
	RBFM_ScanIterator rbfm_ScanIterator;

	void * returndata=(void*)malloc(PAGE_SIZE);
	void * ret=(void*)malloc(PAGE_SIZE);
	void * re=(void*)malloc(PAGE_SIZE);
	int l= (int)tableName.length();
	memcpy((char*)returndata,&l,sizeof(int));
	memcpy((char*)returndata+sizeof(int),tableName.c_str(),sizeof(char) * l);
	RID rid;
	rbfm->openFile(catalog,fileHandle);
	rbfm->scan(fileHandle,catalog_v,catalog_v[1].name, EQ_OP, returndata,catalog_s,rbfm_ScanIterator);
	if(rbfm_ScanIterator.getNextRecord(rid,ret)!=RM_EOF)
	{
		rbfm_ScanIterator.close();
		memcpy((char*)re,(char*)ret,PAGE_SIZE);
	}
	else
	{
		free(returndata);
		free(ret);
		free(re);
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
	cache->insert(pair<string,vector<Attribute> >(tableName,attrs));
	rbfm->closeFile(fileHandle1);

	free(returndata);
	free(ret);
	free(re);
	return 0;
}

RC RelationManager::insertTuple(const string &tableName, const void *data, RID &rid)
{
    RC rc;
    vector<Attribute> attrs=*(new vector<Attribute>());
    vector<Attribute> attr2=*(new vector<Attribute>());
    rc = getAttributes(tableName,attrs);
    if (rc != 0)
    {
    	cout << rc<<"  Error in insert record" << endl;
        return -1;
    }
    FileHandle fileHandle;
    RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
    rc = rbfm->openFile(tableName, fileHandle);

	void *key=(void*)malloc(PAGE_SIZE);
    rc = rbfm->insertRecord(fileHandle, attrs, data, rid);

    for(int i=0;i<(int)attrs.size();i++)
    {
        attr2.clear();
        if(index_cache->find(tableName+"_index_"+attrs.at(i).name)!=index_cache->end())
        {
        	if(index_ca->find(tableName+"_index_"+attrs.at(i).name)!=index_ca->end())
        	{
        		attr2.push_back(index_ca->at(tableName+"_index_"+attrs.at(i).name));
        		rbfm->readAttribute(fileHandle,attrs,rid,attrs.at(i).name,key);
				IndexManager *_index_manager=IndexManager::instance();
				FileHandle fileHandle1;
				rc = _index_manager->openFile(tableName+"_index_"+attrs.at(i).name, fileHandle1);
				rc = _index_manager->insertEntry(fileHandle1, attrs.at(i), key, rid);
				rbfm->closeFile(fileHandle1);
        	}
        }
        else{
        	rc = getAttributes(tableName+"_index_"+attrs.at(i).name,attr2);
			if(rc==0)
			{
				index_cache->insert(pair<string,int>(tableName+"_index_"+attrs.at(i).name,0));
				index_ca->insert(pair<string,Attribute>(tableName+"_index_"+attrs.at(i).name,attrs.at(i)));
				rbfm->readAttribute(fileHandle,attrs,rid,attrs.at(i).name,key);
				IndexManager *_index_manager=IndexManager::instance();
				FileHandle fileHandle1;
				rc = _index_manager->openFile(tableName+"_index_"+attrs.at(i).name, fileHandle1);
				rc = _index_manager->insertEntry(fileHandle1, attrs.at(i), key, rid);
				rbfm->closeFile(fileHandle1);
			}
			else
			{
				index_cache->insert(pair<string,int>(tableName+"_index_"+attrs.at(i).name,1));
			}
        }

    }
	free(key);
    rbfm->closeFile(fileHandle);
    attrs.clear();
    attr2.clear();
    return 0;

}
RC RelationManager::readTuple(const string &tableName, const RID &rid, void *data)
{
	RC rc;
	vector<Attribute> attrs;
	rc = getAttributes(tableName,attrs);
	if (rc != 0)
	{
		return -1;
	}

    FileHandle fileHandle;
    RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
	rc = rbfm->openFile(tableName, fileHandle);

	rc=rbfm->readRecord(fileHandle,attrs,rid,data);
	rbfm->closeFile(fileHandle);

	attrs.clear();
	return rc;
}
RC RelationManager::deleteTuples(const string &tableName)
{
	FileHandle fileHandle;
	vector<Attribute> attrs;
	getAttributes(tableName,attrs);
	RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
	rbfm->openFile(tableName, fileHandle);

	rbfm->deleteRecords(fileHandle);
	rbfm->closeFile(fileHandle);

	IndexManager *_index_manager=IndexManager::instance();
	for(int i=0;i<(int)attrs.size();i++)
	{
		RC rc=_index_manager->destroyFile(tableName+"_index_"+attrs.at(i).name);
		if(rc==0)
			_index_manager->createFile(tableName+"_index_"+attrs.at(i).name);
	}
	return 0;
}

RC RelationManager::deleteTuple(const string &tableName, const RID &rid)
{
	RC rc;
    vector<Attribute> attrs;
    vector<Attribute> attr2;
	rc = getAttributes(tableName,attrs);
	if (rc != 0)
	{
		cout << rc<<"  Error in getting record" << endl;
		return -1;
	}
	void *key=(void*)malloc(PAGE_SIZE);
	FileHandle fileHandle;
	RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
	rc = rbfm->openFile(tableName, fileHandle);
	for(int i=0;i<(int)attrs.size();i++)
	{
		attr2.clear();
		if(index_cache->find(tableName+"_index_"+attrs.at(i).name)!=index_cache->end())
		{
			if(index_ca->find(tableName+"_index_"+attrs.at(i).name)!=index_ca->end())
			{
				attr2.push_back(index_ca->at(tableName+"_index_"+attrs.at(i).name));
				rbfm->readAttribute(fileHandle,attrs,rid,attrs.at(i).name,key);
				IndexManager *_index_manager=IndexManager::instance();
				FileHandle fileHandle1;
				rc = _index_manager->openFile(tableName+"_index_"+attrs.at(i).name, fileHandle1);
				rc = _index_manager->deleteEntry(fileHandle1, attrs.at(i), key, rid);
				rbfm->closeFile(fileHandle1);
			}
		}
		else{
			rc = getAttributes(tableName+"_index_"+attrs.at(i).name,attr2);
			if(rc==0)
			{
				index_cache->insert(pair<string,int>(tableName+"_index_"+attrs.at(i).name,0));
				index_ca->insert(pair<string,Attribute>(tableName+"_index_"+attrs.at(i).name,attrs.at(i)));
				rbfm->readAttribute(fileHandle,attrs,rid,attrs.at(i).name,key);
				IndexManager *_index_manager=IndexManager::instance();
				FileHandle fileHandle1;
				rc = _index_manager->openFile(tableName+"_index_"+attrs.at(i).name, fileHandle1);
				rc = _index_manager->deleteEntry(fileHandle1, attrs.at(i), key, rid);
				rbfm->closeFile(fileHandle1);
			}
			else
			{
				index_cache->insert(pair<string,int>(tableName+"_index_"+attrs.at(i).name,1));
			}
		}

	}
	free(key);
	rbfm->deleteRecord(fileHandle,attrs,rid);
	rbfm->closeFile(fileHandle);
	attrs.clear();
	attr2.clear();
    return 0;
}

RC RelationManager::updateTuple(const string &tableName, const void *data, const RID &rid)
{
	RC rc;
	vector<Attribute> attrs;
	vector<Attribute> attr2;
	rc = getAttributes(tableName,attrs);
	if (rc != 0)
	{
		cout << rc<<"  Error in insert record" << endl;
		return -1;
	}
	void *key=(void*)malloc(PAGE_SIZE);
	FileHandle fileHandle;
	RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
	rc = rbfm->openFile(tableName, fileHandle);
	for(int i=0;i<(int)attrs.size();i++)
	{
		attr2.clear();
		if(index_cache->find(tableName+"_index_"+attrs.at(i).name)!=index_cache->end())
		{
			if(index_ca->find(tableName+"_index_"+attrs.at(i).name)!=index_ca->end())
			{
				attr2.push_back(index_ca->at(tableName+"_index_"+attrs.at(i).name));
				rbfm->readAttribute(fileHandle,attrs,rid,attrs.at(i).name,key);
				IndexManager *_index_manager=IndexManager::instance();
				FileHandle fileHandle1;
				rc = _index_manager->openFile(tableName+"_index_"+attrs.at(i).name, fileHandle1);
				rc = _index_manager->deleteEntry(fileHandle1, attrs.at(i), key, rid);
				rbfm->closeFile(fileHandle1);
			}
		}
		else{
			rc = getAttributes(tableName+"_index_"+attrs.at(i).name,attr2);
			if(rc==0)
			{
				index_cache->insert(pair<string,int>(tableName+"_index_"+attrs.at(i).name,0));
				index_ca->insert(pair<string,Attribute>(tableName+"_index_"+attrs.at(i).name,attrs.at(i)));
				rbfm->readAttribute(fileHandle,attrs,rid,attrs.at(i).name,key);
				IndexManager *_index_manager=IndexManager::instance();
				FileHandle fileHandle1;
				rc = _index_manager->openFile(tableName+"_index_"+attrs.at(i).name, fileHandle1);
				rc = _index_manager->deleteEntry(fileHandle1, attrs.at(i), key, rid);
				rbfm->closeFile(fileHandle1);
			}
			else
			{
				index_cache->insert(pair<string,int>(tableName+"_index_"+attrs.at(i).name,1));
			}
		}
	}
	rc = rbfm->updateRecord(fileHandle, attrs, data, rid);
	for(int i=0;i<(int)attrs.size();i++)
	{
		attr2.clear();
		if(index_cache->find(tableName+"_index_"+attrs.at(i).name)!=index_cache->end())
		{
			if(index_ca->find(tableName+"_index_"+attrs.at(i).name)!=index_ca->end())
			{
				attr2.push_back(index_ca->at(tableName+"_index_"+attrs.at(i).name));
				rbfm->readAttribute(fileHandle,attrs,rid,attrs.at(i).name,key);
				IndexManager *_index_manager=IndexManager::instance();
				FileHandle fileHandle1;
				rc = _index_manager->openFile(tableName+"_index_"+attrs.at(i).name, fileHandle1);
				rc = _index_manager->insertEntry(fileHandle1, attrs.at(i), key, rid);
				rbfm->closeFile(fileHandle1);
			}
		}
		else{
			rc = getAttributes(tableName+"_index_"+attrs.at(i).name,attr2);
			if(rc==0)
			{
				index_cache->insert(pair<string,int>(tableName+"_index_"+attrs.at(i).name,0));
				index_ca->insert(pair<string,Attribute>(tableName+"_index_"+attrs.at(i).name,attrs.at(i)));
				rbfm->readAttribute(fileHandle,attrs,rid,attrs.at(i).name,key);
				IndexManager *_index_manager=IndexManager::instance();
				FileHandle fileHandle1;
				rc = _index_manager->openFile(tableName+"_index_"+attrs.at(i).name, fileHandle1);
				rc = _index_manager->insertEntry(fileHandle1, attrs.at(i), key, rid);
				rbfm->closeFile(fileHandle1);
			}
			else
			{
				index_cache->insert(pair<string,int>(tableName+"_index_"+attrs.at(i).name,1));
			}
		}

	}
	attrs.clear();
	attr2.clear();
    rbfm->closeFile(fileHandle);
	free(key);
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

	RC rc;
	vector<Attribute> attrs;
	rc = getAttributes(tableName,attrs);
	if (rc != 0)
	{
		//cout << rc<<"  Error in insert record" << endl;
		return -1;
	}
	FileHandle fileHandle;
	RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
	rc = rbfm->openFile(tableName, fileHandle);
	rc = rbfm->reorganizePage(fileHandle, attrs, pageNumber);
	rbfm->closeFile(fileHandle);
	return 0;
}

RC RelationManager::scan(const string &tableName,
      const string &conditionAttribute,
      const CompOp compOp,                  
      const void *value,                    
      const vector<string> &attributeNames,
      RM_ScanIterator &rm_ScanIterator)
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
		rc = rbfm->scan(fileHandle,attrs,conditionAttribute,compOp,value,attributeNames,rm_ScanIterator.rbfm_ScanIterator);
		//rbfm->closeFile(fileHandle);
		return 0;
}
RC RelationManager::indexScan(const string &tableName,
                      const string &attributeName,
                      const void *lowKey,
                      const void *highKey,
                      bool lowKeyInclusive,
                      bool highKeyInclusive,
                      RM_IndexScanIterator &rm_IndexScanIterator)
{
	RC rc;
	vector<Attribute> attrs;
	rc = getAttributes(tableName+"_index_"+attributeName,attrs);
	if (rc != 0)
	{
		cout << rc<<"  Error in insert record" << endl;
		return -1;
	}
	FileHandle fileHandle;
	IndexManager *_index_manager=IndexManager::instance();
	rc = _index_manager->openFile(tableName+"_index_"+attributeName, fileHandle);
	rc = _index_manager->scan(fileHandle,attrs.at(0),lowKey,highKey,lowKeyInclusive,highKeyInclusive,rm_IndexScanIterator.ix_scanner);
	return 0;
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
    //rc = rbfm->openFile(catalog, fileHandle);
    rc = rbfm->createFile(catalog);
//   if (rc != 0)
//  {
//    rbfm->createFile(catalog);
        rc = rbfm->openFile(catalog, fileHandle);
//}
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

//    rc = rbfm->openFile(column_name, fileHandle);
//    if (rc != 0)
//    {
    	rbfm->createFile(column_name);
    	rc = rbfm->openFile(column_name, fileHandle);
//    }
    rc = rbfm->insertRecord(fileHandle, column_name_v, data, rid);
    rc = rbfm->closeFile(fileHandle);

    return rc;
}
RC RelationManager::createIndex(const string &tableName, const string &attributeName)
{
	string table=tableName+"_index_"+attributeName;
	vector<Attribute> *att=new vector<Attribute>();
	getAttributes(tableName, *att);
	vector<Attribute> *name=new vector<Attribute>();
	for(int i=0;i<(int)(att->size());i++)
	{
		if(att->at(i).name==attributeName)
		{
			name->push_back(att->at(i));
			break;
		}
	}
	vector<string> *at=new vector<string>();
	at->push_back(name->at(0).name);
	createTable(table,*name);
	FileHandle fileHandle1;
	RM_ScanIterator rmsi;
	if(this->scan(tableName, "", NO_OP, NULL, *at, rmsi)==0)
	{
		RID rid;
		void *returnedData=(void*)malloc(PAGE_SIZE);
		IndexManager *indexManager=IndexManager::instance();
		FileHandle fileHandle2;
		indexManager->openFile(table, fileHandle2);
		while(rmsi.getNextTuple(rid, returnedData) != RM_EOF)
		{
			indexManager->insertEntry(fileHandle2,name->at(0),returnedData,rid);
		}
		indexManager->closeFile(fileHandle2);
		free(returnedData);
	}
	at->clear();
	name->clear();
	att->clear();
	return 0;
}

RC RelationManager::destroyIndex(const string &tableName, const string &attributeName)
{
	string table=tableName+"_index_"+attributeName;
	deleteTable(table);
	return 0;
}
