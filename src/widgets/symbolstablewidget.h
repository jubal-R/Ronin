#ifndef SYMBOLSTABLEWIDGET_H
#define SYMBOLSTABLEWIDGET_H

#include "QTableWidget"
#include "QString"

class SymbolsTableWidget : public QTableWidget
{
public:
    SymbolsTableWidget();
    void insertSymbols(QVector<QStringList> symbolsData);
};

#endif // SYMBOLSTABLEWIDGET_H
