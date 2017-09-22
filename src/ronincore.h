#ifndef RONINCORE_H
#define RONINCORE_H

#include "QString"

#include "r2handler.h"

class RoninCore
{
public:
    RoninCore();
    void loadFile(QString file);
    bool fileIsLoaded();
    void loadFunctionData();
    void seekTo(QString location);
    void undoSeek();
    void redoSeek();

    QString getCurrentSeekAddress();

    QVector<QStringList> getFileInfo();
    QVector<QStringList> getRelocations();
    QVector<QStringList> getSymbols();
    QVector<QStringList> getImports();
    QStringList getStrings();

    QStringList getFunctionNames();
    bool functionExists(QString function);
    QString getFunctionAddress(QString name);
    QString getDisassembly();
    QString getFunctionDisassembly(QString name);
    QString getPseudoCode();
    QString getPseudoCode(QString name);
    QString getFunctionGraph();
    QString getFunctionGraph(QString name);
    QString getFunctionHexDump(QString name);

    QString getHexDump();
    QString getPaddr(QString vaddr);

private:
    R2Handler r2h;
    bool fileLoaded;
    QVector< QVector<QString> > functionsData;

};

#endif // RONINCORE_H
