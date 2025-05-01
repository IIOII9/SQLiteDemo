// mainwindow.cpp
#include "mainwindow.h"
#include "ui_strings.h"
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QHeaderView>
#include <QFileInfo>
//#include <QDateTime>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUI();

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("example.db");
    if (!db.open()) {
        QMessageBox::critical(this, QStringLiteral("DB Error"), db.lastError().text());
        return;
    }

    QSqlQuery query;
    if (!query.exec("CREATE TABLE IF NOT EXISTS records ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "name TEXT,"
               "attachment BLOB,"
               "type TEXT)")) {
        qDebug() << "Create table error:" << query.lastError();
    }

    loadRecords();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central);

    nameEdit = new QLineEdit(this);
    nameEdit->setPlaceholderText(QStringLiteral("名称"));  
    layout->addWidget(nameEdit);

    fileInfoLabel = new QLabel(QString::fromStdWString(UIStrings::Get(UIKey::NoAttachment)), this);
    layout->addWidget(fileInfoLabel);

    uploadButton = new QPushButton(QString::fromStdWString(UIStrings::Get(UIKey::ChooseFile)), this); 
    connect(uploadButton, &QPushButton::clicked, this, &MainWindow::uploadAttachment);
    layout->addWidget(uploadButton);

    insertButton = new QPushButton(QStringLiteral("插入记录"), this);  
    connect(insertButton, &QPushButton::clicked, this, &MainWindow::insertRecord);
    layout->addWidget(insertButton);

    exportButton = new QPushButton(QString::fromStdWString(UIStrings::Get(UIKey::ExportAttachment)), this);
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::exportAttachment);
    layout->addWidget(exportButton);

    table = new QTableWidget(this);
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({
        QStringLiteral("ID"), 
        QStringLiteral("名称"), 
        QStringLiteral("类型")  
    });
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(table);

    setCentralWidget(central);
}

void MainWindow::insertRecord() {
    QString name = nameEdit->text().trimmed();
    if (name.isEmpty()) return;

    QSqlQuery query;
    query.prepare("INSERT INTO records (name, attachment, type) VALUES (?, ?, ?)");
    query.addBindValue(name);
    query.addBindValue(fileData);
    query.addBindValue(fileType);
    
    if (!query.exec()) {
        QMessageBox::warning(this, QStringLiteral("SQL Error"), query.lastError().text());  
    } else {
        nameEdit->clear();
        fileData.clear();
        fileType.clear();
        fileInfoLabel->setText(QString::fromStdWString(UIStrings::Get(UIKey::NoAttachment)));
        uploadButton->setText(QString::fromStdWString(UIStrings::Get(UIKey::ChooseFile)));
        loadRecords();
    }
}

void MainWindow::loadRecords() {
    table->setRowCount(0);
    QSqlQuery query("SELECT id, name, type FROM records");
    if (!query.exec()) {
        qDebug() << "Query error:" << query.lastError();
        return;
    }
    
    int row = 0;
    while (query.next()) {
        table->insertRow(row);
        for (int col = 0; col < 3; ++col) {
            QTableWidgetItem *item = new QTableWidgetItem(query.value(col).toString());
            table->setItem(row, col, item);
        }
        ++row;
    }
}

bool MainWindow::isValidFile(const QString &filePath) {
    QFileInfo info(filePath);
    QString suffix = info.suffix().toLower();
    return suffix == "png" || suffix == "jpg" || suffix == "jpeg" || suffix == "pdf";
}

void MainWindow::uploadAttachment() {
    QString filter = QString::fromStdWString(UIStrings::Get(UIKey::SupportedFiles));
    QString filePath = QFileDialog::getOpenFileName(
        this, 
        QString::fromStdWString(UIStrings::Get(UIKey::ChooseFile)), 
        "", 
        filter
    );
    
    if (filePath.isEmpty()) 
        return;

    if (!isValidFile(filePath)) {
        QString message = QString::fromStdWString(UIStrings::Get(UIKey::InvalidType))
            .arg(QString::fromStdWString(UIStrings::Get(UIKey::OnlyAllow)));
        QMessageBox::warning(this, QStringLiteral("Warning"), message);  // 修正参数顺序
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(
            this, 
            QString::fromStdWString(UIStrings::Get(UIKey::FileReadFailed)), 
            file.errorString()
        );
        return;
    }

    fileData = file.readAll();
    fileType = QFileInfo(filePath).suffix().toLower();
    fileInfoLabel->setText(QStringLiteral("文件大小：%1 字节，类型：%2")  
        .arg(fileData.size())
        .arg(fileType));
    uploadButton->setText(QString::fromStdWString(UIStrings::Get(UIKey::ReplaceAttachment)));
}

void MainWindow::exportAttachment() {
    QList<QTableWidgetSelectionRange> selected = table->selectedRanges();
    if (selected.isEmpty()) {
        QMessageBox::information(
            this, 
            QString(),  // 空标题
            QString::fromStdWString(UIStrings::Get(UIKey::SelectRecordHint))  
        );
        return;
    }

    int row = selected.first().topRow();
    QString id = table->item(row, 0)->text();

    QSqlQuery query;
    query.prepare("SELECT attachment, type FROM records WHERE id = ?");
    query.addBindValue(id);
    
    if (!query.exec() || !query.next()) {
        qDebug() << "Export query error:" << query.lastError();
        return;
    }

    QByteArray data = query.value(0).toByteArray();
    QString type = query.value(1).toString();
    
    if (data.isEmpty()) {
        QMessageBox::information(
            this, 
            QString(), 
            QString::fromStdWString(UIStrings::Get(UIKey::NoAttachmentInRecord))
        );
        return;
    }

    QString defaultName = QStringLiteral("附件_%1.%2").arg(id).arg(type);
    QString filePath = QFileDialog::getSaveFileName(
        this, 
        QStringLiteral("保存附件"),  // 修正宽字符串
        defaultName
    );
    
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(
            this, 
            QString::fromStdWString(UIStrings::Get(UIKey::SaveFailed)), 
            file.errorString()
        );
        return;
    }

    if (file.write(data) == -1) {
        qDebug() << "Write file error:" << file.errorString();
    }
    file.close();

    QMessageBox::information(
        this, 
        QString::fromStdWString(UIStrings::Get(UIKey::SaveSuccess)), 
        QStringLiteral("附件已保存至：%1").arg(filePath)
    );
}