
#ifndef _rm_h_
#define _rm_h_

#include <string>
#include <vector>
#include <map>
#include <sys/stat.h>
#include "../rbf/rbfm.h"
#include "../ix/ix.h"

using namespace std;


# define RM_EOF (-1)  // end of a scan operator

// RM_ScanIterator is an iteratr to go through tuples
// The way to use it is like the following:
//  RM_ScanIterator rmScanIterator;
//  rm.open(..., rmScanIterator);
//  while (rmScanIterator(rid, data) != RM_EOF) {
//    process the data;
//  }
//  rmScanIterator.close();

class RM_ScanIterator {
public:
  RM_ScanIterator() {};
  ~RM_ScanIterator() {};

  // "data" follows the same format as RelationManager::insertTuple()
  RC getNextTuple(RID &rid, void *data) {
	  if(rbfm_ScanIterator.getNextRecord(rid,data)!=RM_EOF)
	  {return 0;}
	  else
		  return RM_EOF;
  };
  RBFM_ScanIterator rbfm_ScanIterator;
  RC close() { RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();
  	  rbfm->closeFile(rbfm_ScanIterator.fileHandle);return 0; };
};

class RM_IndexScanIterator {
 public:
  RM_IndexScanIterator() {};  	// Constructor
  ~RM_IndexScanIterator() {}; 	// Destructor
  IX_ScanIterator ix_scanner;
  // "key" follows the same format as in IndexManager::insertEntry()
  RC getNextEntry(RID &rid, void *key){
	  if(ix_scanner.getNextEntry(rid,key)!=RM_EOF)
	  {return 0;}
	  else
		  return RM_EOF;
  };  	// Get next matching entry
  RC close() {IndexManager *_index_manager=IndexManager::instance();
  _index_manager->closeFile(ix_scanner.fileHandle);return 0;};             			// Terminate index scan
};

// Relation Manager
class RelationManager
{
public:
  static RelationManager* instance();

  RC createTable(const string &tableName, const vector<Attribute> &attrs);

  RC deleteTable(const string &tableName);

  RC getAttributes(const string &tableName, vector<Attribute> &attrs);

  RC insertTuple(const string &tableName, const void *data, RID &rid);

  RC deleteTuples(const string &tableName);

  RC deleteTuple(const string &tableName, const RID &rid);

  // Assume the rid does not change after update
  RC updateTuple(const string &tableName, const void *data, const RID &rid);

  RC readTuple(const string &tableName, const RID &rid, void *data);

  RC readAttribute(const string &tableName, const RID &rid, const string &attributeName, void *data);

  RC reorganizePage(const string &tableName, const unsigned pageNumber);

  // scan returns an iterator to allow the caller to go through the results one by one. 
  RC scan(const string &tableName,
      const string &conditionAttribute,
      const CompOp compOp,                  // comparision type such as "<" and "="
      const void *value,                    // used in the comparison
      const vector<string> &attributeNames, // a list of projected attributes
      RM_ScanIterator &rm_ScanIterator);

  RC createIndex(const string &tableName, const string &attributeName);

  RC destroyIndex(const string &tableName, const string &attributeName);

  // indexScan returns an iterator to allow the caller to go through qualified entries in index
  RC indexScan(const string &tableName,
                        const string &attributeName,
                        const void *lowKey,
                        const void *highKey,
                        bool lowKeyInclusive,
                        bool highKeyInclusive,
                        RM_IndexScanIterator &rm_IndexScanIterator);
// Extra credit
public:
  RC dropAttribute(const string &tableName, const string &attributeName);

  RC addAttribute(const string &tableName, const Attribute &attr);

  RC reorganizeTable(const string &tableName);



protected:
  RelationManager();
  ~RelationManager();

private:
  static RelationManager *_rm;
  string catalog;
  string column_name;
  vector<Attribute> catalog_v;
  vector<Attribute> column_name_v;
  vector<string> catalog_s;
  vector<string> column_name_s;
  unsigned int tableId;
  RC UpdateCatalogTable(const void *data);
  RC UpdateColumnTable(const void *data);
  map<string,vector<Attribute> > *cache;
  map<string,int > *index_cache;
  map<string,Attribute > *index_ca;
  void initial();
};

#endif
