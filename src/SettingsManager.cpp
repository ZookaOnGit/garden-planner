#include "SettingsManager.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

SettingsManager& SettingsManager::instance() {
    static SettingsManager s_instance("garden.db");
    return s_instance;
}

SettingsManager::SettingsManager(const QString& dbPath) : m_dbPath(dbPath), m_connectionName("settings_db") {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    db.setDatabaseName(m_dbPath);
    if (!db.open()) {
        qWarning() << "Failed to open DB:" << db.lastError().text();
    }
    QSqlQuery q(db);
    q.exec("CREATE TABLE IF NOT EXISTS user_settings (key TEXT PRIMARY KEY, value TEXT)");
}

void SettingsManager::executeQuery(const QString& query) {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery q(db);
    if (!q.exec(query)) {
        qWarning() << "Query failed:" << q.lastError().text();
    }
}

QString SettingsManager::getValue(const QString& key) {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery q(db);
    q.prepare("SELECT value FROM user_settings WHERE key = ?");
    q.addBindValue(key);
    if (q.exec() && q.next()) {
        return q.value(0).toString();
    }
    return QString();
}

void SettingsManager::setValue(const QString& key, const QString& value) {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isOpen()) {
        qWarning() << "Database not open!";
        return;
    }
    QSqlQuery q(db);
    q.prepare("INSERT OR REPLACE INTO user_settings (key, value) VALUES (?, ?)");
    q.addBindValue(key);
    q.addBindValue(value);
    if (!q.exec()) {
        qWarning() << "Insert failed:" << q.lastError().text();
    } else {
        qDebug() << "Successfully saved" << key;
    }
}

void SettingsManager::saveAddEditWindowGeometry(QWidget* w) {
    // Save position and size separately for better compatibility
    QPoint pos = w->pos();
    QSize size = w->size();
    
    qDebug() << "Saving dialog - Pos:" << pos << "Size:" << size;
    
    setValue("addedit_window_x", QString::number(pos.x()));
    setValue("addedit_window_y", QString::number(pos.y()));
    setValue("addedit_window_width", QString::number(size.width()));
    setValue("addedit_window_height", QString::number(size.height()));
    
    qDebug() << "Dialog geometry saved";
}

void SettingsManager::loadAddEditWindowGeometry(QWidget* w) {
    QString xStr = getValue("addedit_window_x");
    QString yStr = getValue("addedit_window_y");
    QString widthStr = getValue("addedit_window_width");
    QString heightStr = getValue("addedit_window_height");
    
    qDebug() << "Loading dialog - x:" << xStr << "y:" << yStr << "w:" << widthStr << "h:" << heightStr;
    
    if (!xStr.isEmpty() && !yStr.isEmpty() && !widthStr.isEmpty() && !heightStr.isEmpty()) {
        bool ok_x, ok_y, ok_w, ok_h;
        int x = xStr.toInt(&ok_x);
        int y = yStr.toInt(&ok_y);
        int width = widthStr.toInt(&ok_w);
        int height = heightStr.toInt(&ok_h);
        
        if (ok_x && ok_y && ok_w && ok_h) {
            w->move(x, y);
            w->resize(width, height);
            qDebug() << "Dialog geometry restored to" << QRect(x, y, width, height);
        } else {
            qWarning() << "Failed to parse dialog geometry values";
        }
    } else {
        qDebug() << "No saved dialog geometry found";
    }
}