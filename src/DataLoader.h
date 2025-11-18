
#pragma once
#include "CropWindow.h"
#include <QVector>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <QDate>

inline QVector<CropWindow> loadCropsFromQuery(QSqlQuery& query) {
    QVector<CropWindow> items;
    while (query.next()) {
        CropWindow c;
        // Expecting query to provide columns: id, name, sow_start, sow_end,
        // plant_start, plant_end, harvest_start, harvest_end, notes
        // For backward compatibility, if id is missing, treat it as -1.
        int colCount = query.record().count();
        int idx = 0;
        if (colCount >= 9) {
            c.id = query.value(idx++).toInt();
        } else {
            c.id = -1;
        }
        c.name         = query.value(idx++).toString();
        c.sowStart     = query.value(idx++).toDate();
        c.sowEnd       = query.value(idx++).toDate();
        c.plantStart   = query.value(idx++).toDate();
        c.plantEnd     = query.value(idx++).toDate();
        c.harvestStart = query.value(idx++).toDate();
        c.harvestEnd   = query.value(idx++).toDate();
        c.notes        = query.value(idx++).toString();

        auto fix = [](QDate& a, QDate& b){
            if (a.isValid() && b.isValid() && a > b) std::swap(a,b);
        };
        fix(c.sowStart, c.sowEnd);
        fix(c.plantStart, c.plantEnd);
        fix(c.harvestStart, c.harvestEnd);

        items.push_back(std::move(c));
    }
    return items;
}
