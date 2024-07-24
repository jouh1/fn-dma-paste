#include <Pch.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#include <util.hpp>
#include <settings.hpp>
#include <render.hpp>
#include <offsets.hpp>

struct EntityData {
    uintptr_t mesh;
    uintptr_t player_root;
    uintptr_t bone_array;
};

std::vector<EntityData> entities;
std::vector<EntityData> temp_entities;


void local_pointers() {
    auto scatter_handle = mem.CreateScatterHandle();

    while (true)
    {
        // New UWorld // a

        __int64 va_text = 0;
        for (int i = 0; i < 25; i++) {
            if (mem.Read<__int32>(cache::base + (i * 0x1000) + 0x250) == 0x260E020B) {
                va_text = cache::base + ((i + 1) * 0x1000);
            }
        }

        // Reads

        cache::uworld = mem.Read<__int64>(offsets::UWORLD + va_text);
        cache::game_instance = mem.Read<uintptr_t>(cache::uworld + offsets::GAME_INSTANCE);
        cache::game_state = mem.Read<uintptr_t>(cache::uworld + offsets::GAME_STATE);
        cache::local_players = mem.Read<uintptr_t>(mem.Read<uintptr_t>(cache::game_instance + offsets::LOCAL_PLAYERS));
        cache::player_controller = mem.Read<uintptr_t>(cache::local_players + offsets::PLAYER_CONTROLLER);
        cache::local_pawn = mem.Read<uintptr_t>(cache::player_controller + offsets::LOCAL_PAWN);

        if (cache::local_pawn) {
            mem.AddScatterReadRequest(scatter_handle, cache::local_pawn + offsets::PLAYER_STATE, &cache::player_state, sizeof(uintptr_t));
            mem.AddScatterReadRequest(scatter_handle, cache::player_state + offsets::TEAM_INDEX, &cache::my_team_id, sizeof(int));
            mem.AddScatterReadRequest(scatter_handle, cache::local_pawn + offsets::ROOT_COMPONENT, &cache::root_component, sizeof(uintptr_t));
        }

        mem.AddScatterReadRequest(scatter_handle, cache::uworld + 0x110, &cache::location_pointer, sizeof(uintptr_t));
        mem.AddScatterReadRequest(scatter_handle, cache::uworld + 0x120, &cache::rotation_pointer, sizeof(uintptr_t));
        mem.ExecuteReadScatter(scatter_handle);

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}


void actorloop()
{
    std::array<uintptr_t, 200> player_state_array;
    entities.reserve(200);
    temp_entities.reserve(200);

    while (true)
    {

        cache::player_array = mem.Read<uintptr_t>(cache::game_state + offsets::PLAYER_ARRAY);
        cache::player_count = mem.Read<int>(cache::game_state + (offsets::PLAYER_ARRAY + sizeof(uintptr_t)));

        temp_entities.clear();
        mem.Read(cache::player_array, player_state_array.data(), cache::player_count * sizeof(uintptr_t));

        for (int i = 0; i < cache::player_count; i++) {
            uintptr_t player_state = player_state_array[i];
            if (!player_state) continue;

            int player_team_id = mem.Read<int>(player_state + offsets::TEAM_INDEX);
            if (player_team_id == cache::my_team_id) continue;

            uintptr_t pawn_private = mem.Read<uintptr_t>(player_state + offsets::PAWN_PRIVATE);
            if (!pawn_private || pawn_private == cache::local_pawn) continue;

            auto is_dying = (mem.Read<char>(pawn_private + offsets::IS_DYING) >> 3);
            if (is_dying) continue;

            uintptr_t mesh = mem.Read<uintptr_t>(pawn_private + offsets::MESH);
            if (!mesh) continue;

            uintptr_t bone_array = mem.Read<uintptr_t>(mesh + offsets::BONE_ARRAY);
            if (!bone_array) bone_array = mem.Read<uintptr_t>(mesh + offsets::BONE_ARRAY + 0x10);
            if (!bone_array) continue;

            if (!in_screen(project_world_to_screen(get_entity_bone(bone_array, mesh, bone::BONE_PELVIS)))) continue;

            auto root_component = mem.Read<uintptr_t>(pawn_private + offsets::ROOT_COMPONENT);

            EntityData entity;
            entity.mesh = mesh;
            entity.player_root = root_component;
            entity.bone_array = bone_array;
            temp_entities.push_back(entity);
        }

        entities = temp_entities;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void draw_entities() {

    float closest_screen_distance = FLT_MAX;
    float closest_meter_distance = FLT_MAX;
    uintptr_t closest_mesh = NULL;
    Vector3 closest_bone = { 0, 0 ,0 };

    for (const auto& entity : entities) {

        // Bones 
        Vector3 head3d = get_entity_bone(entity.bone_array, entity.mesh, bone::BONE_HEAD);
        Vector2 head2d = project_world_to_screen(head3d);
        Vector3 bottom3d = get_entity_bone(entity.bone_array, entity.mesh, 0);
        Vector2 bottom2d = project_world_to_screen(bottom3d);

        int box_height = abs(head2d.y - bottom2d.y);
        int box_width = static_cast<int>(box_height * 0.50f);
        float distance = bottom3d.distance(cache::local_camera.location) / 100.0f;

        bool is_vis = is_visible(entity.mesh);

        Vector2 screen_center = Vector2(settings::screen_center_x, settings::screen_center_y);
        double dx = head2d.x - screen_center.x;
        double dy = head2d.y - screen_center.y;
        float dist = sqrtf(dx * dx + dy * dy);

        // Checking the closest mesh and root

        if (is_vis && dist <= settings::aimbot::fov) {
            if (distance < closest_meter_distance) {
                closest_meter_distance = distance;
                closest_screen_distance = dist;
                cache::closest_mesh = entity.mesh;
                cache::closest_player_root = entity.player_root;
                cache::closest_bone_array = entity.bone_array;
            }
        }

        ImColor box_color = is_vis // Check vis color edit
            ? ImColor(settings::visuals::boxColor[0], settings::visuals::boxColor[1], settings::visuals::boxColor[2], settings::visuals::boxColor[3])
            : ImColor(settings::visuals::boxColor2[0], settings::visuals::boxColor2[1], settings::visuals::boxColor2[2], settings::visuals::boxColor2[3]);

        // ESP

        if (settings::visuals::box) {
            draw_cornered_box(head2d.x - (box_width / 2), head2d.y, box_width, box_height, box_color, 1);
        }
        if (settings::visuals::fill_box) {
            ImColor fill_color = box_color;
            fill_color.Value.w = 0.5f;
            draw_filled_rect(head2d.x - (box_width / 2), head2d.y, box_width, box_height, fill_color);
        }
        if (settings::visuals::line) {
            draw_line(bottom2d, box_color);
        }
        if (settings::visuals::distance) {
            draw_distance(head2d, distance, ImColor(250, 250, 250, 250));
        }
    }
        // Aimbot
    
        // Get closest head

    if (cache::closest_mesh && closest_meter_distance < 250.0f) {
        if (settings::aimbot::enable) {

            closest_bone = get_entity_bone(cache::closest_bone_array, cache::closest_mesh, bone::BONE_CHEST);
            Vector3 velocity = mem.Read<Vector3>(cache::closest_player_root + offsets::VELOCITY);

            Vector3 target3d = Prediction(closest_bone, closest_meter_distance, velocity, settings::aimbot::speed, settings::aimbot::grav); // speed, grav... 
            auto target = project_world_to_screen(target3d);
            do_aimbot(target);
        }

        if (settings::aimbot::triggerbot) {
            Vector3 ReticleLocation = mem.Read<Vector3>(cache::player_controller + offsets::LocationUnderReticle);
            if (is_shootable(ReticleLocation, closest_bone)) {
                do_triggerbot();
            }
        }
    }  
}

// Render

WPARAM render_loop() {
    ZeroMemory(&messager, sizeof(MSG));
    while (messager.message != WM_QUIT) {
        if (PeekMessage(&messager, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&messager);
            DispatchMessage(&messager);
        }
        ImGuiIO& io = ImGui::GetIO();
        io.DeltaTime = 1.0f / 60.0f;
        POINT p;
        GetCursorPos(&p);
        io.MousePos.x = static_cast<float>(p.x);
        io.MousePos.y = static_cast<float>(p.y);
        io.MouseDown[0] = GetAsyncKeyState(VK_LBUTTON) != 0;
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        draw_entities();
        render_menu();
        ImGui::EndFrame();
        p_device->SetRenderState(D3DRS_ZENABLE, FALSE);
        p_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        p_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        p_device->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
        if (p_device->BeginScene() >= 0) {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            p_device->EndScene();
        }
        HRESULT result = p_device->Present(nullptr, nullptr, nullptr, nullptr);
        if (result == D3DERR_DEVICELOST && p_device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
            ImGui_ImplDX9_InvalidateDeviceObjects();
            p_device->Reset(&p_params);
            ImGui_ImplDX9_CreateDeviceObjects();
        }
    }
    return messager.wParam;
}

bool init() {
    create_overlay();
    return SUCCEEDED(directx_init());
}
