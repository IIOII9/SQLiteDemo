#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QKeyEvent>
#include <QDebug>

#include "editablemodel.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("example.db");
    if (!db.open()) {
        QMessageBox::critical(this, "Error", "Failed to open database.");
        exit(1);
    }

    QSqlQuery q;
    q.exec("CREATE TABLE IF NOT EXISTS people (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, age INTEGER)");

    EditableSqlModel *editableModel = new EditableSqlModel(this);
    editableModel->setTableName("people");
    editableModel->setPrimaryKeyColumn("id");
    model = editableModel;

    // model = new QSqlQueryModel(this);
    setupUI();
    loadPage();
}

MainWindow::~MainWindow() {}

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
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setSelectionBehavior(QAbstractItemView::SelectItems);  // 改为选中单元格
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);  // 保持单选
    tableView->setEditTriggers(QAbstractItemView::CurrentChanged |
        QAbstractItemView::SelectedClicked |
        QAbstractItemView::EditKeyPressed);
    tableView->installEventFilter(this);

    QHBoxLayout *controlLayout = new QHBoxLayout();
    prevButton = new QPushButton("Previous");
    nextButton = new QPushButton("Next");
    addButton = new QPushButton("Add");
    deleteButton = new QPushButton("Delete");
    exportButton = new QPushButton("Export CSV");
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
            QMessageBox::warning(this, "Invalid Page", QString("Page must be 1 to %1").arg(totalPages));
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

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == tableView && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            QModelIndex current = tableView->currentIndex();
            if (!current.isValid()) return false;

            int row = current.row();
            int col = current.column();

            // 提交当前编辑项（如果有编辑器）
            tableView->closePersistentEditor(current);

            // 移动到下一个单元格（支持跨行跳转）
            int nextRow = row;
            int nextCol = col + 1;
            if (nextCol >= model->columnCount()) {
                nextCol = 0;
                ++nextRow;
                if (nextRow >= model->rowCount()) {
                    return true;  // 已是最后一个格
                }
            }

            QModelIndex nextIndex = model->index(nextRow, nextCol);
            tableView->setCurrentIndex(nextIndex);
            tableView->edit(nextIndex);  // 自动进入编辑状态

            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}


QString MainWindow::getSearchFilter() {
    QString keyword = searchEdit->text().trimmed();
    return keyword.isEmpty() ? "" : QString("WHERE name LIKE '%%1%'").arg(keyword);
}

QString MainWindow::getSortOrder() {
    QString field = sortFieldCombo->currentText();
    QString order = (sortOrderCombo->currentText() == "Ascending") ? "ASC" : "DESC";
    return QString("ORDER BY %1 %2").arg(field, order);
}

int MainWindow::getTotalRowCount() {
    QString filter = getSearchFilter();
    QSqlQuery q("SELECT COUNT(*) FROM people " + filter);
    if (q.next()) return q.value(0).toInt();
    return 0;
}

void MainWindow::loadPage() {
    int offset = currentPage * pageSize;
    QString sql = QString(
        "SELECT * FROM people %1 %2 LIMIT %3 OFFSET %4")
        .arg(getSearchFilter())
        .arg(getSortOrder())
        .arg(pageSize)
        .arg(offset);
    model->setQuery(sql);
    updatePageInfo();
    tableView->resizeColumnsToContents();
    // 保存每一行的主键 id 到 model 中
    for (int row = 0; row < model->rowCount(); ++row) {
        int id = model->data(model->index(row, 0)).toInt();
        static_cast<EditableSqlModel *>(model)->setPrimaryKeyAtRow(row, id);
    }
}

void MainWindow::updatePageInfo() {
    int totalPages = (getTotalRowCount() + pageSize - 1) / pageSize;
    pageLabel->setText(QString("Page %1 / %2").arg(currentPage + 1).arg(qMax(1, totalPages)));
    jumpPageSpin->setMaximum(qMax(1, totalPages));
}

void MainWindow::search() {
    currentPage = 0;
    loadPage();
}

void MainWindow::prevPage() {
    if (currentPage > 0) {
        --currentPage;
        loadPage();
    }
}

void MainWindow::nextPage() {
    if ((currentPage + 1) * pageSize < getTotalRowCount()) {
        ++currentPage;
        loadPage();
    }
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

int MainWindow::getPrimaryKeyOfRow(int row) {
    QModelIndex index = model->index(row, 0); // assume id is first column
    return model->data(index).toInt();
}

void MainWindow::deleteEntry() {
    QModelIndex index = tableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Delete", "Select a row to delete.");
        return;
    }
    int id = getPrimaryKeyOfRow(index.row());
    QSqlQuery q;
    q.prepare("DELETE FROM people WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec()) {
        QMessageBox::warning(this, "Error", "Failed to delete row.");
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
    for (int i = 0; i < model->columnCount(); ++i) {
        out << model->headerData(i, Qt::Horizontal).toString();
        if (i < model->columnCount() - 1) out << ",";
    }
    out << "\n";

    for (int row = 0; row < model->rowCount(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            out << model->data(model->index(row, col)).toString();
            if (col < model->columnCount() - 1) out << ",";
        }
        out << "\n";
    }

    file.close();
    QMessageBox::information(this, "Done", "Exported successfully.");
}
