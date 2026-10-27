// SPDX-License-Identifier: GPL-3.0-or-later
#include "../source/sequence.hpp"
#include <cassert>
#include <limits>
#include <iostream>
using namespace dw;
void complete(Sequence& s){for(int i=0;i<3000&&s.busy();i++)s.tick(.1f);}
int main(){
 for(bool incoming:{false,true})for(int i=0;i<=1000;i++){auto v=appearance(i*.025f,25,incoming);assert(std::isfinite(v.opacity)&&v.opacity>=0&&v.opacity<=1);assert(v.doorsUnlocked==(incoming&&i==1000));}
 assert(appearance(0,25,true).opacity==0);assert(appearance(25,25,true).opacity==1);assert(appearance(0,25,false).opacity==1);assert(appearance(25,25,false).opacity==0);
 Sequence s;assert(!s.requestDeparture(true));assert(s.requestSummon());assert(!s.requestSummon());assert(s.energy==100);s.cancel();assert(s.energy==100&&s.reserved==0);assert(s.requestSummon());assert(s.beginArrival());assert(!s.requestDeparture(false));complete(s);assert(s.phase==Phase::Landed&&s.energy==96);assert(!s.requestDeparture(true));s.passenger=true;assert(s.requestDeparture(true,3000));for(int i=0;i<300;i++)s.tick(.1f);assert(s.phase==Phase::Transit&&s.energy==85);assert(s.beginArrival());complete(s);assert(s.energy==85);assert(s.requestRefuel());complete(s);assert(s.energy==100);
 for(int i=0;i<1000;i++){Sequence x;assert(x.requestSummon());assert(!x.requestSummon());assert(x.beginArrival());x.tick(-1);x.tick(std::numeric_limits<float>::quiet_NaN());assert(x.elapsed==0);x.tick(1000);assert(x.elapsed==.1f);complete(x);assert(x.requestDeparture(false));assert(!x.requestDeparture(false));complete(x);assert(x.phase==Phase::Away&&x.energy==94);}
 Sequence low;low.energy=3;assert(!low.requestSummon());low.phase=Phase::Landed;low.energy=5;assert(!low.requestDeparture(false));std::cout<<"PASS: sequencing, visual bounds, input interlocks, power, cancellation, refuel, 1000 repeat cycles\n";
}
