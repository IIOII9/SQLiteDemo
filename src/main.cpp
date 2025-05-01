#include <QApplication>
#include "mainwindow.h"
#include "ui_strings.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    UIStrings::LoadLanguage("locales/zh.json");
    MainWindow w;
    w.setWindowTitle("Qt + SQLite Example");
    w.resize(600, 400);
    w.show();
    return a.exec();
}