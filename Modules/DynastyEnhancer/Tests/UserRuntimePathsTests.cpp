#include "../Source/UserRuntimePaths.h"
#include <cassert>
#include <iostream>
int main(){
 using namespace dynasty::userpaths;namespace fs=std::filesystem;
 const fs::path root=L"C:/Users/Test User/AppData/Local/Ace/DynastyCOT/BeyondTheField",code=root/L"Runtime-1.0.8";
 assert(dataFromCode(code,root)==root);
 assert(dataFromCode(L"C:/MMC/Plugins/BeyondTheField",root).empty());
 assert(dataFromCode(root/L"Tools",root).empty());
 assert(dataFromCode(root.parent_path()/L"Other"/L"Runtime-1.0.8",root).empty());
 assert(program(root,code,"node","../Shared/node.exe")==code/L"Shared"/L"node.exe");
 assert(program(root,code,"exporter","Tools/export_runtime.mjs")==code/L"Tools"/L"export_runtime.mjs");
 assert(program(root,code,"exporter","Custom/export.mjs")==fs::absolute(root/L"Custom"/L"export.mjs"));
 assert(program(root,code,"node","D:/Custom/node.exe")==fs::path(L"D:/Custom/node.exe"));
 std::cout<<"PASS: native code/data separation, safe module roots, packaged defaults and custom program paths.\n";
}
