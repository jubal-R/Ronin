#include "r2handler.h"

R2Handler::R2Handler()
{
    r2 = NULL;
}

void R2Handler::startR2(QString file){
    // Must specify base address or radare will use base address of zero by default. [https://github.com/radare/radare2/issues/7913]
    QString baseAddress = getBaseAddress(file);
    if (r2 != NULL)
        r2->close();
    r2 = new R2PipeAPI(file + " -B " + baseAddress);
    setConfig();
    analyzeAll();
}

void R2Handler::setConfig(){
    r2->cmd("e scr.color=0");
    r2->cmd("e scr.utf8=1");
    r2->cmd("e asm.flags=0");
    r2->cmd("e asm.fcnlines=0");
    r2->cmd("e asm.section=0");
    r2->cmd("e hex.header=0");
    r2->cmd("e asm.xrefs=0");
    r2->cmd("e asm.lines=0");
}

// Analyze
void R2Handler::analyzeAll(){
    r2->cmd("aa");
}

// Analyze some more
void R2Handler::analyzeAllAutoname(){
    r2->cmd("aaa");
}

// Seek
void R2Handler::seek(QString location){
    location = sanitizeInput(location);
    r2->cmd("s " + location);
}

void R2Handler::undoSeek(){
    r2->cmd("s-");
}

void R2Handler::redoSeek(){
    r2->cmd("s+");
}

QString R2Handler::currentSeek(){
    QString currentSeek = r2->cmd("s");
    return currentSeek;
}

// Binary info
QVector<QStringList> R2Handler::binaryInfo(){
    QVector<QStringList> fileData;
    QString jsonStr = r2->cmd("iIj");
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toLocal8Bit());
    QJsonObject jsonObj = doc.object();
    QStringList keyList = jsonObj.keys();
    int numKeys = keyList.length();

    for (int i = 0; i < numKeys; i++){
        QString key = keyList.at(i);
        QStringList keyValPair;

        if (jsonObj[key].isString()){
            keyValPair << key << jsonObj[key].toString();
            fileData.append(keyValPair);
        } else if (jsonObj[key].isBool()){
            keyValPair << key << (jsonObj[key].toBool() ? "true" : "false");
            fileData.append(keyValPair);
        } else {
            keyValPair << key << QString::number(jsonObj[key].toInt());
            fileData.append(keyValPair);
        }
    }


    return fileData;
}

// Relocations
QVector<QStringList> R2Handler::relocations(){
    QVector<QStringList> relocationData;
    QString jsonStr = r2->cmd("irj");
    QJsonArray jsonArray = strToJsonArray(jsonStr);

    int arrayLen = jsonArray.size();

    for (int i = 0; i < arrayLen; i++){
        QJsonValue indexVal = jsonArray.at(i);
        QJsonObject jsonObj = indexVal.toObject();

        QString name = jsonObj["name"].toString();
        QString type = jsonObj["type"].toString();
        QString vaddr = QString::number(jsonObj["vaddr"].toInt(), 16);
        QString paddr = QString::number(jsonObj["paddr"].toInt(), 16);

        QStringList relocation;
        relocation.append(name);
        relocation.append(type);
        relocation.append("0x" + vaddr);
        relocation.append("0x" + paddr);
        relocationData.append(relocation);
    }

    return relocationData;
}

// Imports
QVector<QStringList> R2Handler::imports(){
    QVector<QStringList> importsData;
    QString jsonStr = r2->cmd("iij");

    QJsonArray jsonArray = strToJsonArray(jsonStr);
    int arrayLen = jsonArray.size();

    for (int i = 0; i < arrayLen; i++){
        QJsonValue indexVal = jsonArray.at(i);
        QJsonObject jsonObj = indexVal.toObject();

        QString name = jsonObj["name"].toString();
        QString ordinal = QString::number(jsonObj["ordinal"].toInt(), 16);
        QString plt = QString::number(jsonObj["plt"].toInt(), 16);
        QString bind = jsonObj["bind"].toString();
        QString type = jsonObj["type"].toString();

        QStringList import;
        import.append(name);
        import.append(ordinal);
        import.append("0x" + plt);
        import.append(bind);
        import.append(type);
        importsData.append(import);
    }

    return importsData;
}

// Symbols
QVector<QStringList> R2Handler::symbols(){
    QVector<QStringList> symbolsData;
    QString jsonStr = r2->cmd("isj");
    QJsonArray jsonArray = strToJsonArray(jsonStr);
    int arrayLen = jsonArray.size();

    for (int i = 0; i < arrayLen; i++){
        QJsonValue indexVal = jsonArray.at(i);
        QJsonObject jsonObj = indexVal.toObject();

        QString name = jsonObj["name"].toString();
        QString size = QString::number(jsonObj["size"].toInt(), 16);
        QString vaddr = QString::number(jsonObj["vaddr"].toInt(), 16);
        QString paddr = QString::number(jsonObj["paddr"].toInt(), 16);
        QString type = jsonObj["type"].toString();

        QStringList symbol;
        symbol.append(name);
        symbol.append("0x" + size);
        symbol.append("0x" + vaddr);
        symbol.append("0x" + paddr);
        symbol.append(type);
        symbolsData.append(symbol);
    }

    return symbolsData;
}

// Sections
QVector< QVector<QString> > R2Handler::sections(){
    QVector< QVector<QString> > sectionsData;
    QString jsonStr = r2->cmd("Sj");
    QJsonArray jsonArray = strToJsonArray(jsonStr);
    int arrayLen = jsonArray.size();

    for (int i = 0; i < arrayLen; i++){
        QJsonValue indexVal = jsonArray.at(i);
        QJsonObject jsonObj = indexVal.toObject();

        QString name = jsonObj["name"].toString();
        QString size = QString::number(jsonObj["size"].toInt(), 16);
        QString vaddr = QString::number(jsonObj["vaddr"].toInt(), 16);
        QString flags = jsonObj["flags"].toString();

        QVector<QString> section(4);
        section[0] = name;
        section[1] = size;
        section[2] = vaddr;
        section[3] = flags;
        sectionsData.append(section);
    }

    return sectionsData;
}

// Strings
QVector< QVector<QString> > R2Handler::strings(){
    QVector< QVector<QString> > stringsData;
    QString jsonStr = r2->cmd("izj");
    QJsonArray jsonArray = strToJsonArray(jsonStr);
    int arrayLen = jsonArray.size();

    for (int i = 0; i < arrayLen; i++){
        QJsonValue indexVal = jsonArray.at(i);
        QJsonObject jsonObj = indexVal.toObject();

        QString vaddr = QString::number(jsonObj["vaddr"].toInt(), 16);
        // String values are base64 encoded
        QByteArray strVal = QByteArray::fromBase64(jsonObj["string"].toString().toLocal8Bit());

        QVector<QString> string(2);
        string[0] = "0x" + vaddr;
        string[1] = strVal;
        stringsData.append(string);
    }

    return stringsData;
}

// Function List
QVector< QVector<QString> > R2Handler::functionList(){
    QVector< QVector<QString> > functionsData;
    QString jsonStr = r2->cmd("aflj");
    QJsonArray jsonArray = strToJsonArray(jsonStr);
    int arrayLen = jsonArray.size();

    for (int i = 0; i < arrayLen; i++){
        QJsonValue indexVal = jsonArray.at(i);
        QJsonObject jsonObj = indexVal.toObject();

        QString name = jsonObj["name"].toString();
        QString size = QString::number(jsonObj["size"].toInt(), 16);
        QString vaddr = QString::number(jsonObj["offset"].toInt(), 16);

        QVector<QString> function(3);
        function[0] = name;
        function[1] = "0x" + size;
        function[2] = "0x" + vaddr;
        functionsData.append(function);
    }

    return functionsData;
}

// Disassembly at current seek position
QString R2Handler::disassembly(int numInstructions){
    QString functionDisassembly = r2->cmd("pd " + QString::number(numInstructions));
    return functionDisassembly;
}

// Function disassembly of specific function
QString R2Handler::functionDisassembly(QString functionName){
    QString functionDisassembly = r2->cmd("pdf @ " + functionName);
    return functionDisassembly;
}

// Function pseudo code at current seek position
QString R2Handler::pseudoCode(){
    QString pseudoCode = r2->cmd("pdc");
    return pseudoCode;
}

// Function pseudo code of specific function
QString R2Handler::pseudoCode(QString functionName){
    QString pseudoCode = r2->cmd("pdc @ " + functionName);
    return pseudoCode;
}

// Ascii graph of function at current seek position
QString R2Handler::functionAsciiGraph(){
    QString asciiGraph = r2->cmd("agf");
    return asciiGraph;
}

// Ascii graph of function of specific function
QString R2Handler::functionAsciiGraph(QString functionName){
    QString asciiGraph = r2->cmd("agf @" + functionName);
    return asciiGraph;
}

// Hexdump
QString R2Handler::hexdump(int numInstructions){
    QString hexdump = r2->cmd("px " + QString::number(numInstructions));
    return hexdump;
}

// Hexdump of function
QString R2Handler::functionHexdump(QString functionName){
    QString hexdump = r2->cmd("pxf @ " + functionName);
    return hexdump;
}

// Convert virtual address to physical address
QString R2Handler::paddr(QString vaddr){
    vaddr = sanitizeInput(vaddr);
    QString paddr = r2->cmd("?p " + vaddr);
    return paddr;
}

QJsonArray R2Handler::strToJsonArray(QString json){
    QJsonDocument doc = QJsonDocument::fromJson(json.toLocal8Bit());
    return doc.array();
}

QString R2Handler::sanitizeInput(QString input){
    input = input.replace("!", "");
    input = input.replace("|", "");
    input = input.replace(";", "");
    input = input.replace("@", "");
    input = input.replace(" ", "");
    return input;
}

QString R2Handler::getBaseAddress(QString file){
    QProcess *process = new QProcess(0);

    QStringList args;
    args << "-ej" << file;
    process->start("rabin2", args);

    if (!process->waitForStarted())
        return "";

    if (!process->waitForFinished())
        return "";

    QByteArray output;
    output.append(process->readAllStandardError());
    output.append(process->readAllStandardOutput());

    QJsonDocument doc = QJsonDocument::fromJson(output);
    QJsonObject Obj = doc.object();
    QJsonArray array = Obj["entries"].toArray();
    QJsonObject entriesObj = array.at(0).toObject();

    QString baseAddress = "0x" + QString::number(entriesObj["baddr"].toVariant().toLongLong(), 16);

    return baseAddress;
}
