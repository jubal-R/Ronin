#include "fileinfotablewidget.h"
#include "QHeaderView"

FileInfoTableWidget::FileInfoTableWidget()
{
    setColumnCount(2);

    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    this->horizontalHeader()->hide();
    this->verticalHeader()->hide();
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

void FileInfoTableWidget::styleTable(QString foregroundColor, QString backgroundColor, QString backgroundColor3){
    QString stylesheet = "QWidget {"
           "background-color: " + backgroundColor + ";"
           "color: " + foregroundColor + ";"
           "font-size: 10pt;"
        "}"
        "QTableWidget::item:selected {"
            "background-color: " + backgroundColor3 + ";"
            "color: " + foregroundColor + ";"
        "}";
    setStyleSheet(stylesheet);
}
