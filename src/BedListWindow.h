#pragma once

#include <QMainWindow>

class BedModel;
class BedGanttWidget;
class QComboBox;
class QCheckBox;
class QSpinBox;

class BedListWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit BedListWindow(QWidget* parent = nullptr);
    ~BedListWindow();

private:
    BedModel* m_model;
    BedGanttWidget* m_gantt;
    QComboBox* m_cropCombo{nullptr};
    QCheckBox* m_includeHidden{nullptr};
    QSpinBox* m_bedCount{nullptr};

    // load crop types into the combo; includeHidden controls whether hidden crops are included
    void loadCropTypes(bool includeHidden);
};
