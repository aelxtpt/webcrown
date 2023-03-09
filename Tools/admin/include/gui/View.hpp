#pragma once

#include <vector>

using std::vector;

class View
{

public:
    View();
    ~View();

    void create_models_widget_view();

    // Called each frame
    void draw();
};
