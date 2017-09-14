#include "importstablewidget.h"
#include "QHeaderView"

ImportsTableWidget::ImportsTableWidget()
{
    setColumnCount(5);
    QStringList tableHeaders;
    tableHeaders << "name" << "ordinal" << "plt" << "bind" << "type";
    setHorizontalHeaderLabels(tableHeaders);

    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    this->verticalHeader()->hide();
}

void ImportsTableWidget::insertImports(QVector<QStringList> importsData){
    this->setRowCount(0);
    int numImports = importsData.length();
    for (int rowNum = 0; rowNum < numImports; rowNum++){
        QStringList row = importsData.at(rowNum);
        QTableWidgetItem *name = new QTableWidgetItem(row.at(0));
        QTableWidgetItem *ordinal = new QTableWidgetItem(row.at(1));
        QTableWidgetItem *plt = new QTableWidgetItem(row.at(2));
        QTableWidgetItem *bind = new QTableWidgetItem(row.at(3));
        QTableWidgetItem *type = new QTableWidgetItem(row.at(4));

        name->setFlags(name->flags() ^ Qt::ItemIsEditable);
        ordinal->setFlags(ordinal->flags() ^ Qt::ItemIsEditable);
        plt->setFlags(plt->flags() ^ Qt::ItemIsEditable);
        bind->setFlags(bind->flags() ^ Qt::ItemIsEditable);
        type->setFlags(type->flags() ^ Qt::ItemIsEditable);

        insertRow(rowNum);
        setItem(rowNum, 0, name);
        setItem(rowNum, 1, ordinal);
        setItem(rowNum, 2, plt);
        setItem(rowNum, 3, bind);
        setItem(rowNum, 4, type);
    }

    this->resizeColumnsToContents();
}

void ImportsTableWidget::styleTable(QString foregroundColor, QString backgroundColor, QString backgroundColor3){
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
