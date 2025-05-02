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
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->setSort(0, Qt::AscendingOrder);

    setupUI();
    loadPage();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // Search row
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Search name...");
    searchButton = new QPushButton("Search");
    connect(searchButton, &QPushButton::clicked, this, &MainWindow::search);
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(searchButton);

    // Table view
    tableView = new QTableView();
    tableView->setModel(model);
    tableView->setSortingEnabled(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);

    // Control buttons
    QHBoxLayout *controlLayout = new QHBoxLayout();
    prevButton = new QPushButton("Previous");
    nextButton = new QPushButton("Next");
    addButton = new QPushButton("Add");
    exportButton = new QPushButton("Export CSV");
    submitButton = new QPushButton("Submit");
    pageSizeSpin = new QSpinBox();
    pageSizeSpin->setRange(1, 100);
    pageSizeSpin->setValue(pageSize);
    pageLabel = new QLabel();

    connect(prevButton, &QPushButton::clicked, this, &MainWindow::prevPage);
    connect(nextButton, &QPushButton::clicked, this, &MainWindow::nextPage);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addEntry);
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::exportCSV);
    connect(submitButton, &QPushButton::clicked, [=]() {
        if (!model->submitAll()) {
            QMessageBox::warning(this, "Submit Error", model->lastError().text());
        } else {
            QMessageBox::information(this, "Saved", "Changes saved.");
        }
    });
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
    controlLayout->addWidget(addButton);
    controlLayout->addWidget(submitButton);
    controlLayout->addWidget(exportButton);

    mainLayout->addLayout(searchLayout);
    mainLayout->addWidget(tableView);
    mainLayout->addLayout(controlLayout);

    setCentralWidget(central);
    resize(800, 500);
}

int MainWindow::getTotalRowCount() {
    QString filterText = searchEdit->text().trimmed();
    QString filter = filterText.isEmpty() ? "" : QString("name LIKE '%%1%'").arg(filterText);
    QString queryStr = "SELECT COUNT(*) FROM people";
    if (!filter.isEmpty()) queryStr += " WHERE " + filter;

    QSqlQuery q(queryStr);
    if (q.next()) return q.value(0).toInt();
    return 0;
}

void MainWindow::loadPage() {
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Database Error", "Database is not open.");
        return;
    }

    QString filterText = searchEdit->text().trimmed();
    QString filter = filterText.isEmpty() ? "" : QString("name LIKE '%%1%'").arg(filterText);
    model->setFilter(filter);
    model->select();

    int totalRows = model->rowCount();
    int offset = currentPage * pageSize;

    for (int row = 0; row < totalRows; ++row) {
        bool show = (row >= offset && row < offset + pageSize);
        tableView->setRowHidden(row, !show);
    }

    pageLabel->setText(QString("Page %1").arg(currentPage + 1));
    tableView->resizeColumnsToContents();
}

void MainWindow::search() {
    currentPage = 0;
    loadPage();
}

void MainWindow::nextPage() {
    if ((currentPage + 1) * pageSize >= getTotalRowCount()) return;
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

    // Header
    for (int i = 0; i < rec.count(); ++i) {
        out << rec.fieldName(i);
        if (i < rec.count() - 1) out << ",";
    }
    out << "\n";

    // Data
    for (int row = 0; row < model->rowCount(); ++row) {
        if (tableView->isRowHidden(row)) continue;
        for (int col = 0; col < rec.count(); ++col) {
            out << model->data(model->index(row, col)).toString();
            if (col < rec.count() - 1) out << ",";
        }
        out << "\n";
    }

    file.close();
    QMessageBox::information(this, "Done", "Exported successfully.");
}
