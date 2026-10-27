#pragma once
// Deterministic native simulation for transition regressions, NOT a GTA IV emulator.
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <array>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <stdexcept>
#define __cdecl
using DWORD=uint32_t;using HMODULE=void*;
inline constexpr int VK_END=35,VK_F4=115,VK_F5=116,VK_F6=117,VK_F7=118,VK_F8=119,VK_F9=120,VK_F10=121,VK_F11=122;
namespace bridge {
struct Vec3{float x=0,y=0,z=0;};
inline bool* userPause=nullptr;inline bool* scriptPause=nullptr;
inline bool find(uint32_t){return true;}struct ThreadScope{~ThreadScope(){}};
inline void originalTick(){}
}
namespace mock {
struct Obj{uint32_t model=0;bridge::Vec3 p{};bool visible=true,held=false,collision=false;int alpha=255;};
inline std::map<int,Obj> objects;
inline std::map<int,bridge::Vec3> blips;
inline std::set<int> cameras;
inline std::array<bool,256> keys{};
inline std::vector<std::string> messages,events;
inline std::set<uint32_t> requested;
inline bridge::Vec3 p{100,100,1.05f},waypoint{120,120,0},scene{};
inline uint32_t now=1000;inline int next=100,unsafeSceneStarts=0,roadQueries=0;
inline bool frozen=false,visible=true,sceneReady=true,modelsReady=true,groundReady=true;
inline bool pause=false,control=true,mission=false,hasWaypoint=false;
inline bool roadAvailable=true,collision=true,faded=false,roomCollision=true,missingRotorModels=false;
inline int remoteGroundQueries=0;
}
inline DWORD GetTickCount(){return mock::now;}
inline int GetAsyncKeyState(int k){return mock::keys[k]?0x8000:0;}
inline void* GetForegroundWindow(){return nullptr;}
inline DWORD GetCurrentProcessId(){return 123;}
inline void GetWindowThreadProcessId(void*,DWORD* p){*p=123;}
namespace dw {
struct Clip{float length=.2f;float duration()const{return length;}};
struct Audio{
 Clip arrival,departure,open,close;const Clip* current=nullptr;uint32_t started=0;
 void stop(){current=nullptr;}void play(const Clip& c){current=&c;started=mock::now;}
 float seconds()const{return current?std::min(current->length,(mock::now-started)/1000.f):-1;}
 void pause(bool){}void volume(float){}
};
}
