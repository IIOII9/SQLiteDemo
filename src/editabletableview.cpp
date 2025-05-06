#include "EditableTableView.h"
#include <QItemDelegate>

void EditableTableView::mousePressEvent(QMouseEvent *event) {
    QTableView::mousePressEvent(event);
    QModelIndex index = indexAt(event->pos());
    if (index.isValid()) {
        edit(index);  // 自动进入编辑状态
    }
}

void EditableTableView::keyPressEvent(QKeyEvent *event) {
    QModelIndex current = currentIndex();
    if (!current.isValid()) {
        QTableView::keyPressEvent(event);
        return;
    }

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter || event->key() == Qt::Key_Tab) {
        // 提交当前编辑（触发 commitData）
        if (state() == EditingState) {
            QWidget *editor = indexWidget(current);
            if (editor) {
                commitData(editor);  // 提交数据到模型
                closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);  // 关闭编辑器
            }
            // closePersistentEditor(current);
        }    

        int row = current.row();
        int col = current.column() + 1;

        if (col >= model()->columnCount()) {
            col = 1;
            row++;
        }

        if (row < model()->rowCount()) {
            QModelIndex nextIndex = model()->index(row, col);
            setCurrentIndex(nextIndex);
            // setCurrentIndex(nextIndex);
            edit(nextIndex);  // 自动编辑下一个单元格
        }
        return;
    }

    QTableView::keyPressEvent(event);
}
