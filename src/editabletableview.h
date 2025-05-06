#pragma once

#include <QTableView>
#include <QKeyEvent>
#include <QMouseEvent>

class EditableTableView : public QTableView {
    Q_OBJECT
public:
    using QTableView::QTableView;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
};
