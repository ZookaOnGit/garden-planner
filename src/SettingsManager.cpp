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
        qDebug() << "Successfully saved" << key << "to settings.";
    }
}

void SettingsManager::saveAddEditWindowGeometry(QWidget* w) {
    qDebug() << "Saving add/edit window geometry";
    QRect rect = w->geometry();  // Gives position and size
    qDebug() << "x:" << rect.x()
            << "y:" << rect.y()
            << "width:" << rect.width()
            << "height:" << rect.height();

    const QByteArray blob = w->saveGeometry();
    setValue("addedit_window_geometry", QString(blob.toBase64()));
}

void SettingsManager::loadAddEditWindowGeometry(QWidget* w) {
    qDebug() << "Loading add/edit window geometry";
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

void SettingsManager::saveMainWindowGeometry(QMainWindow* mw) {
    qDebug() << "Saving main window geometry";
    QRect rect = mw->geometry();  // Gives position and size
    qDebug() << "x:" << rect.x()
            << "y:" << rect.y()
            << "width:" << rect.width()
            << "height:" << rect.height();

    const QByteArray blob = mw->saveGeometry();
    setValue("main_window_geometry", QString(blob.toBase64()));
}

void SettingsManager::loadMainWindowGeometry(QMainWindow* mw) {
    qDebug() << "Loading main window geometry";
    QRect rect = mw->geometry();  // Gives position and size
    qDebug() << "x:" << rect.x()
            << "y:" << rect.y()
            << "width:" << rect.width()
            << "height:" << rect.height();

    const QString encoded = getValue("addedit_window_geometry");
    if (!encoded.isEmpty()) {
        const QByteArray blob = QByteArray::fromBase64(encoded.toUtf8());
        mw->restoreGeometry(blob);
    }
}

void SettingsManager::saveMainSplitterState(QSplitter* s) {
    qDebug() << "Saving main splitter state";
    const QByteArray blob = s->saveState();
    setValue("main_splitter_state", QString(blob.toBase64()));
}

void SettingsManager::loadMainSplitterState(QSplitter* s) {
    qDebug() << "Loading main splitter state";
    const QString encoded = getValue("main_splitter_state");
    if (!encoded.isEmpty()) {
        const QByteArray blob = QByteArray::fromBase64(encoded.toUtf8());
        s->restoreState(blob);
    }
}

void SettingsManager::saveLeftColumnSortMode(int mode) {
    // store as simple integer string
    setValue("left_column_sort", QString::number(mode));
}

int SettingsManager::loadLeftColumnSortMode() {
    const QString v = getValue("left_column_sort");
    if (v.isEmpty()) return 0; // default
    bool ok = false;
    int m = v.toInt(&ok);
    return ok ? m : 0;
}