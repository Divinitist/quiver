#pragma once

#include "Main.h"

class View;

class QuantumLineItem : public QGraphicsItem {
public:
    using coorType = qreal;
    QuantumLineItem(coorType x, coorType y, str text, qreal lineLength) : x(x), y(y), text(text), lineLength(lineLength) {}
    QRectF boundingRect() const override {
        return QRectF(x - 10, y - 10, text.length() * 8 + lineLength, 20);
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        QFont painterFont("Consolas", 12);
        painter->setFont(painterFont);
        painter->drawText(x, y, QString(text.c_str()));
        painter->drawLine(x + text.length() * 8, y - 5, x + text.length() * 8 + lineLength, y - 5);
    }
private:
    coorType x, y;
    str text;
    qreal lineLength;
};

class QuantumGateItem : public QGraphicsItem {
public:
    // 这个是必写的，因为需要重载以下两个函数
    QuantumGateItem(qreal columnCenterX, str name, vec<u32> &ctrls, vec<u32> &idxs) 
        : columnCenterX(columnCenterX), name(name), ctrls(ctrls), idxs(idxs) {}
    QRectF boundingRect() const override {
        // TODO: 计算占地尺寸
        return QRect();
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        // TODO: 分辨类型，先画块，再画控制
    }
private:
    qreal columnCenterX;
    str name;
    vec<u32> &ctrls;
    vec<u32> &idxs;
};

class CircuitScene : public QGraphicsScene {
public:
    CircuitScene(Circuit &_circuit, QMap<str, i32>& params,QObject *parent = nullptr) : QGraphicsScene(parent), circuit(_circuit) {
        printf("[CircuitScene] init_val = %u\n", circuit.init_val);
        auto rowHeight = params["rowHeight"];
        for (auto i = 0; i < circuit.bitnum; ++i) {
            QuantumLineItem *line = new QuantumLineItem(0, i * rowHeight, std::format("{:02d} |{:d}〉", i, (circuit.init_val & (1u << (circuit.bitnum - 1 - i)) ? 1 : 0)), 1000);
            addItem(line);
            lines.push_back(line);
        }
        auto columnWidth = params["columnWidth"];
        auto &stages = circuit.stages;
        auto columnMask = 0u;
        for (auto i = 0; i < stages.size(); ++i) {
            auto &stage = stages[i];
            for (auto j = 0; j < stage.gates.size(); ++j) {
                auto &gate = stage.gates[j];
                // 应给出的信息：列宽，列号，名称，控制比特集，受控比特集，在内部处理。
                QuantumGateItem *gateItem = new QuantumGateItem(columnCenterX, gate.name, gate.ctrls, gate.idxs);
            }
        }
    }
private:
    Circuit &circuit;
    vec<QuantumLineItem*> lines;
    QMap<Gate*, QuantumGateItem*> gateMap;
};

class MovableGraphView : public QGraphicsView {
    Q_OBJECT
public:
    MovableGraphView(QWidget *parent = nullptr) : QGraphicsView(parent) {}
    MovableGraphView(QGraphicsScene *scene, QWidget *parent = nullptr) : QGraphicsView(scene, parent) {}
protected:
    // 右键按下，开始拖动
    void mousePressEvent(QMouseEvent *event) override {
        if(event->button() == Qt::RightButton) {
            isPanning = true;
            lastPos = event->pos();
            // 还能改变光标形状
            setCursor(Qt::ClosedHandCursor);
        }
        QGraphicsView::mousePressEvent(event);
    }
    // 右键松开，停止拖动
    void mouseReleaseEvent(QMouseEvent *event) override {
        if(event->button() == Qt::RightButton) {
            isPanning = false;
            lastPos = event->pos();
            setCursor(Qt::ArrowCursor);
        }
        QGraphicsView::mouseReleaseEvent(event);
    }
    void mouseMoveEvent(QMouseEvent *event) override {
        if(isPanning) {
            // 获取位移差
            QPointF delta = QPointF(event->pos()) - lastPos;
            // 这个是啥？
            setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
            translate(-delta.x(), -delta.y());
            // 更新最终坐标
            lastPos = event->pos();
        }
        QGraphicsView::mouseMoveEvent(event);
    }
    void wheelEvent(QWheelEvent *event) override {
        // 指数比例缩放
        double scaleFactor = std::pow(1.0015, event->angleDelta().y());
        // x和y方向均缩放
        scale(scaleFactor, scaleFactor);
    }
private:
    bool isPanning = false;
    QPoint lastPos;
};

class CircuitView : public QWidget {
    Q_OBJECT
public:
    void readParams() {
        QFile cfg("../ui/View/CircuitView.cfg");
        if (cfg.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&cfg);
        while (!in.atEnd()) {
            str param = in.readLine().toStdString();
            auto pos = param.find_first_of(' ');
            params[param.substr(0, pos)] = atoi(param.substr(pos + 1).c_str());
        }
        } else QMessageBox::warning(this, "错误", "打开../ui/View/CircuitView.cfg失败");
    }
    CircuitView(QWidget *parent = nullptr) : QWidget(parent) {
        readParams();
        QVBoxLayout *layout = new QVBoxLayout(this);
        graph = new MovableGraphView(this);
        layout->addWidget(graph);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
    }
protected:
    void resizeEvent(QResizeEvent *event) override {
        showCircuit();
    }
public slots:
    void runGate() {
        // QMessageBox::information(this, "提示", __FUNCTION__);
        PM::send_cmd("gate");
        PM::wait_signal("/tmp/quiver/circuit_ready");
        circuit.gate_cnt_advance();
        printf("[circuitView] run gate complete\n");
    }
    void runStage() {
        // QMessageBox::information(this, "提示", __FUNCTION__);
        PM::send_cmd("stage");
        PM::wait_signal("/tmp/quiver/circuit_ready");
        circuit.stage_cnt_advance();
        printf("[circuitView] run stage complete\n");
    }
    void runAll() {
        // QMessageBox::information(this, "提示", __FUNCTION__);
        PM::send_cmd("all");
        PM::wait_signal("/tmp/quiver/circuit_ready");
        circuit.stage_cnt = circuit.stages.size();
        circuit.gate_cnt = 0;
        printf("[circuitView] run all complete\n");
    }
    void resetCircuit() {
        PM::send_cmd("reset");
        PM::wait_signal("/tmp/quiver/circuit_ready");
        circuit.stage_cnt = circuit.gate_cnt = 0;
        printf("[circuitView] reset complete\n");
    }
    void showCircuit() {
        readParams();
        CircuitScene* circuitScene = new CircuitScene(circuit, params);
        graph->setScene(circuitScene);
        graph->fitInView(circuitScene->itemsBoundingRect(), Qt::KeepAspectRatio);
    }
public:
    MovableGraphView *graph;
    // 这里的circuit是真正的circuit的虚像，不能执行，state只有bitnum
    Circuit circuit;
    QMap<str, int> params;
};