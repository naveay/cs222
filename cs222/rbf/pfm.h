#ifndef _pfm_h_
#define _pfm_h_
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <map>
typedef int RC;
//1. file already exist while using createFile
//2. file not exist while using deleteFile
//3. file not exist while using OpenFile
//4. fileHandle is already a handle for an open file when it is passed to the OpenFile method
//5. pageNum is not exist;
//6. data is too long. out of page_size;
//7. file is still open while destroying.
//8. file is not empty while initializing the directory
typedef unsigned PageNum;
using namespace std;
#define PAGE_SIZE 4096

class FileHandle;


class PagedFileManager
{
public:
    static PagedFileManager* instance();                     // Access to the _pf_manager instance

    RC createFile    (const char *fileName);                         // Create a new file
    RC destroyFile   (const char *fileName);                         // Destroy a file
    RC openFile      (const char *fileName, FileHandle &fileHandle); // Open a file
    RC closeFile     (FileHandle &fileHandle);
protected:
    PagedFileManager();                                   // Constructor
    ~PagedFileManager();                                  // Destructor

private:
    static PagedFileManager *_pf_manager;
    map<const char*,int> *refer;
};


class FileHandle
{
public:
    FileHandle();                                                    // Default constructor
    ~FileHandle();                                                   // Destructor

    RC readPage(PageNum pageNum, void *data);                           // Get a specific page
    RC writePage(PageNum pageNum, const void *data);                    // Write a specific page
    RC appendPage(const void *data);                                    // Append a specific page
    unsigned getNumberOfPages();                                        // Get the number of pages in the file


    RC setFile(FILE *file);
    RC setFile(	const char * pfile);
    FILE * getFile();
    const char * getFileName();
private:
	FILE *pFile=NULL;
	const char * pfile=NULL;
    unsigned pagenumber;
};

 #endif
