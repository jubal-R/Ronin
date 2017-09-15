#include "disassemblycore.h"

#include "QtConcurrent/QtConcurrent"
#include "QFuture"

DisassemblyCore::DisassemblyCore()
{
    fileLoaded = false;
}

/*
 *  Disassembly
*/

void DisassemblyCore::disassemble(QString file){
    r2h.startR2(file);
    fileLoaded = true;
}

bool DisassemblyCore::disassemblyIsLoaded(){
    if (fileLoaded)
        return true;
    else
        return false;
}

QVector<QStringList> DisassemblyCore::getFileInfo(){
    return r2h.iI();
}

QVector<QStringList> DisassemblyCore::getRelocations(){
    return r2h.ir();
}

QVector<QStringList> DisassemblyCore::getSymbols(){
    return r2h.is();
}

QVector<QStringList> DisassemblyCore::getImports(){
    return r2h.ii();
}

QStringList DisassemblyCore::getStrings(){
    QVector< QVector<QString> > stringsData = r2h.iz();
    int numStrings = stringsData.length();

    QString stringVals = "";
    QString stringAddrs = "";

    for (int i = 0; i < numStrings; i++){
        stringVals.append(stringsData[i][0] + "\n");
        stringAddrs.append(stringsData[i][1] + "\n");
    }

    QStringList stringsOutputList;
    stringsOutputList.append(stringVals);
    stringsOutputList.append(stringAddrs);

    return stringsOutputList;
}

/*
 * Function Data
*/

QStringList DisassemblyCore::getFunctionNames(){
    return r2h.getFunctionList();
}
QString DisassemblyCore::getFunctionDisassembly(QString name){
    return r2h.pdf(name);
}

QString DisassemblyCore::getPseudoCode(QString name){
    return r2h.pdc(name);
}

QString DisassemblyCore::getHexDump(){
    return r2h.px("$s @0");
}

QString DisassemblyCore::getPaddr(QString vaddr){
    return r2h.paddr(vaddr);
}
