
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
}

RelationManager::~RelationManager()
{
}

//this function creates a table which is create a new file for the tables
//for example employee table which is a file will have employee data records

RC RelationManager::createTable(const string &tableName, const vector<Attribute> &attrs)
{
    FileHandle fileHandle;
    RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
    RC rc;
    unsigned int temp;
    unsigned int offset = 0;
    char *cat_data = (char *)malloc(PAGE_SIZE);
    char *col_data = (char *)malloc(PAGE_SIZE);
    rc = rbfm->createFile(tableName.c_str());
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
    unsigned int tableNameLength = (unsigned int)tableName.size();
    memcpy((char*)cat_data + offset,&tableNameLength,sizeof(int));
    offset+=sizeof(int);
    
    //add table name to data//
    memcpy((char*)cat_data+offset,tableName.c_str(),sizeof(char) * tableNameLength);
    offset += tableNameLength;
    
    //length of the file name//
    //NOTE make the file name and the table name the same//
    unsigned int fileNameLength = (unsigned int)tableName.size();
    memcpy((char*)cat_data + offset,&fileNameLength,sizeof(int));
    offset +=sizeof(int);
    
    //add file name to table//
    memcpy((char*)cat_data+offset,tableName.c_str(),sizeof(char) * fileNameLength);
    offset += tableNameLength;
    rc = UpdateCatalogTable(cat_data);

    //updata Column Table//
    offset = 0;
    memcpy((char*)col_data,&tableId,sizeof(int));
    offset +=sizeof(int);
    
    unsigned int numOfCol = (unsigned int)attrs.size();
    
    for (int i=0;i<numOfCol;i++)
    {
    
       temp = (unsigned int)attrs[i].name.size();
        memcpy((char *)col_data+offset,&temp,sizeof(int));
        offset +=sizeof(int);
        memcpy((char *)col_data+offset,&attrs[i].name,temp);
        offset +=temp;
    
        memcpy((char *)col_data+offset,&attrs[i].type,sizeof(int));
    
        offset +=sizeof(int);
        memcpy((char *)col_data+offset,&attrs[i].length,sizeof(int));
    }
    UpdateColumnTable(col_data);
    
    free(col_data);
    free(cat_data);
    
    return -1;
}


//this function updata the catalog table//
RC RelationManager::UpdateCatalogTable(const void *data)
{
    FileHandle fileHandle;
    RID rid;
    RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
    RC rc;
    vector<Attribute> recordDescriptor;
    
    //open the file//
    rc = rbfm->openFile("catalog.bin", fileHandle);

    if (rc != 0)
    {
        cout << "the catalog.bin file isn't exist" << endl;
        return -1;
    }
    catTableRecordDescriptor(recordDescriptor);
    //update the table
    rc = rbfm->insertRecord(fileHandle, recordDescriptor, data, rid);
    
    rc = rbfm->closeFile(fileHandle);

    return -1;
}
RC RelationManager::UpdateColumnTable(const void *data)
{
    FileHandle fileHandle;
    RID rid;
    RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
    RC rc;
    vector<Attribute> recordDescriptor;
    //open the file//
    rc = rbfm->openFile("columnTable.bin", fileHandle);
    if (rc != 0)
    {
        cout << "the columnTable.bin file isn't exist" << endl;
        return -1;
    }
    colTableRecordDescriptor(recordDescriptor);
    
    rc = rbfm->insertRecord(fileHandle, recordDescriptor, data, rid);
    
    rc = rbfm->closeFile(fileHandle);
    
    return -1;
}

//create catalog table record descriptor//
void RelationManager::catTableRecordDescriptor(vector<Attribute> &recordDescriptor)
{
    
    Attribute attr;
    attr.name = "TableId";
    attr.type = TypeInt;
    attr.length = (AttrLength)4;
    recordDescriptor.push_back(attr);
    
    attr.name = "TableName";
    attr.type = TypeVarChar;
    attr.length = (AttrLength)30;
    recordDescriptor.push_back(attr);
    
    attr.name = "FileName";
    attr.type = TypeVarChar;
    attr.length = (AttrLength)30;
    recordDescriptor.push_back(attr);
    
}

//create column table record descriptor//
void RelationManager::colTableRecordDescriptor(vector<Attribute> &recordDescriptor)
{
    
    Attribute attr;
    attr.name = "TableId";
    attr.type = TypeInt;
    attr.length = (AttrLength)4;
    recordDescriptor.push_back(attr);
    
    attr.name = "ColumnName";
    attr.type = TypeVarChar;
    attr.length = (AttrLength)30;
    recordDescriptor.push_back(attr);
    
    attr.name = "ColumnType";
    attr.type = TypeInt;
    attr.length = (AttrLength)4;
    recordDescriptor.push_back(attr);
    
    attr.name = "ColumnLength";
    attr.type = TypeInt;
    attr.length = (AttrLength)4;
    recordDescriptor.push_back(attr);
    
}




RC RelationManager::deleteTable(const string &tableName)
{
    return -1;
}

RC RelationManager::getAttributes(const string &tableName, vector<Attribute> &attrs)
{
    return -1;
}

RC RelationManager::insertTuple(const string &tableName, const void *data, RID &rid)
{
    return -1;
}

RC RelationManager::deleteTuples(const string &tableName)
{
    return -1;
}

RC RelationManager::deleteTuple(const string &tableName, const RID &rid)
{
    return -1;
}

RC RelationManager::updateTuple(const string &tableName, const void *data, const RID &rid)
{
    return -1;
}

RC RelationManager::readTuple(const string &tableName, const RID &rid, void *data)
{
    return -1;
}

RC RelationManager::readAttribute(const string &tableName, const RID &rid, const string &attributeName, void *data)
{
    return -1;
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
