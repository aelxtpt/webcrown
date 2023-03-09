#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <memory>

namespace gui {

using std::shared_ptr;

class Context
{

public:
  void initialize();
  void draw_contents();

  void quit();
};

void main_loop();

}

#endif // CONTEXT_HPP
