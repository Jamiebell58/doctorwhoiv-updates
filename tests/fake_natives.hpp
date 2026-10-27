#pragma once
#include "fake_platform.hpp"
namespace gta {
using Camera=int32_t;using Any=int32_t;using Player=int32_t;using Ped=int32_t;using Vehicle=int32_t;using Car=int32_t;using Object=int32_t;using Blip=int32_t;using Interior=int32_t;using Vector3=bridge::Vec3;
inline void GetPlayerChar(Player playerIndex, Ped* pPed){(void)playerIndex;(void)pPed;*pPed=1;}
inline uint32_t GetPlayerId(){return 0;}
inline bool IsPlayerPlaying(Player playerIndex){(void)playerIndex;return true;}
inline bool IsPlayerControlOn(Player playerIndex){(void)playerIndex;return mock::control;}
inline bool DoesCharExist(Ped ped){(void)ped;return ped==1;}
inline bool IsCharDead(Ped ped){(void)ped;return {};}
inline bool IsCharInjured(Ped ped){(void)ped;return {};}
inline bool IsCharInAnyCar(Ped ped){(void)ped;return {};}
inline bool IsCharInWater(Ped ped){(void)ped;return {};}
inline bool IsCharVisible(Ped ped){(void)ped;return mock::visible;}
inline bool IsPauseMenuActive(){return mock::pause;}
inline Any IsNetworkGameRunning(){return {};}
inline void GetCharCoordinates(Ped ped, float* pX, float* pY, float* pZ){(void)ped;(void)pX;(void)pY;(void)pZ;*pX=mock::p.x;*pY=mock::p.y;*pZ=mock::p.z;}
inline void GetCharHeading(Ped ped, float* pValue){(void)ped;(void)pValue;*pValue=0;}
inline void GetOffsetFromCharInWorldCoords(Ped ped, float x, float y, float z, float* pOffX, float* pOffY, float* pOffZ){(void)ped;(void)x;(void)y;(void)z;(void)pOffX;(void)pOffY;(void)pOffZ;*pOffX=mock::p.x+x;*pOffY=mock::p.y+y;*pOffZ=mock::p.z+z;}
// Ground writes an output only. Model terrain exists only near the player;
// sceneReady deliberately does not determine whether geometry is available.
inline void GetGroundZFor3dCoord(float x,float y,float z,float* out){
 (void)z;
 if(mock::p.z>200||std::hypot(mock::p.x-x,mock::p.y-y)>140){++mock::remoteGroundQueries;return;}
 if(mock::groundReady)*out=0;
}
inline bool GetWaterHeight(float x, float y, float z, float* pheight){(void)x;(void)y;(void)z;(void)pheight;return {};}
inline bool GetSafePositionForChar(float x, float y, float z, bool unknownTrue, float* pSafeX, float* pSafeY, float* pSafeZ){(void)x;(void)y;(void)z;(void)unknownTrue;(void)pSafeX;(void)pSafeY;(void)pSafeZ;*pSafeX=x;*pSafeY=y;*pSafeZ=z;return mock::groundReady;}
inline Vehicle GetClosestCar(float x, float y, float z, float radius, bool unknownFalse, uint32_t unknown70){(void)x;(void)y;(void)z;(void)radius;(void)unknownFalse;(void)unknown70;return {};}
inline bool GetClosestCarNode(float x, float y, float z, float* pResX, float* pResY, float* pResZ){(void)x;(void)y;(void)z;(void)pResX;(void)pResY;(void)pResZ;++mock::roadQueries;*pResX=x;*pResY=y;*pResZ=0;return mock::roadAvailable;}
inline Blip GetFirstBlipInfoId(uint32_t type){(void)type;return mock::hasWaypoint?8:0;}
inline bool DoesBlipExist(Blip blip){(void)blip;return (blip==8&&mock::hasWaypoint)||mock::blips.count(blip);}
inline void GetBlipCoords(Blip blip, Vector3* pVector){(void)blip;(void)pVector;*pVector=mock::waypoint;}
inline void RequestCollisionAtPosn(float x, float y, float z){(void)x;(void)y;(void)z;}
inline void StartLoadScene(float x, float y, float z){(void)x;(void)y;(void)z;mock::events.push_back("scene");mock::scene={x,y,z};if(z<600&&mock::p.z>600&&!mock::frozen){++mock::unsafeSceneStarts;mock::p.z=699;}}
inline bool UpdateLoadScene(){return mock::sceneReady;}
inline void RequestModel(uint32_t model){(void)model;mock::requested.insert(model);}
inline bool HasModelLoaded(uint32_t model){(void)model;return mock::modelsReady;}
inline void MarkModelAsNoLongerNeeded(uint32_t model){(void)model;}
inline void CreateObjectNoOffset(uint32_t model, float x, float y, float z, Object* pObj, bool unknownTrue){(void)model;(void)x;(void)y;(void)z;(void)pObj;(void)unknownTrue;*pObj=mock::next++;mock::objects[*pObj]={model,{x,y,z}};}
inline bool DoesObjectExist(Object obj){(void)obj;return mock::objects.count(obj);}
inline void DeleteObject(Object* pObj){(void)pObj;mock::objects.erase(*pObj);*pObj=0;}
inline void FreezeObjectPosition(Object obj, bool set){(void)obj;(void)set;mock::objects.at(obj).held=set;}
inline void SetObjectDynamic(Object obj, bool set){(void)obj;(void)set;}
inline void SetObjectCollision(Object obj, bool value){(void)obj;(void)value;mock::objects.at(obj).collision=value;}
inline void SetObjectAlpha(Object obj, int32_t alpha){(void)obj;(void)alpha;mock::objects.at(obj).alpha=alpha;}
inline void SetObjectHeading(Object obj, float value){(void)obj;(void)value;}
inline void SetObjectCoordinates(Object obj, float pX, float pY, float pZ){(void)obj;(void)pX;(void)pY;(void)pZ;mock::objects.at(obj).p={pX,pY,pZ};}
inline void DrawLightWithRange(float x, float y, float z, int32_t r, int32_t g, int32_t b, float a7, float a8){(void)x;(void)y;(void)z;(void)r;(void)g;(void)b;(void)a7;(void)a8;}
inline void PrintStringWithLiteralString(char* gxtentry, char* string, int32_t time, int32_t flag){(void)gxtentry;(void)string;(void)time;(void)flag;mock::messages.emplace_back(string);}
inline void FreezeCharPosition(Ped ped, bool frozen){(void)ped;(void)frozen;mock::frozen=frozen;mock::events.push_back(frozen?"freeze":"unfreeze");}
inline void SetCharVisible(Ped ped, bool value){(void)ped;(void)value;mock::visible=value;}
inline void SetCharCoordinatesNoOffset(Ped ped, float x, float y, float z){(void)ped;(void)x;(void)y;(void)z;mock::p={x,y,z};mock::events.push_back("teleport");}
inline void SetCharHeading(Ped ped, float heading){(void)ped;(void)heading;}
inline void GetInteriorFromChar(Ped ped, Interior* pInterior){(void)ped;(void)pInterior;*pInterior=0;}
inline bool DoesVehicleExist(Vehicle vehicle){(void)vehicle;return {};}
inline void SetCarEngineOn(Car car, bool flag0, bool flag1){(void)car;(void)flag0;(void)flag1;}
inline bool GetMissionFlag(){return mock::mission;}
inline bool IsModelInCdimage(uint32_t model){
 if(!mock::missingRotorModels)return true;
 auto hash=[](const char* p){uint32_t h=0;while(*p){h+=uint8_t(*p++);h+=h<<10;h^=h>>6;}h+=h<<3;h^=h>>11;h+=h<<15;return h;};
 return model!=hash("dwiv_rotor_down")&&model!=hash("dwiv_rotor_up");
}
inline void RequestCollisionForModel(uint32_t model){(void)model;}
inline bool HasCollisionForModelLoaded(uint32_t model){(void)model;return mock::modelsReady&&mock::roomCollision;}
inline void SetCharVelocity(Ped ped, float x, float y, float z){(void)ped;(void)x;(void)y;(void)z;}
inline void CreateCam(uint32_t camtype_usually14, Camera* camera){(void)camtype_usually14;(void)camera;*camera=mock::next++;mock::cameras.insert(*camera);}
inline void DestroyCam(Camera camera){(void)camera;mock::cameras.erase(camera);}
inline bool DoesCamExist(Camera camera){(void)camera;return mock::cameras.count(camera);}
inline void SetCamActive(Camera camera, bool value){(void)camera;(void)value;}
inline void SetCamPropagate(Camera camera, bool value){(void)camera;(void)value;}
inline void SetCamPos(Camera camera, float pX, float pY, float pZ){(void)camera;(void)pX;(void)pY;(void)pZ;}
inline void SetCamFov(Camera camera, float fov){(void)camera;(void)fov;}
inline void PointCamAtCoord(int32_t cam, float x, float y, float z){(void)cam;(void)x;(void)y;(void)z;}
inline void ActivateScriptedCams(bool a1, bool a2){(void)a1;(void)a2;}
inline void BeginCamCommands(int32_t* a1){(void)a1;}
inline void EndCamCommands(int32_t* a1){(void)a1;}
inline void DontRemoveObject(Object obj){(void)obj;}
inline void ClearRoomForObject(Object obj){(void)obj;}
inline void SetObjectVisible(Object obj, bool value){(void)obj;(void)value;mock::objects.at(obj).visible=value;}
inline void AddBlipForCoord(float x, float y, float z, Blip* pBlip){(void)x;(void)y;(void)z;(void)pBlip;*pBlip=mock::next++;mock::blips[*pBlip]={x,y,z};}
inline void RemoveBlip(Blip blip){(void)blip;mock::blips.erase(blip);}
inline void ChangeBlipNameFromAscii(Blip blip, char* blipName){(void)blip;(void)blipName;}
inline void SetBlipAsShortRange(Blip blip, bool value){(void)blip;(void)value;}
inline void SetBlipCoordinates(int32_t blip, float x, float y, float z){(void)blip;(void)x;(void)y;(void)z;mock::blips[blip]={x,y,z};}
inline void DrawRect(float x1, float y1, float x2, float y2, uint32_t r, uint32_t g, uint32_t b, uint32_t a){(void)x1;(void)y1;(void)x2;(void)y2;(void)r;(void)g;(void)b;(void)a;}
inline void SetTextFont(uint32_t font){(void)font;}
inline void SetTextScale(float w, float h){(void)w;(void)h;}
inline void SetTextColour(uint32_t r, uint32_t g, uint32_t b, uint32_t a){(void)r;(void)g;(void)b;(void)a;}
inline void SetTextDropshadow(bool displayShadow, uint32_t r, uint32_t g, uint32_t b, uint32_t a){(void)displayShadow;(void)r;(void)g;(void)b;(void)a;}
inline void SetTextProportional(bool value){(void)value;}
inline void SetTextWrap(float unk1, float unk2){(void)unk1;(void)unk2;}
inline void DisplayTextWithLiteralString(float x, float y, char* gxtName, char* literalStr){(void)x;(void)y;(void)gxtName;(void)literalStr;mock::messages.emplace_back(literalStr);}
inline void DoScreenFadeOut(uint32_t ms){(void)ms;mock::faded=true;}
inline void DoScreenFadeIn(uint32_t ms){(void)ms;mock::faded=false;}
inline bool IsScreenFadedOut(){return mock::faded;}
inline void SetPlayerControl(Player p,bool on){(void)p;mock::control=on;}
inline void SetCharCollision(Ped p,bool on){(void)p;mock::collision=on;}
inline void SetLoadCollisionForCharFlag(Ped p,bool on){(void)p;(void)on;}
inline void SetCamBehindPed(Ped p){(void)p;mock::events.push_back("camera-reset");}
}
