#pragma once

#include "Main.h"

class FileView : public QWidget {
    Q_OBJECT
public:
    FileView(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("文件视图");
        // 创建垂直布局，用于安放树形图和按钮
        QVBoxLayout *layout = new QVBoxLayout(this);
        // 创建树形图
        treeWidget = new QTreeWidget(this);
        // 创建按钮
        QPushButton *openFolderButton = new QPushButton("打开文件夹", this);

        layout->addWidget(treeWidget);
        layout->addWidget(openFolderButton);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        // treeWidget->setStyleSheet("QTreeWidget { font-size: 24px; }");
        treeWidget->setHeaderHidden(true);
        treeWidget->setHeaderLabel("文件"); // 把列名改成“文件”
        treeWidget->header()->setDefaultAlignment(Qt::AlignHCenter); // 默认header居中
        treeWidget->setContextMenuPolicy(Qt::CustomContextMenu); // 自定义菜单

        connect(treeWidget, &QTreeWidget::itemClicked, this, &FileView::onItemClicked); // 连接选中文件信号
        connect(treeWidget, &QTreeWidget::clicked, this, [&](const QModelIndex& index) { selectedItemPath = folderPath; }); // 设置选中根目录
        // connect(treeWidget, &QTreeWidget::customContextMenuRequested, this, &FileView::onCustomContextMenuRequested); // 不提供右键菜单
        connect(openFolderButton, &QPushButton::clicked, this, &FileView::openFolder); // 点击按钮->打开文件夹
    }
signals:
    void fileSelected(const QString& fileName, const QString& filePath);
public slots:
    // 打开文件夹选择框
    void openFolder() {
        // 弹出文件夹选择对话框
        folderPath = QFileDialog::getExistingDirectory(this, "选择文件夹", "../");
        if(folderPath.isEmpty()) return;
        treeWidget->clear();
        buildTree(folderPath, treeWidget->invisibleRootItem());
    }
    // 递归建树
    void buildTree(const QString &path, QTreeWidgetItem *parentItem) {
        QDir dir(path);
        // 排除.和..
        dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
        QFileInfoList fileList = dir.entryInfoList();
        for(const QFileInfo &fileInfo : fileList) {
            QTreeWidgetItem *item = new QTreeWidgetItem(parentItem);
            item->setText(0, fileInfo.fileName());
            // 保存完整路径，待查
            item->setData(0, Qt::UserRole, fileInfo.absoluteFilePath());
            // 递归构建子树
            if(fileInfo.isDir()) buildTree(fileInfo.absoluteFilePath(), item);
        }
    }
    // 把itemClicked转化成fileSelected
    void onItemClicked(QTreeWidgetItem *item, int column) {
        selectedItemPath = item->data(column, Qt::UserRole).toString();
        // 不对文件夹做出反应
        if(QFileInfo(selectedItemPath).isDir()) {
            // 展开/收起取反
            item->setExpanded(!(item->isExpanded()));
        } else {
            QString fileName = item->text(column);
            emit fileSelected(fileName, selectedItemPath);
        }
    }
    // void onCustomContextMenuRequested(const QPoint& pos) {
    //     QTreeWidgetItem *item = treeWidget->itemAt(pos);
    //     if(item == nullptr) return;
    //     QMenu *menu = new QMenu(this);
    //     QString filePath = item->data(0, Qt::UserRole).toString();
    //     QFileInfo file(filePath);
    //     if(file.isDir()) {
    //         QAction *openFolderAction = new QAction("打开文件夹", menu);
    //         QAction *newFileAction = new QAction("创建文件", menu);
    //         menu->addActions({newFileAction, openFolderAction});
    //     } else {
    //         QAction *openFileAction = new QAction("打开文件", menu);
    //         QAction *newFileAction = new QAction("创建文件", menu);
    //         QAction *deleteFileAction = new QAction("删除文件", menu);
    //         menu->addActions({openFileAction, newFileAction, deleteFileAction});
    //     }
    // }
    // 如果选中的是文件夹，就在文件夹下级创建新文件
    // 如果选中的是文件，就在文件同级创建新文件
    // 如果什么都没有选中，就在根目录创建新文件
    void createNewFile() {
        QString fileName = QInputDialog::getText(this, "输入新建文件名", "");
        QString filePath = (QFileInfo(selectedItemPath).isDir() ? QFileInfo(selectedItemPath).path() : selectedItemPath)  + "/" + fileName;
        QFile file(filePath);
        if(file.exists()) {
            QMessageBox::warning(this, "错误", "文件已存在");
            return;
        } else {
            file.open(QIODevice::WriteOnly | QIODevice::Text); // 创建文件
            file.close();
            treeWidget->clear();
            buildTree(folderPath, treeWidget->invisibleRootItem());
        }
    }
private:
    QTreeWidget *treeWidget;
    QString folderPath = "";
    QString selectedItemPath = "";
};