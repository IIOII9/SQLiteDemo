// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QSqlDatabase>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void insertRecord();
    void uploadAttachment();
    void exportAttachment();

private:
    void setupUI();
    void loadRecords();
    bool isValidFile(const QString &filePath);

    QSqlDatabase db;
    QLineEdit *nameEdit;
    QLabel *fileInfoLabel;
    QPushButton *uploadButton;
    QPushButton *insertButton;
    QPushButton *exportButton;
    QTableWidget *table;

    QByteArray fileData;
    QString fileType;
};
#endif // MAINWINDOW_H