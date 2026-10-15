// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "actions.h"
#include <stdint.h>
typedef struct { int16_t x, y, rx, ry; } eye_shape;
typedef struct { eye_shape eye[2]; uint32_t action; } eyes_pose;
// Passive input feedback only. No text, key contents, allocation or device I/O.
#ifdef __cplusplus
extern "C" {
#endif
void eyes_reset(uint32_t now_ms);
void eyes_begin(const action *a, uint32_t now_ms);
void eyes_pointer(uint16_t x, uint16_t y);
void eyes_end(bool succeeded, uint32_t now_ms);
// Only the low-priority display task computes geometry and draws strips.
void eyes_sample(uint32_t now_ms, eyes_pose *out);
void eyes_paint_strip(const eyes_pose *pose, uint16_t *pixels, int strip_y);
typedef struct {uint32_t serial,started,ended,pointer,id;int kind,vertical,horizontal;bool running;} eyes_event;
bool eyes_event_snapshot(eyes_event *event);
#ifdef __cplusplus
}
#endif
