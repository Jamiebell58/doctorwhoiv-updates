// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <cmath>
namespace dw {
enum class Phase { Away, Loading, Arriving, Landed, Departing, Transit, Refuelling };
struct Visual { float opacity, lamp; bool doorsUnlocked; };
inline float clamp(float v,float a=0,float b=1){return std::max(a,std::min(v,b));}
inline float smooth(float t){t=clamp(t);return t*t*(3-2*t);}
inline Visual appearance(float elapsed,float duration,bool arriving){
 const float p=clamp(elapsed/std::max(duration,1.f)),q=arriving?p:1-p;
 const float pulse=.5f-.5f*std::cos(q*18.84955592f);
 float a=clamp(smooth((q-.045f)/.91f)-.42f*std::sin(q*3.14159265f)*(1-pulse));
 if(p==0)a=arriving?0:1;
 if(p==1)a=arriving?1:0;
 return {a,(p>.035f&&p<.98f)?(.15f+.85f*pulse):.1f,arriving&&p>=1};
}
struct Sequence {
 Phase phase=Phase::Away;float elapsed=0,energy=100,reserved=0;
 float arrivalSeconds=21.783356f,departureSeconds=19.423175f;
 bool passenger=false,travelling=false;
 bool busy() const{return phase!=Phase::Away&&phase!=Phase::Landed;}
 bool requestSummon(){if(phase!=Phase::Away||energy<4)return false;phase=Phase::Loading;elapsed=0;reserved=4;travelling=false;return true;}
 bool beginArrival(){if(phase!=Phase::Loading&&phase!=Phase::Transit)return false;phase=Phase::Arriving;elapsed=0;return true;}
 bool requestDeparture(bool travel,float distance=0){
  if(phase!=Phase::Landed||(travel&&!passenger)||!std::isfinite(distance))return false;
  float cost=travel?clamp(9+distance/1500,9,30):2;
  if(energy<cost||(!travel&&energy<6))return false;
  reserved=cost;travelling=travel;phase=Phase::Departing;elapsed=0;return true;
 }
 bool requestRefuel(){if(phase!=Phase::Landed||energy>=100)return false;phase=Phase::Refuelling;elapsed=0;return true;}
 void tick(float dt){
  if(!std::isfinite(dt)||dt<0)return;
  elapsed+=std::min(dt,.1f);
  if(phase==Phase::Arriving&&elapsed>=arrivalSeconds){energy=std::max(0.f,energy-reserved);reserved=0;phase=Phase::Landed;elapsed=0;}
  else if(phase==Phase::Departing&&elapsed>=departureSeconds){energy=std::max(0.f,energy-reserved);reserved=0;phase=travelling?Phase::Transit:Phase::Away;elapsed=0;}
  else if(phase==Phase::Refuelling){energy=std::min(100.f,energy+std::min(dt,.1f)*2.5f);if(energy>=100){phase=Phase::Landed;elapsed=0;}}
 }
 void cancel(){phase=Phase::Away;elapsed=0;reserved=0;passenger=false;travelling=false;}
 Visual visual() const {
  if(phase==Phase::Arriving||phase==Phase::Departing)return appearance(elapsed,phase==Phase::Arriving?arrivalSeconds:departureSeconds,phase==Phase::Arriving);
  if(phase==Phase::Landed||phase==Phase::Refuelling)return {1,.1f,phase==Phase::Landed};
  return {0,0,false};
 }
};
}
