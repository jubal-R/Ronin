#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>

class QTextDocument;

class DisassemblyHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    DisassemblyHighlighter(QTextDocument *parent = 0, QString theme = "Default");
    void setTheme(QString theme);

protected:
    void highlightBlock(const QString &text);

private:
    void setupHighlighter(QString theme);

    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat registerFormat;
    QTextCharFormat localFormat;
    QTextCharFormat jumpFormat;
    QTextCharFormat addressFormat;
    QTextCharFormat callFormat;
    QTextCharFormat logicFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat commentFormat;

    QColor registerColor;
    QColor jumpColor;
    QColor callColor;
    QColor logicColor;
    QColor numColor;
    QColor stringColor;
    QColor commentColor;
    QColor varColor;
};

#endif // HIGHLIGHTER_H
