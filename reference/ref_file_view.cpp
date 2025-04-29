#include <QTreeWidget>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QFile>

class MyTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    MyTreeWidget(QWidget* parent = nullptr) : QTreeWidget(parent)
    {
        setContextMenuPolicy(Qt::CustomContextMenu); // 使用自定义右键菜单策略
        connect(this, &QTreeWidget::customContextMenuRequested, this, &MyTreeWidget::showContextMenu);
    }

private slots:
    void showContextMenu(const QPoint& pos)
    {
        QTreeWidgetItem* item = itemAt(pos); // 获取被点击的项目
        if (!item)
            return;

        QString filePath = item->data(0, Qt::UserRole).toString(); // 获取文件路径

        // 创建右键菜单
        QMenu menu(this);
        QAction* openAction = menu.addAction("打开");
        QAction* deleteAction = menu.addAction("删除");

        // 连接菜单项的信号到槽函数
        connect(openAction, &QAction::triggered, [filePath]() {
            QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        });

        connect(deleteAction, &QAction::triggered, [filePath]() {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(nullptr, "确认", "确定要删除这个文件吗？",
                                           QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes)
            {
                QFile::remove(filePath);
            }
        });

        // 在鼠标点击位置显示菜单
        menu.exec(mapToGlobal(pos));
    }
};