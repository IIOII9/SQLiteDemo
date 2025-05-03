#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QtSql>
#include <QTableView>

class MainWindow: public QMainWindow{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    void setupUI();   
    void initDB(); 
    void loadPage();
    QString getSearchFillter();
    QString getSortOrder();
    void updatePageInfo();
    int getTotalRowCount();
    int getPrimaryKeyOfRow(int row);
private:
    QLineEdit* searchEdit;
    QPushButton* searchButton;
    QTableView* tableView;
    QPushButton* prevButton;
    QPushButton* nextButton;
    QSpinBox* pageSizeSpin;
    QLabel* pageLabel;
    QSpinBox* jumpPageSpin;
    QPushButton* jumpPageButton;
    QComboBox* sortFieldCombo;
    QComboBox* sortOrderCombo;
    QPushButton* addButton;
    QPushButton* deleteButton;
    QPushButton* exportButton;

private:
    QSqlDatabase db;
    QSqlQueryModel* model;
    int currentPage = 0;
    int pageSize = 10;
private slots:
    void onSearchClick();
    void onPrePageClick();
    void onNextPageClick();
    void onAddClick();
    void onDeleteClick();
    void onExportCVSClick();
    void onJumpPageButtonClick();
};
#endif