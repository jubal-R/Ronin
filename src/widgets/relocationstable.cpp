#include "relocationstable.h"
#include "QHeaderView"

RelocationsTable::RelocationsTable()
{
    setColumnCount(4);
    QStringList tableHeaders;
    tableHeaders << "name" << "type" << "vaddr" << "paddr";
    setHorizontalHeaderLabels(tableHeaders);

    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    this->verticalHeader()->hide();

}

void RelocationsTable::insertRelocations(QVector<QStringList> relocationData){
    this->setRowCount(0);
    int numRelocations = relocationData.length();
    for (int rowNum = 0; rowNum < numRelocations; rowNum++){
        QStringList row = relocationData.at(rowNum);
        QTableWidgetItem *name = new QTableWidgetItem(row.at(0));
        QTableWidgetItem *type = new QTableWidgetItem(row.at(1));
        QTableWidgetItem *vaddr = new QTableWidgetItem(row.at(2));
        QTableWidgetItem *paddr = new QTableWidgetItem(row.at(3));

        name->setFlags(name->flags() ^ Qt::ItemIsEditable);
        type->setFlags(type->flags() ^ Qt::ItemIsEditable);
        vaddr->setFlags(vaddr->flags() ^ Qt::ItemIsEditable);
        paddr->setFlags(paddr->flags() ^ Qt::ItemIsEditable);

        insertRow(rowNum);
        setItem(rowNum, 0, name);
        setItem(rowNum, 1, type);
        setItem(rowNum, 2, vaddr);
        setItem(rowNum, 3, paddr);
    }

    this->resizeColumnsToContents();
}

void RelocationsTable::styleTable(QString foregroundColor, QString backgroundColor, QString backgroundColor3){
    QString stylesheet = "QWidget {"
           "background-color: " + backgroundColor + ";"
           "color: " + foregroundColor + ";"
           "font-size: 10pt;"
        "}"
        "QTableWidget::item:selected {"
            "background-color: " + backgroundColor3 + ";"
            "color: " + foregroundColor + ";"
        "}"
        "QHeaderView::section {"
           "background-color: " + backgroundColor + ";"
           "color: " + foregroundColor + ";"
           "font-weight: bold;"
           "padding: 4px;"
           "border: none;"
        "}";
    setStyleSheet(stylesheet);
}
