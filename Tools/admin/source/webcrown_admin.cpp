#include <iostream>
#include <functional>
#include <thread>

#include <emscripten.h>

#include "gui/Context.hpp"

extern "C" int main(int argc, char** argv)
{
  gui::Context ctx;
  ctx.initialize();

  emscripten_set_main_loop(
      gui::main_loop, 0, 1);
}
