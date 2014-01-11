#include <fstream>
#include <iostream>
#include <cassert>

#include "pfm.h"
#include "rbfm.h"

using namespace std;


void rbfTest()
{
  const char * path="d:\\a.txt";
  PagedFileManager *pfm = PagedFileManager::Instance();
  pfm->CreateFile(path);
  //pfm->DestroyFile(path);
  FileHandle *file=new FileHandle();
  pfm->OpenFile(path,*file);
  file->WritePage(1,"aaa");
  file->AppendPage("aaa2");
  file->AppendPage("aaa3");
  file->WritePage(2,"a11aa");
  char * data=new char[PAGE_SIZE];
  file->ReadPage(2,data);
  cout<<data;
  // RecordBasedFileManager *rbfm = RecordBasedFileManager::Instance();

  // write your own testing cases here
}


int main() 
{
  cout << "test..." << endl;

  rbfTest();
  // other tests go here

  cout << "OK" << endl;
}
