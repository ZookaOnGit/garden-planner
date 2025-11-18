#pragma once

#include "CropWindow.h"
#include "SettingsManager.h"


#include <QCheckBox>
#include <QDate>
#include <QDateEdit>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>


class QLineEdit;
class QDateEdit;
class QCheckBox;

// Dialog to add or edit a crop row (name + three lanes of date ranges)
class CropEditDialog : public QDialog {
    Q_OBJECT
public:
    // If parent passes an existing crop name, the dialog will be treated as an edit.
    explicit CropEditDialog(QWidget* parent = nullptr);

    // Populate fields from an existing CropWindow
    void setCrop(const CropWindow& c);
    // Fill a CropWindow from the dialog contents
    CropWindow crop() const;

    // Set/Get the original id when editing (used by caller to identify row)
    void setOriginalId(int id) { m_originalId = id; }
    int originalId() const { return m_originalId; }

protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    //void resizeEvent(QResizeEvent* event) override;

private:
    bool m_restored = false;

    QLineEdit* m_nameEdit = nullptr;
    QTextEdit* m_notesEdit = nullptr;

    QCheckBox* m_hasSow = nullptr;
    QDateEdit* m_sowStart = nullptr;
    QDateEdit* m_sowEnd = nullptr;
    QLabel* m_sowDays = nullptr;

    QCheckBox* m_hasPlant = nullptr;
    QDateEdit* m_plantStart = nullptr;
    QDateEdit* m_plantEnd = nullptr;
    QLabel* m_plantDays = nullptr;

    QCheckBox* m_hasHarvest = nullptr;
    QDateEdit* m_harvestStart = nullptr;
    QDateEdit* m_harvestEnd = nullptr;
    QLabel* m_harvestDays = nullptr;

    int m_originalId = -1;
};
