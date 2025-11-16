#include "SettingsManager.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QWidget>
#include <QRect>

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

    QRect rect = w->geometry();  // Gives position and size
    qDebug() << "x:" << rect.x()
            << "y:" << rect.y()
            << "width:" << rect.width()
            << "height:" << rect.height();

    const QByteArray blob = w->saveGeometry();
    setValue("addedit_window_geometry", QString(blob.toBase64()));    
}

void SettingsManager::loadAddEditWindowGeometry(QWidget* w) {

    QRect rect = w->geometry();  // Gives position and size
    qDebug() << "x:" << rect.x()
            << "y:" << rect.y()
            << "width:" << rect.width()
            << "height:" << rect.height();

    const QString encoded = getValue("addedit_window_geometry");
    if (!encoded.isEmpty()) {
        const QByteArray blob = QByteArray::fromBase64(encoded.toUtf8());
        w->restoreGeometry(blob);
    }
}
