#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("example.db");
    if (!db.open()) {
        QMessageBox::critical(this, "Error", "Failed to open database.");
        exit(1);
    }

    QSqlQuery q;
    q.exec("CREATE TABLE IF NOT EXISTS people (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, age INTEGER)");

    model = new QSqlTableModel(this, db);
    model->setTable("people");
    model->setEditStrategy(QSqlTableModel::OnFieldChange);

    setupUI();
    loadPage();
}

MainWindow::~MainWindow() {}

QString MainWindow::getFilterText() {
    QString filter = searchEdit->text().trimmed();
    return filter.isEmpty() ? "" : QString("name LIKE '%%1%'").arg(filter);
}

void MainWindow::setupUI() {
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Search name...");
    searchButton = new QPushButton("Search");
    connect(searchButton, &QPushButton::clicked, this, &MainWindow::search);
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(searchButton);

    tableView = new QTableView();
    tableView->setModel(model);
    tableView->setSortingEnabled(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);

    QHBoxLayout *controlLayout = new QHBoxLayout();
    prevButton = new QPushButton("Previous");
    nextButton = new QPushButton("Next");
    addButton = new QPushButton("Add");
    exportButton = new QPushButton("Export CSV");
    deleteButton = new QPushButton("Delete");
    pageSizeSpin = new QSpinBox();
    pageSizeSpin->setRange(1, 100);
    pageSizeSpin->setValue(pageSize);
    pageLabel = new QLabel();

    sortFieldCombo = new QComboBox();
    sortFieldCombo->addItems({"id", "name", "age"});
    sortOrderCombo = new QComboBox();
    sortOrderCombo->addItems({"Ascending", "Descending"});
    connect(sortFieldCombo, &QComboBox::currentTextChanged, this, &MainWindow::loadPage);
    connect(sortOrderCombo, &QComboBox::currentTextChanged, this, &MainWindow::loadPage);

    jumpPageSpin = new QSpinBox();
    jumpPageSpin->setRange(1, 999);
    jumpPageButton = new QPushButton("Go");
    connect(jumpPageButton, &QPushButton::clicked, [=]() {
        int totalPages = (getTotalRowCount() + pageSize - 1) / pageSize;
        int targetPage = jumpPageSpin->value();
        if (targetPage < 1 || targetPage > totalPages) {
            QMessageBox::warning(this, "Invalid Page", QString("Please enter a page number between 1 and %1.").arg(totalPages));
            return;
        }
        currentPage = targetPage - 1;
        loadPage();
    });

    connect(prevButton, &QPushButton::clicked, this, &MainWindow::prevPage);
    connect(nextButton, &QPushButton::clicked, this, &MainWindow::nextPage);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addEntry);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteEntry);
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::exportCSV);
    connect(pageSizeSpin, qOverload<int>(&QSpinBox::valueChanged), [=](int val) {
        pageSize = val;
        currentPage = 0;
        loadPage();
    });

    controlLayout->addWidget(prevButton);
    controlLayout->addWidget(nextButton);
    controlLayout->addWidget(new QLabel("Page size:"));
    controlLayout->addWidget(pageSizeSpin);
    controlLayout->addWidget(pageLabel);
    controlLayout->addStretch();
    controlLayout->addWidget(new QLabel("Sort by:"));
    controlLayout->addWidget(sortFieldCombo);
    controlLayout->addWidget(sortOrderCombo);
    controlLayout->addSpacing(20);
    controlLayout->addWidget(new QLabel("Jump to page:"));
    controlLayout->addWidget(jumpPageSpin);
    controlLayout->addWidget(jumpPageButton);
    controlLayout->addStretch();
    controlLayout->addWidget(addButton);
    controlLayout->addWidget(deleteButton);
    controlLayout->addWidget(exportButton);

    mainLayout->addLayout(searchLayout);
    mainLayout->addWidget(tableView);
    mainLayout->addLayout(controlLayout);

    setCentralWidget(central);
    resize(1000, 600);
}

int MainWindow::getTotalRowCount() {
    QSqlQuery q("SELECT COUNT(*) FROM people");
    if (q.next()) return q.value(0).toInt();
    return 0;
}

void MainWindow::loadPage() {
    if (!db.isOpen()) return;

    QString filter = getFilterText();
    model->setFilter(filter);

    int column = sortFieldCombo->currentIndex();
    Qt::SortOrder order = (sortOrderCombo->currentIndex() == 0) ? Qt::AscendingOrder : Qt::DescendingOrder;
    model->setSort(column, order);
    model->select();

    int totalRows = model->rowCount();
    int totalPages = (totalRows + pageSize - 1) / pageSize;
    int offset = currentPage * pageSize;

    for (int row = 0; row < totalRows; ++row) {
        tableView->setRowHidden(row, !(row >= offset && row < offset + pageSize));
    }

    pageLabel->setText(QString("Page %1 / %2").arg(currentPage + 1).arg(qMax(1, totalPages)));
    jumpPageSpin->setMaximum(qMax(1, totalPages));
    tableView->resizeColumnsToContents();
}

void MainWindow::search() {
    currentPage = 0;
    loadPage();
}

void MainWindow::nextPage() {
    if ((currentPage + 1) * pageSize >= model->rowCount()) return;
    ++currentPage;
    loadPage();
}

void MainWindow::prevPage() {
    if (currentPage == 0) return;
    --currentPage;
    loadPage();
}

void MainWindow::addEntry() {
    QSqlQuery q;
    q.prepare("INSERT INTO people (name, age) VALUES (?, ?)");
    q.addBindValue("User " + QString::number(rand() % 1000));
    q.addBindValue(20 + rand() % 40);
    if (!q.exec()) {
        QMessageBox::warning(this, "Error", "Failed to insert data.");
    }
    loadPage();
}

void MainWindow::deleteEntry() {
    QModelIndex index = tableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Delete", "Please select a row to delete.");
        return;
    }
    model->removeRow(index.row());
    model->submitAll();
    loadPage();
}

void MainWindow::exportCSV() {
    QString fileName = QFileDialog::getSaveFileName(this, "Export CSV", "", "CSV files (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot open file.");
        return;
    }

    QTextStream out(&file);
    QSqlRecord rec = model->record();

    for (int i = 0; i < rec.count(); ++i) {
        out << rec.fieldName(i);
        if (i < rec.count() - 1) out << ",";
    }
    out << "\n";

    for (int row = 0; row < model->rowCount(); ++row) {
        for (int col = 0; col < rec.count(); ++col) {
            out << model->data(model->index(row, col)).toString();
            if (col < rec.count() - 1) out << ",";
        }
        out << "\n";
    }

    file.close();
    QMessageBox::information(this, "Done", "Exported successfully.");
}
