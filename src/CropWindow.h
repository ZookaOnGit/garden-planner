
#pragma once
#include <QString>
#include <QDate>

struct CropWindow {
    int id = -1; // primary key from DB
    QString name, notes;
    QDate sowStart, sowEnd;
    QDate plantStart, plantEnd;
    QDate harvestStart, harvestEnd;
};
