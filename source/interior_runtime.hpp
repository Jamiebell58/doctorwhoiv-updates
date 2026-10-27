// SPDX-License-Identifier: GPL-3.0-or-later
// Included inside the plugin namespace after the outdoor landing helpers.
Vec3 roomOrigin{0,0,700};
std::array<Object,dw::RoomModelCount> roomObjects{};
std::array<Object,4> doorObjects{};
std::array<Object,dw::RotorModelCount> rotorObjects{};
float rotorPhase=0.f,rotorOffset=0.f;
// Old 0.2.2 IMG archives have the rotor baked into the static console.
// Never lock players outside merely because the new dynamic meshes are absent.
bool dynamicRotorAvailable=false;

bool roomLoading=false,roomReady=false,entering=false,leaving=false,exteriorView=false,roomSettling=false;
bool holdActive=false,holdMoved=false,fadeOwned=false,exitFallback=false;
// A successful waypoint landing has already validated its outdoor exit. A
// subsequent streaming probe must never overwrite that destination with home.
bool waypointExitConfirmed=false;
float roomTimer=0,doorTimer=0,doorAmount=0,doorHold=0,holdTimer=0;
Vec3 roomStandPoint{},holdPoint{},pendingPoint{},homeExit{},homeBox{};
float homeHeading=0;
Camera roomCamera=0;int32_t cameraCommands=0;
void travel();
bool interiorBusy(){return roomLoading||entering||leaving||roomSettling;}
Vec3 roomEntryPoint(){return {roomOrigin.x,roomOrigin.y+4.5f,roomOrigin.z+1.80f};}
bool insideRoom(Vec3 p){
 return finite(p)&&std::abs(p.x-roomOrigin.x)<8.5f&&std::abs(p.y-roomOrigin.y)<7.5f&&p.z>roomOrigin.z+.3f&&p.z<roomOrigin.z+5;
}
void revealTransfer(){if(fadeOwned){DoScreenFadeIn(250);fadeOwned=false;}}
void beginTransitHold(Vec3 p){
 if(!ownedPed||!DoesCharExist(ownedPed)||!finite(p))throw std::runtime_error("Invalid transfer owner or position");
 // Freeze alone does not stop player tasks. Own control only until this transfer
 // ends, and move the streaming anchor (the ped) with the requested scene.
 if(IsPlayerControlOn(GetPlayerId())){SetPlayerControl(GetPlayerId(),false);controlsHeld=true;}
 FreezeCharPosition(ownedPed,true);SetCharVelocity(ownedPed,0,0,0);
 pendingPoint=p;holdPoint=position(ownedPed);holdTimer=0;holdMoved=false;holdActive=true;
 if(!fadeOwned){DoScreenFadeOut(200);fadeOwned=true;}
 log("Transfer hold -> "+std::to_string(p.x)+", "+std::to_string(p.y)+", "+std::to_string(p.z));
}
bool tickTransitHold(float dt){
 if(!holdActive||!ownedPed||!DoesCharExist(ownedPed))return false;
 holdTimer+=dt;
 if(!holdMoved&&(IsScreenFadedOut()||holdTimer>.35f)){
  SetCharVisible(ownedPed,false);SetCharCollision(ownedPed,false);
  holdPoint=pendingPoint;SetCharCoordinatesNoOffset(ownedPed,holdPoint.x,holdPoint.y,holdPoint.z);
  SetCamBehindPed(ownedPed);SetLoadCollisionForCharFlag(ownedPed,true);
  StartLoadScene(holdPoint.x,holdPoint.y,holdPoint.z);holdMoved=true;holdTimer=0;
 }
 FreezeCharPosition(ownedPed,true);SetCharVelocity(ownedPed,0,0,0);
 SetCharCoordinatesNoOffset(ownedPed,holdPoint.x,holdPoint.y,holdPoint.z);
 if(!holdMoved)return false;
 RequestCollisionAtPosn(holdPoint.x,holdPoint.y,holdPoint.z);
 // UPDATE_LOAD_SCENE can remain false despite usable local collision. Readiness
 // is checked by the caller (outdoor ground or the owned room collision/model).
 UpdateLoadScene();return holdTimer>.8f;
}
void releaseTransitHold(){
 if(ownedPed&&DoesCharExist(ownedPed)){
  SetCharCollision(ownedPed,true);SetCharVisible(ownedPed,ownedVisible);
  SetCharVelocity(ownedPed,0,0,0);FreezeCharPosition(ownedPed,false);SetCamBehindPed(ownedPed);
 }
 if(controlsHeld&&!GetMissionFlag()&&ownedPed&&DoesCharExist(ownedPed)&&!IsCharDead(ownedPed))SetPlayerControl(GetPlayerId(),true);
 controlsHeld=false;holdActive=false;holdMoved=false;revealTransfer();
}
void deleteOwned(Object& o){if(o&&DoesObjectExist(o))DeleteObject(&o);o=0;}
void holdInside(Vec3 p){
 if(!ownedPed||!DoesCharExist(ownedPed))return;
 // Never save/reuse an outdoor streaming anchor or the empty doorway edge.
 roomStandPoint=insideRoom(p)?p:roomEntryPoint();roomSettling=true;roomTimer=0;
 beginTransitHold(roomStandPoint);
}
void cameraOff(bool restoreRoom){
 bool wasView=exteriorView;
 if(roomCamera&&DoesCamExist(roomCamera)){
  SetCamActive(roomCamera,false);SetCamPropagate(roomCamera,false);ActivateScriptedCams(false,false);
  DestroyCam(roomCamera);EndCamCommands(&cameraCommands);
 }
 roomCamera=0;exteriorView=false;
 if(wasView&&restoreRoom&&sequence.passenger&&ownedPed&&!leaving)holdInside(roomStandPoint);
}
void clearRoom(){
 cameraOff(false);
 for(auto&o:rotorObjects)deleteOwned(o);
 for(auto name:dw::RotorNames)MarkModelAsNoLongerNeeded(hashName(name));
 rotorPhase=0.f;rotorOffset=0.f;dynamicRotorAvailable=false;
 for(auto&o:roomObjects)deleteOwned(o);
 for(size_t i=2;i<4;i++){deleteOwned(doorObjects[i]);MarkModelAsNoLongerNeeded(hashName(dw::DoorNames[i]));}
 for(auto name:dw::RoomModels)MarkModelAsNoLongerNeeded(hashName(name));
 roomLoading=false;roomReady=false;entering=false;leaving=false;roomSettling=false;roomTimer=0;
}
void clearDoors(){
 for(size_t i=0;i<2;i++){deleteOwned(doorObjects[i]);MarkModelAsNoLongerNeeded(hashName(dw::DoorNames[i]));}
 doorAmount=0;doorHold=0;
}
void staticObject(Object& obj,uint32_t model,Vec3 p,bool collision=false){
 CreateObjectNoOffset(model,p.x,p.y,p.z,&obj,true);
 if(!obj||!DoesObjectExist(obj))throw std::runtime_error("2005 object creation failed: "+std::to_string(model));
 DontRemoveObject(obj);ClearRoomForObject(obj);SetObjectVisible(obj,true);SetObjectAlpha(obj,255);
 SetObjectDynamic(obj,false);FreezeObjectPosition(obj,true);SetObjectCollision(obj,collision);
}
void exteriorDoors(){
 for(size_t i=0;i<2;i++){
  if(doorObjects[i]&&!DoesObjectExist(doorObjects[i]))doorObjects[i]=0;
  if(!doorObjects[i])staticObject(doorObjects[i],hashName(dw::DoorNames[i]),location);
 }
}
bool serviceExterior(){
 if(!placed)return false;
 bool ready=true;
 for(uint32_t h:{tardisModel,hashName(dw::DoorNames[0]),hashName(dw::DoorNames[1])}){RequestModel(h);ready=HasModelLoaded(h)&&ready;}
 RequestCollisionForModel(tardisModel);
 if(!ready)return false;
 if(box&&!DoesObjectExist(box)){log("Exterior handle was lost; recreating at its saved position.");box=0;}
 if(!box)staticObject(box,tardisModel,location);
 SetObjectCoordinates(box,location.x,location.y,location.z);SetObjectHeading(box,heading);
 exteriorDoors();SetObjectVisible(box,true);SetObjectAlpha(box,int(sequence.visual().opacity*255));
 if(!boxBlip||!DoesBlipExist(boxBlip)){
  AddBlipForCoord(location.x,location.y,location.z,&boxBlip);
  if(boxBlip&&DoesBlipExist(boxBlip)){ChangeBlipNameFromAscii(boxBlip,const_cast<char*>("TARDIS"));SetBlipAsShortRange(boxBlip,false);}
 }
 if(boxBlip&&DoesBlipExist(boxBlip))SetBlipCoordinates(boxBlip,location.x,location.y,location.z);
 return true;
}
bool requestRoomResources(){
 bool ready=true;
 for(auto name:dw::RoomModels){auto h=hashName(name);RequestModel(h);ready=HasModelLoaded(h)&&ready;}
 for(size_t i=2;i<4;i++){auto h=hashName(dw::DoorNames[i]);RequestModel(h);ready=HasModelLoaded(h)&&ready;}
 if(dynamicRotorAvailable)for(auto name:dw::RotorNames){auto h=hashName(name);RequestModel(h);ready=HasModelLoaded(h)&&ready;}
 auto floor=hashName(dw::RoomModels[0]);RequestCollisionForModel(floor);
 return HasCollisionForModelLoaded(floor)&&ready;
}
void createRoomObjects(){
 if(dynamicRotorAvailable)for(size_t i=0;i<dw::RotorModelCount;i++){
  if(rotorObjects[i]&&!DoesObjectExist(rotorObjects[i]))rotorObjects[i]=0;
  if(!rotorObjects[i])staticObject(rotorObjects[i],hashName(dw::RotorNames[i]),roomOrigin);
 }
 for(size_t i=0;i<dw::RoomModelCount;i++){
  if(roomObjects[i]&&!DoesObjectExist(roomObjects[i]))roomObjects[i]=0;
  if(!roomObjects[i])staticObject(roomObjects[i],hashName(dw::RoomModels[i]),roomOrigin,i==0);
 }
 for(size_t i=2;i<4;i++){
  if(doorObjects[i]&&!DoesObjectExist(doorObjects[i]))doorObjects[i]=0;
  if(!doorObjects[i])staticObject(doorObjects[i],hashName(dw::DoorNames[i]),roomOrigin);
 }
}
void requestRoom(){
 if(interiorBusy())return;
 waypointExitConfirmed=false;
 for(auto name:dw::RoomModels)if(!IsModelInCdimage(hashName(name))){message(std::string("Coral room model missing: ")+name);return;}
 for(size_t i=2;i<4;i++)if(!IsModelInCdimage(hashName(dw::DoorNames[i]))){message("Coral door models are missing.");return;}
 clearRoom();
 dynamicRotorAvailable=true;
 for(auto name:dw::RotorNames)if(!IsModelInCdimage(hashName(name)))dynamicRotorAvailable=false;
 if(!dynamicRotorAvailable)log("Dynamic rotor WDR meshes unavailable. Existing static rotor retained; animation disabled for this archive.");
 // Keep the room over the departure area instead of a fixed (0,0) locked island.
 roomOrigin={location.x,location.y,700};roomLoading=true;roomTimer=0;
 doorHold=2.1f;requestRoomResources();
 message("Loading the Coral room and floor collision...");
}
void requestRoomExit(){
 if(sequence.phase==dw::Phase::Refuelling)sequence.phase=dw::Phase::Landed;
 if(sequence.phase!=dw::Phase::Landed){message("Wait for the TARDIS to finish landing before exiting.");return;}
 if(entering||leaving||roomSettling||!ownedPed)return;
 cameraOff(false);leaving=true;exitFallback=false;doorTimer=0;roomTimer=0;
 beginTransitHold(safeExit);audio.play(audio.open);
 message("Opening the doors and loading the outside world...");
}
void recoveryExit(const char* reason){
 log(reason);audio.stop();roomSettling=false;entering=false;roomLoading=false;
 sequence.phase=dw::Phase::Landed;sequence.elapsed=0;sequence.reserved=0;sequence.travelling=false;
 streamStarted=false;recall=false;
 if(!leaving)requestRoomExit();
 message("Returning outside; your TARDIS will stay parked.");
}
void finishRoomExit(Vec3 exit){
 safeExit=exit;restorePlayer(true);clearRoom();doorHold=1;serviceExterior();waypointExitConfirmed=false;
 log("Exit complete at "+std::to_string(exit.x)+", "+std::to_string(exit.y)+", "+std::to_string(exit.z));
 message("Left the TARDIS. Its TARDIS marker is on your map.");
}
bool savedOutdoorGround(Vec3 saved,Vec3& exit){
 float z=0,water=0;
 if(!groundAt(saved.x,saved.y,saved.z+3,z)||std::abs(z+1.05f-saved.z)>1.5f)return false;
 if(GetWaterHeight(saved.x,saved.y,z+2,&water)&&water>z-.15f)return false;
 exit={saved.x,saved.y,z+1.05f};return true;
}
void viewExterior(){
 if(!sequence.passenger||sequence.phase==dw::Phase::Transit||interiorBusy())return;
 if(exteriorView){cameraOff();return;}
 roomStandPoint=insideRoom(position(playerPed))?position(playerPed):roomEntryPoint();
 BeginCamCommands(&cameraCommands);CreateCam(14,&roomCamera);
 if(!roomCamera||!DoesCamExist(roomCamera)){EndCamCommands(&cameraCommands);roomCamera=0;return;}
 beginTransitHold(safeExit);
 SetCamFov(roomCamera,55);SetCamActive(roomCamera,true);SetCamPropagate(roomCamera,true);
 ActivateScriptedCams(true,true);exteriorView=true;
}
void animateRotor(float dt){
 if(!roomReady||!dynamicRotorAvailable)return;
 // Do not animate whole room chunks. Only move the new independently exported
 // RotorDown / RotorUp bone-mesh drawables. The collision floor never moves.
 const auto phase=sequence.phase;
 const bool inFlight=phase==dw::Phase::Departing||phase==dw::Phase::Transit||phase==dw::Phase::Arriving;
 const bool running=inFlight||phase==dw::Phase::Refuelling;
 if(running){rotorPhase+=dt*(inFlight?6.0f:2.0f);if(rotorPhase>6.28318531f)rotorPhase=std::fmod(rotorPhase,6.28318531f);}
 // 2005 central column oscillates through its two original bone sections.
 // Never relocate props outside a room that's still being streamed.
 const float desired=running?.78f*(.5f-.5f*std::cos(rotorPhase)):0.f;
 rotorOffset+=dw::clamp(desired-rotorOffset,-dt*2.4f,dt*2.4f);
 for(size_t i=0;i<dw::RotorModelCount;i++)if(rotorObjects[i]&&DoesObjectExist(rotorObjects[i])){
  float offset=rotorOffset;
  // The lower sleeve remains aligned; the upper glass slides independently.
  if(i==0)offset*=.80f;
  SetObjectCoordinates(rotorObjects[i],roomOrigin.x,roomOrigin.y,roomOrigin.z+offset);
  SetObjectVisible(rotorObjects[i],true);
  SetObjectAlpha(rotorObjects[i],255);
 }
 if(running&&sequence.passenger){
  DrawLightWithRange(roomOrigin.x,roomOrigin.y,roomOrigin.z+3.7f+rotorOffset,
                     45,255,140,5.2f,1.6f);
 }
}
void tickInterior(float dt){
 if((roomLoading||entering)&&(!IsPlayerControlOn(GetPlayerId())||GetMissionFlag())){clearRoom();message("Entry cancelled because game control changed.");return;}
 if(roomLoading){
  roomTimer+=dt;
  if(requestRoomResources()){
   createRoomObjects();roomLoading=false;roomReady=true;entering=true;doorTimer=0;
   audio.play(audio.open);log("Coral drawables and model collision loaded. Entry opening.");
  }else if(roomTimer>12){log("Room loading timed out.");clearRoom();message("Room loading failed. You remain outside. See DoctorWhoIV.log.");}
 }
 if(entering){
  doorTimer+=dt;
  if(sequence.phase!=dw::Phase::Landed||distance(position(playerPed),location)>5){clearRoom();message("Entry cancelled.");}
  else if(doorTimer>.7f){
   safeExit=position(playerPed);homeExit=safeExit;homeBox=location;homeHeading=heading;
   GetCharHeading(playerPed,&exitHeading);ownedPed=playerPed;ownedVisible=IsCharVisible(playerPed);
   SetCharHeading(playerPed,180);sequence.passenger=true;entering=false;doorHold=.6f;
   holdInside(roomEntryPoint());
  }
 }
 if(roomSettling){
  roomTimer+=dt;bool anchored=tickTransitHold(dt);
  if(anchored&&requestRoomResources()){
   createRoomObjects();SetObjectCollision(roomObjects[0],true);
   // GET_GROUND_Z is a world-terrain probe, not proof of this custom prop's
   // collision. Require its actual model collision and owned floor object.
   SetCharCoordinatesNoOffset(ownedPed,roomStandPoint.x,roomStandPoint.y,roomStandPoint.z);
   releaseTransitHold();roomSettling=false;
   log("Room models/collision resident; control restored inside.");
   message("Coral room ready. F8: waypoint flight | E by doors: exit | F4: controls.");
  }else if(roomTimer>8)recoveryExit("Interior collision/model restoration timed out.");
 }
 if(leaving){
  doorTimer+=dt;roomTimer+=dt;bool anchored=tickTransitHold(dt);Vec3 exit;
  serviceExterior();
  if(anchored&&(savedOutdoorGround(safeExit,exit)||exitPoint(location,exit)))finishRoomExit(exit);
  else if(roomTimer>12&&waypointExitConfirmed&&finite(safeExit)
          &&std::hypot(safeExit.x-location.x,safeExit.y-location.y)<10.f){
   // The exact point passed the full collision, water and pedestrian-safe
   // checks at arrival. GTA IV may now refuse to answer the same native
   // while the sky interior is being unloaded. Use the confirmed destination
   // rather than silently sending the player to their departure location.
   log("Exit re-probe timed out. Recovering to the confirmed WAYPOINT exit, not home.");
   finishRoomExit(safeExit);
   message("Waypoint arrival preserved. Outdoor ground probe was unavailable.");
  }else if(roomTimer>12&&!exitFallback){
   // One retry at the actual point where this session entered. Never loop back
   // to the sky room on a failed outdoor load, and never invent an exit height.
   exitFallback=true;roomTimer=0;safeExit=homeExit;location=homeBox;heading=homeHeading;
   beginTransitHold(homeExit);serviceExterior();
   log("Exit collision probe timed out; retrying the saved entry location.");
  }else if(roomTimer>12&&exitFallback){
   // Some scene/probe handlers can remain unresponsive. After two full loading
   // windows restore the recorded, previously walkable outdoor point, not Z=700.
   log("Outdoor validation unavailable after retry; using the recorded entry point.");
   finishRoomExit(homeExit);
   message("Recovered to the place you entered. TARDIS kept. See DoctorWhoIV.log.");
  }
 }
 if(sequence.passenger&&!leaving&&!roomSettling&&!exteriorView&&sequence.phase!=dw::Phase::Transit){
  bool intact=requestRoomResources();
  for(auto o:roomObjects)intact=o&&DoesObjectExist(o)&&intact;
  if(dynamicRotorAvailable)for(auto o:rotorObjects)intact=o&&DoesObjectExist(o)&&intact;
  if(!intact){holdInside(roomEntryPoint());log("Room resource lost; holding the player while restoring it.");}
  else{
   auto p=position(playerPed);float x=p.x-roomOrigin.x,y=p.y-roomOrigin.y;
   if(finite(p)&&std::abs(x)<1.3f&&y>=7.5f&&y<9.f){
    if(sequence.phase==dw::Phase::Landed||sequence.phase==dw::Phase::Refuelling)requestRoomExit();
    else{SetCharCoordinatesNoOffset(playerPed,roomEntryPoint().x,roomEntryPoint().y,roomEntryPoint().z);SetCharVelocity(playerPed,0,0,0);SetCamBehindPed(playerPed);message("Doors are locked until landing is complete.");}
   }else if(!finite(p)||std::abs(x)>8.5f||std::abs(y)>8.5f||p.z<roomOrigin.z+.25f||p.z>roomOrigin.z+8){
    recoveryExit("Room bounds/floor guard triggered outside a transition.");
   }
  }
 }
 animateRotor(dt);
 if(sequence.passenger){
  DrawLightWithRange(roomOrigin.x,roomOrigin.y,roomOrigin.z+3.5f,75,220,185,11,2.2f);
  DrawLightWithRange(roomOrigin.x-4,roomOrigin.y,roomOrigin.z+3,255,155,60,9,1.3f);
 }
 float wanted=(roomLoading||entering||leaving||doorHold>0)?1.f:0.f;
 if(wanted==0&&doorAmount>=.999f&&sequence.phase==dw::Phase::Landed)audio.play(audio.close);
 doorAmount+=dw::clamp(wanted-doorAmount,-dt/.6f,dt/.6f);doorHold=std::max(0.f,doorHold-dt);
 for(size_t i=0;i<4;i++)if(doorObjects[i]&&DoesObjectExist(doorObjects[i])){
  float h=i<2?heading:0,r=h*.01745329252f;auto a=dw::DoorAnchors[i];Vec3 base=i<2?location:roomOrigin;
  SetObjectCoordinates(doorObjects[i],base.x+std::cos(r)*a[0]-std::sin(r)*a[1],base.y+std::sin(r)*a[0]+std::cos(r)*a[1],base.z+a[2]);
  SetObjectHeading(doorObjects[i],h+(i%2==0?-95.f:95.f)*doorAmount);
  SetObjectVisible(doorObjects[i],true);SetObjectAlpha(doorObjects[i],i<2?int(sequence.visual().opacity*255):255);
 }
 if(exteriorView&&roomCamera&&DoesCamExist(roomCamera)){
  if(sequence.phase==dw::Phase::Transit){cameraOff(false);return;}
  if(tickTransitHold(dt)&&serviceExterior())revealTransfer();
  float r=heading*.01745329252f;
  SetCamPos(roomCamera,location.x+5*std::cos(r)-7*std::sin(r),location.y+5*std::sin(r)+7*std::cos(r),location.z+3.8f);
  PointCamAtCoord(roomCamera,location.x,location.y,location.z+1.5f);
 }
}
const char* phaseName(){
 switch(sequence.phase){
 case dw::Phase::Away:return "Away";case dw::Phase::Loading:return "Loading";
 case dw::Phase::Arriving:return "Materialising";case dw::Phase::Landed:return "Landed";
 case dw::Phase::Departing:return "Dematerialising";case dw::Phase::Transit:return "Loading destination";
 case dw::Phase::Refuelling:return "Refuelling";
 }return "Waiting";
}
void hudLine(float y,const std::string& text){
 SetTextFont(0);SetTextScale(.25f,.29f);SetTextColour(255,255,255,255);
 SetTextDropshadow(true,0,0,0,255);SetTextProportional(true);SetTextWrap(.565f,.965f);
 DisplayTextWithLiteralString(.57f,y,const_cast<char*>("STRING"),const_cast<char*>(text.c_str()));
}
void showHud(){
 if(!helpVisible&&!sequence.passenger)return;
 DrawRect(.765f,.235f,.41f,helpVisible?.22f:.16f,5,14,22,190);
 hudLine(.17f,std::string("TARDIS 0.2.2 | ")+phaseName()+" | "+std::to_string(int(sequence.energy))+"%");
 if(leaving||roomSettling)hudLine(.202f,"Loading safely... END: return outside");
 else if(exteriorView)hudLine(.202f,"F11: return to the console | END: outside");
 else hudLine(.202f,sequence.passenger?"E: doors/console | F8: flight | F4: help":"F7: call | walk through doors or E | F4: help");
 Blip b=GetFirstBlipInfoId(8);bool waypoint=b&&DoesBlipExist(b);
 if(sequence.phase==dw::Phase::Transit)hudLine(.234f,"Checking destination ground...");
 else if(sequence.phase==dw::Phase::Arriving||sequence.phase==dw::Phase::Departing)hudLine(.234f,"Wait for the flight sequence to finish.");
 else hudLine(.234f,waypoint?"Waypoint selected. F8 flies from inside.":"Set a destination on the pause map.");
 if(helpVisible){hudLine(.266f,"F10: refuel/stop | F11: exterior view");hudLine(.298f,"F9: dismiss | END: return outside");}
}
