#ifndef RELOCATIONSTABLE_H
#define RELOCATIONSTABLE_H

#include "QTableWidget"
#include "QString"

class RelocationsTable : public QTableWidget
{
public:
    RelocationsTable();
    void insertRelocations(QVector<QStringList> relocationData);
    void styleTable(QString foregroundColor, QString backgroundColor, QString backgroundColor3);
};

#endif // RELOCATIONSTABLE_H
