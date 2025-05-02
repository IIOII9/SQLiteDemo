#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QTableView>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void search();
    void nextPage();
    void prevPage();
    void addEntry();
    void exportCSV();
    void deleteEntry();

private:
    void setupUI();
    void loadPage();
    int getTotalRowCount();
    QString getFilterText();

    QSqlDatabase db;
    QSqlTableModel *model;
    QTableView *tableView;
    QLineEdit *searchEdit;
    QPushButton *searchButton;
    QPushButton *nextButton;
    QPushButton *prevButton;
    QPushButton *addButton;
    QPushButton *exportButton;
    QPushButton *deleteButton;
    QLabel *pageLabel;
    QSpinBox *pageSizeSpin;
    QComboBox *sortFieldCombo;
    QComboBox *sortOrderCombo;
    QSpinBox *jumpPageSpin;
    QPushButton *jumpPageButton;

    int currentPage = 0;
    int pageSize = 10;
};

#endif // MAINWINDOW_H