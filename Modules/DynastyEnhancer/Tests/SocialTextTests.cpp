#include "SocialTextTests.h"
#include <iostream>
int main(){try{socialtexttests::run();std::cout<<"PASS: new/native glyph parity, accented names, archived display-only repair and post layout.\n";return 0;}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
