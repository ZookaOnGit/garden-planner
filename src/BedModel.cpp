#include "BedModel.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QDebug>

BedModel::BedModel(QObject* parent) : QObject(parent) {}

void BedModel::loadDefaults(int bedCount) {
    Q_UNUSED(bedCount);
    // Start empty; user will add crops. Keep next id at 1.
    m_crops.clear();
    m_nextId = 1;
    emit modelChanged();
}

static bool ensureBedTableExists() {
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) return false;
    QSqlQuery q(db);
    bool ok = q.exec("CREATE TABLE IF NOT EXISTS beds (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, bed INTEGER, start TEXT, lengthDays INTEGER, color TEXT)");
    if (!ok) qDebug() << "Failed to ensure beds table:" << q.lastError().text();
    return ok;
}

void BedModel::loadFromDatabase() {
    m_crops.clear();
    m_nextId = 1;
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) {
        emit modelChanged();
        return;
    }
    if (!ensureBedTableExists()) {
        emit modelChanged();
        return;
    }
    QSqlQuery q(db);
    if (!q.exec("SELECT id, name, bed, start, lengthDays, color FROM beds ORDER BY id")) {
        qDebug() << "Failed to load beds:" << q.lastError().text();
        emit modelChanged();
        return;
    }
    int maxId = 0;
    while (q.next()) {
        BedCrop c;
        c.id = q.value(0).toInt();
        c.name = q.value(1).toString();
        c.bed = q.value(2).toInt();
        c.start = QDate::fromString(q.value(3).toString(), Qt::ISODate);
        c.lengthDays = q.value(4).toInt();
        QString col = q.value(5).toString();
        if (!col.isEmpty()) c.color = QColor(col);
        
        // Load harvest window from crops catalog
        loadHarvestWindowForCrop(c);
        
        m_crops.append(c);
        if (c.id > maxId) maxId = c.id;
    }
    m_nextId = maxId + 1;
    emit modelChanged();
}

void BedModel::addCrop(const BedCrop& c) {
    QSqlDatabase db = QSqlDatabase::database();
    if (db.isValid() && db.isOpen() && ensureBedTableExists()) {
        QSqlQuery q(db);
        q.prepare("INSERT INTO beds (name, bed, start, lengthDays, color) VALUES (:name, :bed, :start, :len, :color)");
        q.bindValue(":name", c.name);
        q.bindValue(":bed", c.bed);
        q.bindValue(":start", c.start.toString(Qt::ISODate));
        q.bindValue(":len", c.lengthDays);
        q.bindValue(":color", c.color.name());
        if (!q.exec()) {
            qDebug() << "Failed to insert bed crop:" << q.lastError().text();
        } else {
            BedCrop copy = c;
            copy.id = q.lastInsertId().toInt();
            loadHarvestWindowForCrop(copy);
            m_crops.append(copy);
            if (copy.id >= m_nextId) m_nextId = copy.id + 1;
            emit modelChanged();
            return;
        }
    }
    // fallback to in-memory only
    BedCrop copy = c;
    copy.id = nextId();
    loadHarvestWindowForCrop(copy);
    m_crops.append(copy);
    emit modelChanged();
}

void BedModel::updateCropById(int id, const BedCrop& c) {
    for (int i = 0; i < m_crops.size(); ++i) {
        if (m_crops[i].id == id) {
            m_crops[i] = c;
            m_crops[i].id = id; // preserve id
            QSqlDatabase db = QSqlDatabase::database();
            if (db.isValid() && db.isOpen() && ensureBedTableExists()) {
                QSqlQuery q(db);
                q.prepare("UPDATE beds SET name = :name, bed = :bed, start = :start, lengthDays = :len, color = :color WHERE id = :id");
                q.bindValue(":id", id);
                q.bindValue(":name", c.name);
                q.bindValue(":bed", c.bed);
                q.bindValue(":start", c.start.toString(Qt::ISODate));
                q.bindValue(":len", c.lengthDays);
                q.bindValue(":color", c.color.name());
                if (!q.exec()) qDebug() << "Failed to update bed crop:" << q.lastError().text();
            }
            emit modelChanged();
            return;
        }
    }
}

void BedModel::removeCropById(int id) {
    for (int i = 0; i < m_crops.size(); ++i) {
        if (m_crops[i].id == id) {
            QSqlDatabase db = QSqlDatabase::database();
            if (db.isValid() && db.isOpen() && ensureBedTableExists()) {
                QSqlQuery q(db);
                q.prepare("DELETE FROM beds WHERE id = :id");
                q.bindValue(":id", id);
                if (!q.exec()) qDebug() << "Failed to delete bed crop:" << q.lastError().text();
            }
            m_crops.removeAt(i);
            emit modelChanged();
            return;
        }
    }
}

void BedModel::clearAllCrops() {
    QSqlDatabase db = QSqlDatabase::database();
    if (db.isValid() && db.isOpen() && ensureBedTableExists()) {
        QSqlQuery q(db);
        if (!q.exec("DELETE FROM beds")) qDebug() << "Failed to clear beds:" << q.lastError().text();
    }
    m_crops.clear();
    m_nextId = 1;
    emit modelChanged();
}

void BedModel::loadHarvestWindowForCrop(BedCrop& crop) {
    if (crop.name.isEmpty()) return;
    
    QSqlQuery q;
    q.prepare("SELECT plant_start, plant_end, harvest_start, harvest_end FROM crops WHERE name = :name LIMIT 1");
    q.bindValue(":name", crop.name);
    if (q.exec() && q.next()) {
        QDate plantStart = q.value(0).toDate();
        QDate plantEnd = q.value(1).toDate();
        QDate harvestStart = q.value(2).toDate();
        QDate harvestEnd = q.value(3).toDate();
        
        if (plantStart.isValid() && plantEnd.isValid() && harvestStart.isValid() && harvestEnd.isValid()) {
            // Calculate days from plant_start to harvest_start (minimum days to harvest)
            crop.harvestLengthDaysMin = plantStart.daysTo(harvestStart);
            // Calculate days from plant_end to harvest_end (maximum days to harvest)
            crop.harvestLengthDaysMax = plantEnd.daysTo(harvestEnd);
        }
    }
}
int BedModel::nextId() {
    return m_nextId++;
}
