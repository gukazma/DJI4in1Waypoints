#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <boost/filesystem.hpp>
#include <QDesktopServices>
#include <QMessageBox>
#include "GlobalSignal.h"
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
