#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QListWidgetItem"
#include "QPlainTextEdit"
#include "QSettings"

#include "ronincore.h"
#include "files.h"
#include "highlighters/disassemblyhighlighter.h"
#include "highlighters/pseudocodehighlighter.h"

#include "widgets/symbolstablewidget.h"
#include "widgets/relocationstable.h"
#include "widgets/importstablewidget.h"
#include "widgets/fileinfotablewidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void highlightCurrentLine();
    void loadBinary(QString file);
    void displayFunctionData(QString functionName);
    void populateFunctionsList();
    void displayHexData();
    void clearUi();
    void enableMenuItems();
    void displayResults(QVector< QVector<QString> > results, QString resultsLabel);
    void setStatusBarLabelValues();
    void find(QString searchTerm, QPlainTextEdit *targetWidget, bool searchBackwords);

    void setCentralWidgetStyle(QString foregroundColor, QString backgroundColor);
    void setMainStyle(QString backgroundColor2, QString backgroundColor3);
    void setTabWidgetStyle(QString foregroundColor, QString backgroundColor, QString backgroundColor2, QString addressColor);
    void setInfoTabWidgetStyle(QString foregroundColor, QString backgroundColor, QString backgroundColor2, QString backgroundColor3);
    void setSidebarStyle(QString foregroundColor, QString backgroundColor, QString backgroundColor3);

    void on_actionOpen_triggered();

    void on_actionProject_triggered();

    void on_actionExit_triggered();

    void on_actionFullscreen_triggered();

    void on_functionList_itemDoubleClicked(QListWidgetItem *item);

    void on_actionGet_Offset_triggered();

    void on_backButton_clicked();

    void on_forwardButton_clicked();

    void on_actionBack_triggered();

    void on_actionForward_triggered();

    void on_actionFind_2_triggered();

    void on_findButton_clicked();

    void on_findLineEdit_returnPressed();

    void on_findPrevButton_clicked();

    void on_actionDefault_triggered();

    void on_actionSolarized_Dark_triggered();

    void on_actionSolarized_triggered();

    void on_actionDark_triggered();

    void on_stringsSearchBar_returnPressed();

private:
    Ui::MainWindow *ui;
    RoninCore roninCore;
    Files files;
    QSettings settings;

    SymbolsTableWidget *symbolsTableWidget;
    RelocationsTable *relocationsTableWidget;
    ImportsTableWidget *importsTableWidget;
    FileInfoTableWidget *fileInfoTableWidget;

    DisassemblyHighlighter *disHighlighter;
    DisassemblyHighlighter *graphHighlighter;
    PseudoCodeHighlighter *pseudoHighlighter;

    QList< QVector<int> > history;
    QList< QVector<int> >::const_iterator historyIterator;
    QString currentSearchTerm;
    QColor lineColor;
};

#endif // MAINWINDOW_H
