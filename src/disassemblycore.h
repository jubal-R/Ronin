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
    void loadFunctionData();

    QVector<QStringList> getFileInfo();
    QVector<QStringList> getRelocations();
    QVector<QStringList> getSymbols();
    QVector<QStringList> getImports();
    QStringList getStrings();

    QStringList getFunctionNames();
    bool functionExists(QString function);
    QString getFunctionAddress(QString name);
    QString getFunctionDisassembly(QString name);
    QString getPseudoCode(QString name);

    QString getHexDump();
    QString getPaddr(QString vaddr);

private:
    R2Handler r2h;
    bool fileLoaded;
    QVector< QVector<QString> > functionsData;

};

#endif // DISASSEMBLYCORE_H
