#pragma once
#include <d3d9.h>
#include <vector>
#include <offsets.hpp>
#include <settings.hpp>
#include <cmath>
#include <sstream>

#define M_PI 3.14159265358979323846264338327950288419716939937510

enum bone {
    BONE_BOTTOM = 0,
    BONE_HEAD = 110,
    BONE_LOWERHEAD = 106,
    BONE_NECK = 66,
    BONE_NECK2 = 8,
    BONE_CHEST = 7,
    BONE_PELVIS = 2,
    BONE_RSHOULDER = 38,
    BONE_RELBOW = 39,
    BONE_RHAND = 40,
    BONE_RTHIGH = 78,
    BONE_RKNEE = 79,
    BONE_RFOOT = 82,
    BONE_RFOOTOUT = 83,
    BONE_LSHOULDER = 9,
    BONE_LELBOW = 10,
    BONE_LHAND = 11,
    BONE_LTHIGH = 71,
    BONE_LKNEE = 72,
    BONE_LFOOT = 75,
    BONE_LFOOTOUT = 76
};

class Vector2 {
public:
    Vector2() : x(0.f), y(0.f) {}
    Vector2(double _x, double _y) : x(_x), y(_y) {}
    ~Vector2() = default;

    double x, y;
};

class Vector3 {
public:
    Vector3() : x(0.f), y(0.f), z(0.f) {}
    Vector3(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}
    ~Vector3() = default;

    double x, y, z;

    inline double dot(const Vector3& v) const { return x * v.x + y * v.y + z * v.z; }
    inline double distance(const Vector3& v) const {
        return std::sqrt(std::pow(v.x - x, 2) + std::pow(v.y - y, 2) + std::pow(v.z - z, 2));
    }

    Vector3 operator-(const Vector3& v) const {
        return Vector3(x - v.x, y - v.y, z - v.z);
    }
};

struct FQuat {
    double x, y, z, w;
};
struct FTransform
{
    FQuat  rot;
    Vector3 translation;
    char    pad[8];
    Vector3 scale;

    D3DMATRIX to_matrix_with_scale()
    {
        D3DMATRIX m;
        m._41 = translation.x;
        m._42 = translation.y;
        m._43 = translation.z;

        double x2 = rot.x + rot.x;
        double y2 = rot.y + rot.y;
        double z2 = rot.z + rot.z;

        double xx2 = rot.x * x2;
        double yy2 = rot.y * y2;
        double zz2 = rot.z * z2;
        m._11 = (1.0f - (yy2 + zz2)) * scale.x;
        m._22 = (1.0f - (xx2 + zz2)) * scale.y;
        m._33 = (1.0f - (xx2 + yy2)) * scale.z;

        double yz2 = rot.y * z2;
        double wx2 = rot.w * x2;
        m._32 = (yz2 - wx2) * scale.z;
        m._23 = (yz2 + wx2) * scale.y;

        double xy2 = rot.x * y2;
        double wz2 = rot.w * z2;
        m._21 = (xy2 - wz2) * scale.y;
        m._12 = (xy2 + wz2) * scale.x;

        double xz2 = rot.x * z2;
        double wy2 = rot.w * y2;
        m._31 = (xz2 + wy2) * scale.z;
        m._13 = (xz2 - wy2) * scale.x;

        m._14 = 0.0f;
        m._24 = 0.0f;
        m._34 = 0.0f;
        m._44 = 1.0f;

        return m;
    }
};

D3DMATRIX matrix_multiplication(const D3DMATRIX& m1, const D3DMATRIX& m2) {
    D3DMATRIX result{};
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i][j] = m1.m[i][0] * m2.m[0][j] + m1.m[i][1] * m2.m[1][j] + m1.m[i][2] * m2.m[2][j] + m1.m[i][3] * m2.m[3][j];
        }
    }
    return result;
}

D3DMATRIX to_matrix(const Vector3& rot, const Vector3& origin = Vector3(0, 0, 0)) {
    float radpitch = (rot.x * M_PI / 180);
    float radyaw = (rot.y * M_PI / 180);
    float radroll = (rot.z * M_PI / 180);

    float sp = std::sin(radpitch);
    float cp = std::cos(radpitch);
    float sy = std::sin(radyaw);
    float cy = std::cos(radyaw);
    float sr = std::sin(radroll);
    float cr = std::cos(radroll);

    D3DMATRIX matrix{};
    matrix.m[0][0] = cp * cy;
    matrix.m[0][1] = cp * sy;
    matrix.m[0][2] = sp;
    matrix.m[0][3] = 0.f;

    matrix.m[1][0] = sr * sp * cy - cr * sy;
    matrix.m[1][1] = sr * sp * sy + cr * cy;
    matrix.m[1][2] = -sr * cp;
    matrix.m[1][3] = 0.f;

    matrix.m[2][0] = -(cr * sp * cy + sr * sy);
    matrix.m[2][1] = cy * sr - cr * sp * sy;
    matrix.m[2][2] = cr * cp;
    matrix.m[2][3] = 0.f;

    matrix.m[3][0] = origin.x;
    matrix.m[3][1] = origin.y;
    matrix.m[3][2] = origin.z;
    matrix.m[3][3] = 1.f;

    return matrix;
}

struct Camera {
    Vector3 location;
    Vector3 rotation;
    float fov;
};

struct FNRot {
    double a;
    char pad_0008[24];
    double b;
    char pad_0028[424];
    double c;
};

namespace cache {
    inline uintptr_t base;
    inline uintptr_t uworld;
    inline uintptr_t game_instance;
    inline uintptr_t local_players;
    inline uintptr_t player_controller;
    inline uintptr_t local_pawn;
    inline uintptr_t root_component;
    inline uintptr_t player_state;
    inline uintptr_t game_state;
    inline uintptr_t player_array;
    inline uintptr_t pawn_private;
    inline uintptr_t closest_mesh;
    inline uintptr_t closest_player_root;
    inline uintptr_t location_pointer;
    inline uintptr_t rotation_pointer;
    inline uintptr_t closest_bone_array;
    inline Vector3 relative_location;
    inline Camera local_camera;
    inline int player_count;
    inline int my_team_id;

}

Camera get_view_point() {

    float fov;
    FNRot fnrot{};
    Camera view_point{};
    uintptr_t location_pointer = cache::location_pointer;
    uintptr_t rotation_pointer = cache::rotation_pointer;

    auto cache_handle = mem.CreateScatterHandle();

    mem.AddScatterReadRequest(cache_handle, rotation_pointer, &fnrot, sizeof(FNRot));
    mem.AddScatterReadRequest(cache_handle, location_pointer, &view_point.location, sizeof(Vector3));
    mem.AddScatterReadRequest(cache_handle, cache::player_controller + 0x394, &fov, sizeof(float));
    mem.ExecuteReadScatter(cache_handle);
    
    view_point.rotation.x = asin(fnrot.c) * (180.0 / M_PI);
    view_point.rotation.y = ((atan2(fnrot.a * -1, fnrot.b) * (180.0 / M_PI)) * -1) * -1;
    view_point.fov = fov * 90.f;
    
        return view_point;
}

Vector2 project_world_to_screen(const Vector3& world_location) {
    cache::local_camera = get_view_point();
    D3DMATRIX temp_matrix = to_matrix(cache::local_camera.rotation);

    Vector3 vaxisx(temp_matrix.m[0][0], temp_matrix.m[0][1], temp_matrix.m[0][2]);
    Vector3 vaxisy(temp_matrix.m[1][0], temp_matrix.m[1][1], temp_matrix.m[1][2]);
    Vector3 vaxisz(temp_matrix.m[2][0], temp_matrix.m[2][1], temp_matrix.m[2][2]);

    Vector3 vdelta = world_location - cache::local_camera.location;
    Vector3 vtransformed(vdelta.dot(vaxisy), vdelta.dot(vaxisz), vdelta.dot(vaxisx));

    if (vtransformed.z < 1) vtransformed.z = 1;

    float screen_center_x = settings::screen_center_x;
    float screen_center_y = settings::screen_center_y;
    float tan_fov = std::tan(cache::local_camera.fov * M_PI / 360);

    return Vector2(
        screen_center_x + vtransformed.x * (screen_center_x / tan_fov) / vtransformed.z,
        screen_center_y - vtransformed.y * (screen_center_x / tan_fov) / vtransformed.z
    );
}

Vector3 get_entity_bone(uintptr_t bone_array, uintptr_t mesh, int bone_id) {

    FTransform bone;
    FTransform component_to_world;
    auto cache_handle = mem.CreateScatterHandle();

    mem.AddScatterReadRequest(cache_handle, bone_array + (bone_id * 0x60), &bone, sizeof(FTransform));
    mem.AddScatterReadRequest(cache_handle, mesh + offsets::COMPONENT_TO_WORLD, &component_to_world, sizeof(FTransform));
    mem.ExecuteReadScatter(cache_handle);

    D3DMATRIX matrix = matrix_multiplication(bone.to_matrix_with_scale(), component_to_world.to_matrix_with_scale());
    return Vector3(matrix._41, matrix._42, matrix._43);
}


bool is_visible(uintptr_t mesh) {
    float last_submit_time = mem.Read<float>(mesh + offsets::LAST_SUMBIT_TIME);
    float last_render_time_on_screen = mem.Read<float>(mesh + offsets::LAST_SUMBIT_TIME_ON_SCREEN);
    return last_render_time_on_screen + 0.06f >= last_submit_time;
}

float custom_fabsf(float x) {

    __m128 x_vec = _mm_set_ss(x);
    x_vec = _mm_and_ps(x_vec, _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF)));
    return _mm_cvtss_f32(x_vec);
}

Vector3 Prediction(Vector3 CurrentLocation, float Distance, Vector3 Velocity, float speed, float gravity) {

    Vector3 CalculatedPosition = CurrentLocation;
    if (!speed) return CalculatedPosition;

    float TimeToTarget = Distance / custom_fabsf(speed);
    float CalculatedBulletDrop = (((-2800) * 0.5f * (TimeToTarget))) * 10;

    CalculatedPosition.x += Velocity.x * (TimeToTarget) * 120;
    CalculatedPosition.y += Velocity.y * (TimeToTarget) * 120;
    CalculatedPosition.z += Velocity.z * (TimeToTarget) * 120;
    CalculatedPosition.z += custom_fabsf((-49000 / 50) * gravity) / 2.0f * (TimeToTarget * 25);

    return CalculatedPosition;
}

void move(float x, float y) {
    Vector2 center(settings::width / 2, settings::height / 2);
    Vector3 target(0, 0, 0);

    if (x != 0) {
        if (x > center.x) {
            target.x = -(center.x - x);
            target.x /= (settings::aimbot::smoothness);
            if (target.x + center.x > center.x * 2)
                target.x = 0;
        }

        if (x < center.x) {
            target.x = x - center.x;
            target.x /= (settings::aimbot::smoothness);
            if (target.x + center.x < 0)
                target.x = 0;
        }
    }
    if (y != 0) {
        if (y > center.y) {
            target.y = -(center.y - y);
            target.y /= (settings::aimbot::smoothness);
            if (target.y + center.y > center.y * 2)
                target.y = 0;
        }

        if (y < center.y) {
            target.y = y - center.y;
            target.y /= (settings::aimbot::smoothness);
            if (target.y + center.y < 0)
                target.y = 0;
        }
    }

    const float snapThreshold = 1.0f;
    if (std::abs(target.x) < snapThreshold) {
        target.x = 0;
    }
    if (std::abs(target.y) < snapThreshold) {
        target.y = 0;
    }
    if (settings::kmbox::kmboxb)
        kmBox::sendMove(target.x, target.y);

    if (settings::kmbox::kmboxnet)
        kmNet_mouse_move(target.x, target.y);
}

bool kmBox::init() {
    std::string port = kmBox::FindPort("USB-SERIAL CH340");
    if (port.empty()) {
        printf("[KMBOX] failed to find port!\n");
        return false;
    }

    if (!kmBox::OpenPort(hSerial, port.c_str(), CBR_115200)) {
        printf("[KMBOX] failed to open port!\n");
        return false;
    }

    printf("[KMBOX] created successfully!\n");
    return true;
}

void kmBox::sendMove(int x, int y) {
    std::stringstream commandStream;
    commandStream << "km.move(" << x << "," << y << ")\r\n";
    SendCommand(hSerial, commandStream.str());
}

void kmBox::kmclick() {
    std::string command = "km.left(" + std::to_string(1) + ")\r\n";
    Sleep(10);
    std::string command1 = "km.left(" + std::to_string(0) + ")\r\n";
    SendCommand(hSerial, command.c_str());
    SendCommand(hSerial, command1.c_str());
}

std::string read_file(const std::string& filename, int line_number) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }

    std::string line;
    int current_line = 0;
    while (std::getline(file, line)) {
        if (++current_line == line_number) {
            return line;
        }
    }
    return "";
}

void Start(const char* url) {
    ShellExecute(0, 0, url, 0, 0, SW_SHOW);
}

bool in_screen(Vector2 screen_position) {
    if (screen_position.x > 0 && screen_position.x < settings::width && screen_position.y > 0 && screen_position.y < settings::height) return true;
    else return false;
}



bool is_shootable(Vector3 lur, Vector3 wl) {
    return (lur.x >= wl.x - 20 && lur.x <= wl.x + 20 && lur.y >= wl.y - 20 && lur.y <= wl.y + 20);
}


void do_aimbot(Vector2 target) {
    if (settings::aimbot::key) {
        if (mem.GetKeyboard()->IsKeyDown(VK_RBUTTON)) {
            if (settings::kmbox::kmboxb) {
                move(target.x, target.y);
            }
            if (settings::kmbox::kmboxnet) {
                move(target.x, target.y);

            }
        }
    }
    else {
        if (settings::kmbox::kmboxb) {
            move(target.x, target.y);
        }
        if (settings::kmbox::kmboxnet) {
            move(target.x, target.y);
        }
    }
}

void do_triggerbot() {
    bool isdown = false;
    if (settings::aimbot::key) {
        if (mem.GetKeyboard()->IsKeyDown(VK_RBUTTON)) {
            if (settings::kmbox::kmboxb) {
                kmBox::kmclick();
            }
            if (settings::kmbox::kmboxnet) {
                kmNet_mouse_left(isdown);
            }
        }
    }
    else {
    if (settings::kmbox::kmboxb) {
            kmBox::kmclick();
        }
        if (settings::kmbox::kmboxnet) {
            kmNet_mouse_left(isdown);
        }
    }
}
