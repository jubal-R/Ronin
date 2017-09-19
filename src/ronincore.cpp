#include "ronincore.h"

RoninCore::RoninCore()
{
    fileLoaded = false;
}

void RoninCore::loadFile(QString file){
    r2h.startR2(file);
    loadFunctionData();
    fileLoaded = true;
}

bool RoninCore::fileIsLoaded(){
    if (fileLoaded)
        return true;
    else
        return false;
}

void RoninCore::seekTo(QString location){
    r2h.s(location);
}

QString RoninCore::getCurrentSeekAddress(){
    return r2h.currentSeek();
}

QVector<QStringList> RoninCore::getFileInfo(){
    return r2h.iI();
}

QVector<QStringList> RoninCore::getRelocations(){
    return r2h.ir();
}

QVector<QStringList> RoninCore::getSymbols(){
    return r2h.is();
}

QVector<QStringList> RoninCore::getImports(){
    return r2h.ii();
}

QStringList RoninCore::getStrings(){
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

void RoninCore::loadFunctionData(){
    /* Function Data
     * Each element contains vector of data for each function in following format:
     * [Function name], [size], [virtual address]
    */
    functionsData = r2h.afl();
}

QStringList RoninCore::getFunctionNames(){
    QStringList functionNames;
    int numFunctions = functionsData.length();

    for (int i = 0; i < numFunctions; i++){
        functionNames.append(functionsData[i][0]);
    }

    return functionNames;
}

bool RoninCore::functionExists(QString function){
    int numFunctions = functionsData.length();

    for (int i = 0; i < numFunctions; i++){
        if (functionsData[i][0] == function)
            return true;
    }

    return false;
}

QString RoninCore::getFunctionAddress(QString name){
    int numFunctions = functionsData.length();

    for (int i = 0; i < numFunctions; i++){
        if (functionsData[i][0] == name)
            return functionsData[i][2];
    }

    return "";
}

QString RoninCore::getDisassembly(){
    return r2h.pd(200);
}

QString RoninCore::getFunctionDisassembly(QString name){
    return r2h.pdf(name);
}

QString RoninCore::getPseudoCode(){
    return r2h.pdc();
}

QString RoninCore::getPseudoCode(QString name){
    return r2h.pdc(name);
}

QString RoninCore::getFunctionGraph(){
    QString asciiGraph = r2h.agf();

    // Remove unnecessary first line
    int len = asciiGraph.length();
    int index = 0;
    while (index < len && asciiGraph.at(index) != QChar('\n')){
        index++;
    }

    asciiGraph = asciiGraph.mid(index);

    return asciiGraph;
}

QString RoninCore::getFunctionGraph(QString name){
    QString asciiGraph = r2h.agf(name);

    // Remove unnecessary first line
    int len = asciiGraph.length();
    int index = 0;
    while (index < len && asciiGraph.at(index) != QChar('\n')){
        index++;
    }

    asciiGraph = asciiGraph.mid(index);

    return asciiGraph;
}

QString RoninCore::getHexDump(){
    return r2h.px(200);
}

QString RoninCore::getFunctionHexDump(QString name){
    return r2h.pxf(name);
}

QString RoninCore::getPaddr(QString vaddr){
    return r2h.paddr(vaddr);
}
