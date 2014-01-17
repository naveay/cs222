
#include "rbfm.h"
RecordBasedFileManager* RecordBasedFileManager::_rbf_manager = 0;

RecordBasedFileManager* RecordBasedFileManager::instance()
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

RC RecordBasedFileManager::createFile(const string &fileName) {
	PagedFileManager *pfm = PagedFileManager::instance(); // To test the functionality of the paged file manager
	return pfm->createFile(fileName.c_str());
}

RC RecordBasedFileManager::destroyFile(const string &fileName) {
	PagedFileManager *pfm = PagedFileManager::instance(); // To test the functionality of the paged file manager
	return pfm->destroyFile(fileName.c_str());
}

RC RecordBasedFileManager::openFile(const string &fileName, FileHandle &fileHandle) {
	PagedFileManager *pfm = PagedFileManager::instance(); // To test the functionality of the paged file manager
	return pfm->openFile(fileName.c_str(),fileHandle);
}

RC RecordBasedFileManager::closeFile(FileHandle &fileHandle) {
	PagedFileManager *pfm = PagedFileManager::instance(); // To test the functionality of the paged file manager
	return pfm->closeFile(fileHandle);
}

RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid) {
    return -1;
}

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
    return -1;
}

RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor, const void *data) {
    return -1;
}
int32_t swap_int32( int32_t val )
{
	val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
	    val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
	    return (val << 32) | ((val >> 32) & 0xFFFFFFFFULL);
}
/* Maintain a list of page with their free space
 * Directory of file. Starting from the first page.
 * the first 4 byte is a pointer which points to the index of
 * the next page of directory. Default index is 0;
 * the next 4 byte is the number of page included in this page
 * of directory. For each page, we store the page number and
 * free space.
 *
 * We will have several helper function
 *
 */

char* RecordBasedFileManager::intTochar(int num)
{
	char  *result;
	result=(char*)&num;
	return result;
}
//called after openfile;
RC RecordBasedFileManager::initialDirectory(FileHandle &fileHandle,int page)
{
	void *data = malloc(PAGE_SIZE);
	void *data1 = malloc(PAGE_SIZE);
	int index=2;
	for(int i=0;i<PAGE_SIZE/4;i++)
	{
		memcpy((char *)data+i*4,&index,sizeof(int));
	}
	//memcpy((char *)data+sizeof(int),&index,sizeof(int));
	fileHandle.writePage(page,data);
	fileHandle.readPage(page,data1);
	if(memcmp(data,data1,PAGE_SIZE)!=0)
		printf("error !!!!");
	return 0;
}
/*
int RecordBasedFileManager::getfreeSpacePage(FileHandle & fileHandle, unsigned int recordsize)
{
	int index=0;int offset=0;
	void *data=malloc(PAGE_SIZE);
	do{
		offset=0;
		fileHandle.readPage(index,data);
		index=atoi((char*)memset((char*)(data),0,4));
		//index=(int)(*((int *)(data+offset)));
		offset+=sizeof(int);
		int pagenum=(int)(*((int *)(data+offset)));
		offset+=sizeof(int);
		for(int i=0;i<pagenum;i++)
		{
			int page=(int)(*((int *)(data+offset)));
			offset+=sizeof(int);
			int freespace=(int)(*((int *)(data+offset)));
			offset+=sizeof(int);
			if(freespace>=recordsize)
			{

				*((int *)(data+offset-sizeof(int)))=freespace-recordsize;
				fileHandle.writePage(index,data);
				free(data);
				return page;
			}
		}
	}while(index>0);
	if(offset<(PAGE_SIZE-2*sizeof(int)))
	{
		*((int *)(data+offset))=fileHandle.getNumberOfPages();
		(*((int *)(data+offset+sizeof(int))))=PAGE_SIZE-recordsize;
		fileHandle.writePage(index,data);
		free(data);
		return fileHandle.getNumberOfPages();
	}
	else
	{
		*((int *)(data))=(int)fileHandle.getNumberOfPages();
		fileHandle.writePage(index,data);
		free(data);
		initialDirectory(fileHandle,0);
		return fileHandle.getNumberOfPages();
	}

}
*/
