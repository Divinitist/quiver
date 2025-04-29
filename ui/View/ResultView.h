#pragma once

#include "Main.h"
#include "View/ScriptView.h"
#include "View/CircuitView.h"

class ChartScene : public QGraphicsScene {
public:
    ChartScene(const vec<f64>& _values, int totalHeight, int totalWidth, QMap<str, i32>& params, QObject *parent = nullptr) : QGraphicsScene(parent) {
        f64 maxValue = 0.0;
        for (auto i = 0; i < _values.size(); ++i) {
            // if (_values[i] < 1e-6) continue; // 去除0数据
            values.push_back(_values[i]);
            maxValue = std::max(maxValue, _values[i]);
        }
        int dotSize = params["dotSize"];
        int margin = params["chartMargin"];
        // 这个w和h是直径
        QGraphicsEllipseItem *unitCircle = new QGraphicsEllipseItem(-100, -100, 200, 200);
        QPen circlePen(Qt::black);
        circlePen.setWidthF(0.3);
        unitCircle->setPen(circlePen);
        unitCircle->setBrush(Qt::white);
        addItem(unitCircle);
        for (auto i = 0; i < values.size(); ++i) {
            // int y = margin;
            // printf("i = %d, value = %lf, x = %d, y = %d, barHeight = %d\n", i, values[i], x, y, barHeight);

            // qreal x = (f64) i - dotSize / 2;
            // qreal y = totalHeight - ((values[i] * (f64)totalHeight) - dotSize / 2);

            // 对数化，免得缩得太小（爆了，飞到圆外面去了（变成负数了））
            // 开2次根效果不错，试试开4次根
            f64 idx = 1.0 / (f64)params["sqrtFactor"];
            qreal x = std::pow(values[i], idx) * 100.0 * std::cos((f64) i * 2.0 * M_PI / (f64) values.size()) - dotSize / 2.0;
            qreal y = std::pow(values[i], idx) * 100.0 * std::sin((f64) i * 2.0 * M_PI / (f64) values.size()) - dotSize / 2.0;

            QGraphicsEllipseItem *dot = new QGraphicsEllipseItem(x, y, dotSize, dotSize);
            QPen coral(QColor("Coral"));
            coral.setWidthF(0.1);
            dot->setPen(coral);
            QColor dotColor = QColor("orange");
            dotColor.setAlphaF(std::pow(255.0 / (f64)values.size(), 0.25));
            dot->setBrush(dotColor);
            addItem(dot);
            dots.push_back(dot);
        }
    }
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override {
        u32 bitnum = 0;
        while ((1u << bitnum) < values.size()) ++bitnum;
        for (int i = 0; i < dots.size(); ++i) {
            if (dots[i]->contains(event->scenePos())) {
                // QMessageBox::information(nullptr, "提示", std::format("第{}个点被悬浮", i).c_str());
                QToolTip::showText(event->screenPos(), QString::fromStdString(std::format("|{}〉：{}", u32_to_bin(i, bitnum), values[i])));
                return;
            }
        }
        QToolTip::hideText();
    }
private:
    vec<f64> values;
    // vec<i32> values;
    vec<QGraphicsEllipseItem*> dots;
};

class ResultView : public QWidget {
    Q_OBJECT
public:
    str preStdFileName = "";
    void readParams() {
        QFile cfg("../ui/View/ResultView.cfg");
        if (cfg.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&cfg);
        while (!in.atEnd()) {
            str param = in.readLine().toStdString();
            auto pos = param.find_first_of(' ');
            params[param.substr(0, pos)] = atoi(param.substr(pos + 1).c_str());
        }
        } else QMessageBox::warning(this, "错误", "打开../ui/View/ResultView.cfg失败");
    }
    ResultView(QWidget *parent = nullptr) : QWidget(parent) {
        setContentsMargins(0, 0, 0, 0);
        QHBoxLayout *layout = new QHBoxLayout(this);
        // 同样用stackedWidget装
        digitGraphStack = new QStackedWidget(this);
        digitResult = new ZoomableTextEdit(this);
        graphResult = new MovableGraphView(this);

        // 只准读
        digitResult->setReadOnly(true);

        // 让stack充满父容器
        layout->addWidget(digitGraphStack);
        digitGraphStack->addWidget(digitResult);
        digitGraphStack->addWidget(graphResult);

        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        digitGraphStack->setContentsMargins(0, 0, 0, 0);
    }
protected:
    void resizeEvent(QResizeEvent *event) override {
        if (preStdFileName != "") {
            showGraphResult(preStdFileName);   
        }
    }
public slots:
    void showDigitResult(str stdFileName) {
        digitGraphStack->setCurrentIndex(0);
        PM::send_cmd("digit");
        PM::wait_signal("/tmp/quiver/digit_data_ready");
        str path = std::format("../qs_cpp/{}.ddt", stdFileName);
        QFile digitResultFile(QString(path.c_str()));
        if (digitResultFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&digitResultFile);
            digitResult->setPlainText(in.readAll());
            digitResultFile.close();
        } else QMessageBox::warning(this, "错误", std::format("打开{}错误", path).c_str());
    }
    void showGraphResult(str stdFileName) {
        digitGraphStack->setCurrentIndex(1);
        PM::send_cmd("graph");
        PM::wait_signal("/tmp/quiver/graph_data_ready");
        str path = std::format("../qs_cpp/{}.gdt", stdFileName);
        QFile graphResultFile(QString(path.c_str()));
        vec<f64> data;
        if (graphResultFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&graphResultFile);
            while (!in.atEnd()) {
                str val = in.readLine().toStdString();
                data.push_back(std::stod(val));
            }
            graphResultFile.close();
            printf("height = %d, width = %d\n", height(), width());
            readParams();
            chartScene = new ChartScene(data, height(), width(), params);
            graphResult->setScene(chartScene);
            graphResult->fitInView(chartScene->itemsBoundingRect(), Qt::KeepAspectRatio);
        } else QMessageBox::warning(this, "错误", std::format("打开{}失败", path).c_str());
        preStdFileName = stdFileName;
    }
private:
    QStackedWidget *digitGraphStack;
    ZoomableTextEdit *digitResult;
    ChartScene *chartScene;
    MovableGraphView *graphResult;
    QMap<str, i32> params;
};