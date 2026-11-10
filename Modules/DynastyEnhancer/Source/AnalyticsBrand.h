#pragma once
#include <string>

namespace analytics::brand {
inline constexpr const char* Name="GPI";
inline constexpr const char* Handle="@GPIndex";
inline constexpr const char* FullName="Gridiron Performance Index";
// Exact installed ImageLibraryTexture metadata, with the former P icon as a
// control: ButtonIconsColor is library 88; bticl_win64_G_color has asset ID 116.
inline constexpr int ImageLibrary=88,ImageValue=116;
inline bool account(const std::string& author,const std::string& handle){
    return (author==Name&&handle==Handle)||(author=="PFF"&&handle=="@PFF_College");
}
template<class Post>inline void display(Post& post){
    if(!account(post.author,post.handle))return;
    post.author=Name;post.handle=Handle;post.imageLibrary=ImageLibrary;post.imageValue=ImageValue;
    post.displayAuthor=post.author+" ("+post.handle+")";
}
}
