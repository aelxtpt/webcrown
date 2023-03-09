#include "gui/Context.hpp"
#include "gui/GuiConfig.hpp"
#include "gui/View.hpp"

// GLFW
#include <GLFW/glfw3.h>

// IMGUI
#include "imgui/imgui.h"
#include "imgui/impls/imgui_impl_glfw.h"
#include "imgui/impls/imgui_impl_opengl3.h"


#include <cassert>
#include <thread>
#include <sstream>
#include <iostream>
#include <memory>

using std::unique_ptr;
using std::make_unique;


#include <emscripten.h>


// widgets (temporary here)


// static variables
unique_ptr<View> view;

namespace gui {

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
    //NANO_LOG(DEBUG, "Error GLFW %d; %s", error, description);
}

void teardown(GLFWwindow *window)
{
    if (window != NULL) { glfwDestroyWindow(window); }
    glfwTerminate();
}

static float dpiScale = 1.f;
static GLFWwindow* s_glfwWindow = nullptr;
ImGuiContext* imgui = 0;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);



EM_JS(int, canvas_get_width, (), {
  return Module.canvas.width;
});

EM_JS(int, canvas_get_height, (), {
  return Module.canvas.height;
});

EM_JS(void, resizeCanvas, (), {
    document.getElementById('canvas').width = window.innerWidth;
    document.getElementById('canvas').height = window.innerHeight;
});

void
Context::initialize()
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
    {
       assert(glfwInit());
       return;
    }

    // setup GLFW window

    glfwWindowHint(GLFW_DOUBLEBUFFER , 1);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    glfwWindowHint(
       GLFW_OPENGL_PROFILE,
       GLFW_OPENGL_CORE_PROFILE
       );


    std::string glsl_version = "";
#ifdef __APPLE__
    std::cout << "APPLE!!!!\n";
    // GL 3.2 + GLSL 150
    glsl_version = "#version 150";
    glfwWindowHint( // required on Mac OS
       GLFW_OPENGL_FORWARD_COMPAT,
       GL_TRUE
       );
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#elif __linux__
    // GL 3.2 + GLSL 150
    glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#elif _WIN32
    std::cout << "WIN32!!!\n";
    // GL 3.0 + GLSL 130
    glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
#endif

    GLFWwindow* window = glfwCreateWindow(1080, 720, "WebCrown Admin", NULL, NULL);
    if (!window)
    {
       assert(window);
       return;
    }

    //glfwSetWindowPos(window, 200, 200);
    s_glfwWindow = window;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // enable vsync // Lol, disable this ang get 92% cpu


    // Setup ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();

    // Load Fonts
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true; // Garante que move apenas pelo title bar.

    ImFont* font_san = io.Fonts->AddFontFromFileTTF("/home/aex/code/webcrown/Tools/admin/resources/fonts/Roboto-Regular.ttf", 16);
    ImFontConfig cfg;

    io.Fonts->Build();

    auto style = &ImGui::GetStyle();
    gui::style::configure_style(style);

//    style.WindowBorderSize = 1.f * dpiScale;
//    style.FrameBorderSize = 1.f * dpiScale;
//    style.FrameRounding = 5.f;
//    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(1, 1, 1, 0.03f);
//    style.ScaleAllSizes(dpiScale);

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    imgui = ImGui::GetCurrentContext();

    resizeCanvas();

    view = make_unique<View>();
}

void
main_loop()
{
    int width = canvas_get_width();
    int height = canvas_get_height();

    glfwSetWindowSize(s_glfwWindow, width, height);

    ImGui::SetCurrentContext(imgui);
    glfwPollEvents();

    const ImVec4 clear_color = ImColor(114, 144, 154);
    int display_w, display_h;
    glfwGetFramebufferSize(s_glfwWindow, &display_w, &display_h);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();
    //my_window.show();


    // -----------------------------------------------------------------------------


    // Main menu bar
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Models"))
        {
            if(ImGui::MenuItem("Inspecionar Models"))
            {
                view->create_models_widget_view();
            }

            //ImGui::MenuItem("Memory Scan", nullptr, &show_memory_scan_w);


            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    // -----------------------------------------------------------------------------

    //main_w.draw();

    // Rendering
    ImGui::Render();
    glfwMakeContextCurrent(s_glfwWindow);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwMakeContextCurrent(s_glfwWindow);
    glfwSwapBuffers(s_glfwWindow);
}

void
Context::quit()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(s_glfwWindow);
    glfwTerminate();
}

void
Context::draw_contents()
{
    const ImVec4 clear_color = ImColor(114, 144, 154);
    int display_w, display_h;
    glfwGetFramebufferSize(s_glfwWindow, &display_w, &display_h);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //ImGui::ShowDemoWindow();
    //my_window.show();


    //main_w.draw();

    // Rendering
    ImGui::Render();
    glfwMakeContextCurrent(s_glfwWindow);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwMakeContextCurrent(s_glfwWindow);
    glfwSwapBuffers(s_glfwWindow);
}

}
