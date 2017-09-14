#include "fileinfotablewidget.h"
#include "QHeaderView"

FileInfoTableWidget::FileInfoTableWidget()
{
    setColumnCount(2);

    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    this->horizontalHeader()->hide();
    this->verticalHeader()->hide();

    QString stylesheet = "QWidget {"
           " background-color: #333333;"
           " color: #fffff8;"
           "font-size: 10pt;"
        "}"
       "QHeaderView::section {"
            "background-color: #e0e0e0;"
            "color: #4c4c4c;"
            "font-weight: bold;"
            "padding: 4px;"
            "border: 1px solid #c0c0c0;"
        "}";
    setStyleSheet(stylesheet);
}

void FileInfoTableWidget::insertInfo(QVector<QStringList> fileData){
    this->setRowCount(0);
    int numRows = fileData.length();
    for (int rowNum = 0; rowNum < numRows; rowNum++){
        QStringList row = fileData.at(rowNum);
        QTableWidgetItem *label = new QTableWidgetItem(row.at(0));
        QTableWidgetItem *value = new QTableWidgetItem(row.at(1));

        label->setFlags(label->flags() ^ Qt::ItemIsEditable);
        value->setFlags(value->flags() ^ Qt::ItemIsEditable);


        insertRow(rowNum);
        setItem(rowNum, 0, label);
        setItem(rowNum, 1, value);
    }

    this->resizeColumnsToContents();
}
