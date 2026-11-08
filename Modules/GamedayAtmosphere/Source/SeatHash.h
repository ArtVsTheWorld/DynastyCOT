#pragma once
#include <array>
#include <cstdint>
#include <initializer_list>
// Stable BLAKE2b-64 of little-endian signed coordinate keys. This matches the
// listening/visual prototype; it does not depend on pointers or wall-clock time.
namespace attendance::seats {
inline uint64_t rotate(uint64_t x,unsigned n){return (x>>n)|(x<<(64-n));}
inline double random(std::initializer_list<int32_t> keys){
    constexpr uint64_t iv[]={0x6a09e667f3bcc908ull,0xbb67ae8584caa73bull,0x3c6ef372fe94f82bull,0xa54ff53a5f1d36f1ull,0x510e527fade682d1ull,0x9b05688c2b3e6c1full,0x1f83d9abfb41bd6bull,0x5be0cd19137e2179ull};
    constexpr unsigned char sigma[12][16]={
        {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},{14,10,4,8,9,15,13,6,1,12,0,2,11,7,5,3},
        {11,8,12,0,5,2,15,13,10,14,3,6,7,1,9,4},{7,9,3,1,13,12,11,14,2,6,5,10,4,0,15,8},
        {9,0,5,7,2,4,10,15,14,1,11,12,6,8,3,13},{2,12,6,10,0,11,8,3,4,13,7,5,15,14,1,9},
        {12,5,1,15,14,13,4,10,0,7,6,3,9,2,8,11},{13,11,7,14,12,1,3,9,5,0,15,4,8,6,2,10},
        {6,15,14,9,11,3,0,8,12,2,13,7,1,4,10,5},{10,2,8,4,7,6,1,5,15,11,9,14,3,12,13,0},
        {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},{14,10,4,8,9,15,13,6,1,12,0,2,11,7,5,3}};
    std::array<uint64_t,16> m{},v{};size_t i=0;
    for(int32_t key:keys){if(i>=32)return 0; m[i/2]|=static_cast<uint64_t>(static_cast<uint32_t>(key))<<((i%2)*32);++i;}
    for(unsigned k=0;k<8;++k){v[k]=iv[k];v[k+8]=iv[k];}
    const uint64_t h0=iv[0]^0x01010008ull;v[0]=h0;v[12]^=i*4;v[14]=~v[14];
    auto g=[&](unsigned a,unsigned b,unsigned c,unsigned d,uint64_t x,uint64_t y){
        v[a]+=v[b]+x;v[d]=rotate(v[d]^v[a],32);v[c]+=v[d];v[b]=rotate(v[b]^v[c],24);
        v[a]+=v[b]+y;v[d]=rotate(v[d]^v[a],16);v[c]+=v[d];v[b]=rotate(v[b]^v[c],63);
    };
    for(const auto& s:sigma){
        g(0,4,8,12,m[s[0]],m[s[1]]);g(1,5,9,13,m[s[2]],m[s[3]]);g(2,6,10,14,m[s[4]],m[s[5]]);g(3,7,11,15,m[s[6]],m[s[7]]);
        g(0,5,10,15,m[s[8]],m[s[9]]);g(1,6,11,12,m[s[10]],m[s[11]]);g(2,7,8,13,m[s[12]],m[s[13]]);g(3,4,9,14,m[s[14]],m[s[15]]);
    }
    return static_cast<double>(h0^v[0]^v[8])/18446744073709551616.0;
}
}
