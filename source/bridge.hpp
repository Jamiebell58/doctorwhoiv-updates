// SPDX-License-Identifier: GPL-3.0-or-later
// GTA IV native ABI and discovery patterns derived from GTAIV.EFLC.FusionFix.
// See SOURCES.md and licenses/GPL-3.0.txt. Only Complete Edition 1.2.0.59.
#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
namespace bridge {
struct Vec3 {float x=0,y=0,z=0;};
struct Vec4 {float x,y,z,w;};
inline constexpr const char* LOOP="E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? B9";
inline constexpr const char* COUNT="8B 35 ? ? ? ? 85 F6 75 06 33 C0 5E C2 04 00 53 57 8B 7C 24 10";
inline constexpr const char* TABLE="8B 1D ? ? ? ? 8B CF 8B 04 D3 3B C7 74 19 8D 64 24 00 85 C0";
inline constexpr const char* THREAD="8B 35 ? ? ? ? 8B 47 ? FF 77";
inline uint32_t* count=nullptr;
inline uint32_t** table=nullptr;
inline void** currentThread=nullptr;
inline bool* userPause=nullptr;
inline bool* scriptPause=nullptr;
inline void (__cdecl* originalTick)()=nullptr;
alignas(16) inline uint8_t dummyThread[256]{};
inline bool readable(const void* p,size_t n){
 if(!p||!n)return false;
 uintptr_t at=reinterpret_cast<uintptr_t>(p),end=at+n;if(end<at)return false;
 while(at<end){MEMORY_BASIC_INFORMATION m{};if(!VirtualQuery(reinterpret_cast<void*>(at),&m,sizeof(m))||m.State!=MEM_COMMIT||(m.Protect&(PAGE_NOACCESS|PAGE_GUARD)))return false;
  uintptr_t next=reinterpret_cast<uintptr_t>(m.BaseAddress)+m.RegionSize;if(next<=at)return false;at=next;}
 return true;
}
inline std::vector<uint8_t*> scan(const char* signature){
 std::vector<int> p;std::istringstream ss(signature);std::string tok;while(ss>>tok)p.push_back(tok=="?"?-1:std::stoi(tok,nullptr,16));
 auto base=reinterpret_cast<uint8_t*>(GetModuleHandleW(nullptr));auto dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);auto nt=reinterpret_cast<IMAGE_NT_HEADERS32*>(base+dos->e_lfanew);
 std::vector<uint8_t*> out;auto sections=IMAGE_FIRST_SECTION(nt);
 for(unsigned s=0;s<nt->FileHeader.NumberOfSections;s++){auto& sec=sections[s];if(!(sec.Characteristics&IMAGE_SCN_MEM_EXECUTE))continue;
  auto b=base+sec.VirtualAddress;size_t n=sec.Misc.VirtualSize;
  if(n<p.size()||!readable(b,n))continue;
  for(size_t i=0;i<=n-p.size();i++){size_t j=0;for(;j<p.size();j++)if(p[j]>=0&&b[i+j]!=p[j])break;if(j==p.size())out.push_back(b+i);}}
 return out;
}
inline uint8_t* required(const char* pattern,size_t expected){auto found=scan(pattern);if(found.size()!=expected)throw std::runtime_error("Unsupported game code: signature count mismatch");return found.front();}
struct Context {
 void* result;uint32_t count;void* args;uint32_t dataCount;
 Vec3* originals[4]{};Vec4 temporaries[4]{};
 uint32_t storage[32]{},returns[4]{};
 Context():result(returns),count(0),args(storage),dataCount(0){}
 template<class T>void push(T v){static_assert(sizeof(T)<=4,"GTA IV ABI is 32-bit");if(count>=32)throw std::runtime_error("Too many native parameters");std::memcpy(&storage[count++],&v,sizeof(T));}
 void copyVectors(){if(dataCount>4)throw std::runtime_error("Invalid native vector count");while(dataCount){--dataCount;if(originals[dataCount])*originals[dataCount]={temporaries[dataCount].x,temporaries[dataCount].y,temporaries[dataCount].z};}}
};
static_assert(offsetof(Context,originals)==16,"Native ABI layout");
static_assert(offsetof(Context,temporaries)==32,"Native ABI layout");
using Handler=void (__cdecl*)(Context*);
inline Handler find(uint32_t hash){
 if(!count||!table)return nullptr;
 // DoctorWhoIV_KeyProbe v2 identified C1/T1 as the live raw native table
 // on game build 1.2.0.59. Do not treat it as a probing hash table.
 uint32_t n=*count;auto t=*table;
 static uint32_t* oldTable=nullptr;static uint32_t oldCount=0;static std::unordered_map<uint32_t,Handler> cache;
 if(t!=oldTable||n!=oldCount){cache.clear();oldTable=nullptr;oldCount=0;if(n<100||n>10000||!readable(t,size_t(n)*8))return nullptr;oldTable=t;oldCount=n;}
 auto it=cache.find(hash);if(it!=cache.end())return it->second;
 for(uint32_t i=0;i<n;i++)if(t[2*i]==hash&&t[2*i+1]){auto h=reinterpret_cast<Handler>(t[2*i+1]);cache.emplace(hash,h);return h;}
 return nullptr;
}
template<class R,class...A>inline R call(uint32_t hash,A...args){
 auto h=find(hash);if(!h){std::ostringstream s;s<<"Missing native 0x"<<std::hex<<hash;throw std::runtime_error(s.str());}
 Context c;(c.push(args),...);h(&c);c.copyVectors();if constexpr(!std::is_void_v<R>){R out{};static_assert(sizeof(R)<=sizeof(c.returns));std::memcpy(&out,c.returns,sizeof(R));return out;}
}
struct ThreadScope {void* previous;ThreadScope():previous(*currentThread){*currentThread=dummyThread;}~ThreadScope(){*currentThread=previous;}ThreadScope(const ThreadScope&)=delete;};
inline bool correctVersion(){
 wchar_t path[32768]{};GetModuleFileNameW(nullptr,path,32768);DWORD ignored=0,n=GetFileVersionInfoSizeW(path,&ignored);if(!n)return false;
 std::vector<uint8_t> data(n);if(!GetFileVersionInfoW(path,0,n,data.data()))return false;VS_FIXEDFILEINFO* v=nullptr;UINT len=0;
 return VerQueryValueW(data.data(),L"\\",reinterpret_cast<void**>(&v),&len)&&len>=sizeof(*v)&&v->dwSignature==0xFEEF04BD&&v->dwFileVersionMS==0x00010002&&v->dwFileVersionLS==59;
}
inline void install(void (__cdecl* hook)()){
 if(!correctVersion())throw std::runtime_error("Requires GTA IV Complete Edition 1.2.0.59; no hook installed");
 auto site=required(LOOP,1);
 auto counts=scan(COUNT),tables=scan(TABLE);
 if(counts.size()!=2||tables.size()!=2)throw std::runtime_error("Expected two count/table signatures on GTA IV 1.2.0.59");
 count=*reinterpret_cast<uint32_t**>(counts[1]+2);
 table=*reinterpret_cast<uint32_t***>(tables[1]+2);
 currentThread=*reinterpret_cast<void***>(required(THREAD,1)+2);
 if(!readable(count,4)||!readable(table,4)||!readable(currentThread,4))throw std::runtime_error("Native globals not readable");
 auto p=scan("0F B6 0D ? ? ? ? 0F B6 C0 0B C1");if(!p.empty()){userPause=*reinterpret_cast<bool**>(p[0]+3);scriptPause=*reinterpret_cast<bool**>(p[0]+15);}else{
 p=scan("0F B6 0D ? ? ? ? 0F B6 15 ? ? ? ? 33 C0 0B C1");if(!p.empty()){userPause=*reinterpret_cast<bool**>(p[0]+3);scriptPause=*reinterpret_cast<bool**>(p[0]+10);}}
 if(userPause&&!readable(userPause,1))userPause=nullptr;
 if(scriptPause&&!readable(scriptPause,1))scriptPause=nullptr;
 int32_t relative=0;std::memcpy(&relative,site+1,4);originalTick=reinterpret_cast<decltype(originalTick)>(site+5+relative);
 DWORD old=0;if(!VirtualProtect(site,5,PAGE_EXECUTE_READWRITE,&old))throw std::runtime_error("Cannot install game callback");
 relative=int32_t(reinterpret_cast<uintptr_t>(hook)-reinterpret_cast<uintptr_t>(site+5));std::memcpy(site+1,&relative,4);DWORD unused=0;VirtualProtect(site,5,old,&unused);FlushInstructionCache(GetCurrentProcess(),site,5);
}
}
