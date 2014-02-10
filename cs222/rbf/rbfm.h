
#ifndef _rbfm_h_
#define _rbfm_h_

#include <string>
#include <vector>
#include <map>
#include "../rbf/pfm.h"

using namespace std;


// Record ID
typedef struct
{
  unsigned pageNum;
  unsigned slotNum;
} RID;


// Attribute
typedef enum { TypeInt = 0, TypeReal, TypeVarChar } AttrType;

typedef unsigned AttrLength;

struct Attribute {
    string   name;     // attribute name
    AttrType type;     // attribute type
    AttrLength length; // attribute length
};

// Comparison Operator (NOT needed for part 1 of the project)
typedef enum { EQ_OP = 0,  // =
           LT_OP,      // <
           GT_OP,      // >
           LE_OP,      // <=
           GE_OP,      // >=
           NE_OP,      // !=
           NO_OP       // no condition
} CompOp;



/****************************************************************************
The scan iterator is NOT required to be implemented for part 1 of the project 
*****************************************************************************/

# define RBFM_EOF (-1)  // end of a scan operator

// RBFM_ScanIterator is an iteratr to go through records
// The way to use it is like the following:
//  RBFM_ScanIterator rbfmScanIterator;
//  rbfm.open(..., rbfmScanIterator);
//  while (rbfmScanIterator(rid, data) != RBFM_EOF) {
//    process the data;
//  }
//  rbfmScanIterator.close();

class RecordBasedFileManager;
class RBFM_ScanIterator;
class RBFM_ScanIterator {
public:
  RBFM_ScanIterator(){};
  ~RBFM_ScanIterator() {};
  RC readRecord_Att(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data);

  // "data" follows the same format as RecordBasedFileManager::insertRecord()
  int reverse_changeData_Att(const vector<Attribute> &recordDescriptor, const void *data,void* result);
  RC getNextRecord(RID &rid, void *data);
  RC readRecordMem(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data);
  bool condition(char * data);
  RC close() { free(memory);return 0; };
  RecordBasedFileManager *rbfm;
  FileHandle fileHandle;
  vector<Attribute> recordDescriptor;
  string conditionAttribute;
  CompOp compOp;                  // comparision type such as "<" and "="
  void const *value;                    // used in the comparison
  vector<string> attributeNames;
  void *returndata;
  RID currentid;
  int currentpage;
  bool readintoMem;
  void *memory;
  int slot_num;
};


class RecordBasedFileManager
{
public:
  static RecordBasedFileManager* instance();

  RC createFile(const string &fileName);

  RC destroyFile(const string &fileName);

  RC openFile(const string &fileName, FileHandle &fileHandle);

  RC closeFile(FileHandle &fileHandle);

  //  Format of the data passed into the function is the following:
  //  1) data is a concatenation of values of the attributes
  //  2) For int and real: use 4 bytes to store the value;
  //     For varchar: use 4 bytes to store the length of characters, then store the actual characters.
  //  !!!The same format is used for updateRecord(), the returned data of readRecord(), and readAttribute()
  RC insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid);
  RC readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data);

  // This method will be mainly used for debugging/testing
  RC printRecord(const vector<Attribute> &recordDescriptor, const void *data);

/**************************************************************************************************************************************************************
***************************************************************************************************************************************************************
IMPORTANT, PLEASE READ: All methods below this comment (other than the constructor and destructor) are NOT required to be implemented for part 1 of the project
***************************************************************************************************************************************************************
***************************************************************************************************************************************************************/
  RC deleteRecords(FileHandle &fileHandle);

  RC deleteRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid);
  RC getNext(RID &rid, void *data);
  // Assume the rid does not change after update
  RC updateRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, const RID &rid);

  RC readAttribute(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, const string attributeName, void *data);

  RC reorganizePage(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const unsigned pageNumber);

  // scan returns an iterator to allow the caller to go through the results one by one. 
  RC scan(FileHandle &fileHandle,
      const vector<Attribute> &recordDescriptor,
      const string &conditionAttribute,
      const CompOp compOp,                  // comparision type such as "<" and "="
      const void *value,                    // used in the comparison
      const vector<string> &attributeNames, // a list of projected attributes
      RBFM_ScanIterator &rbfm_ScanIterator);


// Extra credit for part 2 of the project, please ignore for part 1 of the project
public:

  RC reorganizeFile(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor);
protected:
  RecordBasedFileManager();
  ~RecordBasedFileManager();

private:
  static RecordBasedFileManager *_rbf_manager;

  RC initialDirectory(FileHandle &fileHandle,unsigned int page);
  RC initialPage(FileHandle &fileHandle,unsigned int page);
  RC managePage(FileHandle &fileHandle,unsigned int recordsize,const void* records,RID &rid);
  int insert(FileHandle &fileHandle,unsigned int page,const void* data,int length);
  PagedFileManager *pfm;
  RC managePage_tmp(FileHandle &fileHandle,unsigned int recordsize,const void* records,RID &rid);
  int insert_tmp(FileHandle &fileHandle,unsigned int page,const void* data,int length);
  int changeData(const vector<Attribute> &recordDescriptor, const void *data,void* result);
  int reverse_changeData(const vector<Attribute> &recordDescriptor, const void *data,void* result);
  //int reverse_changeData_Att(const vector<Attribute> &recordDescriptor, const void *data,void* result);
  RC insertRecord_tmp(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid);
  //RC readRecordMem(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data);
  //bool condition(char * data);
  //get the first page number whose free space is bigger than the size of record
  //update the free space by add the size of records

  //initialize the directory of the file
};

#endif
