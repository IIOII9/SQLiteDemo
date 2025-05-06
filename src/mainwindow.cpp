#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

#include "editabletableview.h"

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
    
    //tableView = new QTableView();
    tableView = new EditableTableView();
    tableView->setItemDelegate(new QItemDelegate(this));
    tableView->setModel(model);
    mainLaylout->addWidget(tableView);
    tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    
    QHBoxLayout* controllLayout = new QHBoxLayout();
    mainLaylout->addLayout(controllLayout);
    prevButton = new QPushButton("Previous");
    connect(prevButton, &QPushButton::clicked, this, &MainWindow::onPrePageClick);
    nextButton = new QPushButton("Next");
    connect(nextButton, &QPushButton::clicked, this, &MainWindow::onNextPageClick);
    pageSizeSpin = new QSpinBox();
    pageSizeSpin->setRange(1, 100);
    pageSizeSpin->setValue(pageSize);
    connect(pageSizeSpin, qOverload<int>(&QSpinBox::valueChanged), 
        [=](int val){
            pageSize = val;
            currentPage = 0;
            loadPage();
        });
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
    jumpPageSpin->setMinimum(1);
    jumpPageButton = new QPushButton("Go");
    connect(jumpPageButton, &QPushButton::clicked, this, &MainWindow::onJumpPageButtonClick);

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
        " AUTOINCREMENT, name TEXT, age INTEGER, photo	BLOB)";
    query.exec(sqlText);
    //model = new QSqlQueryModel(this);
    model = new EditableQueryModel(this);
    model->setDB(db);
    model->setTableName("people");
    model->setPrimaryKeyColumn("id");
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
    for (int row = 0; row < model->rowCount(); ++row){
        int id = model->data(model->index(row, 0)).toInt();
        model->setPrimaryKeyAtRow(row, id);
    }
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
    int totalPages = (getTotalRowCount() + pageSize - 1) / pageSize;
    pageLabel->setText(QString("Page %1 / %2").arg(currentPage + 1)
        .arg(qMax(1, totalPages)));
    jumpPageSpin->setMaximum(qMax(1, totalPages));
}
void MainWindow::onSearchClick()
{
    currentPage = 0;
    loadPage();
}
void MainWindow::onPrePageClick()
{
    if(currentPage > 0)
    {
      --currentPage;
      loadPage();
    }   
}
void MainWindow::onNextPageClick()
{
    if((currentPage + 1) * pageSize < getTotalRowCount())
    {
        ++currentPage;
        loadPage();
    }
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
    QModelIndex index = tableView->currentIndex();
    if (!index.isValid())
    {
        QMessageBox::warning(this, "Delete", "Select a row to delete.");
        return;
    }
    int id = getPrimaryKeyOfRow(index.row());
    QSqlQuery query;
    query.prepare("DELETE FROM people WHERE id = ?");
    query.addBindValue(id);
    if(!query.exec()){
        QMessageBox::warning(this, "Error", "Failed to delete row." + query.lastError().text());
    }
    loadPage();
}
void MainWindow::onExportCVSClick()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export CVS", "",
        "CSV files (*.csv)");
    if(fileName.isEmpty())
        return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QMessageBox::warning(this, "Error", "Can not open file");
        return;
    }

    QTextStream out(&file);
    for(int i = 0; i < model->columnCount(); ++i){
        out << model->headerData(i, Qt::Horizontal).toString();
        if (i < model->columnCount() - 1)
            out  << ",";
    }
    out << "\n";
    for(int row = 0; row < model->rowCount(); ++row)
    {
        for (int col = 0; col < model->columnCount(); ++col){
            out << model->data(model->index(row, col)).toString();
            if (col < model->columnCount() - 1)
                out << ",";
        }
        out << "\n";
    }
    file.close();
    QMessageBox::information(this, "Exprot CSV", "Exported successfully.");
}

void MainWindow::onJumpPageButtonClick()
{
    currentPage = jumpPageSpin->value() - 1;
    loadPage();
}

int MainWindow::getTotalRowCount()
{
    QString filter = getSearchFillter();
    QSqlQuery query("SELECT COUNT(*) FROM people " + filter);
    if (query.next())
        return query.value(0).toInt();
    else
        QMessageBox::warning(this, "Error", query.lastError().text());   
    return 0;
}

int MainWindow::getPrimaryKeyOfRow(int row)
{
    QModelIndex index = model->index(row, 0);
    return model->data(index).toInt();
}
