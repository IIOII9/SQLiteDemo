#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QTableView>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
private:
    QSqlDatabase db;
    QSqlQueryModel *model;
    QTableView *tableView;
    QLineEdit *searchEdit;
    QPushButton *searchButton;
    QPushButton *prevButton;
    QPushButton *nextButton;
    QPushButton *addButton;
    QPushButton *deleteButton;
    QPushButton *exportButton;
    QSpinBox *pageSizeSpin;
    QSpinBox *jumpPageSpin;
    QPushButton *jumpPageButton;
    QComboBox *sortFieldCombo;
    QComboBox *sortOrderCombo;
    QLabel *pageLabel;

    int currentPage = 0;
    int pageSize = 10;

    void setupUI();
    void loadPage();
    void updatePageInfo();
    int getTotalRowCount();
    QString getSortOrder();
    QString getSearchFilter();
    int getPrimaryKeyOfRow(int row);

private slots:
    void search();
    void prevPage();
    void nextPage();
    void addEntry();
    void deleteEntry();
    void exportCSV();
};

#endif // MAINWINDOW_H
