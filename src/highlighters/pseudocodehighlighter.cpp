#include "pseudocodehighlighter.h"

PseudoCodeHighlighter::PseudoCodeHighlighter(QTextDocument *parent, QString theme)
    : QSyntaxHighlighter(parent)
{
    setupHighlighter(theme);
}

void PseudoCodeHighlighter::setupHighlighter(QString theme){
    HighlightingRule rule;

    if (theme == "solarized"){
        registerColor = QColor(42, 161, 152);
        jumpColor = QColor(181, 137, 0);
        callColor = QColor(133, 153, 0);
        numColor = QColor(38, 139, 210);
        logicColor = QColor(108,113,196);
        stringColor = QColor(42, 161, 152);
        commentColor = QColor(178, 179, 191);
    } else {
        registerColor = QColor(29, 160, 185);
        jumpColor = QColor(249, 38, 114);
        callColor = QColor(79, 153, 0);
        numColor = QColor(38, 139, 210);
        logicColor = QColor(174,129,255);
        stringColor = QColor(230,219,116);
        commentColor = QColor(126,142,145);
    }

    // Numbers
    numbersFormat.setForeground(numColor);
    rule.pattern = QRegExp("\\b(0x)?[0-9a-f\\.]+\\b");
    rule.format = numbersFormat;
    highlightingRules.append(rule);

    // Control Flow
    controlFlowFormat.setForeground(jumpColor);
    QStringList logicPatterns;
    logicPatterns << "\\bx?if\\b"
                     << "\\bwhile\\b"
                     << "\\bgoto\\b"
                     << "\\bdo\\b"
                     << "\\breturn\\b";

    foreach (const QString &pattern, logicPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = controlFlowFormat;
        highlightingRules.append(rule);
    }

    // Calls
    callFormat.setForeground(callColor);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = callFormat;
    highlightingRules.append(rule);

    //Comments
    commentFormat.setForeground(commentColor);
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = commentFormat;
    highlightingRules.append(rule);

    // Local
    localFormat.setForeground(numColor);
    rule.pattern = QRegExp("local_[a-zA-Z0-9@_-\\+]+");
    rule.format = localFormat;
    highlightingRules.append(rule);

    // Strings
    stringFormat.setForeground(stringColor);
    rule.pattern = QRegExp("str\\.[a-zA-Z0-9@_-\\+]+");
    rule.format = stringFormat;
    highlightingRules.append(rule);

    // x86_64 Registers
    registerFormat.setForeground(registerColor);
    registerFormat.setFontWeight(QFont::Bold);
    QStringList registerPatterns;
    registerPatterns << "\\b[re]?[sabcdi][xhlpi]l?\\b"
                     << "\\br1?[890-5][dwb]?\\b";

    foreach (const QString &pattern, registerPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = registerFormat;
        highlightingRules.append(rule);
    }
}

void PseudoCodeHighlighter::setTheme(QString theme){
    setupHighlighter(theme);
    rehighlight();
}

void PseudoCodeHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);

}
