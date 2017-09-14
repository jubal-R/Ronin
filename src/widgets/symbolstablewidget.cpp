#include "symbolstablewidget.h"
#include "QHeaderView"

SymbolsTableWidget::SymbolsTableWidget()
{
    setColumnCount(5);
    QStringList tableHeaders;
    tableHeaders << "name" << "size" << "vaddr" << "paddr" << "type";
    setHorizontalHeaderLabels(tableHeaders);

    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    this->verticalHeader()->hide();

}

void SymbolsTableWidget::insertSymbols(QVector<QStringList> symbolsData){
    this->setRowCount(0);
    int numSymbols = symbolsData.length();
    for (int rowNum = 0; rowNum < numSymbols; rowNum++){
        QStringList row = symbolsData.at(rowNum);
        QTableWidgetItem *name = new QTableWidgetItem(row.at(0));
        QTableWidgetItem *size = new QTableWidgetItem(row.at(1));
        QTableWidgetItem *vaddr = new QTableWidgetItem(row.at(2));
        QTableWidgetItem *paddr = new QTableWidgetItem(row.at(3));
        QTableWidgetItem *type = new QTableWidgetItem(row.at(4));

        name->setFlags(name->flags() ^ Qt::ItemIsEditable);
        size->setFlags(size->flags() ^ Qt::ItemIsEditable);
        vaddr->setFlags(vaddr->flags() ^ Qt::ItemIsEditable);
        paddr->setFlags(paddr->flags() ^ Qt::ItemIsEditable);
        type->setFlags(type->flags() ^ Qt::ItemIsEditable);

        insertRow(rowNum);
        setItem(rowNum, 0, name);
        setItem(rowNum, 1, size);
        setItem(rowNum, 2, vaddr);
        setItem(rowNum, 3, paddr);
        setItem(rowNum, 4, type);
    }

    this->resizeColumnsToContents();
}

void SymbolsTableWidget::styleTable(QString foregroundColor, QString backgroundColor, QString backgroundColor3){
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
