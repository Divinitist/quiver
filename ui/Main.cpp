#include "Main.h"
#include "View/View.h"

template <typename T>
void setStretches(T *widget, const vec<i32> &stretches) {
    for (i32 i = 0; i < stretches.size(); ++i) {
        widget->setStretch(i, stretches[i]);
    }
}

template <>
void setStretches(QSplitter *splitter, const vec<i32> &stretches) {
    for (i32 i = 0; i < stretches.size(); ++i) {
        splitter->setStretchFactor(i, stretches[i]);
    }
}

int main(int argc, char *argv[]) {
    system("ulimit -s 10485760");
    QApplication app(argc, argv);
    View view;
    QFile styleFile("../ui/View/Style.css");
    if(styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&styleFile);
        QString styleSheet = stream.readAll();
        // qDebug() << "style sheet:" << styleSheet;
        // app.setStyleSheet(styleSheet);
        view.setStyleSheet(styleSheet);
        styleFile.close();
    } else qDebug() << styleFile.errorString();
    // qDebug() << view.styleSheet();
    view.show();
    return app.exec();
}