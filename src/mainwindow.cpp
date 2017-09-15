#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qfiledialog.h"
#include "qmessagebox.h"
#include "QScrollBar"
#include "QInputDialog"
#include "QProgressDialog"
#include "QFuture"
#include "QtConcurrent/QtConcurrent"

#include "QDebug"

#include "resultsdialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    relocationsTableWidget = new RelocationsTable;
    ui->infoTabWidget->addTab(relocationsTableWidget, "Relocations");
    symbolsTableWidget = new SymbolsTableWidget;
    ui->infoTabWidget->addTab(symbolsTableWidget, "Symbols");
    importsTableWidget = new ImportsTableWidget;
    ui->infoTabWidget->addTab(importsTableWidget, "Imports");
    fileInfoTableWidget = new FileInfoTableWidget;
    ui->infoTabWidget->addTab(fileInfoTableWidget, "Info");

    QObject::connect(ui->stringsAddressBrowser->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->stringsBrowser->verticalScrollBar(), SLOT(setValue(int)));
    QObject::connect(ui->stringsBrowser->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->stringsAddressBrowser->verticalScrollBar(), SLOT(setValue(int)));

    /*
     *  Setup builtin fonts
    */

    // Sans serif
    int sansid = QFontDatabase::addApplicationFont(":/fonts/NotoSans-Regular.ttf");
    QString sansfamily = QFontDatabase::applicationFontFamilies(sansid).at(0);
    QFont sans(sansfamily);
    sans.setPointSize(11);

    this->setFont(sans);
    ui->disTabWidget->setFont(sans);
    ui->functionListLabel->setFont(sans);
    ui->functionList->setFont(sans);
    ui->stringsAddressBrowser->setFont(sans);
    ui->stringsBrowser->setFont(sans);

    // Sans serif bold
    int sansBoldId = QFontDatabase::addApplicationFont(":/fonts/NotoSans-Bold.ttf");
    QString sansBoldFamily = QFontDatabase::applicationFontFamilies(sansBoldId).at(0);
    QFont sansBold(sansBoldFamily);
    sansBold.setPointSize(11);
    sansBold.setBold(true);

    ui->functionLabel->setFont(sansBold);
    ui->addressLabel->setFont(sansBold);
    ui->stringsAddressLabel->setFont(sansBold);
    ui->stringsLabel->setFont(sansBold);

    // Monospace
    int monoid = QFontDatabase::addApplicationFont(":/fonts/Anonymous Pro.ttf");
    QString monofamily = QFontDatabase::applicationFontFamilies(monoid).at(0);
    QFont mono(monofamily);
    mono.setPointSize(12);

    ui->codeBrowser->setFont(mono);
    ui->pseudoCodeBrowser->setFont(mono);
    ui->hexBrowser->setFont(mono);
    ui->addressValueLabel->setFont(mono);

    // Monospace Bold
    int monoBoldId = QFontDatabase::addApplicationFont(":/fonts/Anonymous Pro B.ttf");
    QString monoBoldFamily = QFontDatabase::applicationFontFamilies(monoBoldId).at(0);
    QFont monoBold(monoBoldFamily);
    monoBold.setPointSize(12);
    monoBold.setBold(true);

    ui->hexLabel->setFont(monoBold);


    this->setWindowTitle("Ronin");

    // Set Window Size
    MainWindow::resize(settings.value("windowWidth", 1000).toInt(), settings.value("windowHeight", 600).toInt());
    ui->splitter->restoreState(settings.value("splitterSizes").toByteArray());

    ui->searchBar->hide();
    currentSearchTerm = "";

    /*
     *  Set options from saved settings
    */

    // Style
    disHighlighter = new DisassemblyHighlighter(ui->codeBrowser->document(), "Default");

    QString theme = settings.value("theme", "default").toString();

    if (theme == "dark"){
            on_actionDark_triggered();
    } else if (theme == "solarized"){
        on_actionSolarized_triggered();
    }else if (theme == "solarizedDark"){
        on_actionSolarized_Dark_triggered();
    } else {
        on_actionDefault_triggered();
    }

    QString menuStyle = "QMenu::item:selected {background-color: #3ba1a1; color: #fafafa;}"
            "QMenu::item::disabled {color: #aaaaaa}"
            "QMenu::item {background-color: #e0e0e0; color: #555555;}"
            "QMenuBar::item {background-color: #e0e0e0; color: #555555;}"
            "QMenuBar {background-color: #e0e0e0;}";
    ui->menuBar->setStyleSheet(menuStyle);
    QString navBarStyle = "#navBar {border-bottom: 1px solid #d4d4d4;}";
    ui->navBar->setStyleSheet(navBarStyle);

    connect(ui->codeBrowser, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    currentFunctionIndex = 0;

}

MainWindow::~MainWindow()
{
    /*
     *  Save Settings
    */

    // Get Window Size
    QRect windowRect = MainWindow::normalGeometry();
    settings.setValue("windowWidth", windowRect.width());
    settings.setValue("windowHeight", windowRect.height());
    settings.setValue("splitterSizes", ui->splitter->saveState());

    delete ui;
}

/*
 *  Load Disassembly
*/

//  Load binary and display disassembly
void MainWindow::loadBinary(QString file){

    if (file != ""){
        this->setWindowTitle("Ronin - " + file);

        clearUi();

        if (true) {
            QProgressDialog progress("Loading Disassembly", "", 0, 4, this);
            progress.setCancelButton(0);
            progress.setWindowModality(Qt::WindowModal);
            progress.setMinimumDuration(500);
            progress.setValue(0);

            // Disassemble in seperate thread
            QFuture<void> disassemblyThread = QtConcurrent::run(&disassemblyCore, &DisassemblyCore::disassemble, file);

            while (!disassemblyThread.isFinished()){
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            }

            progress.setValue(1);

            if (!disassemblyCore.disassemblyIsLoaded()){
                ui->codeBrowser->setPlainText("File format not recognized.");
                ui->addressLabel->setText("");
                ui->functionLabel->setText("");
            } else {
                // If all good, display disassembly data
                displayFunctionData();

                // Add initial location to history
                addToHistory(currentFunctionIndex, 0);

                enableMenuItems();
            }

            progress.setValue(2);

            displayHexData();

            progress.setValue(3);

            setUpdatesEnabled(false);
            relocationsTableWidget->insertRelocations(disassemblyCore.getRelocations());
            symbolsTableWidget->insertSymbols(disassemblyCore.getSymbols());
            fileInfoTableWidget->insertInfo(disassemblyCore.getFileInfo());
            importsTableWidget->insertImports(disassemblyCore.getImports());
            setUpdatesEnabled(true);

            // Load strings data
            QStringList strOutputList = disassemblyCore.getStrings();
            ui->stringsAddressBrowser->setPlainText(strOutputList.at(0));
            ui->stringsBrowser->setPlainText(strOutputList.at(1));

            progress.setValue(4);
        }
    }
}

// Disassemble
void MainWindow::on_actionOpen_triggered()
{
    // Prompt user for file
    QString file = QFileDialog::getOpenFileName(this, tr("Open File"), files.getCurrentDirectory(), tr("All (*)"));

    // Update current directory and load file
    if (file != ""){
        files.setCurrentDirectory(file);

        loadBinary(file);
        ui->disTabWidget->setCurrentIndex(0);
        ui->codeBrowser->setFocus();
    }

}

/*
 *  Display Disassembly Data
*/

// Set lables and code browser to display function info and contents
void MainWindow::displayFunctionText(QString functionName){
    if (disassemblyCore.disassemblyIsLoaded()){
        setUpdatesEnabled(false);
        ui->functionLabel->setText(functionName);
        ui->codeBrowser->setPlainText(disassemblyCore.getFunctionDisassembly(functionName));
        ui->pseudoCodeBrowser->setPlainText(disassemblyCore.getPseudoCode(functionName));
        setUpdatesEnabled(true);

    }
}

// Setup functionlist and display function data
void MainWindow::displayFunctionData(){
    if (disassemblyCore.disassemblyIsLoaded()){
        // Populate function list in sidebar
        ui->functionList->addItems(disassemblyCore.getFunctionNames());

    }
}

//  Highlight current line of function
void MainWindow::highlightCurrentLine(){
   QList<QTextEdit::ExtraSelection> extraSelections;

   QTextEdit::ExtraSelection selections;
   selections.format.setBackground(lineColor);
   selections.format.setProperty(QTextFormat::FullWidthSelection, true);
   selections.cursor = ui->codeBrowser->textCursor();
   selections.cursor.clearSelection();
   extraSelections.append(selections);

   ui->codeBrowser->setExtraSelections(extraSelections);

}

void MainWindow::displayHexData(){
    // Set hex view values
    setUpdatesEnabled(false);
    ui->hexBrowser->setPlainText(disassemblyCore.getHexDump());
    setUpdatesEnabled(true);
}

void MainWindow::clearUi(){
    while (ui->functionList->count() > 0){
        ui->functionList->takeItem(0);
    }
    ui->addressValueLabel->clear();
    ui->functionLabel->clear();
    ui->codeBrowser->clear();
    ui->hexBrowser->clear();
    ui->fileFormatlabel->clear();
    ui->stringsAddressBrowser->clear();
    ui->stringsBrowser->clear();

    // Clear history
    history.clear();
}

void MainWindow::enableMenuItems(){
    // Enable navigation and tools
    ui->actionGo_To_Address->setEnabled(true);
    ui->actionGo_to_Address_at_Cursor->setEnabled(true);
    ui->actionGet_Offset->setEnabled(true);
    ui->actionFind_References->setEnabled(true);
    ui->actionFind_Calls_to_Current_Function->setEnabled(true);
    ui->actionFind_Calls_to_Current_Location->setEnabled(true);
}

/*
 *  Navigation
*/

// Go to virtual memory address
void MainWindow::goToAddress(QString targetAddress){

}

// Go to Address triggered
void MainWindow::on_actionGo_To_Address_triggered()
{
    bool ok = true;
    QString targetAddress = QInputDialog::getText(this, tr("Go to Address"),tr("Address"), QLineEdit::Normal,"", &ok).trimmed();
    if (ok)
        goToAddress(targetAddress);

}

// Go to Address at Cursor triggered
void MainWindow::on_actionGo_to_Address_at_Cursor_triggered()
{
    QTextCursor cursor = ui->codeBrowser->textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    QString targetAddress = cursor.selectedText();
    if (targetAddress.startsWith("0x")){
        targetAddress = targetAddress.mid(2);
    }

    goToAddress(targetAddress);
}

// Display function clicked in sidebar
void MainWindow::on_functionList_itemDoubleClicked(QListWidgetItem *item)
{
    // Display function
    displayFunctionText(item->text());
    ui->disTabWidget->setCurrentIndex(0);
    // Add new location to history
    addToHistory(currentFunctionIndex, 0);
}

// Get file offset of current line of disassembly
void MainWindow::on_actionGet_Offset_triggered()
{
    bool ok;
    QString virtualAddress = QInputDialog::getText(this, tr("Get File Offset"),tr("Get file offset of address or function"), QLineEdit::Normal,"", &ok).trimmed();
    if (ok && !virtualAddress.isEmpty()){
        QString paddr = disassemblyCore.getPaddr(virtualAddress);

        if(paddr != "0xffffffffffffffff\n"){
            QString offsetMsg = "File offset of address " + virtualAddress + "\npaddr: " + paddr;
            QMessageBox::information(this, tr("File Offset"), offsetMsg,QMessageBox::Close);
        } else {
            QMessageBox::warning(this, tr("File Offset"), "Invalid address.",QMessageBox::Close);
        }

    } else {
        QMessageBox::warning(this, tr("File Offset"), "No address entered.",QMessageBox::Close);
    }
}


/*
 *  History
*/

// Add location to history and update iterator
void MainWindow::addToHistory(int functionIndex, int lineNum){
    QVector<int> item(2);
    item[0] = functionIndex;
    item[1] = lineNum;

    // Note: constEnd() points to imaginary item after last item
    if (historyIterator != history.constEnd() - 1)
        history = history.mid(0, historyIterator - history.constBegin() + 1);

    history.append(item);
    historyIterator = history.constEnd() - 1;
}

// Back button
void MainWindow::on_backButton_clicked()
{
    if (!history.isEmpty() && historyIterator != history.constBegin()){
        historyIterator--;
        QVector<int> prevLocation = historyIterator.i->t();

        // Display prev function
        setUpdatesEnabled(false);
        if (currentFunctionIndex != prevLocation[0]){
//            displayFunctionText(prevLocation[0]);
//            ui->functionList->setCurrentRow(prevLocation[0]);
        }
        // Go to prev line
        QTextCursor cursor(ui->codeBrowser->document()->findBlockByLineNumber(prevLocation[1]));
        ui->codeBrowser->setTextCursor(cursor);
        ui->disTabWidget->setCurrentIndex(0);
        ui->codeBrowser->setFocus();
        setUpdatesEnabled(true);
    }
}

// Forward button
void MainWindow::on_forwardButton_clicked()
{
    if (!history.isEmpty() && historyIterator != history.constEnd() - 1){
        historyIterator++;
        QVector<int> nextLocation = historyIterator.i->t();

        // Display prev function
        setUpdatesEnabled(false);
        if (currentFunctionIndex != nextLocation[0]){
//            displayFunctionText(nextLocation[0]);
//            ui->functionList->setCurrentRow(nextLocation[0]);
        }
        // Go to prev line
        QTextCursor cursor(ui->codeBrowser->document()->findBlockByLineNumber(nextLocation[1]));
        ui->codeBrowser->setTextCursor(cursor);
        ui->disTabWidget->setCurrentIndex(0);
        ui->codeBrowser->setFocus();
        setUpdatesEnabled(true);
    }
}

void MainWindow::on_actionBack_triggered()
{
    on_backButton_clicked();
}

void MainWindow::on_actionForward_triggered()
{
    on_forwardButton_clicked();
}


/*
 *  Searching
*/

void MainWindow::displayResults(QVector< QVector<QString> > results, QString resultsLabel){
    if (!results.isEmpty()){
        QString resultsStr = "";
        for (int i = 0; i < results.length(); i++){
            QVector<QString> result = results[i];
            if (result.length() == 2)
                resultsStr.append(result[1] + "    " + result[0] + "\n");
        }

        // Display results
        ResultsDialog resultsDialog;
        resultsDialog.setWindowModality(Qt::WindowModal);
        resultsDialog.setResultsLabelText(resultsLabel);
        resultsDialog.setResultsText(resultsStr);
        resultsDialog.exec();
    }
}

// Find calls to the current function
void MainWindow::on_actionFind_Calls_to_Current_Function_triggered()
{
//    QString functionName = disassemblyCore.getFunction(currentFunctionIndex).getName();
//    QVector< QVector<QString> > results = disassemblyCore.findCallsToFunction(functionName);

//    if (!results.isEmpty()){
//        // Display results
//        displayResults(results, "Calls to function " + functionName);

//    } else {
//        QMessageBox::information(this, tr("Calls to Function"), "No calls found to function " + functionName,QMessageBox::Close);
//    }
}

// Find all references to a target location
void MainWindow::findReferencesToLocation(QString target){
//    if (!target.isEmpty()){
//        QVector< QVector<QString> > results = disassemblyCore.findReferences(target);

//        if (!results.isEmpty()){
//            // Display results
//            displayResults(results, "References to " + target);

//        } else {
//            QMessageBox::information(this, tr("References"), "No references found to " + target,QMessageBox::Close);
//        }

//    } else {
//        QMessageBox::warning(this, tr("Search failed"), "Cannot search for empty string.",QMessageBox::Close);
//    }

}

// Find References
void MainWindow::on_actionFind_References_triggered()
{
    bool ok = true;
    QString targetAddress = QInputDialog::getText(this, tr("Find References"),tr("Find References to"), QLineEdit::Normal,"", &ok).trimmed();
    if (ok)
        findReferencesToLocation(targetAddress);
}

// Find all calls to current location
void MainWindow::on_actionFind_Calls_to_Current_Location_triggered(){
//    if (disassemblyCore.disassemblyIsLoaded()){
//        QTextCursor cursor = ui->codeBrowser->textCursor();
//        int lineNum = cursor.blockNumber();
//        QString targetLocation = disassemblyCore.getFunction(currentFunctionIndex).getAddressAt(lineNum);

//        QVector< QVector<QString> > results = disassemblyCore.findReferences(targetLocation);

//        if (!results.isEmpty()){
//            // Display results
//            displayResults(results, "Calls to address " + targetLocation);

//        } else {
//            QMessageBox::information(this, tr("Calls to address"), "No calls found to address " + targetLocation,QMessageBox::Close);
//        }
//    }
}

// Toggle searchbar
void MainWindow::on_actionFind_2_triggered()
{
    if (ui->searchBar->isHidden()){
        ui->searchBar->show();
        ui->findLineEdit->setFocus();
    }else {
        ui->searchBar->hide();
    }
}

// Find and highlight search term in the target widget
void MainWindow::find(QString searchTerm, QPlainTextEdit *targetWidget, bool searchBackwords){
    if (targetWidget != NULL){
        QTextCursor cursor = targetWidget->textCursor();
        int currentPosition = cursor.position();
        bool found = false;

        // Start new search from begining of document
        if (searchTerm != currentSearchTerm){
            if (!searchBackwords)
                cursor.movePosition(QTextCursor::Start);
            else
                cursor.movePosition(QTextCursor::End);

            targetWidget->setTextCursor(cursor);

            currentSearchTerm = searchTerm;
            if (!searchBackwords)
                found = targetWidget->find(searchTerm);
            else
                found = targetWidget->find(searchTerm, QTextDocument::FindBackward);

            // Call vertical scrollbar value changed to keep widgets scrolling synced
            targetWidget->verticalScrollBar()->valueChanged(targetWidget->verticalScrollBar()->value());

            // If not found move cursor back to original position and display not found message
            if(!found){
                cursor.setPosition(currentPosition);
                targetWidget->setTextCursor(cursor);
                QMessageBox::information(this, tr("Not Found"), "\"" + searchTerm + "\" not found.", QMessageBox::Close);
            }

        } else {
            if (!searchBackwords)
                found = targetWidget->find(searchTerm);
            else
                found = targetWidget->find(searchTerm, QTextDocument::FindBackward);

            // Call vertical scrollbar value changed to keep widgets scrolling synced
            targetWidget->verticalScrollBar()->valueChanged(targetWidget->verticalScrollBar()->value());

            // If not found wrap to begining and search again
            if (!found){
                if (!searchBackwords)
                    cursor.movePosition(QTextCursor::Start);
                else
                    cursor.movePosition(QTextCursor::End);

                targetWidget->setTextCursor(cursor);

                if (!searchBackwords)
                    found = targetWidget->find(searchTerm);
                else
                    found = targetWidget->find(searchTerm, QTextDocument::FindBackward);

                // Call vertical scrollbar value changed to keep widgets scrolling synced
                targetWidget->verticalScrollBar()->valueChanged(targetWidget->verticalScrollBar()->value());
                if (!found){
                    cursor.setPosition(currentPosition);
                    targetWidget->setTextCursor(cursor);
                    QMessageBox::information(this, tr("Not Found"), "\"" + searchTerm + "\" not found.", QMessageBox::Close);
                }
            }
        }

    }
}

void MainWindow::on_findButton_clicked()
{
    QString searchTerm = ui->findLineEdit->text();
    int currentTabIndex = ui->disTabWidget->currentIndex();
    QPlainTextEdit *targetWidget = NULL;

    // Set pointer to target widget given current tab index
    switch (currentTabIndex) {
    case 0:
        targetWidget = ui->codeBrowser;
        break;
    case 1:
        targetWidget = ui->hexBrowser;
        break;
    default:
        break;
    }

    find(searchTerm, targetWidget, false);

}

void MainWindow::on_findLineEdit_returnPressed()
{
    on_findButton_clicked();
}

// Find Prev (search backwards)
void MainWindow::on_findPrevButton_clicked()
{
    QString searchTerm = ui->findLineEdit->text();
    int currentTabIndex = ui->disTabWidget->currentIndex();
    QPlainTextEdit *targetWidget = NULL;

    // Set pointer to target widget given current tab index
    switch (currentTabIndex) {
    case 0:
        targetWidget = ui->codeBrowser;
        break;
    case 1:
        targetWidget = ui->hexBrowser;
        break;
    default:
        break;
    }

    find(searchTerm, targetWidget, true);
}

void MainWindow::on_stringsSearchBar_returnPressed()
{
    QString searchTerm = ui->stringsSearchBar->text();
    QPlainTextEdit *stringsBrowser = ui->stringsBrowser;
    find(searchTerm, stringsBrowser, false);
}

/*
 * Themes
*/

// Style central widget
void MainWindow::setCentralWidgetStyle(QString foregroundColor2, QString backgroundColor2){
    QString centralWidgetStyle = "background-color: " + backgroundColor2 + ";"
            "color: " + foregroundColor2 + ";";
    ui->centralWidget->setStyleSheet(centralWidgetStyle);
}

void MainWindow::setMainStyle(QString backgroundColor, QString backgroundColor3){
    QString mainStyle = "QScrollBar:vertical {"
                        "background: "+ backgroundColor3 +";"
                        "width: 10px;"
                    "}"
                    "QScrollBar:horizontal {"
                        "background: " + backgroundColor3 +";"
                        "height: 10px;"
                    "}"
                    "QScrollBar::handle:vertical,QScrollBar::handle:horizontal {"
                        "background-color: " + backgroundColor + ";"
                        "border: 1px solid " + backgroundColor3 + ";"
                        "border-radius: 5px;"
                        "min-height: 25px;"
                     "}"
                     "QScrollBar::add-line:vertical,QScrollBar::add-line:horizontal {"
                          "background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
                          "stop: 0 rgb(32, 47, 130), stop: 0.5 rgb(32, 47, 130),  stop:1 rgb(32, 47, 130));"
                          "height: 0px;"
                          "subcontrol-position: bottom;"
                          "subcontrol-origin: margin;"
                      "}"
                      "QScrollBar::sub-line:vertical,QScrollBar::sub-line:horizontal {"
                            "background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
                            "stop: 0  rgb(32, 47, 130), stop: 0.5 rgb(32, 47, 130),  stop:1 rgb(32, 47, 130));"
                            "height: 0 px;"
                            "subcontrol-position: top;"
                            "subcontrol-origin: margin;"
                      "}";
    ui->main->setStyleSheet(mainStyle);
}

// Style tab widget
void MainWindow::setTabWidgetStyle(QString foregroundColor, QString backgroundColor, QString backgroundColor2, QString addressColor){
    QString style = "#disTab, #hexTab, #pseudoTab {"
                "background-color: " + backgroundColor + ";"
                "color: " + foregroundColor + ";"
            "}"
            "#hexAddressBrowser {"
                "color: " + addressColor + ";"
            "}"
            "QTabBar::tab:selected{"
                "color: #3ba1a1;"
                "border-bottom:2px solid #3ba1a1;"
                "border-top: 1px solid #d4d4d4;"
            "}"
            "QTabBar::tab {"
                "background-color: " + backgroundColor2 +";"
                "border-bottom:2px solid " + backgroundColor2 +";"
                "min-width: 102px;"
                "height: 30px;"
            "}"
            "QTabWidget::tab-bar {"
                "left: 5px;"
            "}"
            "QTabWidget::pane {"
                "border: 1px solid #c0c0c0;"
            "}"
            "QComboBox {"
                "background-color: #fafafa;"
                "color: #555555;"
            "}"
            "QCheckBox {"
                "background-color: " + backgroundColor + ";"
                "color: " + foregroundColor + ";"
            "}"
            "QPlainTextEdit {"
                "background-color: "+ backgroundColor +";"
                "color:"+ foregroundColor +";"
                "border: 0px; selection-background-color: #404f4f;"
                "}"
            "QLabel {"
                "background-color: " + backgroundColor + ";"
                "color: " + foregroundColor + ";"
            "}"
            "QLineEdit {"
                "background-color: " + backgroundColor + ";"
                "color: " + foregroundColor + ";"
            "}";
   ui->disTabWidget->setStyleSheet(style);

}

void MainWindow::setInfoTabWidgetStyle(QString foregroundColor, QString backgroundColor, QString backgroundColor2, QString backgroundColor3){
    QString style = "#relocationsTab, #stringsTab, #infoTab, #importsTab {"
                "background-color: " + backgroundColor + ";"
                "color: " + foregroundColor + ";"
             "}"
             "QTabBar::tab:selected{"
                "border-bottom:2px solid #3ba1a1;"
             "}"
             "QTabBar::tab {"
                 "background-color: " + backgroundColor2 +";"
                 "min-width: 70px;"
             "}"
             "QTabWidget::pane {"
                "border: 1px solid #c0c0c0;"
             "}"
             "QPlainTextEdit {"
                "background-color: "+ backgroundColor +";"
                "color:"+ foregroundColor +";"
                "border: 0px;"
                "selection-background-color: #404f4f;"
                "font-size: 10pt;"
             "}"
             "QLabel {"
                "background-color: " + backgroundColor + ";"
                "color: " + foregroundColor + ";"
                "font-size: 10pt;"
             "}"
             "QLineEdit {"
                "background-color: " + backgroundColor + ";"
                "color: " + foregroundColor + ";"
                "border: 1px solid "+ foregroundColor + ";"
             "}"
             "QScrollBar:vertical {"
                "background: "+ backgroundColor +";"
             "}"
             "QScrollBar:horizontal {"
                "background: "+ backgroundColor +";"
             "}";
    ui->infoTabWidget->setStyleSheet(style);
    fileInfoTableWidget->styleTable(foregroundColor, backgroundColor, backgroundColor3);
    importsTableWidget->styleTable(foregroundColor, backgroundColor, backgroundColor3);
    relocationsTableWidget->styleTable(foregroundColor, backgroundColor, backgroundColor3);
    symbolsTableWidget->styleTable(foregroundColor, backgroundColor, backgroundColor3);
}

void MainWindow::setSidebarStyle(QString foregroundColor, QString backgroundColor, QString backgroundColor3){
    QString sidebarStyle = "#functionList {"
                                "background-color: " + backgroundColor + ";"
                                "color: " + foregroundColor + ";"
                                "font-size: 10pt;"
                                "border: 1px solid #c0c0c0;"
                            "}"
                            "#functionList::item:selected {"
                                "background-color: " + backgroundColor3 + ";"
                                "color: " + foregroundColor + ";"
                            "}";
    ui->sidebar_2->setStyleSheet(sidebarStyle);
}

// Set theme default
void MainWindow::on_actionDefault_triggered()
{
    settings.setValue("theme", "default");

    QString fgc = "#4c4c4c";
    QString bgc = "#e6e6e6";
    QString fgc2 = "#4c4c4c";
    QString bgc2 = "#e0e0e0";
    QString addrc = "#268BD2";
    QString bgc3 = "#d7d7d7";

    setCentralWidgetStyle(fgc2, bgc2);
    setMainStyle(bgc, bgc3);
    setTabWidgetStyle(fgc, bgc, bgc2, addrc);
    setInfoTabWidgetStyle(fgc, bgc, bgc2, bgc3);
    setSidebarStyle(fgc, bgc, bgc3);

    disHighlighter->setTheme("Default");

    lineColor = QColor(215,215,215);
    highlightCurrentLine();
}

// Set dark theme
void MainWindow::on_actionDark_triggered()
{
    settings.setValue("theme", "dark");

    QString fgc = "#fafafa";
    QString bgc = "#333333";
    QString fgc2 = "#4c4c4c";
    QString bgc2 = "#e0e0e0";
    QString addrc = "#268BD2";
    QString bgc3 = "#414141";

    setCentralWidgetStyle(fgc2, bgc2);
    setMainStyle(bgc, bgc3);
    setTabWidgetStyle(fgc, bgc, bgc2, addrc);
    setInfoTabWidgetStyle(fgc, bgc, bgc2, bgc3);
    setSidebarStyle(fgc, bgc, bgc3);

    disHighlighter->setTheme("Default");

    lineColor = QColor(65,65,65);
    highlightCurrentLine();
}

// Set theme solarized
void MainWindow::on_actionSolarized_triggered()
{
    settings.setValue("theme", "solarized");

    QString fgc = "#839496";
    QString bgc = "#fdf6e3";
    QString fgc2 = "#4c4c4c";
    QString bgc2 = "#e0e0e0";
    QString addrc = "#268BD2";
    QString bgc3 = "#eee8d5";

    setCentralWidgetStyle(fgc2, bgc2);
    setMainStyle(bgc, bgc3);
    setTabWidgetStyle(fgc, bgc, bgc2, addrc);
    setInfoTabWidgetStyle(fgc, bgc, bgc2, bgc3);
    setSidebarStyle(fgc, bgc, bgc3);

    disHighlighter->setTheme("solarized");

    lineColor = QColor(238, 232, 213);
    highlightCurrentLine();
}

// Set theme solarized dark
void MainWindow::on_actionSolarized_Dark_triggered()
{
    settings.setValue("theme", "solarizedDark");

    QString fgc = "#839496";
    QString bgc = "#002b36";
    QString fgc2 = "#4c4c4c";
    QString bgc2 = "#e0e0e0";
    QString addrc = "#268BD2";
    QString bgc3 = "#073642";

    setCentralWidgetStyle(fgc2, bgc2);
    setMainStyle(bgc, bgc3);
    setTabWidgetStyle(fgc, bgc, bgc2, addrc);
    setInfoTabWidgetStyle(fgc, bgc, bgc2, bgc3);
    setSidebarStyle(fgc, bgc, bgc3);

    disHighlighter->setTheme("solarized");

    lineColor = QColor(7, 54, 66);
    highlightCurrentLine();
}

/*
 *  Window
*/

void MainWindow::on_actionProject_triggered()
{
    QString aboutStr = "Ronin - GUI frontend to radare2 reverse engineering framework.\n\n"
                       "Project Page: https://github.com/jubal-r/ronin\n\n"

                       "Copyright (C) 2017\n"

                       "License: GNU Lesser General Public License(LGPL) Version 3\n";
    QMessageBox::information(this, tr("About Ronin"), aboutStr,QMessageBox::Close);
}

void MainWindow::on_actionExit_triggered()
{
    QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Exit", "Are you sure you want to exit?", QMessageBox::Yes|QMessageBox::No);

        if (reply == QMessageBox::Yes) {
          QApplication::quit();
        }
}

void MainWindow::on_actionFullscreen_triggered()
{
    if(MainWindow::isFullScreen())
        {
            MainWindow::showNormal();
        }else{
            MainWindow::showFullScreen();
        }
}

