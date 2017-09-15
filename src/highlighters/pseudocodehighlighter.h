#ifndef PSEUDOCODEHIGHLIGHTER_H
#define PSEUDOCODEHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class QTextDocument;

class PseudoCodeHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    PseudoCodeHighlighter(QTextDocument *parent = 0, QString theme = "Default");
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
    QTextCharFormat numbersFormat;
    QTextCharFormat callFormat;
    QTextCharFormat controlFlowFormat;
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

#endif // PSEUDOCODEHIGHLIGHTER_H
