#ifndef SYMBOLSTABLEWIDGET_H
#define SYMBOLSTABLEWIDGET_H

#include "QTableWidget"
#include "QString"

class SymbolsTableWidget : public QTableWidget
{
public:
    SymbolsTableWidget();
    void insertSymbols(QVector<QStringList> symbolsData);
    void styleTable(QString foregroundColor, QString backgroundColor, QString backgroundColor3);
};

#endif // SYMBOLSTABLEWIDGET_H
