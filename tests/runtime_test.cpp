#define DWIV_RUNTIME_TEST
#include "../source/plugin.cpp"
#include <cassert>
#include <iostream>
#include <type_traits>
static_assert(std::is_void_v<decltype(GetGroundZFor3dCoord(0,0,0,nullptr))>);
void step(float seconds){int n=int(std::ceil(seconds/.05f));for(int i=0;i<n;i++){mock::now+=50;tick();assert(enabled);}}
void key(int k){mock::keys[k]=true;step(.05f);mock::keys[k]=false;step(.05f);}
void released(){assert(!mock::frozen&&mock::control&&mock::visible&&mock::collision&&!mock::faded&&!controlsHeld&&!holdActive);}
void fresh(){
 reset(false);mock::objects.clear();mock::blips.clear();mock::messages.clear();mock::events.clear();
 mock::p={100,100,1.05f};mock::frozen=false;mock::visible=true;mock::sceneReady=true;mock::modelsReady=true;
 mock::groundReady=true;mock::pause=false;mock::control=true;mock::mission=false;mock::hasWaypoint=false;
 mock::unsafeSceneStarts=0;mock::roadQueries=0;mock::remoteGroundQueries=0;mock::roadAvailable=true;
 mock::roomCollision=true;mock::collision=true;mock::faded=false;mock::missingRotorModels=false;
 mock::keys.fill(false);keys.fill(false);keyEdges.fill(false);
 enabled=true;announced=true;wasPaused=false;sequence.energy=100;playerPed=1;helpVisible=false;
 sequence.arrivalSeconds=.2f;sequence.departureSeconds=.2f;lastTick=mock::now;
 sequence.requestSummon();arrive({100,100,0});step(.4f);assert(sequence.phase==dw::Phase::Landed);
}
void enter(){board();step(2);assert(sequence.passenger&&roomReady&&!roomSettling);released();}
bool messageContains(const std::string& needle){for(const auto& m:mock::messages)if(m.find(needle)!=std::string::npos)return true;return false;}
int main(){
 (void)self;
 // Real IV ground native has an output parameter, not a Boolean result.
 fresh();Vec3 found{};assert(landing({100,105,1},found)&&found.z==0);
 mock::groundReady=false;assert(!landing({100,105,1},found));mock::groundReady=true;
 // Entry has camera clearance, a local XY origin and a control lease which ends.
 enter();assert(roomOrigin.x==100&&roomOrigin.y==100);
 assert(std::abs(mock::p.y-roomOrigin.y-4.5f)<.01f&&mock::p.z>701);
 // Reproduce floor eviction during exit. The player streams the OUTDOOR point
 // under a fade, while hidden, collisionless and with gameplay control disabled.
 int originalBox=box;mock::sceneReady=false;mock::groundReady=false;requestRoomExit();
 assert(mock::frozen&&leaving&&!mock::control&&mock::unsafeSceneStarts==0);
 mock::objects.erase(roomObjects[0]);step(1);
 assert(sequence.passenger&&mock::p.z<100&&!mock::visible&&!mock::collision&&mock::faded);
 mock::groundReady=true;step(.2f); // UPDATE_LOAD_SCENE still false: must not gate exit
 assert(!sequence.passenger&&box==originalBox&&mock::objects.at(box).alpha==255);released();
 assert(DoesBlipExist(boxBlip));
 // Lost streamed objects are recreated. Room resources must exist before release.
 fresh();enter();originalBox=box;mock::objects.erase(box);mock::objects.erase(doorObjects[0]);step(.1f);
 assert(box!=originalBox&&sequence.passenger&&DoesObjectExist(doorObjects[0]));
 mock::objects.erase(roomObjects[3]);step(.05f);assert(roomSettling&&mock::frozen);
 mock::roomCollision=false;step(1);assert(roomSettling&&mock::frozen&&!mock::control);
 mock::roomCollision=true;step(.2f);assert(!roomSettling&&DoesObjectExist(roomObjects[3]));released();
 // Failed custom-floor loading stays outside; failure after entry recovers outdoors.
 fresh();mock::roomCollision=false;board();step(13);assert(!sequence.passenger&&mock::p.z<2);released();
 fresh();enter();mock::roomCollision=false;step(10);assert(!sequence.passenger&&placed&&mock::p.z<2);released();
 // Falling out of the room triggers an outdoor transfer, never a release in sky.
 fresh();enter();originalBox=box;mock::p.z=699;step(.05f);assert(leaving&&mock::frozen);
 step(1.2f);assert(!sequence.passenger&&box==originalBox&&mock::p.z<2);released();
 // F8 is edge triggered; missing waypoint leaves the room intact with guidance.
 fresh();enter();key(VK_F8);assert(sequence.phase==dw::Phase::Landed&&messageContains("waypoint"));
 // Waypoint Z=700 and missing road nodes are valid inputs. Collision only answers
 // near an outdoor player anchor, even when UpdateLoadScene never reports true.
 mock::roadAvailable=false;mock::hasWaypoint=true;mock::waypoint={1250,1180,700};mock::sceneReady=false;
 key(VK_F8);step(3);
 assert(sequence.phase==dw::Phase::Landed&&sequence.passenger&&!roomSettling);released();
 assert(std::abs(location.x-1250)<.01f&&std::abs(location.y-1180)<.01f&&target.z==0);
 assert(mock::roadQueries==0&&mock::remoteGroundQueries==0&&mock::unsafeSceneStarts==0);
 assert(insideRoom(mock::p));requestRoomExit();step(1.2f);
 assert(!sequence.passenger&&std::hypot(mock::p.x-location.x,mock::p.y-location.y)<10);released();
 // A slow destination stays held; once collision arrives the trip completes.
 fresh();enter();mock::hasWaypoint=true;mock::waypoint={1800,-300,700};mock::groundReady=false;
 key(VK_F8);step(3);assert(sequence.phase==dw::Phase::Transit&&mock::p.z<100&&mock::frozen&&!mock::control);
 mock::groundReady=true;step(3);assert(sequence.phase==dw::Phase::Landed&&sequence.passenger);released();
 // Failed destination returns to the original box. It cannot leave the avatar at
 // the rejected waypoint or send it back into an entry/exit sky-room loop.
 fresh();enter();originalBox=box;mock::hasWaypoint=true;mock::waypoint={1600,400,700};mock::groundReady=false;
 key(VK_F8);step(13);assert(leaving&&mock::frozen&&placed&&box==originalBox);
 mock::groundReady=true;step(1.2f);assert(!sequence.passenger&&location.x==100&&mock::p.z<2);released();
 // Even unresponsive probes have a bounded recovery at the previously walked
 // entry point. This fallback is not evidence of real engine collision readiness.
 fresh();enter();mock::groundReady=false;requestRoomExit();step(25);
 assert(!sequence.passenger&&!roomSettling&&placed&&mock::p.x==100&&mock::p.z<2);released();
 assert(messageContains("Recovered to the place you entered"));
 // Rotor: both genuinely separated objects must respond to engine state without
 // disturbing collision-bearing static room geometry.
 fresh();enter();
 assert(rotorObjects[0]&&rotorObjects[1]);
 const int rotorDown=rotorObjects[0],rotorUp=rotorObjects[1];
 assert(std::abs(mock::objects.at(rotorDown).p.z-roomOrigin.z)<.05f);
 assert(std::abs(mock::objects.at(rotorUp).p.z-roomOrigin.z)<.05f);
 assert(sequence.requestDeparture(true,15));audio.play(audio.departure);step(.2f);
 assert(rotorOffset>.01f);
 assert(mock::objects.at(rotorUp).p.z>mock::objects.at(rotorDown).p.z);
 assert(!mock::objects.at(rotorUp).collision);
 sequence.phase=dw::Phase::Landed;step(.8f);
 assert(std::abs(rotorOffset)<.02f);
 // The old 0.2.2 map lacks dynamic rotor assets. Entry must still work,
 // without spawning invalid models or freezing the player indefinitely.
 fresh();mock::missingRotorModels=true;board();step(2);
 assert(sequence.passenger&&roomReady&&!roomSettling&&!dynamicRotorAvailable);
 assert(rotorObjects[0]==0&&rotorObjects[1]==0);released();
 // Enter by walking across the front doorway; no E event. Pass-by and rear
 // approaches must not accidentally board the TARDIS.
 fresh();mock::p={100,99.0f,1.05f};step(.05f);assert(!roomLoading);
 mock::p={100,101.52f,1.05f};step(.05f);assert(!roomLoading);
 mock::p={100,101.22f,1.05f};step(.05f);assert(!roomLoading);
 mock::p={100,100.96f,1.05f};step(.05f);assert(roomLoading||entering);
 step(2);assert(sequence.passenger&&roomReady&&!roomSettling);released();
 // The E key remains a fallback for players whose collision stops the crossing.
 fresh();mock::p={100,101.5f,1.05f};key('E');step(2);
 assert(sequence.passenger&&roomReady);released();
 // Door threshold automatically exits while parked; flight keeps it locked.
 fresh();enter();mock::p={roomOrigin.x,roomOrigin.y+7.6f,roomOrigin.z+1.2f};step(.05f);
 assert(leaving);step(1.2f);assert(!sequence.passenger);released();
 fresh();enter();sequence.departureSeconds=10;sequence.requestDeparture(true);audio.play(audio.departure);
 mock::p={roomOrigin.x,roomOrigin.y+7.6f,roomOrigin.z+1.2f};step(.1f);
 assert(sequence.passenger&&!leaving&&insideRoom(mock::p));
 // END during flight restores control and retains the exterior.
 originalBox=box;key(VK_END);step(1.2f);assert(!sequence.passenger&&box==originalBox);released();
 // Refuel does not trap the player or obscure the exit action.
 fresh();enter();key(VK_F10);assert(sequence.phase==dw::Phase::Refuelling);
 key(VK_F10);assert(sequence.phase==dw::Phase::Landed);key(VK_END);step(1.2f);released();
 fresh();enter();sequence.requestRefuel();requestRoomExit();step(1.2f);assert(!sequence.passenger);released();
 // F11 remains usable while we own disabled gameplay control. It anchors outside,
 // then returns to the saved interior point rather than reusing the outdoor ped.
 fresh();enter();Vec3 indoor=mock::p;key(VK_F11);step(1.2f);
 assert(exteriorView&&mock::p.z<100&&mock::frozen&&!mock::control&&!mock::faded);
 key(VK_F11);step(1.2f);assert(!exteriorView&&!roomSettling&&distance(mock::p,indoor)<.01f);released();
 key(VK_F11);step(1.2f);key(VK_END);step(1.2f);assert(!sequence.passenger&&mock::cameras.empty());released();
 // Pause retains the hold; resetting recovers both camera/control/fade ownership.
 fresh();enter();requestRoomExit();mock::pause=true;step(2);assert(leaving&&mock::frozen);
 mock::pause=false;step(1.2f);assert(!sequence.passenger);released();
 for(int i=0;i<30;i++){fresh();enter();requestRoomExit();step(1.2f);assert(mock::objects.size()==3);released();}
 reset(false);assert(mock::objects.empty()&&mock::blips.empty()&&mock::cameras.empty());released();
 std::cout<<"PASS: ground output ABI; local streaming anchors; false scene result; slow/failed loads; floor/object loss; UV-independent flight controls; doorway guard; bounded recovery; refuel; camera; END; pause; and 30 enter/exit cycles.\n";
}
