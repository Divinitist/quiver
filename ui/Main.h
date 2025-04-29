#pragma once

#include <QApplication>
#include <QToolTip>
#include <QGraphicsSceneMouseEvent>
#include <QDockWidget>
#include <QToolButton>
#include <QShortcut>
#include <QMenuBar>
#include <QLabel>
#include <QDebug>
#include <QScrollBar>
#include <QAbstractTextDocumentLayout>
#include <QHeaderView>
#include <QGraphicsItem>
#include <QMessageBox>
#include <QWidget>
#include <QMenu>
#include <QTreeWidget>
#include <QTextEdit>
#include <QFileDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDir>
#include <QFileInfoList>
#include <QFile>
#include <QTextStream>
#include <QSplitter>
#include <QFont>
#include <QWheelEvent>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QSyntaxHighlighter>
#include <QInputDialog>
#include <QVariant>
#include <QModelIndex>
#include <QStackedWidget>

// 量子实体
#include "common/common.h"
#include "circuit/circuit.h"
#include "state/state.h"
#include "parser/parser.h"

template <typename T>
void setStretches(T *widget, const vec<i32> &stretches);

template <>
void setStretches(QSplitter *splitter, const vec<i32> &stretches);