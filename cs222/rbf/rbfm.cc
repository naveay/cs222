
#include "rbfm.h"

RecordBasedFileManager* RecordBasedFileManager::_rbf_manager = 0;

RecordBasedFileManager* RecordBasedFileManager::Instance()
{
    if(!_rbf_manager)
        _rbf_manager = new RecordBasedFileManager();

    return _rbf_manager;
}

RecordBasedFileManager::RecordBasedFileManager()
{
}

RecordBasedFileManager::~RecordBasedFileManager()
{
}

RC RecordBasedFileManager::insertTuple(const string &fileName, const vector<Attribute> &recordDescriptor, const void *data, RID &rid) {
	PagedFileManager *pfm = PagedFileManager::Instance();
	pfm->CreateFile(fileName.c_str());
	unsigned len=0;
	for(int i=0;i<recordDescriptor.size();i++)
	{
		if(recordDescriptor[i].type==TypeInt||recordDescriptor[i].type==TypeReal)
		{
			len+=4;
		}
		else
		{
			char* tmp=new char[4];
			char *test = (char *)data+len;
			memcpy(tmp,test,4);
			len+=4;
			len+=strtol(tmp,NULL,16);
			delete(tmp);
		}
	}
	FileHandle fileHandle;
	char *cache=new char[PAGE_SIZE];
	pfm->OpenFile(fileName.c_str(),fileHandle);
	for(int i=0;i<fileHandle.GetNumberOfPages();i++)
	{
		//----------------
		int result=fileHandle.ReadPage(i,cache);
		if(result!=0)
		{
			return result;
		}

		if(strlen((char*)data)+len<=PAGE_SIZE)
		{
			rid.pageNum=i;
			//rid.slotNum=(char*)data;
			return 0;
		}
	}
	return 0;
}

RC RecordBasedFileManager::readTuple(const string &fileName, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
    return -1;
}

RC RecordBasedFileManager::printTuple(const vector<Attribute> &recordDescriptor, const void *data) {
    return -1;
}
