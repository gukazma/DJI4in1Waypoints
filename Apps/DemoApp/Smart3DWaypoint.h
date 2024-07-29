#pragma once
#include <vector>
#include <string>
#include <boost/filesystem/path.hpp>
#include <array>
namespace soarscape
{
typedef std::array<double, 3> Vec3;
struct Smart3DWaypoints
{
    /*
        飞行器机型主类型：
      *  89（机型：M350 RTK）,
      *  60（机型：M300 RTK）,
      *  67（机型：M30 / M30T）,
      *  77（机型：M3E / M3T / M3M）,
      *  91（机型：M3D / M3TD）
     */
    int droneEnumValue = 77;
    /*
        飞行器机型子类型：
        当“飞行器机型主类型”为“67（机型：M30/M30T）”时：
        0（机型：M30双光）,
        1（机型：M30T三光）
        当“飞行器机型主类型“为”77（机型：M3E/M3T/M3M）“时：
        0（机型：M3E）
        1（机型：M3T）
        2（机型：M3M）
        当“飞行器机型主类型“为”91（机型：M3D/M3TD）“时：
        0（机型：M3D）
        1（机型：M3TD）
     */
    int droneSubEnumValue = 0;

    /*
        负载机型主类型：
        42（H20）,
        43（H20T）,
        52（M30双光相机）,
        53（M30T三光相机）,
        61（H20N）,
        66（Mavic 3E 相机）
        67（Mavic 3T 相机）
        68（Mavic 3M 相机）
        80（Matrice 3D 相机）
        81（Matrice 3TD 相机）
        65534（PSDK 负载）
     */
    int payloadEnumValue  = 66;

    int payloadSubEnumValue = 0;

    /*
        负载挂载位置：
        0：飞行器1号挂载位置。M300 RTK，M350 RTK机型，对应机身左前方。其它机型，对应主云台。
        1：飞行器2号挂载位置。M300 RTK，M350 RTK机型，对应机身右前方。
        2：飞行器3号挂载位置。M300 RTK，M350 RTK机型，对应机身上方。
     */
    int payloadPositionIndex = 0;

    std::string imageFormat = "visable";

    std::vector<Vec3> positions;
    std::vector<int> isTakePhoto;

    int speed = 15;
};
struct Setting
{
    typedef std::array<float, 2> Vec2;
    int                          speed    = 15;
    float                        distance = 100.0;
    std::array<Vec2, 6>          sixVS;
};

void ExportWaypoints(const boost::filesystem::path& outputPath, Smart3DWaypoints& waypoints);
void AttachSetting(const Setting& setting_);
}