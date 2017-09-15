#ifndef R2HANDLER_H
#define R2HANDLER_H

#include "r2pipe/r2pipe.h"

class R2Handler
{
public:
    R2Handler();
    void startR2(QString file);
    void setConfig();
    void aa();
    void aaa();
    QVector<QStringList> iI();
    QVector<QStringList> ir();
    QVector<QStringList> ii();
    QVector<QStringList> is();
    QVector< QVector<QString> > S();
    QVector< QVector<QString> > iz();
    QVector< QVector<QString> > afl();
    QString pdf(QString functionName);
    QString pdc(QString functionName);
    QString px(QString args);
    QString paddr(QString vaddr);
private:
    R2PipeAPI *r2;
    QJsonArray strToJsonArray(QString json);
    QString getBaseAddress(QString file);
    QString sanitizeInput(QString input);
};

#endif // R2HANDLER_H
