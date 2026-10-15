// SPDX-License-Identifier: AGPL-3.0-or-later
// Native graphics facade for the attributed engine, not M5Unified.
#pragma once
#include "avatar.h"
#include "avatar_activity.hpp"
#include <cstddef>
#include <cstdint>
constexpr uint16_t TFT_BLACK=0,TFT_WHITE=0xffff;
class AvatarDisplay {
public:
    int width()const{return 466;} int height()const{return 466;}
    void startWrite();void endWrite();
    void fillScreen(uint16_t color);
    void clearDamage(int x,int y,int w,int h);
    void fillRect(int x,int y,int w,int h,uint16_t color);
    void fillRoundRect(int x,int y,int w,int h,int radius,uint16_t color);
    void fillCircle(int x,int y,int radius,uint16_t color);
    void fillTriangle(int x0,int y0,int x1,int y1,int x2,int y2,uint16_t color);
    void drawLine(int x0,int y0,int x1,int y1,uint16_t color);
    void drawWideLine(float x0,float y0,float x1,float y1,float radius,uint16_t color);
    uint16_t color565(uint8_t r,uint8_t g,uint8_t b)const{return (r>>3)<<11|(g>>2)<<5|(b>>3);}
};
struct AvatarHardware {AvatarDisplay Display;};
extern AvatarHardware M5;
// Native five-second timing summaries replace upstream serial diagnostics.
struct AvatarQuietLog {template<typename... T>void printf(const char*,T...)const{}};
extern AvatarQuietLog Serial;
uint32_t millis();uint32_t micros();long random(long minimum,long maximum);
void avatar_native_clock(uint32_t now);
avatar_pose avatar_native_frame();
bool avatar_native_overflow();
size_t avatar_native_command_count();
bool avatar_native_has_brow();
unsigned avatar_native_expression();
void avatar_native_set_activity(const AvatarActivityFrame &activity);
AvatarActivityFrame avatar_native_activity();
