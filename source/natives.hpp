// SPDX-License-Identifier: GPL-3.0-or-later
// Generated from pinned FusionFix source; see SOURCES.md.
#pragma once
#include "bridge.hpp"
namespace gta {
using Camera=int32_t;using Any=int32_t;using Player=int32_t;using Ped=int32_t;using Vehicle=int32_t;using Car=int32_t;using Object=int32_t;using Blip=int32_t;using Interior=int32_t;using Vector3=bridge::Vec3;
inline void GetPlayerChar(Player playerIndex, Ped* pPed){return bridge::call<void>(0x511454A9, playerIndex, pPed);}
inline uint32_t GetPlayerId(){return bridge::call<uint32_t>(0x62E319C6);}
inline bool IsPlayerPlaying(Player playerIndex){return bridge::call<bool>(0x8274BA4, playerIndex);}
inline bool IsPlayerControlOn(Player playerIndex){return bridge::call<bool>(0x30CD2F1F, playerIndex);}
inline bool DoesCharExist(Ped ped){return bridge::call<bool>(0x46531797, ped);}
inline bool IsCharDead(Ped ped){return bridge::call<bool>(0x6A6B4F18, ped);}
inline bool IsCharInjured(Ped ped){return bridge::call<bool>(0x4ECB2267, ped);}
inline bool IsCharInAnyCar(Ped ped){return bridge::call<bool>(0x71184DA3, ped);}
inline bool IsCharInWater(Ped ped){return bridge::call<bool>(0x7B1F0130, ped);}
inline bool IsCharVisible(Ped ped){return bridge::call<bool>(0xA0F19D1, ped);}
inline bool IsPauseMenuActive(){return bridge::call<bool>(0x6C4568A7);}
inline Any IsNetworkGameRunning(){return bridge::call<Any>(0x1CF773D4);}
inline void GetCharCoordinates(Ped ped, float* pX, float* pY, float* pZ){return bridge::call<void>(0x2B5C06E6, ped, pX, pY, pZ);}
inline void GetCharHeading(Ped ped, float* pValue){return bridge::call<void>(0x57A3AC7, ped, pValue);}
inline void GetOffsetFromCharInWorldCoords(Ped ped, float x, float y, float z, float* pOffX, float* pOffY, float* pOffZ){return bridge::call<void>(0x737F24F9, ped, x, y, z, pOffX, pOffY, pOffZ);}
inline void GetGroundZFor3dCoord(float x, float y, float z, float* pGroundZ){return bridge::call<void>(0x6D902EE3, x, y, z, pGroundZ);}
inline bool GetWaterHeight(float x, float y, float z, float* pheight){return bridge::call<bool>(0x2BB9620F, x, y, z, pheight);}
inline bool GetSafePositionForChar(float x, float y, float z, bool unknownTrue, float* pSafeX, float* pSafeY, float* pSafeZ){return bridge::call<bool>(0x5D877285, x, y, z, unknownTrue, pSafeX, pSafeY, pSafeZ);}
inline Vehicle GetClosestCar(float x, float y, float z, float radius, bool unknownFalse, uint32_t unknown70){return bridge::call<Vehicle>(0x2CB303F8, x, y, z, radius, unknownFalse, unknown70);}
inline bool GetClosestCarNode(float x, float y, float z, float* pResX, float* pResY, float* pResZ){return bridge::call<bool>(0x27F87222, x, y, z, pResX, pResY, pResZ);}
inline Blip GetFirstBlipInfoId(uint32_t type){return bridge::call<Blip>(0x3BD729E9, type);}
inline bool DoesBlipExist(Blip blip){return bridge::call<bool>(0x590A6FF4, blip);}
inline void GetBlipCoords(Blip blip, Vector3* pVector){return bridge::call<void>(0x4C1E75DB, blip, pVector);}
inline void RequestCollisionAtPosn(float x, float y, float z){return bridge::call<void>(0x12ED0BC9, x, y, z);}
inline void StartLoadScene(float x, float y, float z){return bridge::call<void>(0x54320B58, x, y, z);}
inline bool UpdateLoadScene(){return bridge::call<bool>(0x513D68DB);}
inline void RequestModel(uint32_t model){return bridge::call<void>(0x502B5185, model);}
inline bool HasModelLoaded(uint32_t model){return bridge::call<bool>(0x4E61480A, model);}
inline void MarkModelAsNoLongerNeeded(uint32_t model){return bridge::call<void>(0xFA0E33, model);}
inline void CreateObjectNoOffset(uint32_t model, float x, float y, float z, Object* pObj, bool unknownTrue){return bridge::call<void>(0x75C51A26, model, x, y, z, pObj, unknownTrue);}
inline bool DoesObjectExist(Object obj){return bridge::call<bool>(0x6DAB78CD, obj);}
inline void DeleteObject(Object* pObj){return bridge::call<void>(0x62FE6290, pObj);}
inline void FreezeObjectPosition(Object obj, bool set){return bridge::call<void>(0x7CA8382B, obj, set);}
inline void SetObjectDynamic(Object obj, bool set){return bridge::call<void>(0x2C591CCD, obj, set);}
inline void SetObjectCollision(Object obj, bool value){return bridge::call<void>(0x24A40229, obj, value);}
inline void SetObjectAlpha(Object obj, int32_t alpha){return bridge::call<void>(0x7F0040DE, obj, alpha);}
inline void SetObjectHeading(Object obj, float value){return bridge::call<void>(0x4F5D027C, obj, value);}
inline void SetObjectCoordinates(Object obj, float pX, float pY, float pZ){return bridge::call<void>(0x52FD30EB, obj, pX, pY, pZ);}
inline void DrawLightWithRange(float x, float y, float z, int32_t r, int32_t g, int32_t b, float a7, float a8){return bridge::call<void>(0x30D27EB1, x, y, z, r, g, b, a7, a8);}
inline void PrintStringWithLiteralString(char* gxtentry, char* string, int32_t time, int32_t flag){return bridge::call<void>(0x3F89280B, gxtentry, string, time, flag);}
inline void FreezeCharPosition(Ped ped, bool frozen){return bridge::call<void>(0x20266A86, ped, frozen);}
inline void SetCharVisible(Ped ped, bool value){return bridge::call<void>(0x4CF0105, ped, value);}
inline void SetCharCoordinatesNoOffset(Ped ped, float x, float y, float z){return bridge::call<void>(0x57C758F0, ped, x, y, z);}
inline void SetCharHeading(Ped ped, float heading){return bridge::call<void>(0x46B5523B, ped, heading);}
inline void GetInteriorFromChar(Ped ped, Interior* pInterior){return bridge::call<void>(0x28227F7, ped, pInterior);}
inline bool DoesVehicleExist(Vehicle vehicle){return bridge::call<bool>(0x67A42263, vehicle);}
inline void SetCarEngineOn(Car car, bool flag0, bool flag1){return bridge::call<void>(0xCAA42D0, car, flag0, flag1);}
inline bool GetMissionFlag(){return bridge::call<bool>(0x2BC64736);}
inline bool IsModelInCdimage(uint32_t model){return bridge::call<bool>(0x771C2838, model);}
inline void RequestCollisionForModel(uint32_t model){return bridge::call<void>(0x66E93537, model);}
inline bool HasCollisionForModelLoaded(uint32_t model){return bridge::call<bool>(0x7C3939E7, model);}
inline void SetCharVelocity(Ped ped, float x, float y, float z){return bridge::call<void>(0x7C76803, ped, x, y, z);}
inline void CreateCam(uint32_t camtype_usually14, Camera* camera){return bridge::call<void>(0x694A0DC1, camtype_usually14, camera);}
inline void DestroyCam(Camera camera){return bridge::call<void>(0x14334EEE, camera);}
inline bool DoesCamExist(Camera camera){return bridge::call<bool>(0x46953225, camera);}
inline void SetCamActive(Camera camera, bool value){return bridge::call<void>(0x43E42686, camera, value);}
inline void SetCamPropagate(Camera camera, bool value){return bridge::call<void>(0x44414E60, camera, value);}
inline void SetCamPos(Camera camera, float pX, float pY, float pZ){return bridge::call<void>(0x152F6314, camera, pX, pY, pZ);}
inline void SetCamFov(Camera camera, float fov){return bridge::call<void>(0x55D470C2, camera, fov);}
inline void PointCamAtCoord(int32_t cam, float x, float y, float z){return bridge::call<void>(0x4496175C, cam, x, y, z);}
inline void ActivateScriptedCams(bool a1, bool a2){return bridge::call<void>(0x3EBE11B9, a1, a2);}
inline void BeginCamCommands(int32_t* a1){return bridge::call<void>(0x351F4C86, a1);}
inline void EndCamCommands(int32_t* a1){return bridge::call<void>(0x627F3275, a1);}
inline void DontRemoveObject(Object obj){return bridge::call<void>(0x74FF26F9, obj);}
inline void ClearRoomForObject(Object obj){return bridge::call<void>(0x12ED69A6, obj);}
inline void SetObjectVisible(Object obj, bool value){return bridge::call<void>(0x372C7B2A, obj, value);}
inline void AddBlipForCoord(float x, float y, float z, Blip* pBlip){return bridge::call<void>(0x3E7D3074, x, y, z, pBlip);}
inline void RemoveBlip(Blip blip){return bridge::call<void>(0x7BBF3625, blip);}
inline void ChangeBlipNameFromAscii(Blip blip, char* blipName){return bridge::call<void>(0x6C9F2330, blip, blipName);}
inline void SetBlipAsShortRange(Blip blip, bool value){return bridge::call<void>(0x2ED90276, blip, value);}
inline void SetBlipCoordinates(int32_t blip, float x, float y, float z){return bridge::call<void>(0x3D91564E, blip, x, y, z);}
inline void DrawRect(float x1, float y1, float x2, float y2, uint32_t r, uint32_t g, uint32_t b, uint32_t a){return bridge::call<void>(0x3B2526E3, x1, y1, x2, y2, r, g, b, a);}
inline void SetTextFont(uint32_t font){return bridge::call<void>(0x75363BB5, font);}
inline void SetTextScale(float w, float h){return bridge::call<void>(0x2C069E5, w, h);}
inline void SetTextColour(uint32_t r, uint32_t g, uint32_t b, uint32_t a){return bridge::call<void>(0x19C967B5, r, g, b, a);}
inline void SetTextDropshadow(bool displayShadow, uint32_t r, uint32_t g, uint32_t b, uint32_t a){return bridge::call<void>(0x58F5023F, displayShadow, r, g, b, a);}
inline void SetTextProportional(bool value){return bridge::call<void>(0x15585A65, value);}
inline void SetTextWrap(float unk1, float unk2){return bridge::call<void>(0x19D006EB, unk1, unk2);}
inline void DisplayTextWithLiteralString(float x, float y, char* gxtName, char* literalStr){return bridge::call<void>(0x661B239A, x, y, gxtName, literalStr);}
inline void DoScreenFadeOut(uint32_t timeMS){return bridge::call<void>(0x65DE621C, timeMS);}
inline void DoScreenFadeIn(uint32_t timeMS){return bridge::call<void>(0x4D72200, timeMS);}
inline bool IsScreenFadedOut(){return bridge::call<bool>(0x59EE3A11);}
inline void SetPlayerControl(Player playerIndex, bool value){return bridge::call<void>(0x1A6203EA, playerIndex, value);}
inline void SetCharCollision(Ped ped, bool set){return bridge::call<void>(0x2A7413EB, ped, set);}
inline void SetLoadCollisionForCharFlag(Ped ped, bool set){return bridge::call<void>(0x4AA762A4, ped, set);}
inline void SetCamBehindPed(Ped ped){return bridge::call<void>(0x48740598, ped);}
}
