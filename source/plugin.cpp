// SPDX-License-Identifier: GPL-3.0-or-later
#ifdef DWIV_RUNTIME_TEST
#include "../tests/fake_platform.hpp"
#include "../tests/fake_natives.hpp"
#else
#include "bridge.hpp"
#include "natives.hpp"
#include "audio.hpp"
#endif
#include "sequence.hpp"
#include "assets.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <array>
#include <limits>
using bridge::Vec3;
using namespace gta;
namespace {
HMODULE self=nullptr;std::filesystem::path dataDir;std::ofstream logfile;
dw::Sequence sequence;dw::Audio audio;Object box=0;Blip boxBlip=0;Ped playerPed=0,ownedPed=0;
Vec3 location{},target{},departure{},safeExit{};float heading=0,exitHeading=0,scanTimer=0,noticeTimer=0,landingProbeTimer=0;
bool ownedVisible=true,enabled=false,announced=false,streamStarted=false,returning=false,recall=false,wasPaused=false;
bool waypointEnabled=true,sonicEnabled=true,placed=false,helpVisible=false;
bool controlsHeld=false;
// Portal: cross the front doorway towards the centre; no 'E' input needed.
// GTA IV still performs a very short, guarded streaming transition rather than
// providing a live portal camera; E remains a safe fallback.
float previousPortalY=10.f;bool portalWasOutside=false;

float volume=.65f;std::array<bool,256> keys{},keyEdges{};DWORD lastTick=0;
uint32_t hashName(const char* str){uint32_t h=0;for(;*str;str++){h+=uint8_t(*str);h+=h<<10;h^=h>>6;}h+=h<<3;h^=h>>11;h+=h<<15;return h;}
const uint32_t tardisModel=hashName("dwiv_tardis");
void log(const std::string& msg){if(logfile){logfile<<GetTickCount()<<" "<<msg<<std::endl;}}
void message(const std::string& msg){PrintStringWithLiteralString(const_cast<char*>("STRING"),const_cast<char*>(msg.c_str()),5000,1);log(msg);}
Vec3 position(Ped p){Vec3 v;GetCharCoordinates(p,&v.x,&v.y,&v.z);return v;}
float distance(const Vec3&a,const Vec3&b){return std::sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y)+(a.z-b.z)*(a.z-b.z));}
bool finite(const Vec3& p){return std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z);}
bool pressed(int k){return keyEdges[k];}
bool foreground(){DWORD pid=0;GetWindowThreadProcessId(GetForegroundWindow(),&pid);return pid==GetCurrentProcessId();}
void pollInput(){
 keyEdges.fill(false);bool active=foreground();
 for(int k:{VK_END,VK_F4,VK_F5,VK_F6,VK_F7,int('E'),VK_F8,VK_F9,VK_F10,VK_F11}){
  bool now=active&&(GetAsyncKeyState(k)&0x8000)!=0;keyEdges[k]=now&&!keys[k];keys[k]=now;
 }
}
bool interiorBusy();void requestRoom();void clearRoom();void clearDoors();void cameraOff(bool restoreRoom=true);void exteriorDoors();
bool serviceExterior();void recoverPlayer(const char* reason);void holdInside(Vec3 p);void showHud();
void releaseTransitHold();
void removeBox(){portalWasOutside=false;previousPortalY=10.f;clearDoors();if(box&&DoesObjectExist(box))DeleteObject(&box);box=0;if(boxBlip&&DoesBlipExist(boxBlip))RemoveBlip(boxBlip);boxBlip=0;placed=false;}
void restorePlayer(bool move){cameraOff(false);
 if(ownedPed&&DoesCharExist(ownedPed)){
  if(move&&!IsCharDead(ownedPed)&&!IsCharInAnyCar(ownedPed)){RequestCollisionAtPosn(safeExit.x,safeExit.y,safeExit.z);SetCharVelocity(ownedPed,0,0,0);SetCharCoordinatesNoOffset(ownedPed,safeExit.x,safeExit.y,safeExit.z);SetCharHeading(ownedPed,exitHeading);}
  SetCharVisible(ownedPed,ownedVisible);SetCamBehindPed(ownedPed);
 }
 releaseTransitHold();
 ownedPed=0;sequence.passenger=false;
}
void reset(bool move){audio.stop();restorePlayer(move);clearRoom();removeBox();sequence.cancel();streamStarted=false;returning=false;recall=false;scanTimer=0;}
bool groundAt(float x,float y,float probe,float& out){
 // IV writes the height through its pointer; its return register is not a bool.
 float z=std::numeric_limits<float>::quiet_NaN();GetGroundZFor3dCoord(x,y,probe,&z);
 if(!std::isfinite(z)||z<-100.f||z>probe+.1f)return false;
 out=z;return true;
}
bool landing(Vec3 candidate,Vec3& out){
 static const float offsets[][2]={{0,0},{3,0},{-3,0},{0,3},{0,-3},{4,4},{4,-4},{-4,4},{-4,-4},{8,0},{-8,0},{0,8},{0,-8}};
 for(auto& d:offsets){float x=candidate.x+d[0],y=candidate.y+d[1],z=0,water=0;
  if(!groundAt(x,y,candidate.z+8,z)||z>550.f)continue;
  if(GetWaterHeight(x,y,z+2,&water)&&water>z-.15f)continue;
  bool flat=true;for(float dx:{-.85f,.85f})for(float dy:{-.85f,.85f}){float gz=0;if(!groundAt(x+dx,y+dy,z+3,gz)||std::abs(gz-z)>.3f)flat=false;}
  if(!flat||GetClosestCar(x,y,z,2.2f,false,70))continue;
  Vec3 safe;if(!GetSafePositionForChar(x,y,z+1,true,&safe.x,&safe.y,&safe.z)||!finite(safe)||std::hypot(safe.x-x,safe.y-y)>2.5f)continue;
  out={x,y,z};return true;
 }return false;
}
bool exitPoint(Vec3 base,Vec3& out){
 for(float a:{0.f,90.f,180.f,270.f}){float rad=(heading+a)*.01745329252f;Vec3 p{base.x-std::sin(rad)*2.4f,base.y+std::cos(rad)*2.4f,base.z+1};Vec3 safe;
  if(!GetSafePositionForChar(p.x,p.y,p.z,true,&safe.x,&safe.y,&safe.z)||!finite(safe)||std::hypot(safe.x-base.x,safe.y-base.y)>8)continue;
  float z=0,water=0;if(!groundAt(safe.x,safe.y,base.z+4,z)||std::abs(z-base.z)>1)continue;
  if(GetWaterHeight(safe.x,safe.y,z+2,&water)&&water>z-.15f)continue;
  if(GetClosestCar(safe.x,safe.y,z,1.2f,false,70))continue;
  out={safe.x,safe.y,z+1.05f};return true;
 }return false;
}
bool waypointLanding(Vec3& found,Vec3& exit){
 // Waypoint Z is not a reliable road altitude, especially from the remote room.
 // Called after the player streaming anchor has moved to the destination.
 // Try the selected XY first; a nearby road is an optional fallback, never a gate.
 for(float z:{1000.f,300.f,80.f,10.f}){
  Vec3 probe{target.x,target.y,z};
  if(landing(probe,found)&&std::hypot(found.x-target.x,found.y-target.y)<=13.f&&exitPoint(found,exit))return true;
 }
 Vec3 road{};
 if(GetClosestCarNode(target.x,target.y,0,&road.x,&road.y,&road.z)&&finite(road)&&std::hypot(road.x-target.x,road.y-target.y)<=35.f){
  if(landing(road,found)&&std::hypot(found.x-target.x,found.y-target.y)<=48.f&&exitPoint(found,exit))return true;
 }
 return false;
}
void arrive(Vec3 where){
 if(!box||!DoesObjectExist(box)){box=0;CreateObjectNoOffset(tardisModel,where.x,where.y,where.z,&box,true);if(!box||!DoesObjectExist(box))throw std::runtime_error("TARDIS object creation failed");}
 DontRemoveObject(box);ClearRoomForObject(box);SetObjectVisible(box,true);
 SetObjectAlpha(box,0);SetObjectCollision(box,false);SetObjectDynamic(box,false);FreezeObjectPosition(box,true);
 SetObjectCoordinates(box,where.x,where.y,where.z);SetObjectHeading(box,heading);location=where;placed=true;exteriorDoors();
 if(!sequence.beginArrival())throw std::runtime_error("Arrival in invalid state");
 // The transfer owns the saved indoor point; playerPed may currently anchor
 // streaming at the destination, so never reuse that outdoor position here.
 audio.play(audio.arrival);message("TARDIS materialising. Wait for the landing sound to finish.");
}
void checkWalkThroughEntry(){
 if(!placed||sequence.phase!=dw::Phase::Landed||sequence.passenger||
    interiorBusy()||!playerPed||!DoesCharExist(playerPed)||GetMissionFlag()||
    IsCharInAnyCar(playerPed)) { portalWasOutside=false;return; }
 const Vec3 p=position(playerPed);float r=heading*.01745329252f;
 const float dx=p.x-location.x,dy=p.y-location.y;
 const float localX=std::cos(r)*dx+std::sin(r)*dy;
 const float localY=-std::sin(r)*dx+std::cos(r)*dy;
 // A doorway narrower than the police box; cannot activate from its side/rear.
 const bool heightOK=p.z>location.z-.25f&&p.z<location.z+2.25f;
 const bool approach=heightOK&&std::abs(localX)<.54f&&localY>.30f&&localY<1.75f;
 if(!approach){portalWasOutside=false;previousPortalY=10.f;return;}
 // Never auto-board somebody merely standing in front of a spawned box.
 if(localY>1.10f)portalWasOutside=true;
 if(portalWasOutside&&previousPortalY>1.02f&&localY<=1.02f){
   portalWasOutside=false;log("2005 doorway threshold crossed: assisted walk-through entry");
   requestRoom();
 }
 previousPortalY=localY;
}
void summon(){
 if(GetMissionFlag()){message("Use the TARDIS in free roam, outside a mission.");return;}
 if(sequence.busy()||sequence.passenger||interiorBusy()){message("TARDIS busy. Wait for it to finish.");return;}
 if(sequence.phase==dw::Phase::Landed){if(sequence.requestDeparture(false)){recall=true;audio.play(audio.departure);message("TARDIS departing before recall.");}else message("Refuel with F10 before recalling.");return;}
 Interior inside=0;GetInteriorFromChar(playerPed,&inside);
 if(inside||IsCharInAnyCar(playerPed)||IsCharInWater(playerPed)){message("Stand outdoors, on foot, on dry ground to summon.");return;}
 Vec3 candidate,found;GetOffsetFromCharInWorldCoords(playerPed,0,5,0,&candidate.x,&candidate.y,&candidate.z);
 if(!landing(candidate,found)){message("No clear, flat landing area. Move to an open pavement and try again.");return;}
 if(!sequence.requestSummon()){message("Insufficient TARDIS power. END resets this prototype.");return;}
 if(!IsModelInCdimage(tardisModel)){sequence.cancel();message("TARDIS model not registered. Recheck installation and restart GTA IV.");return;}
 target=found;GetCharHeading(playerPed,&heading);heading+=180;RequestModel(tardisModel);RequestCollisionForModel(tardisModel);RequestModel(hashName("dwiv_ext_l"));RequestModel(hashName("dwiv_ext_r"));scanTimer=0;message("Calling the TARDIS...");
}
#include "interior_runtime.hpp"
void recoverPlayer(const char* reason){
 if(placed&&sequence.passenger&&ownedPed&&DoesCharExist(ownedPed)){
  recoveryExit(reason);return;
 }
 bool keep=placed;audio.stop();restorePlayer(true);clearRoom();sequence.cancel();
 streamStarted=false;returning=false;recall=false;scanTimer=0;
 if(keep){sequence.phase=dw::Phase::Landed;serviceExterior();}
 else removeBox();
 message(reason);
}
void board(){
 if(interiorBusy()){message("Door transition in progress. Please wait.");return;}
 if(sequence.passenger){
  auto p=position(playerPed);
  if(std::hypot(p.x-roomOrigin.x,p.y-roomOrigin.y-8.061f)<2.6f)requestRoomExit();
  else if(std::hypot(p.x-roomOrigin.x,p.y-roomOrigin.y)<3.5f)travel();
  else message("E at the doors exits; E by the console or F8 starts waypoint flight.");
  return;
 }
 if(sequence.phase!=dw::Phase::Landed){message("Wait for the TARDIS to finish landing.");return;}
 if(IsCharInAnyCar(playerPed)||GetMissionFlag())return;
 requestRoom();
}
void travel(){
 if(!waypointEnabled){message("Waypoint travel is disabled in DoctorWhoIV.ini.");return;}
 if(sequence.phase==dw::Phase::Refuelling)sequence.phase=dw::Phase::Landed;
 if(exteriorView){message("Press F11 to return to the console before starting flight.");return;}
 if(!sequence.passenger||sequence.phase!=dw::Phase::Landed||interiorBusy()){message("Enter the TARDIS, wait for the doors, then set a map waypoint.");return;}
 Blip blip=GetFirstBlipInfoId(8);if(!blip||!DoesBlipExist(blip)){message("Place a waypoint on the pause map first.");return;}
 Vec3 dest;GetBlipCoords(blip,&dest);if(!finite(dest)||std::abs(dest.x)>5000||std::abs(dest.y)>5000){message("Waypoint is outside supported Liberty City coordinates.");return;}
 // Do not query distant path nodes while the player is in the room at Z=700.
 // The raw map XY is enough to begin travel; validate ground after streaming.
 if(!sequence.requestDeparture(true,std::hypot(location.x-dest.x,location.y-dest.y))){message("Not enough power for this journey. Refuel with F10.");return;}
 roomStandPoint=position(playerPed);waypointExitConfirmed=false;
 doorHold=0;target={dest.x,dest.y,0};departure=location;returning=false;streamStarted=false;landingProbeTimer=0;audio.play(audio.departure);
 log("Waypoint accepted: XY="+std::to_string(dest.x)+", "+std::to_string(dest.y)+" raw Z="+std::to_string(dest.z)+". Ground validation deferred until destination is loaded.");
 message("Waypoint accepted. Dematerialising; loading the destination before landing.");
}
void sonic(bool on){
 if(!sonicEnabled||sequence.passenger||sequence.busy())return;
 Vec3 p=position(playerPed);Vehicle car=GetClosestCar(p.x,p.y,p.z,12,false,70);
 if(car&&DoesVehicleExist(car)){SetCarEngineOn(car,on,true);message(on?"Sonic: nearest vehicle engine started.":"Sonic: nearest vehicle engine stopped.");}
 else message("No vehicle within sonic range (12 metres).");
}
void update(){
 DWORD now=GetTickCount();float dt=std::min(float(now-lastTick)/1000,.1f);lastTick=now;
 pollInput();
 if(!enabled)return;
 // The 0.1.0 native lookup gate silently skipped F4/F7. Log failures once.
 static bool warnedMissingNative=false;
 if(!bridge::find(0x6DAB78CD)){
  if(!warnedMissingNative){log("Native registry unavailable: gameplay waiting for initialisation");warnedMissingNative=true;}
  return;
 }
 warnedMissingNative=false;
 bridge::ThreadScope thread;
 bool paused=IsPauseMenuActive()||(bridge::userPause&&*bridge::userPause)||(bridge::scriptPause&&*bridge::scriptPause);
 if(paused!=wasPaused){audio.pause(paused);wasPaused=paused;}if(paused)return;
 if(IsNetworkGameRunning()){if(box||ownedPed)reset(false);return;}
 Player player=GetPlayerId();if(!IsPlayerPlaying(player)){if(box||ownedPed)reset(false);announced=false;return;}
 Ped current=0;GetPlayerChar(player,&current);if(!current||!DoesCharExist(current))return;
 if(playerPed&&playerPed!=current){reset(false);announced=false;}playerPed=current;
 if(IsCharDead(playerPed)||IsCharInjured(playerPed)){if(box||ownedPed)reset(false);return;}
 if(!announced){message("Doctor Who IV 0.2.2: F7 summon / F4 controls / END return outside.");announced=true;}
 if(foreground()){
  for(int k:{VK_END,VK_F4,VK_F5,VK_F6,VK_F7,int('E'),VK_F8,VK_F9,VK_F10,VK_F11})if(pressed(k))log("Key "+std::to_string(k)+" phase="+std::to_string(int(sequence.phase))+" inside="+std::to_string(sequence.passenger));
  if(pressed(VK_END)){recoverPlayer("Recovery requested. Loading the outside world; TARDIS kept.");sequence.energy=100;return;}
  if(pressed(VK_F4))helpVisible=!helpVisible;
  if(IsPlayerControlOn(player)||controlsHeld){
   if(pressed(VK_F7))summon();
   if(pressed('E')&&(sequence.passenger||(box&&DoesObjectExist(box)&&distance(position(playerPed),location)<3.5f)))board();
   if(pressed(VK_F8))travel();
   if(pressed(VK_F11))viewExterior();
   if(pressed(VK_F9)&&sequence.phase==dw::Phase::Landed&&!sequence.passenger&&!interiorBusy()){if(sequence.requestDeparture(false))audio.play(audio.departure);else message("Refuel before dismissing (F10).");}
   if(pressed(VK_F10)&&!interiorBusy()&&(sequence.passenger||(placed&&distance(position(playerPed),location)<5))){
    if(sequence.phase==dw::Phase::Refuelling){sequence.phase=dw::Phase::Landed;message("Refuelling stopped. Doors and flight unlocked.");}
    else if(sequence.requestRefuel())message("Refuelling. F10 stops refuelling; E by the doors exits.");
    else message("TARDIS is busy or its power is already full.");
   }
   if(pressed(VK_F5))sonic(false);
   if(pressed(VK_F6))sonic(true);
  }
 }else keys.fill(false);
 if(ownedPed&&!leaving&&((!IsPlayerControlOn(player)&&!controlsHeld)||GetMissionFlag())){recoverPlayer("Travel cancelled because game control changed. TARDIS kept.");return;}
 if(placed){serviceExterior();checkWalkThroughEntry();}
 if(sequence.phase==dw::Phase::Loading){scanTimer+=dt;if(HasModelLoaded(tardisModel)&&HasModelLoaded(hashName("dwiv_ext_l"))&&HasModelLoaded(hashName("dwiv_ext_r")))arrive(target);else if(scanTimer>10){reset(true);message("TARDIS model load timed out. Check DoctorWhoIV.log.");}}
 tickInterior(dt);
 if(sequence.phase==dw::Phase::Transit){
  if(!streamStarted){cameraOff(false);beginTransitHold({target.x,target.y,50.f});scanTimer=0;landingProbeTimer=0;streamStarted=true;}
  scanTimer+=dt;bool destinationReady=tickTransitHold(dt);Vec3 found,exit;
  landingProbeTimer+=dt;bool probe=destinationReady&&landingProbeTimer>.25f;
  if(probe)landingProbeTimer=0;
  if(probe&&waypointLanding(found,exit)){
   safeExit=exit;waypointExitConfirmed=true;
   // Stay inside the Coral room until the player exits through the doors.
   log("Confirmed landing: "+std::to_string(found.x)+", "+std::to_string(found.y)+", "+std::to_string(found.z));
   arrive(found);streamStarted=false;holdInside(roomStandPoint);
  }else if(scanTimer>12){
   // Return control at the original safeExit, not at an invented destination.
   recoveryExit("Waypoint scene/landing validation timed out; returning to the original TARDIS.");
  }
 }
 auto before=sequence.phase;bool animated=before==dw::Phase::Arriving||before==dw::Phase::Departing;
 float soundTime=animated?audio.seconds():-1;if(soundTime>=0)sequence.elapsed=soundTime;
 sequence.tick(soundTime>=0?0:dt);
 if(sequence.phase!=before){
  if(sequence.phase==dw::Phase::Away){removeBox();MarkModelAsNoLongerNeeded(tardisModel);if(recall){recall=false;summon();}}
  else if(sequence.phase==dw::Phase::Transit&&box)SetObjectAlpha(box,0);
  else if(sequence.phase==dw::Phase::Landed)message("TARDIS landed. Power "+std::to_string(int(sequence.energy))+"%. E: enter/exit / F10: refuel.");
 }
 if(box&&DoesObjectExist(box)){auto v=sequence.visual();SetObjectAlpha(box,int(v.opacity*255));SetObjectCollision(box,sequence.phase==dw::Phase::Landed&&HasCollisionForModelLoaded(tardisModel));if(v.opacity>.01f)DrawLightWithRange(location.x,location.y,location.z+3.15f,170,205,255,3.5f,v.lamp*2);audio.volume(volume*(sequence.passenger?1.f:dw::clamp(1-distance(position(playerPed),location)/90)));}
 if(sequence.phase==dw::Phase::Refuelling){noticeTimer+=dt;if(noticeTimer>2){noticeTimer=0;message("Refuelling: "+std::to_string(int(sequence.energy))+"%.");}}
 showHud();
}
void __cdecl tick(){
 bridge::originalTick();try{update();}catch(const std::exception& e){log(std::string("Disabled after error: ")+e.what());try{bridge::ThreadScope thread;reset(true);}catch(...){log("Recovery also failed; restart game.");}enabled=false;}
}
}
#ifndef DWIV_RUNTIME_TEST
extern "C" __declspec(dllexport) void InitializeASI(){
 static bool started=false;if(started)return;started=true;
 try{
  wchar_t path[32768]{};GetModuleFileNameW(self,path,32768);dataDir=std::filesystem::path(path).parent_path()/L"DoctorWhoIV";
  std::filesystem::create_directories(dataDir);logfile.open(dataDir/L"DoctorWhoIV.log",std::ios::app);log("Starting Doctor Who IV 0.2.2 Coral UV/streaming repair - requires in-game validation");
  auto ini=(dataDir/L"DoctorWhoIV.ini").wstring();waypointEnabled=GetPrivateProfileIntW(L"Features",L"WaypointTravel",1,ini.c_str())!=0;sonicEnabled=GetPrivateProfileIntW(L"Features",L"SonicEngines",1,ini.c_str())!=0;
  volume=dw::clamp(float(GetPrivateProfileIntW(L"Audio",L"VolumePercent",65,ini.c_str()))/100);
  try{audio.init(dataDir/L"sounds");sequence.arrivalSeconds=audio.arrival.duration();sequence.departureSeconds=audio.departure.duration();audio.volume(volume);}catch(const std::exception& e){log(std::string("Audio unavailable, using silent timed sequence: ")+e.what());}
  lastTick=GetTickCount();bridge::install(tick);enabled=true;log("Game callback installed; waiting for single-player free roam.");
 }catch(const std::exception& e){log(std::string("Initialization stopped: ")+e.what());}
}
BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,LPVOID){if(reason==DLL_PROCESS_ATTACH){self=module;DisableThreadLibraryCalls(module);}return TRUE;}
#endif
