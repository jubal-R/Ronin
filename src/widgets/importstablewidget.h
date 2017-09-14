#ifndef IMPORTSTABLEWIDGET_H
#define IMPORTSTABLEWIDGET_H

#include "QTableWidget"
#include "QString"

class ImportsTableWidget : public QTableWidget
{
public:
    ImportsTableWidget();
    void insertImports(QVector<QStringList> importsData);
    void styleTable(QString foregroundColor, QString backgroundColor, QString backgroundColor3);
};

#endif // IMPORTSTABLEWIDGET_H
