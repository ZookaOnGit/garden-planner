#include "CropEditDialog.h"
#include "SettingsManager.h"


CropEditDialog::CropEditDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Add / Edit Crop");
    
    auto* mainLay = new QVBoxLayout(this);

    auto* form = new QFormLayout;
    m_nameEdit = new QLineEdit;
    form->addRow("Name:", m_nameEdit);

    // Sow
    {
        auto* h = new QHBoxLayout;
        m_hasSow = new QCheckBox("Sow:");
        m_sowStart = new QDateEdit;
        m_sowStart->setCalendarPopup(true);
        m_sowEnd = new QDateEdit;
        m_sowEnd->setCalendarPopup(true);
        m_sowStart->setDisplayFormat("yyyy-MM-dd");
        m_sowEnd->setDisplayFormat("yyyy-MM-dd");
        // Default dates to today so when adding/editing they show a sensible value
        m_sowStart->setDate(QDate::currentDate());
        m_sowEnd->setDate(QDate::currentDate());
        h->addWidget(m_hasSow);
        h->addWidget(m_sowStart);
        h->addWidget(m_sowEnd);
        form->addRow(h);
    }

    // Plant
    {
        auto* h = new QHBoxLayout;
        m_hasPlant = new QCheckBox("Plant:");
        m_plantStart = new QDateEdit;
        m_plantStart->setCalendarPopup(true);
        m_plantEnd = new QDateEdit;
        m_plantEnd->setCalendarPopup(true);
        m_plantStart->setDisplayFormat("yyyy-MM-dd");
        m_plantEnd->setDisplayFormat("yyyy-MM-dd");
            m_plantStart->setDate(QDate::currentDate());
            m_plantEnd->setDate(QDate::currentDate());
        h->addWidget(m_hasPlant);
        h->addWidget(m_plantStart);
        h->addWidget(m_plantEnd);
        form->addRow(h);
    }

    // Harvest
    {
        auto* h = new QHBoxLayout;
        m_hasHarvest = new QCheckBox("Harvest:");
        m_harvestStart = new QDateEdit;
        m_harvestStart->setCalendarPopup(true);
        m_harvestEnd = new QDateEdit;
        m_harvestEnd->setCalendarPopup(true);
        m_harvestStart->setDisplayFormat("yyyy-MM-dd");
        m_harvestEnd->setDisplayFormat("yyyy-MM-dd");
            m_harvestStart->setDate(QDate::currentDate());
            m_harvestEnd->setDate(QDate::currentDate());
        h->addWidget(m_hasHarvest);
        h->addWidget(m_harvestStart);
        h->addWidget(m_harvestEnd);
        form->addRow(h);
    }

    m_notesEdit = new QTextEdit;
    m_notesEdit->setPlaceholderText("Enter notes...");
    m_notesEdit->setAcceptRichText(false);
    m_notesEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    form->addRow(m_notesEdit);

    mainLay->addLayout(form);

    // Buttons
    auto* btnLay = new QHBoxLayout;
    auto* ok = new QPushButton("OK");
    auto* cancel = new QPushButton("Cancel");
    ok->setMinimumWidth(80);
    cancel->setMinimumWidth(80);
    ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    btnLay->addStretch();
    btnLay->addWidget(ok);
    btnLay->addSpacing(12);        // optional: space between buttons
    btnLay->addWidget(cancel);
    btnLay->addStretch();
    mainLay->addLayout(btnLay);

    connect(ok, &QPushButton::clicked, this, [this]() {
        SettingsManager::instance().saveAddEditWindowGeometry(this);

        if (m_nameEdit->text().trimmed().isEmpty()) {
            // simple validation: require name
            m_nameEdit->setFocus();
            return;
        }

        auto checkRange = [&](QCheckBox* cb, QDateEdit* s, QDateEdit* e, const QString& label) -> bool {
            if (!cb->isChecked()) return true;
            QDate sd = s->date();
            QDate ed = e->date();
            if (sd.isValid() && ed.isValid() && sd > ed) {
                QMessageBox::warning(this, "Invalid date range",
                                     QString("%1 start must be on or before its end.").arg(label));
                return false;
            }
            return true;
        };

        if (!checkRange(m_hasSow, m_sowStart, m_sowEnd, "Sow")) return;
        if (!checkRange(m_hasPlant, m_plantStart, m_plantEnd, "Plant")) return;
        if (!checkRange(m_hasHarvest, m_harvestStart, m_harvestEnd, "Harvest")) return;

        accept();
    });
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

    // Default state: checkboxes on
    m_hasSow->setChecked(true);
    m_hasPlant->setChecked(true);
    m_hasHarvest->setChecked(true);
}

void CropEditDialog::showEvent(QShowEvent* event) {
    SettingsManager::instance().loadAddEditWindowGeometry(this);
    QDialog::showEvent(event);
}

void CropEditDialog::closeEvent(QCloseEvent* event) {
    SettingsManager::instance().saveAddEditWindowGeometry(this);
    QDialog::closeEvent(event);
}

/*
void CropEditDialog::resizeEvent(QResizeEvent* event) {
    SettingsManager::instance().saveAddEditWindowGeometry(this);
    QDialog::resizeEvent(event);
}
*/

void CropEditDialog::setCrop(const CropWindow& c) {
    m_nameEdit->setText(c.name);

    if (c.sowStart.isValid() || c.sowEnd.isValid()) {
        m_hasSow->setChecked(true);
        if (c.sowStart.isValid()) m_sowStart->setDate(c.sowStart);
        else m_sowStart->setDate(QDate::currentDate());
        if (c.sowEnd.isValid())   m_sowEnd->setDate(c.sowEnd);
        else m_sowEnd->setDate(QDate::currentDate());
    } else {
        m_hasSow->setChecked(false);
        // ensure edits show today's date when not provided
        m_sowStart->setDate(QDate::currentDate());
        m_sowEnd->setDate(QDate::currentDate());
    }

    if (c.plantStart.isValid() || c.plantEnd.isValid()) {
        m_hasPlant->setChecked(true);
        if (c.plantStart.isValid()) m_plantStart->setDate(c.plantStart);
        else m_plantStart->setDate(QDate::currentDate());
        if (c.plantEnd.isValid())   m_plantEnd->setDate(c.plantEnd);
        else m_plantEnd->setDate(QDate::currentDate());
    } else {
        m_hasPlant->setChecked(false);
        m_plantStart->setDate(QDate::currentDate());
        m_plantEnd->setDate(QDate::currentDate());
    }

    if (c.harvestStart.isValid() || c.harvestEnd.isValid()) {
        m_hasHarvest->setChecked(true);
        if (c.harvestStart.isValid()) m_harvestStart->setDate(c.harvestStart);
        else m_harvestStart->setDate(QDate::currentDate());
        if (c.harvestEnd.isValid())   m_harvestEnd->setDate(c.harvestEnd);
        else m_harvestEnd->setDate(QDate::currentDate());
    } else {
        m_hasHarvest->setChecked(false);
        m_harvestStart->setDate(QDate::currentDate());
        m_harvestEnd->setDate(QDate::currentDate());
    }
}

CropWindow CropEditDialog::crop() const {
    CropWindow c;
    c.name = m_nameEdit->text().trimmed();

    if (m_hasSow->isChecked()) {
        c.sowStart = m_sowStart->date();
        c.sowEnd = m_sowEnd->date();
    }
    if (m_hasPlant->isChecked()) {
        c.plantStart = m_plantStart->date();
        c.plantEnd = m_plantEnd->date();
    }
    if (m_hasHarvest->isChecked()) {
        c.harvestStart = m_harvestStart->date();
        c.harvestEnd = m_harvestEnd->date();
    }
    return c;
}
