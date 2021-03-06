
#ifndef _ix_h_
#define _ix_h_

#include <vector>
#include <string>

#include "../rbf/rbfm.h"

# define IX_EOF (-1)  // end of the index scan
typedef struct node
{
	int pagenum;
	int nextinsert_pos;
	int freespace;
	int isleaf;
	int parent;
	int leftnode;
	int rightnode;
	int num;
	int  *start;
	int  *length;
	void * data;
}node;

class IX_ScanIterator;
class IndexManager;
class IndexManager {
 public:
  static IndexManager* instance();

  RC createFile(const string &fileName);

  RC destroyFile(const string &fileName);

  RC openFile(const string &fileName, FileHandle &fileHandle);

  RC closeFile(FileHandle &fileHandle);

  // The following two functions are using the following format for the passed key value.
  //  1) data is a concatenation of values of the attributes
  //  2) For int and real: use 4 bytes to store the value;
  //     For varchar: use 4 bytes to store the length of characters, then store the actual characters.
  RC insertEntry(FileHandle &fileHandle, const Attribute &attribute, const void *key, const RID &rid);  // Insert new index entry
  RC deleteEntry(FileHandle &fileHandle, const Attribute &attribute, const void *key, const RID &rid);  // Delete index entry

  // scan() returns an iterator to allow the caller to go through the results
  // one by one in the range(lowKey, highKey).
  // For the format of "lowKey" and "highKey", please see insertEntry()
  // If lowKeyInclusive (or highKeyInclusive) is true, then lowKey (or highKey)
  // should be included in the scan
  // If lowKey is null, then the range is -infinity to highKey
  // If highKey is null, then the range is lowKey to +infinity
  RC scan(FileHandle &fileHandle,
      const Attribute &attribute,
	  const void        *lowKey,
      const void        *highKey,
      bool        lowKeyInclusive,
      bool        highKeyInclusive,
      IX_ScanIterator &ix_ScanIterator);
  RC insert_into_leaf(FileHandle &fileHandle, int page, const Attribute &attribute, const void *key, const RID &rid);
   RC initialIndex(FileHandle &fileHandle,unsigned int page,node &res);
   RC initialDirectory(FileHandle &fileHandle);
   RC readNode(FileHandle &fileHandle,unsigned int page, node &res);
   RC writeNode(FileHandle &fileHandle,node &res);
   RC freeNode(node &res);
   int get_left_index(node &parent, node& left);
   int cut(int length);
   RC insert_into_leaf_after_splitting(FileHandle &fileHandle, int page, const Attribute &attribute, const void *key, const RID &rid);
   RC insert_into_node(FileHandle &fileHandle, const Attribute &attribute, const void *key,node &parent,int left_index, node &right);
   RC insert_into_node_after_splitting(FileHandle &fileHandle, const Attribute &attribute, const void *key,node &parent,int left_index, node &right);
   RC insert_into_new_root(FileHandle &fileHandle,const Attribute &attribute,node &left,const void* key,node &right);
   RC start_new_tree(FileHandle &fileHandle,const Attribute &attribute,const void* key,const RID &rid);
   RC insert_into_parent(FileHandle &fileHandle,const Attribute &attribute,node &left,const void* key,node &right);
   void print(FileHandle &fileHandle,const Attribute &attribute,int root,const void* key);
   int find_leaf(FileHandle &fileHandle,const Attribute &attribute,int root,const void* key);
 protected:
  IndexManager   ();                            // Constructor
  ~IndexManager  ();                            // Destructor

 private:
  static IndexManager *_index_manager;
  PagedFileManager *pfm;

};

class IX_ScanIterator {
 public:
  IX_ScanIterator();  							// Constructor
  ~IX_ScanIterator(); 							// Destructor

  RC getNextEntry(RID &rid, void *key);  		// Get next matching entry
  RC close();             						// Terminate index scan
  IndexManager *_index_manager;
  FileHandle fileHandle;
  Attribute attribute;
  void const       *lowKey;
  void const       *highKey;
  bool lowKeyInclusive;
  bool highKeyInclusive;
  int currentpage;
  int slot;
  void * ck;
};

// print out the error message for a given return code
void IX_PrintError (RC rc);


#endif
