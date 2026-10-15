// SPDX-License-Identifier: AGPL-3.0-or-later
// Modified 2026-10-15 for native ESP-IDF; see ORIGIN.md.

#pragma once

#include "avatar_native.hpp"

enum class ExpressionId : uint8_t {
  Idle = 0,
  Listening,
  Thinking,
  Happy,
  Excited,
  Curious,
  Confused,
  Angry,
  Surprised,
  Sad,
  Sleepy,
  Dizzy,
  Count,
};

class AvatarEngine {
 public:
  enum class Easing : uint8_t { Smooth, Snappy, Spring };
  enum class PlaybackMode : uint8_t { Once, Loop, PingPong };

  struct Oscillator {
    float offset;
    float amplitude;
    float angularFrequency;
    float phase;
  };

  struct MotionProfile {
    Oscillator faceX;
    Oscillator faceY;
    Oscillator eyeX;
    Oscillator eyeY;
    Oscillator scale;
    uint16_t blendInMs;
    uint16_t blendOutMs;
  };

  struct BlinkSettings {
    bool enabled;
    uint16_t initialDelayMs;
    uint16_t minIntervalMs;
    uint16_t maxIntervalMs;
    uint16_t closeMs;
    uint16_t openMs;
  };

  struct EyePose {
    float x;
    float y;
    float width;
    float height;
    float roundness;
    float upperLid;
    float upperLidTilt;
    float lowerLid;
    float browY;
    float browTilt;
    float browOpacity;
    float angle;
  };

  struct Pose {
    EyePose leftEye;
    EyePose rightEye;
    float faceScale;
    float faceX;
    float faceY;
    float headYaw;
    float headPitch;
    float headRoll;
  };

  struct Keyframe {
    Pose pose;
    uint16_t transitionMs;
    uint16_t holdMs;
    Easing easing;
  };

  struct ExpressionSpec {
    const char* name;
    const Keyframe* keyframes;
    uint8_t keyframeCount;
    PlaybackMode defaultPlaybackMode;
    BlinkSettings blink;
    MotionProfile motion;
    bool persistent;
  };

  bool begin();
  void update(uint32_t nowMs);
  void show(ExpressionId expression, uint32_t nowMs, bool autoReturn = true,
            uint16_t firstTransitionMs = 0);
  void play(ExpressionId expression, uint32_t nowMs, PlaybackMode mode,
            bool returnToBase = false, uint16_t firstTransitionMs = 0);
  void next(uint32_t nowMs, uint16_t firstTransitionMs = 0);
  void previous(uint32_t nowMs, uint16_t firstTransitionMs = 0);
  void setTouchTarget(int16_t screenX, int16_t screenY);
  void releaseTouch();
  void setSwipeOffset(float screenDeltaX, float screenDeltaY,
                      int8_t previewDirection);
  void releaseSwipe();
  void commitSwipe(int8_t direction, uint32_t nowMs,
                   uint16_t firstTransitionMs = 160);
  void setTiltTarget(float normalizedX, float normalizedY,
                     float motionLeadX = 0.0f, float motionLeadY = 0.0f);
  void setShakeTarget(float normalizedX, float normalizedY, float intensity);
  void invalidate();

  ExpressionId activeExpression() const { return targetExpression_; }
  ExpressionId baseExpression() const { return baseExpression_; }
  const char* activeName() const;
  bool ready() const { return ready_; }

 private:
  struct DirtyRect {
    DirtyRect() = default;
    DirtyRect(int16_t rectX, int16_t rectY, int16_t rectWidth,
              int16_t rectHeight, bool isValid)
        : x(rectX),
          y(rectY),
          width(rectWidth),
          height(rectHeight),
          valid(isValid) {}

    int16_t x = 0;
    int16_t y = 0;
    int16_t width = 0;
    int16_t height = 0;
    bool valid = false;
  };

  static const ExpressionSpec& spec(ExpressionId expression);
  static Pose interpolate(const Pose& from, const Pose& to, float progress,
                          Easing easing);
  static EyePose interpolateEye(const EyePose& from, const EyePose& to,
                                float amount);
  static float ease(Easing easing, float progress);
  static DirtyRect mergeRects(const DirtyRect& first, const DirtyRect& second);

  ExpressionId adjacentExpression(int8_t direction) const;
  void startKeyframe(uint8_t index, uint32_t nowMs,
                     uint16_t transitionOverrideMs = 0);
  void advanceTimeline(uint32_t nowMs);
  void completePlayback(uint32_t nowMs);
  void scheduleNextBlink(uint32_t nowMs, bool useInitialDelay);
  void updateInteraction(uint32_t nowMs);
  void render(uint32_t nowMs);
  void drawEye(const EyePose& eye, float centerX, float centerY, float blink);
  void drawDizzyEyePattern(const EyePose& eye, float centerX, float centerY,
                           int8_t side, uint32_t nowMs);
  void drawDizzyLightning(const EyePose& eye, float centerX, float centerY,
                          int8_t side, uint32_t nowMs);
  DirtyRect eyeBounds(const EyePose& eye, float centerX, float centerY,
                      float blink) const;
  void clearDirtyRect(const DirtyRect& rect);
  float blinkScale(uint32_t nowMs);
  void recordRenderMetrics(uint32_t nowMs, uint32_t renderStartedUs,
                           uint32_t renderFinishedUs);

  bool ready_ = false;
  bool forceRender_ = true;
  bool requiresFullClear_ = true;
  Pose currentPose_{};
  Pose fromPose_{};
  Pose targetPose_{};
  ExpressionId targetExpression_ = ExpressionId::Idle;
  ExpressionId baseExpression_ = ExpressionId::Idle;
  ExpressionId browseExpression_ = ExpressionId::Idle;
  uint8_t activeKeyframeIndex_ = 0;
  PlaybackMode activePlaybackMode_ = PlaybackMode::Loop;
  int8_t playbackDirection_ = 1;
  bool playbackComplete_ = false;
  bool returnToBase_ = false;
  uint32_t transitionStartedMs_ = 0;
  uint32_t transitionDurationMs_ = 260;
  Easing transitionEasing_ = Easing::Smooth;
  uint32_t expressionStartedMs_ = 0;
  uint32_t expressionEndsAtMs_ = 0;
  uint32_t nextBlinkAtMs_ = 0;
  uint32_t blinkStartedMs_ = 0;
  bool touchActive_ = false;
  float touchTargetX_ = 0.0f;
  float touchTargetY_ = 0.0f;
  float tiltTargetX_ = 0.0f;
  float tiltTargetY_ = 0.0f;
  float tiltLeadTargetX_ = 0.0f;
  float tiltLeadTargetY_ = 0.0f;
  float shakeTargetX_ = 0.0f;
  float shakeTargetY_ = 0.0f;
  float shakeOffsetX_ = 0.0f;
  float shakeOffsetY_ = 0.0f;
  float shakeVelocityX_ = 0.0f;
  float shakeVelocityY_ = 0.0f;
  float shakeIntensityTarget_ = 0.0f;
  float shakeIntensity_ = 0.0f;
  float interactionX_ = 0.0f;
  float interactionY_ = 0.0f;
  float interactionVelocityX_ = 0.0f;
  float interactionVelocityY_ = 0.0f;
  float headYaw_ = 0.0f;
  float headPitch_ = 0.0f;
  float headRoll_ = 0.0f;
  float headYawVelocity_ = 0.0f;
  float headPitchVelocity_ = 0.0f;
  float headRollVelocity_ = 0.0f;
  float attentionWeight_ = 1.0f;
  bool swipeActive_ = false;
  float swipeTargetX_ = 0.0f;
  float swipeTargetY_ = 0.0f;
  float swipeOffsetX_ = 0.0f;
  float swipeOffsetY_ = 0.0f;
  float swipeVelocityX_ = 0.0f;
  float swipeVelocityY_ = 0.0f;
  float swipePreviewAmount_ = 0.0f;
  float swipePreviewTarget_ = 0.0f;
  int8_t swipePreviewDirection_ = 0;
  uint32_t lastInteractionUpdateMs_ = 0;
  uint32_t nextFrameUs_ = 0;
  uint32_t metricsStartedMs_ = 0;
  uint32_t metricsFrameCount_ = 0;
  uint32_t totalRenderTimeUs_ = 0;
  uint32_t maximumRenderTimeUs_ = 0;
  uint32_t totalFrameIntervalUs_ = 0;
  uint32_t maximumFrameIntervalUs_ = 0;
  uint32_t previousRenderStartedUs_ = 0;
  DirtyRect previousLeftBounds_{};
  DirtyRect previousRightBounds_{};
};
