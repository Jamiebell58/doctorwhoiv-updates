#pragma once
#include <cstddef>
namespace dw {
inline constexpr const char* RoomModels[]={"dwiv_room00","dwiv_room01","dwiv_room02","dwiv_room03","dwiv_room04","dwiv_room05","dwiv_room06","dwiv_room07","dwiv_room08","dwiv_room09","dwiv_room10","dwiv_room11","dwiv_room12","dwiv_room13","dwiv_room14","dwiv_room15","dwiv_room16","dwiv_room17","dwiv_room18"};
inline constexpr size_t RoomModelCount=sizeof(RoomModels)/sizeof(RoomModels[0]);
// The 2005 interior2.mdl has actual RotorDown/RotorUp bones. The converter
// extracts their weighted faces into two independently animated drawables.
inline constexpr const char* RotorNames[]={"dwiv_rotor_down","dwiv_rotor_up"};
inline constexpr size_t RotorModelCount=sizeof(RotorNames)/sizeof(RotorNames[0]);
inline constexpr const char* DoorNames[]={"dwiv_ext_l","dwiv_ext_r","dwiv_int_l","dwiv_int_r"};
inline constexpr float DoorAnchors[4][3]={{-0.5691205f,0.6510528f,0.1197864f},{0.5691205f,0.6510528f,0.1197864f},{-0.5691205f,8.061f,0.1455674f},{0.5691205f,8.061f,0.1455674f}};
}
