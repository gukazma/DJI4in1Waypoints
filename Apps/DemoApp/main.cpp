#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

struct Point {
    int id;
    double latitude;
    double longitude;
    int elevation;
};

std::vector<Point> parseFile(const std::string& filename) {
    std::vector<Point> points;

    std::ifstream file(filename);
    if (!file) {
        std::cout << "Failed to open file: " << filename << std::endl;
        return points;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }

        if (tokens.size() == 4) {
            Point point;
            point.id = std::stoi(tokens[0]);
            point.latitude = std::stod(tokens[1]);
            point.longitude = std::stod(tokens[2]);
            point.elevation = std::stoi(tokens[3]);
            points.push_back(point);
        }
    }

    file.close();

    return points;
}

int main() {
    std::string filename = "C:/mydatas/航线规划/多边形_多边形_2 .txt";
    std::vector<Point> points = parseFile(filename);

    // 打印解析的数据
    for (const auto& point : points) {
        std::cout << "ID: " << point.id << ", Latitude: " << point.latitude
            << ", Longitude: " << point.longitude << ", Elevation: " << point.elevation << std::endl;
    }

    return 0;
}