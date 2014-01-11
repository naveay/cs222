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
  FileHandle f=*file;
  pfm->OpenFile(path,f);
  pfm->OpenFile(path,f);
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
