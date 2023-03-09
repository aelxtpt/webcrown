#pragma once

#include <imgui.h>

namespace gui {
namespace style {


// TODO: move to compile time
inline
ImVec4
GuiColor(unsigned R, unsigned G, unsigned B, unsigned A)
{
  auto r = (R * 1) / 255.0;
  auto g = (G * 1) / 255.0;
  auto b = (B * 1) / 255.0;
  auto a = (A * 1) / 255.0;

  return ImVec4(r,g,b,a);
}

inline
void
configure_style(ImGuiStyle* style)
{
  style->WindowRounding = 5.0f;
  style->FrameBorderSize = 1.0f;
  style->FrameRounding = 5.0f;


  // Colors
  style->Colors[ImGuiCol_WindowBg] = GuiColor(61, 60, 63, 255);
  style->Colors[ImGuiCol_Border] = GuiColor(104, 103, 108, 128);
  style->Colors[ImGuiCol_FrameBg] = GuiColor(36, 36, 37, 138);
}

}
}
