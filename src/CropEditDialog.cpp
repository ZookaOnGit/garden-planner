#include "CropEditDialog.h"
#include "SettingsManager.h"


CropEditDialog::CropEditDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Add / Edit Crop");
    setMinimumSize(500, 400);
    
    auto* mainLay = new QVBoxLayout(this);

    auto* form = new QFormLayout;
    m_nameEdit = new QLineEdit;
    form->addRow("Name:", m_nameEdit);
    auto* daysValidator = new QIntValidator(0, 100000, this); // for the days fields

    // Labels
    {
        auto* h = new QHBoxLayout;
        h->addWidget(new QLabel(""));
        h->addWidget(new QLabel("Start"));
        h->addWidget(new QLabel("End"));
        h->addWidget(new QLabel("Duration"));
        form->addRow(h);
    }
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
        m_sowStart->setDate(QDate::currentDate());
        m_sowEnd->setDate(QDate::currentDate());
        m_sowDays = new QLineEdit;
        m_sowDays->setValidator(daysValidator);
        m_sowDays->setPlaceholderText(tr("0"));
        m_sowDays->setAlignment(Qt::AlignRight);
        m_sowDays->setMinimumWidth(5);
        m_sowDays->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_sowDaysLabel = new QLabel;
        h->addWidget(m_hasSow);
        h->addWidget(m_sowStart);
        h->addWidget(m_sowEnd);
        h->addWidget(m_sowDays);
        h->addWidget(m_sowDaysLabel);
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
        m_plantDays = new QLineEdit;
        m_plantDays->setValidator(daysValidator);
        m_plantDays->setPlaceholderText(tr("0"));
        m_plantDays->setAlignment(Qt::AlignRight);
        m_plantDays->setMinimumWidth(5);
        m_plantDays->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_plantDaysLabel = new QLabel;
        h->addWidget(m_hasPlant);
        h->addWidget(m_plantStart);
        h->addWidget(m_plantEnd);
        h->addWidget(m_plantDays);
        h->addWidget(m_plantDaysLabel);
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
        m_harvestDays = new QLineEdit;
        m_harvestDays->setValidator(daysValidator);
        m_harvestDays->setPlaceholderText(tr("0"));
        m_harvestDays->setAlignment(Qt::AlignRight);        
        m_harvestDays->setMinimumWidth(5);
        m_harvestDays->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_harvestDaysLabel = new QLabel;
        h->addWidget(m_hasHarvest);
        h->addWidget(m_harvestStart);
        h->addWidget(m_harvestEnd);
        h->addWidget(m_harvestDays);
        h->addWidget(m_harvestDaysLabel);
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
    btnLay->addSpacing(12);
    btnLay->addWidget(cancel);
    btnLay->addStretch();
    mainLay->addLayout(btnLay);

    // update days count when dates fields change
    auto updateDays = [this]() {
        qDebug() << "updating days labels";
        int n = m_sowStart->date().daysTo(m_sowEnd->date());
        m_sowDays->setText(tr("%1").arg(n));
        m_sowDaysLabel->setText(tr("%1").arg(n == 1 ? tr("day") : tr("days")));

        n = m_plantStart->date().daysTo(m_plantEnd->date());
        m_plantDays->setText(tr("%1").arg(n));
        m_plantDaysLabel->setText(tr("%1").arg(n == 1 ? tr("day") : tr("days")));

        n = m_harvestStart->date().daysTo(m_harvestEnd->date());
        m_harvestDays->setText(tr("%1").arg(n));
        m_harvestDaysLabel->setText(tr("%1").arg(n == 1 ? tr("day") : tr("days")));
    };
    connect(m_sowStart, &QDateEdit::dateChanged, this, updateDays);
    connect(m_sowEnd, &QDateEdit::dateChanged, this, updateDays);
    connect(m_plantStart, &QDateEdit::dateChanged, this, updateDays);
    connect(m_plantEnd, &QDateEdit::dateChanged, this, updateDays);
    connect(m_harvestStart, &QDateEdit::dateChanged, this, updateDays);
    connect(m_harvestEnd, &QDateEdit::dateChanged, this, updateDays);
    updateDays();

    // update end date calendar when days field changes
    auto updateEndDate = [this](QLineEdit* daysEdit, QDateEdit* startEdit, QDateEdit* endEdit) {
        if (daysEdit->text().isEmpty()) {
            QSignalBlocker blocker(endEdit);
            daysEdit->setText("0");
            endEdit->setDate(startEdit->date());
            daysEdit->selectAll();
            return;
        }

        bool ok = false;
        int days = daysEdit->text().toInt(&ok);
        if (ok) {
            QSignalBlocker blocker(endEdit); // prevent recursion
            QDate newEnd = startEdit->date().addDays(days);
            endEdit->setDate(newEnd);
        }
        else {
            // reset to start date if invalid, validator should prevent this
            endEdit->setDate(startEdit->date());
            return;
        }
    };
    connect(m_sowDays, &QLineEdit::textChanged, this, [this, updateEndDate]() {
        updateEndDate(m_sowDays, m_sowStart, m_sowEnd);
    });
    connect(m_plantDays, &QLineEdit::textChanged, this, [this, updateEndDate]() {
        updateEndDate(m_plantDays, m_plantStart, m_plantEnd);
    });
    connect(m_harvestDays, &QLineEdit::textChanged, this, [this, updateEndDate]() {
        updateEndDate(m_harvestDays, m_harvestStart, m_harvestEnd);
    });


    // validate and save on OK
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

    m_notesEdit->setText(c.notes);
}

CropWindow CropEditDialog::crop() const {
    CropWindow c;
    c.name = m_nameEdit->text().trimmed();
    c.notes = m_notesEdit->toPlainText().trimmed();

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
