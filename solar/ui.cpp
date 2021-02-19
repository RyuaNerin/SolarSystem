#include "ui.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "camera.h"
#include "config.h"
#include "constants.h"
#include "defer.h"
#include "planet.h"
#include "render.h"

namespace ui
{
    ImGuiIO io;

    bool visible;

    ImVec2 size, location;

    void init(GLFWwindow* window)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        
        io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.IniFilename = nullptr;
        io.LogFilename = nullptr;
        ImGui::StyleColorsDark();

        ImGui_ImplOpenGL3_Init("#version 330");
        ImGui_ImplGlfw_InitForOpenGL(window, false);
    }

    bool hitTest(v::Point2f pos) noexcept
    {
        return
            visible &&
            location.x <= pos.x && pos.x <= location.x + size.x &&
            location.y <= pos.y && pos.y <= location.y + size.y;
    }

    void render(v::Size2f winRect, const config::Config cfgc, const camera::Camera camc)
    {
        if (!visible) return;

        // ImGui 라이브러리 참조

        config::Config cfg = cfgc;
        camera::Camera cam = camc;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        {
            ImGui::NewFrame();
            defer(ImGui::EndFrame());

            ImGui::Begin("CONFIG", &visible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
            defer(ImGui::End());

            // 화면 밖으로 나간 경우 위치 재설정
            {
                location = ImGui::GetWindowPos();
                size = ImGui::GetWindowSize();

                location.x = std::clamp(location.x, -size.x / 2, winRect.w - size.x / 2);
                location.y = std::clamp(location.y, -size.y / 2, winRect.h - size.y / 2);

                ImGui::SetWindowPos(location);
            }

            {
                ImGui::BeginTabBar("#tab");
                defer(ImGui::EndTabBar());

                ImGui::Dummy(ImVec2(0, 10));

                if (ImGui::BeginTabItem("DISPLAY"))
                {
                    defer(ImGui::EndTabItem());

                    ImGui::Checkbox("Show fps", &cfg.showFPS);
                    ImGui::Checkbox("Show help", &cfg.showHelp);

                    ImGui::Dummy(ImVec2(0, 10));
                    ImGui::Separator();
                    ImGui::Dummy(ImVec2(0, 10));

                    ImGui::Checkbox("Play animation", &cfg.playAnimation);
                    ImGui::Checkbox("Show animation speed", &cfg.showSpeed);
                    ImGui::Checkbox("Show Date", &cfg.showDate);
                    ImGui::Text("Animation speed");
                    ImGui::SliderFloat("##speed", &cfg.speed, SPEED_MIN, SPEED_MAX, "%.2f days / sec", ImGuiSliderFlags_Logarithmic);

                    if (ImGui::Button("Reset to J2000"))
                    {
                        render::resetDate();
                    }

                    ImGui::Dummy(ImVec2(0, 10));
                    ImGui::Separator();
                    ImGui::Dummy(ImVec2(0, 10));

                    ImGui::Checkbox("Show background", &cfg.showBackground);

                    ImGui::Dummy(ImVec2(0, 10));
                    ImGui::Separator();
                    ImGui::Dummy(ImVec2(0, 10));

                    ImGui::Checkbox("Show planet name", &cfg.showPlanetName);
                    ImGui::Checkbox("Show orbit", &cfg.showOrbit);

                    if (!cfg.showOrbit)
                    {
                        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
                    }
                    ImGui::Dummy(ImVec2(15, 0)); ImGui::SameLine(); ImGui::Checkbox("Use color", &cfg.showOrbitColored);
                    ImGui::Dummy(ImVec2(15, 0)); ImGui::SameLine(); ImGui::Checkbox("Use gradient", &cfg.showOrbitGradient);

                    ImGui::Dummy(ImVec2(15, 0)); ImGui::SameLine(); ImGui::Text("Orbit style");
                    ImGui::Dummy(ImVec2(25, 0)); ImGui::SameLine(); ImGui::RadioButton("Dotted", &cfg.showOrbitType, config::OrbitType::Dotted);
                    ImGui::Dummy(ImVec2(25, 0)); ImGui::SameLine(); ImGui::RadioButton("Solid",  &cfg.showOrbitType, config::OrbitType::Solid );
                    if (!cfg.showOrbit)
                    {
                        ImGui::PopItemFlag();
                        ImGui::PopStyleVar();
                    }

                    ImGui::Dummy(ImVec2(0, 10));
                    ImGui::Separator();
                    ImGui::Dummy(ImVec2(0, 10));

                    if (ImGui::Button("Reset"))
                    {
                        cfg = config::reset();
                    }
                }

                if (ImGui::BeginTabItem("CAMERA"))
                {
                    defer(ImGui::EndTabItem());
                    
                    ImGui::Text("Distance");  ImGui::SliderFloat("##disance",   &cam.radius,    CAMERA_RADIUS_MIN,     CAMERA_RADIUS_MAX, "%.3f", ImGuiSliderFlags_Logarithmic);

                    ImGui::Text("Elevation"); ImGui::SliderFloat("##elevation", &cam.elevation, CAMERA_ELEVATION_MIN, CAMERA_ELEVATION_MAX);
                    ImGui::Text("Azimuth");   ImGui::SliderFloat("##azimuth",   &cam.azimuth,   0,                    360                 );

                    ImGui::Dummy(ImVec2(0, 10));
                    ImGui::Separator();
                    ImGui::Dummy(ImVec2(0, 10));

                    ImGui::Text("Focusing");
                    ImGui::Dummy(ImVec2(15, 0)); ImGui::SameLine(); ImGui::RadioButton("Sun",     &cam.focusedPlanet, planet::PlanetIndex::Sun    );
                    ImGui::Dummy(ImVec2(15, 0)); ImGui::SameLine(); ImGui::RadioButton("Mercury", &cam.focusedPlanet, planet::PlanetIndex::Mercury);
                    ImGui::Dummy(ImVec2(15, 0)); ImGui::SameLine(); ImGui::RadioButton("Venus",   &cam.focusedPlanet, planet::PlanetIndex::Venus  );
                    ImGui::Dummy(ImVec2(15, 0)); ImGui::SameLine(); ImGui::RadioButton("Earth",   &cam.focusedPlanet, planet::PlanetIndex::Earth  );
                    ImGui::Dummy(ImVec2(30, 0)); ImGui::SameLine(); ImGui::RadioButton("Moon",    &cam.focusedPlanet, planet::PlanetIndex::Moon   );
                    ImGui::Dummy(ImVec2(15, 0)); ImGui::SameLine(); ImGui::RadioButton("Mars",    &cam.focusedPlanet, planet::PlanetIndex::Mars   );
                    ImGui::Dummy(ImVec2(15, 0)); ImGui::SameLine(); ImGui::RadioButton("Jupiter", &cam.focusedPlanet, planet::PlanetIndex::Jupiter);
                    ImGui::Dummy(ImVec2(15, 0)); ImGui::SameLine(); ImGui::RadioButton("Saturn",  &cam.focusedPlanet, planet::PlanetIndex::Saturn );
                    ImGui::Dummy(ImVec2(15, 0)); ImGui::SameLine(); ImGui::RadioButton("Uranus",  &cam.focusedPlanet, planet::PlanetIndex::Uranus );
                    ImGui::Dummy(ImVec2(15, 0)); ImGui::SameLine(); ImGui::RadioButton("Neptune", &cam.focusedPlanet, planet::PlanetIndex::Neptune);
                    ImGui::Dummy(ImVec2(15, 0)); ImGui::SameLine(); ImGui::RadioButton("Pluto",   &cam.focusedPlanet, planet::PlanetIndex::Pluto  );

                    ImGui::Dummy(ImVec2(0, 10));
                    ImGui::Separator();
                    ImGui::Dummy(ImVec2(0, 10));

                    if (ImGui::Button("Reset"))
                    {
                        cam = camera::reset();
                    }
                }

                if (ImGui::BeginTabItem("GRAPHICS"))
                {
                    defer(ImGui::EndTabItem());

                    ImGui::Checkbox("VSync", &cfg.vsync);
                    ImGui::Checkbox("MSAA MultiSampling", &cfg.enableMSAA);
                }
            }
        }

        config::update(cfg);
        camera::update(cam);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void toggleVisible() noexcept
    {
        visible = !visible;
    }

    void close() noexcept
    {
        visible = false;
    }
}
