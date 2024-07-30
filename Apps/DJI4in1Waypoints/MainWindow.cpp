#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <boost/filesystem.hpp>
#include <QDesktopServices>
#include <QMessageBox>
#include "GlobalSignal.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "Smart3DWaypoint.h"
struct Point {
    int id;
    double latitude;
    double longitude;
    int elevation;
};

std::vector<Point> parseTxtFile(const std::string& filename) {
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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->toolButton, &QPushButton::clicked, [&]() {
        auto result = QFileDialog::getOpenFileName(this, tr("Open input waypoints txt file"), "", "*.txt");
        if (!result.isEmpty())
        {
            ui->lineEdit->setText(result);
        }
    });

    connect(ui->toolButton_2, &QPushButton::clicked, [&]() {
        auto result = QFileDialog::getSaveFileName(this, tr("Open output waypoints kmz file"), "", "*.kmz");
        if (!result.isEmpty())
        {
            ui->lineEdit_2->setText(result);
        }
    });
    connect(ui->pushButton_2, &QPushButton::clicked, [&]() {
        if (ui->lineEdit_2->text().isEmpty())
        {
            return;
        }
        boost::filesystem::path outputDir = ui->lineEdit_2->text().toLocal8Bit().toStdString();
        QUrl _url = QUrl::fromLocalFile(QString::fromLocal8Bit(outputDir.parent_path().string().c_str()));
        QDesktopServices::openUrl(_url);
    });
    connect(&g_globalSignal, &GLobalSignal::signal_log, [&](const QString& text) {
        ui->plainTextEdit->appendPlainText(text);
    });

    connect(ui->pushButton, &QPushButton::clicked, [&]() {
        if (ui->lineEdit->text().isEmpty() || ui->lineEdit_2->text().isEmpty())
        {
            QMessageBox::critical(this, "error", tr("Check input or output path!"));
            return;
        }
        auto points = parseTxtFile(ui->lineEdit->text().toLocal8Bit().constData());
        soarscape::Smart3DWaypoints smart3DWaypoints;
        for (size_t i = 0; i < points.size(); i++)
        {
            auto pos = points[i];
            soarscape::Vec3 vec3 = { pos.latitude, pos.longitude, pos.elevation };
            smart3DWaypoints.positions.push_back(vec3);
        }

        soarscape::ExportWaypoints(ui->lineEdit_2->text().toLocal8Bit().constData(), smart3DWaypoints);
        });
    //LOG("asdasd");
   /* connect(ui->pushButton, &QPushButton::clicked, [&]() { 
        ui->label->setText("2");
    });
    connect(ui->pushButton_2, &QPushButton::clicked, [&]() { ui->label->setText("4"); });*/
}

MainWindow::~MainWindow()
{
    delete ui;
}
