// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "eyes.h"
#include "tilt.h"
typedef struct {int x,y,w,h;} avatar_rect;
// Screen-task-owned token and dirty rectangles, no image/text in UI snapshots.
#define AVATAR_DAMAGE_REGIONS 3
typedef struct {uint32_t frame;avatar_rect damage[AVATAR_DAMAGE_REGIONS];} avatar_pose;
typedef struct {uint32_t born,last;} avatar_state;
#ifdef __cplusplus
extern "C" {
#endif
void avatar_reset(avatar_state *state,uint32_t now);
// Touch task only publishes bounded visual metadata; never sends phone input.
void avatar_touch(bool down,uint16_t x,uint16_t y,uint32_t now);
void avatar_step(avatar_state *state,uint32_t now,const eyes_pose *target,tilt_gaze tilt,avatar_pose *out);
avatar_rect avatar_damage_eye(const avatar_pose *old,const avatar_pose *next,unsigned eye);
void avatar_paint_strip(const avatar_pose *pose,uint16_t *pixels,int strip_y);
// Compact RGB565 stride equals width; caller supplies width*rows pixels.
void avatar_paint_region(const avatar_pose *pose,uint16_t *pixels,int x,int y,int width,int rows);
bool avatar_frame_ready(const avatar_pose *pose);
#ifdef __cplusplus
}
#endif
