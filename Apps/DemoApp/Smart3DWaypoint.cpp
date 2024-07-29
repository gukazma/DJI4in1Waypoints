#include "Smart3DWaypoint.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <minizip/zip.h>
#include <iostream>
namespace soarscape {
Setting g_settings;

void   AttachSetting(const Setting& settings_)
{
    g_settings = settings_;
}
boost::property_tree::ptree GenerateTemplate(Smart3DWaypoints& waypoints);
    boost::property_tree::ptree GenerateWaylines(Smart3DWaypoints& waypoints);
void AttachMissionConfig(boost::property_tree::ptree& missionConfig,
                        Smart3DWaypoints&      waypoints);

void AttachWaypoints(boost::property_tree::ptree& document, Smart3DWaypoints& waypoints);
void AttachActionGroup(boost::property_tree::ptree& placemark, int distance, int speed, int id,
                       bool isEnd = false);
void AttachActionGimbalRotate(boost::property_tree::ptree& actionGroup, int pitch, int roll,
                              int yaw, int id);
void AttachActionTakePhoto(boost::property_tree::ptree& actionGroup, int actionId, int pointId);

void StartSmartOblique(boost::property_tree::ptree& actionGroup, int distance, int speeed);
void SmartObliquePoint(boost::property_tree::ptree& actionActuatorFuncParam, int pitch, int roll,
                       int yaw, float staytime);
void StopSmartOblique(boost::property_tree::ptree& actionGroup);

void ExportWaypoints(const boost::filesystem::path& outputPath, Smart3DWaypoints& waypoints) {
    waypoints.speed   = g_settings.speed;
    auto waylines_xml = GenerateWaylines(waypoints);
    auto templatekmz  = GenerateTemplate(waypoints);

    boost::property_tree::xml_writer_settings<std::string> settings(' ', 2);
    std::stringstream                                      template_ss, waylines_ss;
    boost::property_tree::write_xml(waylines_ss, waylines_xml, settings);
    boost::property_tree::write_xml(template_ss, templatekmz, settings);


    const char* zipfilename     = outputPath.string().c_str();
    const char* template_in_zip = "wpmz/template.kml";
    const char* waylines_in_zip = "wpmz/waylines.wpml";

    std::string content_template = template_ss.str();
    std::string content_waylines = waylines_ss.str();

    // 打开 ZIP 文件进行写入
    zipFile zf = zipOpen(zipfilename, APPEND_STATUS_CREATE);
    if (zf == NULL) {
        std::cout << "Could not create zip file '" << zipfilename << "'" << std::endl;
        return;
    }

    // 初始化 ZIP 文件的内部结构
    zip_fileinfo zi;
    memset(&zi, 0, sizeof(zi));

    // 向 ZIP 文件添加一个新文件
    int err = zipOpenNewFileInZip(zf,
                                  template_in_zip,
                                  &zi,
                                  NULL,
                                  0,   // extrafield_local, size_extrafield_local
                                  NULL,
                                  0,           // extrafield_global, size_extrafield_global
                                  "Comment",   // 文件注释
                                  Z_DEFLATED,
                                  Z_DEFAULT_COMPRESSION);
    if (err != ZIP_OK) {
        std::cout << "Could not add file to zip '" << template_in_zip << "'" << std::endl;
        zipClose(zf, NULL);
        return;
    }

    // 将 std::string 的内容写入 ZIP
    err = zipWriteInFileInZip(zf, content_template.c_str(), content_template.size());
    if (err < 0) {
        std::cout << "Error writing string in zip" << std::endl;
        zipCloseFileInZip(zf);
        zipClose(zf, "Closing comment");
        return;
    }

    // 向 ZIP 文件添加一个新文件
    err = zipOpenNewFileInZip(zf,
                              waylines_in_zip,
                                  &zi,
                                  NULL,
                                  0,   // extrafield_local, size_extrafield_local
                                  NULL,
                                  0,           // extrafield_global, size_extrafield_global
                                  "Comment",   // 文件注释
                                  Z_DEFLATED,
                                  Z_DEFAULT_COMPRESSION);
    if (err != ZIP_OK) {
        std::cout << "Could not add file to zip '" << waylines_in_zip << "'" << std::endl;
        zipClose(zf, NULL);
        return;
    }

    // 将 std::string 的内容写入 ZIP
    err = zipWriteInFileInZip(zf, content_waylines.c_str(), content_waylines.size());
    if (err < 0) {
        std::cout << "Error writing string in zip" << std::endl;
        zipCloseFileInZip(zf);
        zipClose(zf, "Closing comment");
        return;
    }

    // 关闭文件和 ZIP
    zipCloseFileInZip(zf);
    zipClose(zf, "Closing comment");

    std::cout << "Successfully created '" << zipfilename << "' with string content." << std::endl;
    return;
}


boost::property_tree::ptree GenerateTemplate(Smart3DWaypoints& waypoints)
{
    boost::property_tree::ptree  xml;
    boost::property_tree::ptree& kml = xml.add("kml", "");
    kml.put("<xmlattr>.xmlns", "http://www.opengis.net/kml/2.2");
    kml.put("<xmlattr>.xmlns:wpml", "http://www.dji.com/wpmz/1.0.2");
    boost::property_tree::ptree& document      = kml.add("Document", "");
    boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
    // 将当前时间转换为 Unix 时间戳
    boost::posix_time::ptime         epoch(boost::gregorian::date(1970, 1, 1));
    boost::posix_time::time_duration duration       = now - epoch;
    long                             unix_timestamp = duration.total_seconds();
    document.add("wpml:createTime", std::to_string(unix_timestamp));
    document.add("wpml:updateTime", std::to_string(unix_timestamp));

    auto&                        missionConfig = document.add("wpml:missionConfig", "");
    missionConfig.add("wpml:flyToWaylineMode", "safely");
    missionConfig.add("wpml:finishAction", "goHome");
    missionConfig.add("wpml:exitOnRCLost", "executeLostAction");
    missionConfig.add("wpml:executeRCLostAction", "goBack");
    missionConfig.add("wpml:takeOffSecurityHeight", 50);
    missionConfig.add("wpml:globalTransitionalSpeed", waypoints.speed);
    auto& droneInfo   = missionConfig.add("wpml:droneInfo", "");
    auto& payloadInfo = missionConfig.add("wpml:payloadInfo", "");
    droneInfo.add("wpml:droneEnumValue", waypoints.droneEnumValue);
    droneInfo.add("wpml:droneSubEnumValue", waypoints.droneSubEnumValue);
    payloadInfo.add("wpml:payloadEnumValue", waypoints.payloadEnumValue);
    payloadInfo.add("wpml:payloadSubEnumValue", waypoints.droneSubEnumValue);
    payloadInfo.add("wpml:payloadPositionIndex", waypoints.payloadPositionIndex);

    boost::property_tree::ptree& folder = document.add("Folder", "");
    folder.add("wpml:templateType", "mapping2d");
    folder.add("wpml:templateId", 0);
    auto& waylineCoordinateSysParam = folder.add("wpml:waylineCoordinateSysParam", "");
    waylineCoordinateSysParam.add("wpml:coordinateMode", "WGS84");
    waylineCoordinateSysParam.add("wpml:heightMode", "relativeToStartPoint");
    waylineCoordinateSysParam.add("wpml:globalShootHeight", 100);
    folder.add("wpml:autoFlightSpeed", waypoints.speed);
    auto& payloadParam = folder.add("wpml:payloadParam", "");
    payloadParam.add("wpml:payloadPositionIndex", 0);
    payloadParam.add("wpml:dewarpingEnable", 0);
    payloadParam.add("wpml:returnMode", "singleReturnFirst");
    payloadParam.add("wpml:samplingRate", "240000");
    payloadParam.add("wpml:scanningMode", "nonRepetitive");
    payloadParam.add("wpml:modelColoringEnable", "0");
    payloadParam.add("wpml:imageFormat", waypoints.imageFormat);

    return xml;
}

boost::property_tree::ptree GenerateWaylines(Smart3DWaypoints& waypoints)
{
    boost::property_tree::ptree  xml;
    boost::property_tree::ptree& kml = xml.add("kml", "");
    kml.put("<xmlattr>.xmlns", "http://www.opengis.net/kml/2.2");
    kml.put("<xmlattr>.xmlns:wpml", "http://www.dji.com/wpmz/1.0.2");
    boost::property_tree::ptree& document      = kml.add("Document", "");
    auto&                        missionConfig = document.add("wpml:missionConfig", "");
    AttachMissionConfig(missionConfig, waypoints);

    AttachWaypoints(document, waypoints);
    return xml;
}

void AttachMissionConfig(boost::property_tree::ptree& missionConfig,
                        Smart3DWaypoints&      waypoints)
{
    missionConfig.add("wpml:flyToWaylineMode", "safely");
    missionConfig.add("wpml:finishAction", "goHome");
    missionConfig.add("wpml:exitOnRCLost", "executeLostAction");
    missionConfig.add("wpml:executeRCLostAction", "goBack");
    missionConfig.add("wpml:takeOffSecurityHeight", 50);
    missionConfig.add("wpml:globalTransitionalSpeed", 8);
    auto& droneInfo = missionConfig.add("wpml:droneInfo", "");
    auto& payloadInfo = missionConfig.add("wpml:payloadInfo", "");
    droneInfo.add("wpml:droneEnumValue", waypoints.droneEnumValue);
    droneInfo.add("wpml:droneSubEnumValue", waypoints.droneSubEnumValue);
    payloadInfo.add("wpml:payloadEnumValue", waypoints.payloadEnumValue);
    payloadInfo.add("wpml:payloadSubEnumValue", waypoints.droneSubEnumValue);
    payloadInfo.add("wpml:payloadPositionIndex", waypoints.payloadPositionIndex);
}

void AttachWaypoints(boost::property_tree::ptree& document, Smart3DWaypoints& waypoints)
{
    boost::property_tree::ptree& folder = document.add("Folder", "");
    folder.add("wpml:templateId", 0);
    folder.add("wpml:executeHeightMode", "relativeToStartPoint");
    folder.add("wpml:waylineId", 0);
    folder.add("wpml:autoFlightSpeed", waypoints.speed);
    // first
    {
        auto&      placemark = folder.add("Placemark", "");
        auto&      point     = placemark.add("Point", "");
        Vec3 pos       = waypoints.positions[0];
        std::string coord = std::to_string(pos[0]) + "," + std::to_string(pos[1]);
        point.add("coordinates", coord);
        placemark.add("wpml:index", 0);
        placemark.add("wpml:executeHeight", (int)pos[2]);
        placemark.add("wpml:waypointSpeed", waypoints.speed);
        auto& waypointHeadingParam = placemark.add("wpml:waypointHeadingParam", "");
        waypointHeadingParam.add("wpml:waypointHeadingMode", "followWayline");
        waypointHeadingParam.add("wpml:waypointHeadingAngleEnable", 1);
        waypointHeadingParam.add("wpml:waypointHeadingPathMode", "followBadArc");
        auto& waypointTurnParam = placemark.add("wpml:waypointTurnParam", "");
        waypointTurnParam.add("wpml:waypointTurnMode", "toPointAndStopWithDiscontinuityCurvature");
        waypointTurnParam.add("wpml:waypointTurnDampingDist", 0);
        placemark.add("wpml:useStraightLine", 1);
    }
    int num = 0;
    while (num < waypoints.positions.size() && (num + 1) < waypoints.positions.size()) {
        
        int start  = num;
        int end   = num +1;

        // start
        {
            auto& placemark                  = folder.add("Placemark", "");
            auto& waypointGimbalHeadingParam = placemark.add("wpml:waypointGimbalHeadingParam", "");
            waypointGimbalHeadingParam.add("wpml:waypointGimbalPitchAngle", 0);
            waypointGimbalHeadingParam.add("wpml:waypointGimbalYawAngle", 0);
            placemark.add("wpml:isRisky", 0);
            auto&      point     = placemark.add("Point", "");
            auto pos   = waypoints.positions[start];
            auto startPos  = waypoints.positions[start];
            auto endPos  = waypoints.positions[end];

            
            std::string coord = std::to_string(pos[0]) + "," + std::to_string(pos[1]);
            point.add("coordinates", coord);
            placemark.add("wpml:index", (start + 1));
            placemark.add("wpml:executeHeight", (int)pos[2]);
            placemark.add("wpml:waypointSpeed", waypoints.speed);
            auto& waypointHeadingParam = placemark.add("wpml:waypointHeadingParam", "");
            waypointHeadingParam.add("wpml:waypointHeadingMode", "followWayline");
            waypointHeadingParam.add("wpml:waypointHeadingAngleEnable", 1);
            waypointHeadingParam.add("wpml:waypointHeadingPathMode", "followBadArc");
            auto& waypointTurnParam = placemark.add("wpml:waypointTurnParam", "");
            waypointTurnParam.add("wpml:waypointTurnMode",
                                  "toPointAndStopWithDiscontinuityCurvature");
            waypointTurnParam.add("wpml:waypointTurnDampingDist", 0);
            placemark.add("wpml:useStraightLine", 1);
            auto& actionGroup = placemark.add("wpml:actionGroup", "");
            actionGroup.add("wpml:actionGroupId", (start+1));
            actionGroup.add("wpml:actionGroupStartIndex", (start+1));
            actionGroup.add("wpml:actionGroupEndIndex", (start+2));
            actionGroup.add("wpml:actionGroupMode", "sequence");
            auto& actionTrigger = actionGroup.add("wpml:actionTrigger", "");
            actionTrigger.add("wpml:actionTriggerType", "betweenAdjacentPoints");

            auto& action = actionGroup.add("wpml:action", "");
            action.add("wpml:actionId", 0);
            action.add("wpml:actionActuatorFunc", "startSmartOblique");
            auto& actionActuatorFuncParam = action.add("wpml:actionActuatorFuncParam", "");
            actionActuatorFuncParam.add("wpml:smartObliqueCycleMode", "unlimited");
            actionActuatorFuncParam.add("wpml:payloadPositionIndex", 0);
            float staytime = g_settings.distance /6.0 /  g_settings.speed;
            SmartObliquePoint(actionActuatorFuncParam,
                              g_settings.sixVS[0][0],
                              g_settings.sixVS[0][1],
                              0,
                              staytime);
            SmartObliquePoint(actionActuatorFuncParam,
                              g_settings.sixVS[1][0],
                              g_settings.sixVS[1][1],
                              0,
                              staytime);
            SmartObliquePoint(actionActuatorFuncParam,
                              g_settings.sixVS[2][0],
                              g_settings.sixVS[2][1],
                              0,
                              staytime);
            SmartObliquePoint(actionActuatorFuncParam,
                              g_settings.sixVS[3][0],
                              g_settings.sixVS[3][1],
                              0,
                              staytime);
            SmartObliquePoint(actionActuatorFuncParam,
                              g_settings.sixVS[4][0],
                              g_settings.sixVS[4][1],
                              0,
                              staytime);
            SmartObliquePoint(actionActuatorFuncParam,
                              g_settings.sixVS[5][0],
                              g_settings.sixVS[5][1],
                              0,
                              staytime);
            placemark.add("wpml:isRisky", 0);
        }

        num += 2;
    }
    // last point
    {
        auto&      placemark = folder.add("Placemark", "");
        auto&      point     = placemark.add("Point", "");
        auto pos       = waypoints.positions.back();
        std::string coord = std::to_string(pos[0]) + "," + std::to_string(pos[1]);
        point.add("coordinates", coord);
        placemark.add("wpml:index", waypoints.positions.size());
        placemark.add("wpml:executeHeight", (int)pos[2]);
        placemark.add("wpml:waypointSpeed", waypoints.speed);
        auto& waypointHeadingParam = placemark.add("wpml:waypointHeadingParam", "");
        waypointHeadingParam.add("wpml:waypointHeadingMode", "followWayline");
        waypointHeadingParam.add("wpml:waypointHeadingAngleEnable", 1);
        waypointHeadingParam.add("wpml:waypointHeadingPathMode", "followBadArc");
        auto& waypointTurnParam = placemark.add("wpml:waypointTurnParam", "");
        waypointTurnParam.add("wpml:waypointTurnMode", "toPointAndStopWithDiscontinuityCurvature");
        waypointTurnParam.add("wpml:waypointTurnDampingDist", 0);
        placemark.add("wpml:useStraightLine", 1);
        auto& actionGroup = placemark.add("wpml:actionGroup", "");
        actionGroup.add("wpml:actionGroupId", waypoints.positions.size());
        actionGroup.add("wpml:actionGroupMode", "sequence");
        auto& actionTrigger = actionGroup.add("wpml:actionTrigger", "");
        actionTrigger.add("wpml:actionTriggerType", "reachPoint");
        auto& action = actionGroup.add("wpml:action", "");
        action.add("wpml:actionId", 0);
        action.add("wpml:actionActuatorFunc", "goBack");
    }
}


void AttachActionGroup(boost::property_tree::ptree& placemark, int distance, int speed, int id, bool isEnd) {
    if (!isEnd && id%2) {
        return;
    }
    auto& actionGroup = placemark.add("wpml:actionGroup", "");
    actionGroup.add("wpml:actionGroupId", id);
    if (isEnd) {
        actionGroup.add("wpml:actionGroupStartIndex", id);
        actionGroup.add("wpml:actionGroupEndIndex", id);
    }
    else {
        actionGroup.add("wpml:actionGroupStartIndex", id);
        actionGroup.add("wpml:actionGroupEndIndex", id + 1);
    }
    actionGroup.add("wpml:actionGroupMode", "sequence");
    auto& actionTrigger = actionGroup.add("wpml:actionTrigger", "");
    actionTrigger.add("wpml:actionTriggerType", "betweenAdjacentPoints");
    

     if (isEnd) {
        StopSmartOblique(actionGroup);
    }
    else {
        StartSmartOblique(actionGroup, distance, speed);
    }
    /*AttachActionGimbalRotate(actionGroup, -15, 0, 0, 0);
    AttachActionTakePhoto(actionGroup, 1, id);
    AttachActionGimbalRotate(actionGroup, -15, 40, 0, 2);
    AttachActionTakePhoto(actionGroup, 3, id);

    AttachActionGimbalRotate(actionGroup, -40, 40, 0, 4);
    AttachActionTakePhoto(actionGroup, 5, id);

    AttachActionGimbalRotate(actionGroup, -40, 0, 0, 6);
    AttachActionTakePhoto(actionGroup, 7, id);

    AttachActionGimbalRotate(actionGroup, -40, -40, 0, 8);
    AttachActionTakePhoto(actionGroup, 9, id);

    AttachActionGimbalRotate(actionGroup, -15, -40, 0, 10);
    AttachActionTakePhoto(actionGroup, 11, id);*/
}
void AttachActionGimbalRotate(boost::property_tree::ptree& actionGroup, int pitch, int roll,
    int yaw, int id)
{
    auto& action = actionGroup.add("wpml:action", "");
    action.add("wpml:actionId", id);
    action.add("wpml:actionActuatorFunc", "gimbalRotate");
    auto& actionActuatorFuncParam = action.add("wpml:actionActuatorFuncParam", "");
    actionActuatorFuncParam.add("wpml:gimbalRotateMode", "absoluteAngle");
    actionActuatorFuncParam.add("wpml:gimbalPitchRotateEnable", 1);
    actionActuatorFuncParam.add("wpml:gimbalPitchRotateAngle", pitch);
    actionActuatorFuncParam.add("wpml:gimbalRollRotateEnable", 1);
    actionActuatorFuncParam.add("wpml:gimbalRollRotateAngle", roll);
    actionActuatorFuncParam.add("wpml:gimbalYawRotateEnable", 1);
    actionActuatorFuncParam.add("wpml:gimbalYawRotateAngle", yaw);
    actionActuatorFuncParam.add("wpml:gimbalRotateTimeEnable", 1);
    actionActuatorFuncParam.add("wpml:gimbalRotateTime", "0.1");
    actionActuatorFuncParam.add("wpml:payloadPositionIndex", 0);
}

void AttachActionTakePhoto(boost::property_tree::ptree& actionGroup, int actionId, int pointId)
{
    auto& action = actionGroup.add("wpml:action", "");
    action.add("wpml:actionId", actionId);
    action.add("wpml:actionActuatorFunc", "takePhoto");
    auto& actionActuatorFuncParam = action.add("wpml:actionActuatorFuncParam", "");
    /*std::string fileSuffix = "action_" + std::to_string(actionId) + "_pos_" + std::to_string(pointId);
    actionActuatorFuncParam.add("wpml:fileSuffix", fileSuffix);*/
    actionActuatorFuncParam.add("wpml:payloadPositionIndex", 0);
    actionActuatorFuncParam.add("wpml:payloadLensIndex", "visable");
}

void StartSmartOblique(boost::property_tree::ptree& actionGroup, int distance, int speeed)
{
    auto& action = actionGroup.add("wpml:action", "");
    action.add("wpml:actionId", 0);
    action.add("wpml:actionActuatorFunc", "startSmartOblique");
    auto& actionActuatorFuncParam = action.add("wpml:actionActuatorFuncParam", "");
    actionActuatorFuncParam.add("wpml:smartObliqueCycleMode", "unlimited");
    actionActuatorFuncParam.add("wpml:payloadPositionIndex", 0);
    float staytime = (float)(distance) / (float)speeed / 6.0;
    SmartObliquePoint(actionActuatorFuncParam,
                      g_settings.sixVS[0][0],
                      g_settings.sixVS[0][1],
                      g_settings.sixVS[0][2],
                      staytime);
    SmartObliquePoint(actionActuatorFuncParam,
                      g_settings.sixVS[1][0],
                      g_settings.sixVS[1][1],
                      g_settings.sixVS[1][2],
                      staytime);
    SmartObliquePoint(actionActuatorFuncParam,
                      g_settings.sixVS[2][0],
                      g_settings.sixVS[2][1],
                      g_settings.sixVS[2][2],
                      staytime);
    SmartObliquePoint(actionActuatorFuncParam,
                      g_settings.sixVS[3][0],
                      g_settings.sixVS[3][1],
                      g_settings.sixVS[3][2],
                      staytime);
    SmartObliquePoint(actionActuatorFuncParam,
                      g_settings.sixVS[4][0],
                      g_settings.sixVS[4][1],
                      g_settings.sixVS[4][2],
                      staytime);
    SmartObliquePoint(actionActuatorFuncParam,
                      g_settings.sixVS[5][0],
                      g_settings.sixVS[5][1],
                      g_settings.sixVS[5][2],
                      staytime);
}

void StopSmartOblique(boost::property_tree::ptree& actionGroup)
{
    auto& action = actionGroup.add("wpml:action", "");
    action.add("wpml:actionId", 0);
    action.add("wpml:actionActuatorFunc", "stopSmartOblique");
    auto& actionActuatorFuncParam = action.add("wpml:actionActuatorFuncParam", "");
    actionActuatorFuncParam.add("wpml:payloadPositionIndex", 0);
}


void SmartObliquePoint(boost::property_tree::ptree& actionActuatorFuncParam, int pitch, int roll, int yaw, float staytime) {
    auto& smartObliquePoint = actionActuatorFuncParam.add("wpml:smartObliquePoint", "");
    smartObliquePoint.add("wpml:smartObliqueRunningTime", std::to_string(staytime));
    smartObliquePoint.add("wpml:smartObliqueStayTime", std::to_string(0.1));
    smartObliquePoint.add("wpml:smartObliqueEulerPitch", std::to_string(pitch));
    smartObliquePoint.add("wpml:smartObliqueEulerRoll", std::to_string(roll));
    smartObliquePoint.add("wpml:smartObliqueEulerYaw", std::to_string(yaw));
}

}   // namespace soarscape
