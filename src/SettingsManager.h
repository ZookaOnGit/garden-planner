#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QString>
#include <QSize>
#include <QPoint>
#include <QWidget>
#include <QRect>
#include <QMainWindow>
#include <QSplitter>

class SettingsManager {
public:
    // Get the global singleton instance
    static SettingsManager& instance();
    
    void saveAddEditWindowGeometry(QWidget* w);    
    void loadAddEditWindowGeometry(QWidget* w);
    void saveMainWindowGeometry(QMainWindow* mw);
    void loadMainWindowGeometry(QMainWindow* mw);
    void saveMainSplitterState(QSplitter* s);
    void loadMainSplitterState(QSplitter* s);

private:
    // Private constructor for singleton
    SettingsManager(const QString& dbPath = "garden.db");
    
    // Prevent copying
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;
    
    QString m_dbPath;
    QString m_connectionName;
    void executeQuery(const QString& query);
    QString getValue(const QString& key);
    void setValue(const QString& key, const QString& value);
};

#endif // SETTINGSMANAGER_H
