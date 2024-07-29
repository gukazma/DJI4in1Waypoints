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
        ���������������ͣ�
      *  89�����ͣ�M350 RTK��,
      *  60�����ͣ�M300 RTK��,
      *  67�����ͣ�M30 / M30T��,
      *  77�����ͣ�M3E / M3T / M3M��,
      *  91�����ͣ�M3D / M3TD��
     */
    int droneEnumValue = 77;
    /*
        ���������������ͣ�
        �������������������͡�Ϊ��67�����ͣ�M30/M30T����ʱ��
        0�����ͣ�M30˫�⣩,
        1�����ͣ�M30T���⣩
        �������������������͡�Ϊ��77�����ͣ�M3E/M3T/M3M����ʱ��
        0�����ͣ�M3E��
        1�����ͣ�M3T��
        2�����ͣ�M3M��
        �������������������͡�Ϊ��91�����ͣ�M3D/M3TD����ʱ��
        0�����ͣ�M3D��
        1�����ͣ�M3TD��
     */
    int droneSubEnumValue = 0;

    /*
        ���ػ��������ͣ�
        42��H20��,
        43��H20T��,
        52��M30˫�������,
        53��M30T���������,
        61��H20N��,
        66��Mavic 3E �����
        67��Mavic 3T �����
        68��Mavic 3M �����
        80��Matrice 3D �����
        81��Matrice 3TD �����
        65534��PSDK ���أ�
     */
    int payloadEnumValue  = 66;

    int payloadSubEnumValue = 0;

    /*
        ���ع���λ�ã�
        0��������1�Ź���λ�á�M300 RTK��M350 RTK���ͣ���Ӧ������ǰ�����������ͣ���Ӧ����̨��
        1��������2�Ź���λ�á�M300 RTK��M350 RTK���ͣ���Ӧ������ǰ����
        2��������3�Ź���λ�á�M300 RTK��M350 RTK���ͣ���Ӧ�����Ϸ���
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