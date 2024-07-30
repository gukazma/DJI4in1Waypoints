#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
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
   /* connect(ui->pushButton, &QPushButton::clicked, [&]() { 
        ui->label->setText("2");
    });
    connect(ui->pushButton_2, &QPushButton::clicked, [&]() { ui->label->setText("4"); });*/
}

MainWindow::~MainWindow()
{
    delete ui;
}
