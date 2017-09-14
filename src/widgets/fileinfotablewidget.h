#ifndef FILEINFOTABLEWIDGET_H
#define FILEINFOTABLEWIDGET_H

#include "QTableWidget"
#include "QString"

class FileInfoTableWidget : public QTableWidget
{
public:
    FileInfoTableWidget();
    void insertInfo(QVector<QStringList> fileData);
};

#endif // FILEINFOTABLEWIDGET_H
