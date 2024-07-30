#include "MainWindow.h"
#include <QApplication>
#include <QSplashScreen>
#include <QTranslator>
int main(int argc, char** argv)
{
    QApplication  app(argc, argv);

    QTranslator* translator = new QTranslator;
    translator->load(":/Translations/zh_CN.qm");
    app.installTranslator(translator);

    MainWindow mainwindow;
    mainwindow.showMaximized();

    return app.exec();
}
