#include "OrbitCamera.h"

#include <algorithm>
#include <cmath>

namespace mdEngine
{
  namespace
  {
    constexpr float PI = 3.14159265359f;
    constexpr float HALF_PI = PI / 2.0f;
    constexpr float KEY_STEP = PI / 12.0f; // 15 degrees

    constexpr float ORBIT_SPEED = 0.01f; // radians per pixel
    constexpr float SCROLL_ZOOM = 0.1f; // fraction of the distance per scroll step
    constexpr float DRAG_ZOOM = 0.01f; // per pixel
    constexpr float MIN_DISTANCE = 0.5f;
    constexpr float MAX_DISTANCE = 200.0f;

    float wrapAngle(const float a)
    {
      return std::remainder(a, 2.0f * PI);
    }

    bool pressed(const ImGuiKey numpad, const ImGuiKey numberRow)
    {
      return ImGui::IsKeyPressed(numpad) || ImGui::IsKeyPressed(numberRow);
    }
  } // namespace

  OrbitCamera::OrbitCamera(const float distance) : initialDistance(distance), distance(distance)
  {
  }

  void OrbitCamera::update(const ImGuiIO& io, const bool hovered, const float viewportHeight)
  {
    updateDrag(io, hovered, viewportHeight);

    if (!hovered)
      return;

    if (io.MouseWheel != 0.0f)
      setDistance(distance * std::exp(-io.MouseWheel * SCROLL_ZOOM));
    if (!io.WantTextInput)
      updateKeys(io);
  }

  void OrbitCamera::reset()
  {
    yaw = 0.0f;
    pitch = 0.0f;
    target = Vec3{};
    distance = initialDistance;
    dragMode = DragMode::None;
  }

  Mat4 OrbitCamera::getViewMatrix() const
  {
    return Mat4::translate(0.0f, 0.0f, -distance)
      * Mat4::rotate(pitch, 1.0f, 0.0f, 0.0f)
      * Mat4::rotate(yaw, 0.0f, 1.0f, 0.0f)
      * Mat4::translate(-target.x, -target.y, -target.z);
  }

  void OrbitCamera::setDistance(const float d)
  {
    distance = std::clamp(d, MIN_DISTANCE, MAX_DISTANCE);
  }

  void OrbitCamera::updateDrag(const ImGuiIO& io, const bool hovered, const float viewportHeight)
  {
    // a drag can only start over the viewport, but keeps going until the button is released
    if (dragMode == DragMode::None)
    {
      if (!hovered)
        return;

      if (io.MouseClicked[ImGuiMouseButton_Middle])
        dragButton = ImGuiMouseButton_Middle;
      else if (io.KeyAlt && io.MouseClicked[ImGuiMouseButton_Left])
        dragButton = ImGuiMouseButton_Left;
      else
        return;

      dragMode = io.KeyCtrl ? DragMode::Zoom : io.KeyShift ? DragMode::Pan : DragMode::Orbit;
      return;
    }

    // the release can arrive in the same frame as the last bit of motion, so apply that first
    const bool released = !io.MouseDown[dragButton];

    const float dx = io.MouseDelta.x;
    const float dy = io.MouseDelta.y;

    switch (dragMode)
    {
      case DragMode::Orbit:
        yaw = wrapAngle(yaw + dx * ORBIT_SPEED);
        pitch = std::clamp(pitch + dy * ORBIT_SPEED, -HALF_PI, HALF_PI);
        break;
      case DragMode::Pan:
      {
        // the scene follows the cursor 1:1 at the target's depth
        const float unitsPerPixel = 2.0f * distance * std::tan(fov * 0.5f) / std::max(viewportHeight, 1.0f);
        const float sy = std::sin(yaw), cy = std::cos(yaw), sp = std::sin(pitch), cp = std::cos(pitch);

        // camera right and up in world space
        const Vec3 right{cy, 0.0f, sy};
        const Vec3 up{sp * sy, cp, -sp * cy};
        target = target - right * (dx * unitsPerPixel) + up * (dy * unitsPerPixel);
        break;
      }
      case DragMode::Zoom:
        setDistance(distance * std::exp(dy * DRAG_ZOOM)); // drag up to zoom in
        break;
      default:
        break;
    }

    if (released)
      dragMode = DragMode::None;
  }

  void OrbitCamera::updateKeys(const ImGuiIO& io)
  {
    const float side = io.KeyCtrl ? PI : 0.0f;

    if (pressed(ImGuiKey_Keypad1, ImGuiKey_1)) // front, ctrl: back
    {
      yaw = side;
      pitch = 0.0f;
    }
    if (pressed(ImGuiKey_Keypad3, ImGuiKey_3)) // right, ctrl: left
    {
      yaw = io.KeyCtrl ? HALF_PI : -HALF_PI;
      pitch = 0.0f;
    }
    if (pressed(ImGuiKey_Keypad7, ImGuiKey_7)) // top, ctrl: bottom
    {
      yaw = 0.0f;
      pitch = io.KeyCtrl ? -HALF_PI : HALF_PI;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Keypad4) || ImGui::IsKeyPressed(ImGuiKey_4))
      yaw = wrapAngle(yaw + KEY_STEP);
    if (ImGui::IsKeyPressed(ImGuiKey_Keypad6) || ImGui::IsKeyPressed(ImGuiKey_6))
      yaw = wrapAngle(yaw - KEY_STEP);
    if (ImGui::IsKeyPressed(ImGuiKey_Keypad8) || ImGui::IsKeyPressed(ImGuiKey_8))
      pitch = std::min(pitch + KEY_STEP, HALF_PI);
    if (ImGui::IsKeyPressed(ImGuiKey_Keypad2) || ImGui::IsKeyPressed(ImGuiKey_2))
      pitch = std::max(pitch - KEY_STEP, -HALF_PI);

    if (ImGui::IsKeyPressed(ImGuiKey_KeypadDecimal) || ImGui::IsKeyPressed(ImGuiKey_Home))
      reset();
  }
} // mdEngine
