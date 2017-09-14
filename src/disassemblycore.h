#ifndef DISASSEMBLYCORE_H
#define DISASSEMBLYCORE_H

#include "QString"

#include "r2handler.h"

class DisassemblyCore
{
public:
    DisassemblyCore();
    void disassemble(QString file);
    bool disassemblyIsLoaded();
    QVector<QStringList> getFileInfo();
    QVector<QStringList> getRelocations();
    QVector<QStringList> getSymbols();
    QVector<QStringList> getImports();
    QStringList getStrings();

    QStringList getFunctionNames();
    QString getFunctionDisassembly(QString name);
    QString getPseudoCode(QString name);

    QString getHexDump();

    bool functionExists(QString name);

private:
    R2Handler r2h;
    bool fileLoaded;

};

#endif // DISASSEMBLYCORE_H
