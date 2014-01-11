#ifndef _pfm_h_
#define _pfm_h_
#include <stdio.h>
#include <map>
typedef int RC;
//1. file already exist while using createFile
//2. file not exist while using deleteFile
//3. file not exist while using OpenFile
//4. fileHandle is already a handle for an open file when it is passed to the OpenFile method

typedef unsigned PageNum;
using namespace std;
#define PAGE_SIZE 4096

class FileHandle;


class PagedFileManager
{
public:
    static PagedFileManager* Instance();                     // Access to the _pf_manager instance

    RC CreateFile    (const char *fileName);                         // Create a new file
    RC DestroyFile   (const char *fileName);                         // Destroy a file
    RC OpenFile      (const char *fileName, FileHandle &fileHandle); // Open a file
    RC CloseFile     (FileHandle &fileHandle);                       // Close a file

protected:
    PagedFileManager();                                   // Constructor
    ~PagedFileManager();                                  // Destructor

private:
    static PagedFileManager *_pf_manager;
};


class FileHandle
{
public:
    FileHandle();                                                    // Default constructor
    ~FileHandle();                                                   // Destructor

    RC ReadPage(PageNum pageNum, void *data);                           // Get a specific page
    RC WritePage(PageNum pageNum, const void *data);                    // Write a specific page
    RC AppendPage(const void *data);                                    // Append a specific page
    unsigned GetNumberOfPages();                                        // Get the number of pages in the file


    RC setFile(const char *file);
    const char * getFile();
private:
    char const* pfile=NULL;
    unsigned pagenumber;
};

 #endif
