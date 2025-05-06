#include "editablequerymodel.h"
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug>

EditableQueryModel::EditableQueryModel(QObject *parent)
    : QSqlQueryModel(parent)
{}

void EditableQueryModel::setDB(QSqlDatabase db) {
    m_db = db;
}

void EditableQueryModel::setTableName(const QString& name) {
    m_tableName = name;
}

Qt::ItemFlags EditableQueryModel::flags(const QModelIndex &index) const {
    if (index.column() == 0)  // 假设 id 不可编辑
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
}

bool EditableQueryModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role != Qt::EditRole || !index.isValid())
        return false;

    // int id = data(this->index(index.row(), 0)).toInt();  // 假设第0列是id
    int row = index.row();
    int id = rowIdMap.value(row, -1);
    if (id == -1)
    {
        return false;
    }
    QString columnName = headerData(index.column(), Qt::Horizontal).toString();

    QSqlQuery query(m_db);
    QString sql = QString("UPDATE %1 SET %2 = ? WHERE %3 = ?")
                    .arg(m_tableName, columnName, primaryKey);

    qDebug() << "Executing SQL:" << sql << "Value:" << value << "ID:" << id;

    query.prepare(sql);
    query.addBindValue(value);
    query.addBindValue(id);

    if (!query.exec()) {
        qWarning() << "Update failed:" << query.lastError().text();
        return false;
    }

    // emit dataChanged(index, index);
    refresh();
    return true;
}

void EditableQueryModel::setPrimaryKeyColumn(const QString& name)
{
    primaryKey = name;
}
void EditableQueryModel::setPrimaryKeyAtRow(int row, int id)
{
    rowIdMap[row] = id;
}

void EditableQueryModel::refresh()
{
    setQuery(currentQuery, currentParams);
}
void EditableQueryModel::setQuery(const QString &query, const QVariantList &params)
{
    currentQuery = query;
    currentParams = params;
    QSqlQuery q;
    q.prepare(query);
    for(const QVariant &param : params)
    {
        q.addBindValue(param);
    }
    q.exec();
    QSqlQueryModel::setQuery(q);
}