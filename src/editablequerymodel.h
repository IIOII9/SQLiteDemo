#pragma once

#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QVariantList>

class EditableQueryModel : public QSqlQueryModel {
    Q_OBJECT
public:
    EditableQueryModel(QObject* parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    void setPrimaryKeyColumn(const QString& name);
    void setPrimaryKeyAtRow(int row, int id);
    void setDB(QSqlDatabase db);
    void setTableName(const QString& name);
    void refresh();
    void setQuery(const QString &query, const QVariantList &params = {});

private:
    QSqlDatabase m_db;
    QString m_tableName;
    QString primaryKey = "id";
    QHash<int, int> rowIdMap;
    QString currentQuery;
    QVariantList currentParams;
};
