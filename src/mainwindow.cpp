#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent){
    initDB();
    setupUI();
    loadPage();
}

MainWindow::~MainWindow(){
}

void MainWindow::setupUI(){
    QWidget* central = new QWidget(this);
    QVBoxLayout* mainLaylout = new QVBoxLayout(central);
    QHBoxLayout* searchLayout = new QHBoxLayout();
    mainLaylout->addLayout(searchLayout);
    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Search name...");
    searchButton = new QPushButton("Search");
    connect(searchButton, QPushButton::clicked, this, &MainWindow::onSearchClick);
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(searchButton);
    
    tableView = new QTableView();
    tableView->setModel(model);
    mainLaylout->addWidget(tableView);

    QHBoxLayout* controllLayout = new QHBoxLayout();
    mainLaylout->addLayout(controllLayout);
    prevButton = new QPushButton("Previous");
    nextButton = new QPushButton("Next");
    pageSizeSpin = new QSpinBox();
    pageSizeSpin->setRange(1, 100);
    pageSizeSpin->setValue(pageSize);
    pageLabel = new QLabel();   //页码
    sortFieldCombo = new QComboBox();
    sortFieldCombo->addItems({"id", "name", "age"});
    sortOrderCombo = new QComboBox();
    sortOrderCombo->addItems({"Asecending", "Descending"});
    addButton = new QPushButton("Add");
    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddClick);

    deleteButton = new QPushButton("Delete");
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteClick);

    exportButton = new QPushButton("ExportCVS");
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::onExportCVSClick);

    jumpPageSpin = new QSpinBox();
    jumpPageButton = new QPushButton("Go");
    
    controllLayout->addWidget(prevButton);
    controllLayout->addWidget(nextButton);
    controllLayout->addWidget(new QLabel("Page size:"));
    controllLayout->addWidget(pageSizeSpin);
    controllLayout->addWidget(pageLabel);
    controllLayout->addStretch();
    controllLayout->addWidget(new QLabel("Sort by:"));
    controllLayout->addWidget(sortFieldCombo);
    controllLayout->addWidget(sortOrderCombo);
    controllLayout->addSpacing(20);
    controllLayout->addWidget(new QLabel("Jump to page:"));
    controllLayout->addWidget(jumpPageSpin);
    controllLayout->addWidget(jumpPageButton);
    controllLayout->addStretch();
    controllLayout->addWidget(addButton);
    controllLayout->addWidget(deleteButton);
    controllLayout->addWidget(exportButton);

    setCentralWidget(central);
    resize(1000, 600);
}

void MainWindow::initDB()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("example.db");
    if(!db.open()){
        QMessageBox::critical(this, "Error", "Failed to open database.");
        exit(1);
    }
    QSqlQuery query;
     QString sqlText = "CREATE TABLE IF NOT EXISTS people (id INTEGER PRIMARY KEY "
        " AUTOINCREMENT, name TEXT, age INTEGER)";
    query.exec(sqlText);
    model = new QSqlQueryModel(this);
}

void MainWindow::loadPage()
{
    int offset = currentPage * pageSize;
    QString sqlText = QString(
        "SELECT * FROM people %1 %2 LIMIT %3 OFFSET %4")
        .arg(getSearchFillter())
        .arg(getSortOrder())
        .arg(pageSize)
        .arg(offset);
    model->setQuery(sqlText);
    updatePageInfo();
}

QString MainWindow::getSearchFillter()
{
    QString keyword = searchEdit->text().trimmed();
    if (!keyword.isEmpty())
    {
        return QString("WHERE name LIKE '%%1%'").arg(keyword);
    }
    return "";
}

QString MainWindow::getSortOrder()
{
    QString field = sortFieldCombo->currentText().trimmed();
    QString order = (sortOrderCombo->currentIndex() == 0) ? "ASC" : "DESC";
    return QString("ORDER BY %1 %2").arg(field, order);
}

void MainWindow::updatePageInfo()
{

}
void MainWindow::onSearchClick()
{

}
void MainWindow::onPrePageClick()
{

}
void MainWindow::onNextPageClick()
{

}
void MainWindow::onAddClick()
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO people (name, age) VALUES (?, ?)");
    query.addBindValue("User " + QString::number(rand() % 1000));
    query.addBindValue(20 + rand() % 40);
    if (!query.exec()){
        QMessageBox::warning(this, "Error", "Failed to insert data.");
    }
    loadPage();
}
void MainWindow::MainWindow::onDeleteClick()
{

}
void MainWindow::onExportCVSClick()
{

}