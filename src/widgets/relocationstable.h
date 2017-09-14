#ifndef RELOCATIONSTABLE_H
#define RELOCATIONSTABLE_H

#include "QTableWidget"
#include "QString"

class RelocationsTable : public QTableWidget
{
public:
    RelocationsTable();
    void insertRelocations(QVector<QStringList> relocationData);
};

#endif // RELOCATIONSTABLE_H
