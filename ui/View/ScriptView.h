#pragma once

#include "Main.h"

class QSHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    QSHighlighter(QTextDocument *parent = nullptr) : QSyntaxHighlighter(parent) {

        QTextCharFormat format;
        format.setFontFamily("Consolas");
        format.setForeground(QColor("yellow"));
        rules[STRING] = format;
        format.setForeground(QColor("grey"));
        rules[NOTATION] = format;
        format.setForeground(QColor("white"));
        rules[SYMBOL] = format;
        format.setForeground(QColor("lightblue"));
        rules[BRACE] = format;
        format.setForeground(QColor("lime"));
        rules[FUNCTION] = format;
        format.setForeground(QColor("tomato"));
        rules[KEYWORD] = format;
        format.setForeground(QColor("orange"));
        rules[NUMBER] = format;
        format.setForeground(QColor("lime"));
        rules[VARIABLE] = format;
        format.setForeground(QColor("turquoise"));
        format.setFontItalic(true);
        rules[TYPE] = format;
    }
protected:
    void highlightBlock(const QString &text) override {
        Parser parser;
        QStringList q_qs_lines = text.split('\n');
        vec<str> qs_lines;
        for(const auto& str : q_qs_lines) {
            qs_lines.push_back(str.toStdString());
        }
        parser.parse(qs_lines); // 注意，这里要区分text和path！
        for(auto &token : parser.tokens) {
            setFormat(token.pos, token.size, rules[token.type]);
        }
    }
private:
    QTextCharFormat rules[114]; //类型个数
};

class ZoomableTextEdit : public QTextEdit {
    Q_OBJECT
public:
    ZoomableTextEdit(QWidget *parent = nullptr) : QTextEdit(parent) {
        setFontSize(12);
    }
    void wheelEvent(QWheelEvent *event) override {
        // 检查是否按住Ctrl键，没有按住时是上下滑动，按住时是放缩
        if(event->modifiers() & Qt::ControlModifier) {
            // 获取当前视口中心位置对应的文本光标
            // viewport是当前textedit的视窗部件，rect即获取矩形区域
            // 所以centerPos是视窗中心点
            QPoint centerPos = viewport()->rect().center();
            // 根据视窗中心坐标产生一个文本光标位置
            QTextCursor cursor = cursorForPosition(centerPos);
            // 记录光标所在的块号和列号。块通常是段落，列就是在当前行的位置
            int originalBlock = cursor.blockNumber();
            int originalColumn = cursor.columnNumber();

            auto curFontSize = font().pointSize();
            // 缩放
            event->angleDelta().y() > 0 ? setFontSize(curFontSize + 1) : setFontSize(std::max(curFontSize - 1, 0));

            // 重新定位光标到原来的位置
            // 根据块号在当前textedit的document对象中找到对应的块，然后在该块起始处产生光标
            QTextCursor newCursor(document()->findBlockByNumber(originalBlock));
            // 加上列数，使得光标到达当前的视窗中心
            newCursor.setPosition(newCursor.position() + originalColumn);
            // 也可以不改变光标位置，让它在原来的位置
            // setTextCursor(newCursor);

            // 计算并设置视口滚动位置，使得原来的中心位置仍在中心
            // 获取newCursor所在文本对应的矩形区域，的中心位置
            QPoint newCenterPos = cursorRect(newCursor).center();
            // 获取现在的视窗
            QRect viewportRect = viewport()->rect();
            // 计算要把newCursor所在文本矩形区域的中心滚动到中央位置需要的offset
            // 感觉可以忽略Width上的滚动？
            // int offsetX = viewportRect.width() / 2 - newCenterPos.x();
            int offsetY = viewportRect.height() / 2 - newCenterPos.y();
            // ensureCursorVisible();
            verticalScrollBar()->setValue(verticalScrollBar()->value() - offsetY);
            // horizontalScrollBar()->setValue(horizontalScrollBar()->value() - offsetX);
            // 如果按住了shift
        } else if(event->modifiers() & Qt::ShiftModifier) {
            QScrollBar *hScrollbar = horizontalScrollBar();
            // 上滚左滑，下滚右滑
            hScrollbar->setValue(hScrollbar->value() + 4 * hScrollbar->singleStep() * (event->angleDelta().y() > 0 ? -1 : 1));
        } else {
            QTextEdit::wheelEvent(event);
        }
    }
private:
    void setFontSize(int size) {
        QFont font = this->font();
        font.setPointSize(size);
        this->setFont(font);
        // 更新样式表以确保字体大小生效
        this->setStyleSheet(QString("QTextEdit { font-size: %1pt; }").arg(size));
    }
};

class ScriptView : public QWidget {
    Q_OBJECT
public:
    ScriptView(QWidget *parent = nullptr) : QWidget(parent) {
        textEdit = new ZoomableTextEdit(this);
        textEdit->setObjectName("textEdit");
        // 行号比较难写，直接不换行
        textEdit->setLineWrapMode(QTextEdit::NoWrap);
        // 作用：让子组件充满父容器
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(textEdit);
        textEdit->setContentsMargins(0, 0, 0, 0);
        // 与父窗口关联则自动set，不用自己写
        // setLayout(layout);
        
        layout->setContentsMargins(10, 0, 10, 10);
        layout->setSpacing(0);
        connect(textEdit, &QTextEdit::textChanged, this, &ScriptView::textChanged);

        QSHighlighter *highlighter = new QSHighlighter(textEdit->document());
    }
signals:
    void textChanged();
    void textSaved();
public slots:
    void openFile(const QString &fileName, const QString &_filePath) {
        filePath = _filePath;
        QFile file(filePath);
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            textEdit->setPlainText(in.readAll());
            file.close();
        } else QMessageBox::warning(this, tr("Error"), tr("Unable to open file"));
    }
    void saveFile() {
        QFile file(filePath);
        if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << textEdit->toPlainText();
            file.close();
            emit textSaved();
        } else QMessageBox::warning(this, tr("Error"), tr("Unable to save file"));
    }
    str getText() {
        return textEdit->toPlainText().toStdString();
    }
private:
    QString filePath;
    ZoomableTextEdit *textEdit;
};