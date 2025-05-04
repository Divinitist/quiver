#pragma once

#include "Main.h"
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qpair.h>

class View;

class QuantumLineItem : public QGraphicsItem {
public:
    using coorType = qreal;
    QuantumLineItem(coorType x, coorType y, str text, qreal lineLength) : x(x), y(y), text(text), lineLength(lineLength) {}
    QRectF boundingRect() const override {
        return QRectF(x - 10, y - 10, text.length() * 8 + lineLength, 40);
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        QFont painterFont("Fira Code", 12);
        painter->setFont(painterFont);
        painter->drawText(x, y + 5, QString(text.c_str()));
        painter->drawLine(x + text.length() * 8, y, x + text.length() * 8 + lineLength, y);
    }
private:
    coorType x, y;
    str text;
    qreal lineLength;
};

class QuantumGateItem : public QGraphicsItem {
public:
    // 这个是必写的，因为需要重载以下两个函数
    // 这部分应该最后写，因为paint需要用到的参数暂时不知道。
    QuantumGateItem(int columnIdx, std::pair<int, int> rowIdxRange, str name, vec<u32> &ctrls, vec<u32> &idxs, QMap<str, int> &params)
     : columnIdx(columnIdx), rowIdxRange(rowIdxRange), name(name), ctrls(ctrls), idxs(idxs), params(params) {
        rectMargin = params["rectMargin"];
        columnWidth = params["columnWidth"];
        rowHeight = params["rowHeight"];
        columnPosOffset = std::strlen("xx |x〉") * 8;
        fontSize = params["fontSize"];
    }
    QRectF boundingRect() const override {
        // TODO: 计算占地尺寸
        auto w = columnWidth;
        auto h = (double)(rowIdxRange.second - rowIdxRange.first + 1) * rowHeight;
        auto x = columnIdx * columnWidth + columnPosOffset;
        auto y = rowIdxRange.first * rowHeight - rowHeight / 2.0;
        return QRect(x, y, w, h);
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        QFont painterFont("Fira Code", fontSize);
        painter->setFont(painterFont);
        // 对于SWAP
        if (idxs.size() == 2) {
            // 画线
            auto x = columnIdx * columnWidth + columnPosOffset + columnWidth / 2;
            auto y1 = idxs[0] * rowHeight;
            auto y2 = idxs[1] * rowHeight;
            painter->drawLine(x, y1, x, y2);
            // 画叉
            auto crossSize = 12.0;
            painter->drawLine(x - crossSize / 2.0, y1 - crossSize / 2.0, x + crossSize / 2.0, y1 + crossSize / 2.0);
            painter->drawLine(x + crossSize / 2.0, y1 - crossSize / 2.0, x - crossSize / 2.0, y1 + crossSize / 2.0);

            painter->drawLine(x - crossSize / 2.0, y2 - crossSize / 2.0, x + crossSize / 2.0, y2 + crossSize / 2.0);
            painter->drawLine(x + crossSize / 2.0, y2 - crossSize / 2.0, x - crossSize / 2.0, y2 + crossSize / 2.0);
        }
        // 单门
        else if (idxs.size() == 1) {
            // 画框
            auto rectLeft = columnIdx * columnWidth + columnPosOffset + rectMargin / 2.0;
            auto rectTop = idxs[0] * rowHeight - rowHeight / 2.0 + rectMargin / 2.0;
            auto rectWidth = columnWidth - rectMargin;
            auto rectHeight = rowHeight - rectMargin;
            painter->setBrush(QColor("green"));
            painter->drawRect(rectLeft, rectTop, rectWidth, rectHeight);
            // 画文字
            auto textLeft = rectLeft + rectWidth / 2.0 - name.length() * fontSize / 2.5;
            auto textTop = rectTop + rectHeight / 2.0 + fontSize / 2.0;
            painter->drawText(textLeft, textTop, QString(name.c_str()));
        }
        else if (idxs.size() == 0) {
            // 画大框
            auto rectLeft = columnIdx * columnWidth + columnPosOffset + rectMargin / 2.0;
            auto rectTop = - rowHeight / 2.0 + rectMargin / 2.0;
            auto rectWidth = columnWidth - rectMargin;
            auto rectHeight = (rowIdxRange.second - rowIdxRange.first + 1) * rowHeight - rectMargin;
            painter->setBrush(QColor("darkorange"));
            painter->drawRect(rectLeft, rectTop, rectWidth, rectHeight);
            // 画文字
            auto textLeft = rectLeft + rectWidth / 2.0 - name.length() * fontSize / 2.5;
            auto textTop = rectTop + rectHeight / 2.0 + fontSize / 2.0;
            painter->drawText(textLeft, textTop, QString(name.c_str()));
        }
        else QMessageBox::warning(nullptr, "错误", "受控比特数量不正常");
        if (ctrls.empty() == false) {
            // 如果是交换门，则控制线一直画到比特线
            if (idxs.size() == 2) {
                for (auto &ctrl : ctrls) {
                    auto centerX = columnIdx * columnWidth + columnPosOffset + columnWidth / 2.0;
                    auto centerY = ctrl * rowHeight;
                    // 画控制点
                    auto ctrlPointSize = 6.0;
                    painter->setBrush(Qt::transparent);
                    painter->drawEllipse(centerX - ctrlPointSize / 2.0, centerY - ctrlPointSize / 2.0, ctrlPointSize, ctrlPointSize);
                    // 画控制线
                    // 如果小于idx0则画到idx0，如果大于idx1则画到idx1，如果在中间则不用画控制线，只用画控制点即可。
                    if (ctrl < idxs[0]) {
                        painter->drawLine(centerX, centerY, centerX, idxs[0] * rowHeight);
                    } else if (ctrl > idxs[1]) {
                        painter->drawLine(centerX, centerY, centerX, idxs[1] * rowHeight);
                    }
                }
            }
            // 否则，控制线画到框边则停止
            else if (idxs.size() == 1) {
                for (auto &ctrl : ctrls) {
                    auto centerX = columnIdx * columnWidth + columnPosOffset + columnWidth / 2.0;
                    auto centerY = ctrl * rowHeight;
                    // 画控制点
                    auto ctrlPointSize = 6.0;
                    painter->setBrush(Qt::white);
                    painter->drawEllipse(centerX - ctrlPointSize / 2.0, centerY - ctrlPointSize / 2.0, ctrlPointSize, ctrlPointSize);
                    // 画控制线
                    if (ctrl < idxs[0]) {
                        painter->drawLine(centerX, centerY, centerX, idxs[0] * rowHeight - rowHeight / 2.0 + rectMargin / 2.0);
                    } else {
                        painter->drawLine(centerX, centerY, centerX, idxs[0] * rowHeight + rowHeight / 2.0 - rectMargin / 2.0);
                    }
                }
            } else QMessageBox::warning(nullptr, "错误", "Oracle门出现了控制比特");
        }
    }
private:
    // 以下来自params
    qreal rectMargin;
    qreal columnWidth;
    qreal rowHeight;
    qreal fontSize;
    qreal columnPosOffset;
    // 以下来自构造函数
    int columnIdx;
    std::pair<int, int> rowIdxRange;
    str name;
    vec<u32> &ctrls;
    vec<u32> &idxs;
    QMap<str, int> &params;
    // TODO：通过保存Item的指针来实现对其后续的调整
};

class CircuitScene : public QGraphicsScene {
public:
    CircuitScene(Circuit &circuit, QMap<str, i32>& params,QObject *parent = nullptr) : QGraphicsScene(parent), circuit(circuit) {
        // printf("[CircuitScene] init_val = %u\n", circuit.init_val);
        auto rowHeight = params["rowHeight"];
        auto columnWidth = params["columnWidth"];
        auto &stages = circuit.stages;
        auto rowMask = 0u, columnIdx = 0u;
        for (auto i = 0; i < stages.size(); ++i) {
            auto &stage = stages[i];
            for (auto j = 0; j < stage.gates.size(); ++j) {
                auto &gate = stage.gates[j];
                auto &ctrls = gate.ctrls;
                auto &idxs = gate.idxs;
                std::sort(idxs.begin(), idxs.end());
                std::sort(ctrls.begin(), ctrls.end());
                int l, r;
                // Oracle门，占据一整列
                if (idxs.empty()) {
                    l = 0, r = circuit.bitnum - 1;
                } else {
                    // 单门
                    if (idxs.size() == 1) {
                        l = r = idxs[0];
                    } else if (idxs.size() == 2) {
                        l = idxs[0];
                        r = idxs[1];
                    } else QMessageBox::warning(nullptr, "错误", "idxs数量错误");
                    if (ctrls.empty() == false) {
                        l = std::min(l, (int)ctrls[0]);
                        r = std::max(r, (int)ctrls[ctrls.size() - 1]);
                    }
                }
                u32 curMask = ((1u << (r - l + 1)) - 1) << l;
                // 有交叉，则新开一列，否则计入交叉
                if (curMask & rowMask) {
                    rowMask = curMask;
                    columnIdx++;
                } else {
                    rowMask |= curMask;
                }
                // printf("l = %d, r = %d\n", l, r);
                QuantumGateItem *gateItem = new QuantumGateItem(columnIdx, {l, r}, gate.name, ctrls, idxs, params);
                // addItem(gateItem);
                gateItem->setOpacity(0.5);
                gateMap[&gate] = gateItem;
            }
        }
        if (rowMask)
            columnIdx++;
        for (auto i = 0; i < circuit.bitnum; ++i) {
            QuantumLineItem *line = new QuantumLineItem(0, i * rowHeight, std::format("{:02d} |{:d}〉", i, (circuit.init_val & (1u << (circuit.bitnum - 1 - i)) ? 1 : 0)), columnIdx * columnWidth);
            addItem(line);
            lines.push_back(line);
        }
        for (auto &stage : circuit.stages) {
            for (auto &gate : stage.gates) {
                addItem(gateMap[&gate]);
            }
        }
    }
    void setCurGateOpacity1() {
        if (circuit.is_end()) return;
        for (auto i = 0; i < circuit.stages.size(); ++i) {
            auto &stage = circuit.stages[i];
            for (auto j = 0; j < stage.gates.size(); ++j) {
                auto &gate = stage.gates[j];
                if (i == circuit.stage_cnt && j == circuit.gate_cnt) {
                    gateMap[&gate]->setOpacity(1.0);
                } else gateMap[&gate]->setOpacity(0.5);
            }
        }
    }
public:
    Circuit &circuit;
    // 线分布在y = i * rowHeight上
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
        circuitScene->setCurGateOpacity1();
        graph->fitInView(circuitScene->itemsBoundingRect(), Qt::KeepAspectRatio);
    }
public:
    MovableGraphView *graph;
    // 这里的circuit是真正的circuit的虚像，不能执行，state只有bitnum
    Circuit circuit;
    QMap<str, int> params;
};