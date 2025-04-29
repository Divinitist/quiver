#pragma once

#include "Main.h"

class CircuitModel : public QObject {
    Q_OBJECT
public:
private:
    // 根据名字存储的Circuit
    std::map<str, Circuit> m_circuits;
};