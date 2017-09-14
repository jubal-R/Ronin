#ifndef FILEINFOTABLEWIDGET_H
#define FILEINFOTABLEWIDGET_H

#include "QTableWidget"
#include "QString"

class FileInfoTableWidget : public QTableWidget
{
public:
    FileInfoTableWidget();
    void insertInfo(QVector<QStringList> fileData);
    void styleTable(QString foregroundColor, QString backgroundColor, QString backgroundColor3);
};

#endif // FILEINFOTABLEWIDGET_H
