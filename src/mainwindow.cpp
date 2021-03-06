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
     *  Setup built-in fonts
    */

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

    int sansBoldId = QFontDatabase::addApplicationFont(":/fonts/NotoSans-Bold.ttf");
    QString sansBoldFamily = QFontDatabase::applicationFontFamilies(sansBoldId).at(0);
    QFont sansBold(sansBoldFamily);
    sansBold.setPointSize(11);
    sansBold.setBold(true);

    ui->functionLabel->setFont(sansBold);
    ui->stringsAddressLabel->setFont(sansBold);
    ui->stringsLabel->setFont(sansBold);

    int monoid = QFontDatabase::addApplicationFont(":/fonts/Anonymous Pro.ttf");
    QString monofamily = QFontDatabase::applicationFontFamilies(monoid).at(0);
    QFont mono(monofamily);
    mono.setPointSize(12);

    ui->codeBrowser->setFont(mono);
    ui->pseudoCodeBrowser->setFont(mono);
    ui->graphBrowser->setFont(mono);
    ui->hexBrowser->setFont(mono);

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
    graphHighlighter = new DisassemblyHighlighter(ui->graphBrowser->document(), "Default");
    pseudoHighlighter = new PseudoCodeHighlighter(ui->pseudoCodeBrowser->document(), "Default");

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

}

MainWindow::~MainWindow()
{
    QRect windowRect = MainWindow::normalGeometry();
    settings.setValue("windowWidth", windowRect.width());
    settings.setValue("windowHeight", windowRect.height());
    settings.setValue("splitterSizes", ui->splitter->saveState());

    delete ui;
}


/*
 *  Display Data
*/

void MainWindow::loadBinary(QString file){

    if (file != ""){
        this->setWindowTitle("Ronin - " + file);

        clearUi();

        QProgressDialog progress("Loading File", "", 0, 4, this);
        progress.setCancelButton(0);
        progress.setWindowModality(Qt::WindowModal);
        progress.setMinimumDuration(500);
        progress.setValue(0);

        // Disassemble in seperate thread
        QFuture<void> loadFileThread = QtConcurrent::run(&roninCore, &RoninCore::loadFile, file);

        while (!loadFileThread.isFinished()){
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        }

        progress.setValue(1);

        if (roninCore.fileIsLoaded()){
            populateFunctionsList();

            // Display entrypoint on file load
            if (roninCore.functionExists("entry0")){
                displayFunctionData("entry0");
            }

            enableMenuItems();

            progress.setValue(2);

            displayHexData();

            progress.setValue(3);

            setUpdatesEnabled(false);
            relocationsTableWidget->insertRelocations(roninCore.getRelocations());
            symbolsTableWidget->insertSymbols(roninCore.getSymbols());
            importsTableWidget->insertImports(roninCore.getImports());
            fileInfoTableWidget->insertInfo(roninCore.getFileInfo());
            setStatusBarLabelValues();

            QStringList strOutputList = roninCore.getStrings();
            ui->stringsAddressBrowser->setPlainText(strOutputList.at(0));
            ui->stringsBrowser->setPlainText(strOutputList.at(1));
            setUpdatesEnabled(true);

            progress.setValue(4);
        }
    }
}

// Disassemble
void MainWindow::on_actionOpen_triggered()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Open File"), files.getCurrentDirectory(), tr("All (*)"));

    if (file != ""){
        files.setCurrentDirectory(file);
        loadBinary(file);
    }

}

void MainWindow::updateDisassemblyViews(){
    setUpdatesEnabled(false);
    ui->functionLabel->clear();
    ui->codeBrowser->setPlainText(roninCore.getDisassembly());
    ui->pseudoCodeBrowser->setPlainText(roninCore.getPseudoCode());
    ui->graphBrowser->setPlainText(roninCore.getFunctionGraph());
    ui->hexBrowser->setPlainText(roninCore.getHexDump());
    setUpdatesEnabled(true);
}

void MainWindow::displayFunctionData(QString functionName){
    if (roninCore.fileIsLoaded()){
        roninCore.seekTo(functionName);
        updateSeekAddressLabel();

        setUpdatesEnabled(false);
        ui->functionLabel->setText(functionName);
        ui->codeBrowser->setPlainText(roninCore.getFunctionDisassembly(functionName));
        ui->pseudoCodeBrowser->setPlainText(roninCore.getPseudoCode(functionName));
        ui->graphBrowser->setPlainText(roninCore.getFunctionGraph(functionName));
        ui->hexBrowser->setPlainText(roninCore.getFunctionHexDump(functionName));
        setUpdatesEnabled(true);

    }
}

void MainWindow::populateFunctionsList(){
    if (roninCore.fileIsLoaded()){
        ui->functionList->addItems(roninCore.getFunctionNames());
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
    setUpdatesEnabled(false);
    ui->hexBrowser->setPlainText(roninCore.getHexDump());
    setUpdatesEnabled(true);
}

void MainWindow::setStatusBarLabelValues(){
    QVector<QStringList> fileInfo = roninCore.getFileInfo();
    if (fileInfo.length() >= 4){
        ui->archValueLabel->setText(fileInfo[0].at(1));
        ui->bitsValueLabel->setText(fileInfo[3].at(1));
        ui->sizeValueLabel->setText(fileInfo[1].at(1));
        ui->formatValueLabel->setText(fileInfo[2].at(1));
    }
}

void MainWindow::updateSeekAddressLabel(){
    QString seekAddress = roninCore.getCurrentSeekAddress();
    ui->seekAddressLabel->setText(seekAddress);
}

void MainWindow::clearUi(){
    while (ui->functionList->count() > 0){
        ui->functionList->takeItem(0);
    }
    ui->functionLabel->clear();
    ui->codeBrowser->clear();
    ui->pseudoCodeBrowser->clear();
    ui->hexBrowser->clear();
    ui->stringsAddressBrowser->clear();
    ui->stringsBrowser->clear();

    // Clear history
    history.clear();
}

// To enable navigation and tools once a file is loaded
void MainWindow::enableMenuItems(){
    ui->actionGet_Offset->setEnabled(true);
}

/*
 *  Navigation
*/

void MainWindow::on_actionSeek_triggered()
{
    bool ok;
    QString seekLocation = QInputDialog::getText(this, tr("Seek"),tr("Seek to location"), QLineEdit::Normal,"", &ok).trimmed();

    if (ok && !seekLocation.isEmpty()){
        roninCore.seekTo(seekLocation);
        updateSeekAddressLabel();
        updateDisassemblyViews();
    }
}

// Display function when double clicked in sidebar
void MainWindow::on_functionList_itemDoubleClicked(QListWidgetItem *item)
{
    QString functionName = item->text();
    displayFunctionData(functionName);
}

// Prompt for address or function and return its offset(physical address)
void MainWindow::on_actionGet_Offset_triggered()
{
    bool ok;
    QString virtualAddress = QInputDialog::getText(this, tr("Get File Offset"),tr("Get file offset of address or function"), QLineEdit::Normal,"", &ok).trimmed();
    if (ok && !virtualAddress.isEmpty()){
        QString paddr = roninCore.getPaddr(virtualAddress);

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

// Back button
void MainWindow::on_backButton_clicked()
{
    roninCore.undoSeek();
    updateSeekAddressLabel();
    updateDisassemblyViews();

}

// Forward button
void MainWindow::on_forwardButton_clicked()
{
    roninCore.redoSeek();
    updateSeekAddressLabel();
    updateDisassemblyViews();
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
        targetWidget = ui->graphBrowser;
        break;
    case 2:
        targetWidget = ui->pseudoCodeBrowser;
        break;
    case 3:
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
        targetWidget = ui->graphBrowser;
        break;
    case 2:
        targetWidget = ui->pseudoCodeBrowser;
        break;
    case 3:
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

// Main styling
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

// Style primary tab widget
void MainWindow::setTabWidgetStyle(QString foregroundColor, QString backgroundColor, QString backgroundColor2, QString addressColor){
    QString style = "#disTab, #hexTab, #pseudoTab, #graphTab {"
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
    pseudoHighlighter->setTheme("Default");

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
    pseudoHighlighter->setTheme("Default");

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
    pseudoHighlighter->setTheme("solarized");

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
    pseudoHighlighter->setTheme("solarized");

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

