
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
//for example employee table which is a file which will have employee data records

RC RelationManager::createTable(const string &tableName, const vector<Attribute> &attrs)
{
    FileHandle fileHandle;
    RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
    RC rc;
    unsigned int offset = 0;
    char *data = (char *)malloc(PAGE_SIZE);
    rc = rbfm->createFile(tableName.c_str());
    if (rc != 0)
    {
        cout << "file is already exist" << endl;
        return -1;
    }
    //table Id count up//
    tableId ++;
    memcpy((char*)data,&tableId,sizeof(int));
    offset +=sizeof(int);
    
    //length of the file name//
    unsigned int tableNameLength = (unsigned int)tableName.size();
    memcpy((char*)data + offset,&tableNameLength,sizeof(int));
    offset+=sizeof(int);
    
    //add table name to data//
    memcpy((char*)data+offset,tableName.c_str(),sizeof(char) * tableNameLength);
    offset += tableNameLength;
    
    //length of the file name//
    //NOTE make the file name and the table name the same//
    unsigned int fileNameLength = (unsigned int)tableName.size();
    memcpy((char*)data + offset,&fileNameLength,sizeof(int));
    offset +=sizeof(int);
    
    //add file name to table//
    memcpy((char*)data+offset,tableName.c_str(),sizeof(char) * fileNameLength);
    offset += tableNameLength;
    
//changeData(recordDescriptor,data,tmp);
    rc = UpdateCatalogTable(attrs,data);
    return -1;
}


//this function updata the catalog table//
RC RelationManager::UpdateCatalogTable(const vector<Attribute> &recordDescriptor, const void *data)
{
    FileHandle fileHandle;
    RID rid;
    RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
    RC rc;
    
    //open the file//
    rc = rbfm->openFile("catalog.bin", fileHandle);

    if (rc != 0)
    {
        cout << "the catalog.bin file isn't exist" << endl;
        return -1;
    }
    //update the table
    rc = rbfm->insertRecord(fileHandle, recordDescriptor, data, rid);

    return -1;
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
