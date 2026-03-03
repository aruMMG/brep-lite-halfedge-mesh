#include <cstdio>
#include <filesystem>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "mesh_renderer.hpp"
#include "mesh_extract_user.hpp"
#include "math.hpp"
#include "obj_loader.hpp"
#include "brep_lite/cube_builder.hpp"

static void glfw_error_callback(int error, const char* description)
{
    std::fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main()
{
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit())
        return 1;

    // OpenGL 3.3 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "HalfEdge Viewer (Milestone 1)", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::fprintf(stderr, "Failed to initialize OpenGL loader (GLEW)\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    // =========================================================================
    // SimpleRenderer renderer;
    // if (!renderer.init())
    // {
    //     std::fprintf(stderr, "Failed to init SimpleRenderer\n");
    //     glfwDestroyWindow(window);
    //     glfwTerminate();
    //     return 1;
    // }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    brep_lite::HalfEdgeMesh he = brep_lite::build_cube_quads(1.0);

    auto errs = he.validate(true);
    if (!errs.empty()) {
        std::cerr << "Mesh validation failed:\n";
        for (auto& e : errs) std::cerr << "  - " << e << "\n";
        return 1;
    }

    std::filesystem::path tmp = std::filesystem::temp_directory_path() / "halfedge_viewer_tmp.obj";
    if (!he.write_obj(tmp)) {
        std::cerr << "Failed to write temp OBJ: " << tmp << "\n";
        return 1;
    }

    MeshData md;
    std::string load_err;
    if (!viewer::LoadObjMesh(tmp, md, load_err)) {
        std::cerr << "Failed to load temp OBJ: " << load_err << "\n";
        return 1;
    }

    MeshRenderer mesh_r;
    if (!mesh_r.init()) {
        std::cerr << "MeshRenderer init failed\n";
        return 1;
    }
    mesh_r.upload(md);
    // =========================================================================
    // GLEW can leave a benign GL error on init; clear it.
    glGetError();

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Optional (recommended later): io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char* glsl_version = "#version 330";
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool show_demo = true;

    // =======================================================================
    static float yaw_deg = 45.0f;
    static float pitch_deg = 25.0f;
    static float distance = 4.5f;

    static bool pause_rotation = false;
    static bool show_edges = true;

    static float model_angle = 0.0f;
    // =======================================================================


    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ---- UI ----
        ImGui::Begin("Milestone 1");
        ImGui::Text("GLFW + OpenGL3 + ImGui is running.");

        // =======================================================================
        ImGui::SliderFloat("Yaw (deg)", &yaw_deg, -180.0f, 180.0f);
        ImGui::SliderFloat("Pitch (deg)", &pitch_deg, -89.0f, 89.0f);
        ImGui::SliderFloat("Distance", &distance, 1.0f, 20.0f);

        ImGui::Checkbox("Pause rotation", &pause_rotation);
        ImGui::Checkbox("Show edge overlay", &show_edges);

        if (ImGui::Button("Reset view")) {
            yaw_deg = 45.0f;
            pitch_deg = 25.0f;
            distance = 4.5f;
            model_angle = 0.0f;
            pause_rotation = false;
        }
        // =======================================================================

        ImGui::Checkbox("Show ImGui demo window", &show_demo);
        ImGui::End();

        if (show_demo)
            ImGui::ShowDemoWindow(&show_demo);

        // ---- Render ----
        ImGui::Render();

        int display_w = 0, display_h = 0;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT);

        // =======================================================================
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // static bool wireframe = false;
        // static float angle = 0.0f;
        // angle += 0.01f;

        // Mat4 P = perspective(60.0f * 3.1415926f / 180.0f,
        //                     (float)display_w / (float)display_h,
        //                     0.1f, 100.0f);
        // Mat4 V = lookAt({3.0f, 3.0f, 3.0f}, {0,0,0}, {0,1,0});
        // Mat4 M = rotateY(angle);
        // Mat4 MVP = mul(P, mul(V, M));

        // mesh_r.render(MVP, wireframe);
        // renderer.render(display_w, display_h);
        if (!pause_rotation) model_angle += 0.01f;

        const float deg2rad = 3.1415926f / 180.0f;
        float yaw = yaw_deg * deg2rad;
        float pitch = pitch_deg * deg2rad;

        // Orbit camera around origin, Y-up
        Vec3 eye {
            distance * std::cos(pitch) * std::cos(yaw),
            distance * std::sin(pitch),
            distance * std::cos(pitch) * std::sin(yaw)
        };

        Mat4 P = perspective(60.0f * deg2rad,
                            (float)display_w / (float)display_h,
                            0.1f, 100.0f);
        Mat4 V = lookAt(eye, {0,0,0}, {0,1,0});
        Mat4 M = rotateY(model_angle);
        Mat4 MVP = mul(P, mul(V, M));

        // 1) Filled pass
        mesh_r.render(MVP, false, 0.85f, 0.85f, 0.92f);

        // 2) Edge overlay pass
        if (show_edges) {
            glDepthMask(GL_FALSE);
            glEnable(GL_POLYGON_OFFSET_LINE);
            glPolygonOffset(-1.0f, -1.0f);

            mesh_r.render(MVP, true, 0.10f, 0.10f, 0.10f);

            glDisable(GL_POLYGON_OFFSET_LINE);
            glDepthMask(GL_TRUE);
        }
        // =======================================================================

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // renderer.shutdown();
    mesh_r.shutdown();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}