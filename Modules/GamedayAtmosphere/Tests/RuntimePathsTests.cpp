#include "../Source/RuntimePaths.h"
#include <cassert>
#include <iostream>
using namespace attendance::runtimepaths;
int main(){
    Layout moved,legacy;
    assert(resolve(L"C:/MMC/DGA-Runtime/Runtime-2.1.2/DynamicGamedayAtmosphereNative-2.1.2.dll",moved));
    assert(resolve(L"C:/MMC/Plugins/DynamicGamedayAtmosphere/Runtime-2.0.35/DynamicGamedayAtmosphereNative-2.0.35.dll",legacy));
    assert(moved.plugins==legacy.plugins&&moved.data==legacy.data);
    assert(moved.data==std::filesystem::path(L"C:/MMC/Plugins/DynamicGamedayAtmosphere"));
    assert(resolve(L"C:/MMC/dga-runtime/Runtime-2.1.2/native.dll",moved));
    assert(resolve(L"C:/MMC/DGA-Runtime/./Runtime-2.1.2/native.dll",moved));
    assert(resolve(L"C:/Users/Test User/CFB 27/DGA-Runtime/Runtime-2.1.2/native.dll",moved));
    assert(moved.plugins==std::filesystem::path(L"C:/Users/Test User/CFB 27/Plugins"));
    auto before=moved.data;
    assert(!resolve(L"C:/Unrelated/Runtime-2.1.2/native.dll",moved)&&moved.data==before);
    assert(!resolve(L"C:/MMC/DGA-Runtime/Unrelated/native.dll",moved)&&moved.data==before);
    std::cout<<"PASS: native runtime outside recursive plugin scan resolves the original settings/log/history and plugin paths; legacy layout, spaces, case, normalization and unsupported-path rejection.\n";
}
