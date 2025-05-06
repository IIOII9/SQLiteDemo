#pragma once

#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QSqlQuery>

class EditableQueryModel : public QSqlQueryModel {
    Q_OBJECT
public:
    EditableQueryModel(QObject* parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    void setDB(QSqlDatabase db);
    void setTableName(const QString& name);

private:
    QSqlDatabase m_db;
    QString m_tableName;
};
