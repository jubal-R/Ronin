#ifndef R2HANDLER_H
#define R2HANDLER_H

#include "r2pipe/r2pipe.h"

class R2Handler
{
public:
    R2Handler();
    void startR2(QString file);
    void setConfig();
    void analyzeAll();
    void analyzeAllAutoname();
    void seek(QString location);
    QString currentSeek();
    QVector<QStringList> binaryInfo();
    QVector<QStringList> relocations();
    QVector<QStringList> imports();
    QVector<QStringList> symbols();
    QVector< QVector<QString> > sections();
    QVector< QVector<QString> > strings();
    QVector< QVector<QString> > functionList();
    QString disassembly(int numInstructions);
    QString functionDisassembly(QString functionName);
    QString pseudoCode();
    QString pseudoCode(QString functionName);
    QString functionAsciiGraph();
    QString functionAsciiGraph(QString functionName);
    QString hexdump(int numInstructions);
    QString functionHexdump(QString functionName);
    QString paddr(QString vaddr);
private:
    R2PipeAPI *r2;
    QJsonArray strToJsonArray(QString json);
    QString getBaseAddress(QString file);
    QString sanitizeInput(QString input);
};

#endif // R2HANDLER_H
