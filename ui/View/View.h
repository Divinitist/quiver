#pragma once

#include "Main.h"
#include "View/FileView.h"
#include "View/ScriptView.h"
#include "View/CircuitView.h"
#include "View/ResultView.h"

class View : public QWidget {
    Q_OBJECT
public:
    // 初始化
    int windowWidth = 1280;
    int windowHeight = 720;
    View(QWidget *parent = nullptr) : QWidget(parent) {
        configMainWindow();
        initMembers();
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(initMainPanel());
        mainLayout->setContentsMargins(0, 0, 0, 0); // 设置主布局的边距为 0
        mainLayout->setSpacing(0);                  // 设置主布局的间距为 0
    }
    ~View() {
        killChild();
    }
private:
    // 界面构造
    void configMainWindow() {
        // 去掉边框（用于一体化开发）
        // setWindowFlags(Qt::FramelessWindowHint);
        resize(1280, 720);
        // 创建主页面的layout
        setContentsMargins(0, 0, 0, 0);
    }
    void initMembers() {
        fileView = new FileView(this);
        circuitView = new CircuitView(this);
        scriptView = new ScriptView(this);
        resultView = new ResultView(this);
        fileView->setContentsMargins(0, 0, 0, 0);
        fileView->setObjectName("fileView");
        circuitView->setContentsMargins(0, 0, 0, 0);
        scriptView->setContentsMargins(0, 0, 0, 0);
        resultView->setContentsMargins(0, 0, 0, 0);

        QShortcut *shortcutSave = new QShortcut(QKeySequence((int)Qt::CTRL + (int)Qt::Key_S), this);
        connect(shortcutSave, &QShortcut::activated, scriptView, &ScriptView::saveFile);
        connect(scriptView, &ScriptView::textSaved, this, &View::onTextSaved);

        connect(fileView, &FileView::fileSelected, scriptView, &ScriptView::openFile);
        connect(fileView, &FileView::fileSelected, this, &View::onOpenFile);
        connect(scriptView, &ScriptView::textChanged, this, &View::onTextChanged);
    }
    // 手动拆解成层级结构
    QWidget *initMainPanel() {
        auto leftRightSplitter = new QSplitter(this);
        leftRightSplitter->addWidget(fileView);         // 左边文件
        leftRightSplitter->addWidget(initRightPanel()); // 右边电路、结果、文本
        setStretches(leftRightSplitter, {2, 3});
        leftRightSplitter->setContentsMargins(0, 0, 0, 0);
        leftRightSplitter->setHandleWidth(0);
        return leftRightSplitter;
    }
    QWidget *initRightPanel() {
        auto upDownSplitter = new QSplitter(Qt::Vertical, this);
        upDownSplitter->addWidget(initRightUpPanel());  // 上边电路、结果
        upDownSplitter->addWidget(scriptView);          // 下边文本
        setStretches(upDownSplitter, {1, 2});
        upDownSplitter->setContentsMargins(0, 0, 0, 0);
        upDownSplitter->setHandleWidth(10);
        return upDownSplitter;
    }
    QWidget *initRightUpPanel() {
        auto panel = new QWidget(this);
        panel->setContentsMargins(0, 0, 0, 0);
        auto layout = new QVBoxLayout(panel);
        layout->addWidget(initMenuBar());               // 上边菜单栏
        layout->addWidget(initCircuitResultPanel());    // 下边电路、结果
        setStretches(layout, {1, 100}); // 极度压缩菜单栏
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        return panel;
    }
    QWidget *initMenuBar() {
        auto menuBar = new QMenuBar(this);
        compile = new QAction("编译脚本 (F5)", this);
        runGate = new QAction("单门运行 (F6)", this);
        runStage = new QAction("单段运行 (F7)", this);
        runAll = new QAction("全部运行", this);
        resetCircuit = new QAction("电路复位", this);
        showResult = new QMenu("显示选项", this);
        showCircuit = new QAction("电路图示", this);
        showDigit = new QAction("数字结果", this);
        showGraph = new QAction("图形结果", this);

        setComponentsStatus(false);

        showResult->addActions({showCircuit, showDigit, showGraph});

        menuBar->addActions({compile, runGate, runStage, runAll, resetCircuit});
        menuBar->addMenu(showResult);
        menuBar->setContentsMargins(0, 0, 0, 0);

        connect(compile, &QAction::triggered, this, &View::compileFile); // 写成C++，编译运行，读取circuit二进制数据，赋给circuitView
        connect(runGate, &QAction::triggered, this, &View::onRunGate); // 注意都要改变graph的高亮状态。graph永远显示下一个会执行的门
        connect(runStage, &QAction::triggered, this, &View::onRunStage);
        connect(runAll, &QAction::triggered, this, &View::onRunAll);
        connect(resetCircuit, &QAction::triggered, this, &View::onResetCircuit);
        connect(showCircuit, &QAction::triggered, this, &View::onShowCircuit);
        connect(showDigit, &QAction::triggered, this, &View::onShowDigit); // 在已经显示的状态对应的action上打勾
        connect(showGraph, &QAction::triggered, this, &View::onShowGraph);
        return menuBar;
    }
    QWidget *initCircuitResultPanel() {
        QWidget *circuitResult = new QWidget(this);
        circuitResult->setContentsMargins(0, 0, 0, 0);
        // circuitResult->setStyleSheet("background-color: #272822;");
        QHBoxLayout *layout = new QHBoxLayout(circuitResult);
        circuitResultStack = new QStackedWidget(this);

        layout->addWidget(circuitResultStack);
        layout->setContentsMargins(10, 10, 10, 0);
        layout->setSpacing(0);
        circuitResultStack->addWidget(circuitView);
        circuitResultStack->addWidget(resultView);
        circuitResultStack->setContentsMargins(0, 0, 0, 0);
        return circuitResult;
    }
    // 工具函数
    void killChild() {
        if (childPid != -1) {
            // 杀掉二层子进程../qs_cpp/xxx.qse
            PM::send_cmd("off");
            // 杀掉子进程./quiver
            kill(childPid, SIGTERM);
            childPid = -1;
        }
    }
    // 跑完后禁止再跑
    void checkCircuitEnd() {
        if (circuitView->circuit.is_end()) {
            runGate->setEnabled(false);
            runStage->setEnabled(false);
            runAll->setEnabled(false);
        }
    }
    // 设置显示子菜单的加粗
    void setShowMenuBold(QAction *menuItemPtr) {
        QFont showCircuitFont = showCircuit->font();
        QFont showDigitFont = showDigit->font();
        QFont showGraphFont = showGraph->font();

        showCircuitFont.setBold(menuItemPtr == showCircuit);
        showDigitFont.setBold(menuItemPtr == showDigit);
        showGraphFont.setBold(menuItemPtr == showGraph);

        showCircuit->setFont(showCircuitFont);
        showDigit->setFont(showDigitFont);
        showGraph->setFont(showGraphFont);
    }
    void setComponentsStatus(bool status) {
        compile->setEnabled(status);
        runGate->setEnabled(status);
        runStage->setEnabled(status);
        runAll->setEnabled(status);
        resetCircuit->setEnabled(status);
        showResult->setEnabled(status);
        showCircuit->setEnabled(status);
        showDigit->setEnabled(status);
        showGraph->setEnabled(status);
    }
    void updateShow() {
        if (curShowPtr == showCircuit) {
            onShowCircuit();
        }
        if (curShowPtr == showDigit) {
            onShowDigit();
        }
        if (curShowPtr == showGraph) {
            onShowGraph();
        }
    }
private slots:
    // 打开文件后，需要编译才能够有其他操作
    void onOpenFile(const QString& _fileName) {
        fileName = _fileName;
        setWindowTitle(fileName + " - quiver");
        compile->setEnabled(true);
        // setComponentsStatus(true);
    }
    void onTextChanged() {
        setWindowTitle(fileName + "* - quiver");
    }
    void onTextSaved() {
        setWindowTitle(fileName + " - quiver");
    }
    void compileFile() {
        stdFileName = fileName.toStdString();
        // 只搞qs文件
        if (stdFileName.substr(stdFileName.length() - 3) != ".qs") {
            QMessageBox::warning(this, "错误", "当前文件不是qs脚本文件");
            return;
        }
        // 截掉.qs
        stdFileName = stdFileName.substr(0, stdFileName.find_last_of('.'));
        // 获取当前代码（注意：不允许run_xxx和show_xxx命令，这些在circuitView操控下执行）
        auto codeStr = scriptView->getText();
        // 插入对qs_cpp.h的引用
        codeStr = "#include \"parser/qs_cpp.h\"\n\n" + codeStr;
        // 打开qs_cpp下同名文件进行写入
        auto qsCppFilePath = std::format("../qs_cpp/{}.cpp", stdFileName);
        auto qsCppFile = fopen(qsCppFilePath.c_str(), "w");
        if (qsCppFile == nullptr) {
            qDebug() << "unable to create qs_cpp file";
            return;
        }
        // 写入文件
        auto writeSize = fwrite(static_cast<const void *>(codeStr.c_str()), sizeof(char), codeStr.length(), qsCppFile);
        if (writeSize != codeStr.length()) {
            qDebug() << "write failure. write size = " << writeSize;
            return;
        }
        fclose(qsCppFile);
        // 如果之前的进程还在，就先杀
        if (childPid != -1) {
            PM::send_cmd("off");
            kill(childPid, SIGTERM);
            childPid = -1;
        }
        pid_t pid = fork(); // 需要开子进程，否则会卡死
        if (pid < 0) { // 创建失败
            perror("fork failed");
            exit(1);
        } else if (pid == 0) { // 子进程
            system(std::format("../compile.sh {}", stdFileName).c_str());
            exit(0);
        } else {
            // 存储子进程的PID
            childPid = pid;
            // 等待电路文件输出完毕
            PM::wait_signal("/tmp/quiver/circuit_file_ready");
            // 清空circuit
            circuitView->circuit.stages.clear();
            // 读入电路文件
            circuitView->circuit.read_from(std::format("../qs_cpp/{}.cir", stdFileName));
            // 执行状态回到最初
            circuitView->circuit.stage_cnt = circuitView->circuit.gate_cnt = 0;
            // 输出测试
            // circuit.write_to();
            // 这个还是不能少，得把二层子进程杀掉
            // PM::send_cmd("off");
            // 不等待子进程，直接退出 = 让子进程单飞
            // kill(pid, SIGTERM); // 大义灭亲，不怕孤儿进程
            QMessageBox::information(this, "提示", "编译完成");
            setComponentsStatus(true);
            onShowCircuit();
            return;
            // 这部分是后面resultView的事
            // u8 graph_grain_size = 4;
            // PM::write_to("/tmp/graph_grain_size", &graph_grain_size, 1);
            // printf("[View::compileFile] write to /tmp/graph_grain_size complete\n");
        }
    }

    void onRunGate() {
        circuitView->runGate();
        checkCircuitEnd();
        updateShow();
    }
    void onRunStage() {
        circuitView->runStage();
        checkCircuitEnd();
        updateShow();
    }
    void onRunAll() {
        circuitView->runAll();
        checkCircuitEnd();
        updateShow();
    }
    void onResetCircuit() {
        circuitView->resetCircuit();
        // 恢复所有键位功能
        setComponentsStatus(true);
        updateShow();
    }

    void onShowCircuit() {
        circuitResultStack->setCurrentIndex(0);
        setShowMenuBold(showCircuit);
        // 进去处理
        circuitView->showCircuit();
        curShowPtr = showCircuit;
    }
    void onShowDigit() {
        // QMessageBox::information(this, "提示", __FUNCTION__);
        circuitResultStack->setCurrentIndex(1);
        setShowMenuBold(showDigit);
        resultView->showDigitResult(stdFileName);
        curShowPtr = showDigit;
    }
    void onShowGraph() {
        // QMessageBox::information(this, "提示", __FUNCTION__);
        circuitResultStack->setCurrentIndex(1);
        setShowMenuBold(showGraph);
        resultView->showGraphResult(stdFileName);
        curShowPtr = showGraph;
    }

private:
    QString fileName;
    str stdFileName;
    FileView *fileView;
    ScriptView *scriptView;
    CircuitView *circuitView;
    ResultView *resultView;
    QStackedWidget *circuitResultStack;

    pid_t childPid = -1;

    QAction *curShowPtr;

    QAction *compile;
    QAction *resetCircuit;
    QAction *runGate;
    QAction *runStage;
    QAction *runAll;
    QMenu *showResult;
    QAction *showCircuit;
    QAction *showDigit;
    QAction *showGraph;
};