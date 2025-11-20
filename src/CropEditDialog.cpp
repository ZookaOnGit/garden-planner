#include "CropEditDialog.h"
#include "SettingsManager.h"


CropEditDialog::CropEditDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Add / Edit Crop");
    setMinimumSize(600, 400);

    const int DAYS_FIELD_WIDTH = 70;
    
    auto* mainLay = new QVBoxLayout(this);

    auto* form = new QFormLayout;
    m_nameEdit = new QLineEdit;
    form->addRow("Name:", m_nameEdit);
    auto* daysValidator = new QIntValidator(0, 10000, this); // for the days fields
    auto* positiveDaysValidator = new QIntValidator(1, 10000, this); // for germination days field

    // Sow
    {
        auto* h = new QHBoxLayout;
        m_hasSow = new QCheckBox("Sow from");
        m_sowStart = new QDateEdit;
        m_sowStart->setCalendarPopup(true);
        m_sowEnd = new QDateEdit;
        m_sowEnd->setCalendarPopup(true);
        m_sowStart->setDisplayFormat("dd-MM-yyyy");
        m_sowEnd->setDisplayFormat("dd-MM-yyyy");
        m_sowStart->setDate(QDate::currentDate());
        QLabel* toLabel = new QLabel("to");
        toLabel->setAlignment(Qt::AlignCenter);
        toLabel->setMinimumWidth(20);
        toLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_sowEnd->setDate(QDate::currentDate());
        m_sowDays = new QLineEdit;
        m_sowDays->setValidator(daysValidator);
        m_sowDays->setPlaceholderText(tr("0"));
        m_sowDays->setAlignment(Qt::AlignRight);
        m_sowDays->setMaximumWidth(DAYS_FIELD_WIDTH);
        m_sowDays->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_sowDaysLabel = new QLabel;
        h->addWidget(m_hasSow);
        h->addWidget(m_sowStart);
        h->addWidget(toLabel);
        h->addWidget(m_sowEnd);
        h->addWidget(m_sowDays);
        h->addWidget(m_sowDaysLabel);
        form->addRow(h);
    }

    // Plant
    {
        auto* h = new QHBoxLayout;
        m_hasPlant = new QCheckBox("Germination takes");

        m_plantStartDays = new QLineEdit;
        m_plantStartDays->setValidator(positiveDaysValidator);
        m_plantStartDays->setText(tr("1"));
        m_plantStartDays->setAlignment(Qt::AlignRight);
        m_plantStartDays->setMaximumWidth(DAYS_FIELD_WIDTH);

        QLabel* toLabel = new QLabel("to");
        toLabel->setAlignment(Qt::AlignCenter);
        toLabel->setMinimumWidth(20);
        toLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        
        m_plantEndDays = new QLineEdit;
        m_plantEndDays->setValidator(positiveDaysValidator);
        m_plantEndDays->setText(tr("1"));
        m_plantEndDays->setAlignment(Qt::AlignRight);
        m_plantEndDays->setMaximumWidth(DAYS_FIELD_WIDTH);

        m_plantStart = new QDateEdit;
        //m_plantStart->setCalendarPopup(true);
        m_plantStart->hide();
        m_plantEnd = new QDateEdit;
        //m_plantEnd->setCalendarPopup(true);
        m_plantEnd->hide();
        //m_plantStart->setDisplayFormat("dd-MM-yyyy");
        //m_plantEnd->setDisplayFormat("dd-MM-yyyy");
        m_plantStart->setDate(m_sowStart->date().addDays(m_plantStartDays->text().toInt()));
        m_plantEnd->setDate(m_sowEnd->date().addDays(m_plantEndDays->text().toInt()));

        /*m_plantDays = new QLineEdit;
        m_plantDays->setValidator(daysValidator);
        m_plantDays->setPlaceholderText(tr("0"));
        m_plantDays->setAlignment(Qt::AlignRight);
        m_plantDays->setMaximumWidth(DAYS_FIELD_WIDTH);
        m_plantDays->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);*/
        m_plantDaysLabel = new QLabel;
        h->addWidget(m_hasPlant);
        h->addWidget(m_plantStartDays);
        h->addWidget(toLabel);
        h->addWidget(m_plantEndDays);
        //h->addWidget(m_plantStart);
        //h->addWidget(m_plantEnd);
        //h->addWidget(m_plantDays);
        h->addWidget(m_plantDaysLabel);
        form->addRow(h);
    }

    // Harvest
    {
        auto* h = new QHBoxLayout;
        m_hasHarvest = new QCheckBox("Harvest in");

        m_harvestStartDays = new QLineEdit;
        m_harvestStartDays->setValidator(positiveDaysValidator);
        m_harvestStartDays->setPlaceholderText(tr("1"));
        m_harvestStartDays->setAlignment(Qt::AlignRight);
        m_harvestStartDays->setMaximumWidth(DAYS_FIELD_WIDTH);

        QLabel* toLabel = new QLabel("to");
        toLabel->setAlignment(Qt::AlignCenter);
        toLabel->setMinimumWidth(20);
        toLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);      
        
        m_harvestEndDays = new QLineEdit;
        m_harvestEndDays->setValidator(positiveDaysValidator);
        m_harvestEndDays->setText(tr("1"));
        m_harvestEndDays->setAlignment(Qt::AlignRight);
        m_harvestEndDays->setMaximumWidth(DAYS_FIELD_WIDTH);        

        m_harvestStart = new QDateEdit;
        //m_harvestStart->setCalendarPopup(true);
        m_harvestStart->hide();
        m_harvestEnd = new QDateEdit;
        //m_harvestEnd->setCalendarPopup(true);
        m_harvestEnd->hide();
        //m_harvestStart->setDisplayFormat("dd-MM-yyyy");
        //m_harvestEnd->setDisplayFormat("dd-MM-yyyy");
        m_harvestStart->setDate(m_plantStart->date().addDays(m_harvestStartDays->text().toInt()));
        m_harvestEnd->setDate(m_plantEnd->date().addDays(m_harvestEndDays->text().toInt()));

        /*m_harvestDays = new QLineEdit;
        m_harvestDays->setValidator(daysValidator);
        m_harvestDays->setPlaceholderText(tr("0"));
        m_harvestDays->setAlignment(Qt::AlignRight);        
        m_harvestDays->setMaximumWidth(DAYS_FIELD_WIDTH);
        m_harvestDays->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);*/
        m_harvestDaysLabel = new QLabel;
        h->addWidget(m_hasHarvest);
        h->addWidget(m_harvestStartDays);
        h->addWidget(toLabel);
        h->addWidget(m_harvestEndDays);
        //h->addWidget(m_harvestStart);
        //h->addWidget(m_harvestEnd);
        //h->addWidget(m_harvestDays);
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

    updateSowDays();

    connect(m_sowStart, &QDateEdit::dateChanged, this, &CropEditDialog::updateSowDays);
    connect(m_sowEnd, &QDateEdit::dateChanged, this, &CropEditDialog::updateSowDays);
    connect(m_sowDays, &QLineEdit::textChanged, this, &CropEditDialog::updateSowEndDate);
    connect(m_plantStartDays, &QLineEdit::textChanged, this, &CropEditDialog::updateGerminationDates);
    connect(m_plantEndDays, &QLineEdit::textChanged, this, &CropEditDialog::updateGerminationDates);
    connect(m_harvestStartDays, &QLineEdit::textChanged, this, &CropEditDialog::updateHarvestDates);
    connect(m_harvestEndDays, &QLineEdit::textChanged, this, &CropEditDialog::updateHarvestDates);

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
        if (!checkRange(m_hasPlant, m_plantStart, m_plantEnd, "Germination")) return;
        if (!checkRange(m_hasHarvest, m_harvestStart, m_harvestEnd, "Harvest")) return;

        accept();
    });
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

    // Default state: checkboxes on
    m_hasSow->setChecked(true);
    m_hasPlant->setChecked(true);
    m_hasHarvest->setChecked(true);
}

void CropEditDialog::updateSowDays()
{
    int n = m_sowStart->date().daysTo(m_sowEnd->date());
    m_sowDays->setText(tr("%1").arg(n));
    m_sowDaysLabel->setText(tr("%1").arg(n == 1 ? tr("day") : tr("days")));
    // changing sow dates affects germination dates
    updateGerminationDates();
}

// update end date calendar when sow days field changes
void CropEditDialog::updateSowEndDate()
{
    if (m_sowDays->text().isEmpty()) {
        QSignalBlocker blocker(m_sowEnd);
        m_sowDays->setText("1");
        m_sowEnd->setDate(m_sowStart->date().addDays(m_sowDays->text().toInt()));
        m_sowDays->selectAll();
    }

    bool ok = false;
    int days = m_sowDays->text().toInt(&ok);
    if (ok) {
        QSignalBlocker blocker(m_sowEnd); // prevent recursion
        QDate newEnd = m_sowStart->date().addDays(days);
        m_sowEnd->setDate(newEnd);
    }
    else {
        m_sowEnd->setDate(m_sowStart->date());
    }

    updateGerminationDates();
}

// update germination dates when plant days fields change
void CropEditDialog::updateGerminationDates()
{
    if (m_plantStartDays->text().isEmpty()) {
        QSignalBlocker blocker(m_plantStartDays);
        m_plantStartDays->setText("1");
        m_plantStart->setDate(m_sowStart->date().addDays(m_plantStartDays->text().toInt()));
        m_plantStartDays->selectAll();
    }
    if (m_plantEndDays->text().isEmpty()) {
        QSignalBlocker blocker(m_plantEndDays);
        m_plantEndDays->setText("1");
        m_plantEnd->setDate(m_sowEnd->date().addDays(m_plantEndDays->text().toInt()));
        m_plantEndDays->selectAll();
    }

    bool ok = false;
    int days = m_plantStartDays->text().toInt(&ok);
    if (ok) {
        QSignalBlocker blocker(m_plantStartDays); // prevent recursion
        QDate newStart = m_sowStart->date().addDays(days);
        m_plantStart->setDate(newStart);
    }
    else {
        // reset to current date if invalid, validator should prevent this
        m_plantStart->setDate(m_sowStart->date().addDays(1));
    }
    ok = false;
    days = m_plantEndDays->text().toInt(&ok);
    if (ok) {
        QSignalBlocker blocker(m_plantEndDays); // prevent recursion
        QDate newEnd = m_sowEnd->date().addDays(days);
        m_plantEnd->setDate(newEnd);
    }
    else {
        // reset to current date if invalid, validator should prevent this
        m_plantEnd->setDate(m_sowEnd->date().addDays(1));
    }

    m_plantDaysLabel->setText(tr("%1 (%2 -> %3)").arg(m_plantEndDays->text().toInt() == 1 ? tr("day") : tr("days")).arg(m_plantStart->date().toString("dd-MM-yyyy")).arg(m_plantEnd->date().toString("dd-MM-yyyy")));
    // changing germination dates affects harvest dates
    updateHarvestDates();    
}

// update harvest dates
void CropEditDialog::updateHarvestDates()
{
    if (m_harvestStartDays->text().isEmpty()) {
        QSignalBlocker blocker(m_harvestStartDays);
        m_harvestStartDays->setText("1");
        m_harvestStart->setDate(m_sowStart->date().addDays(m_harvestStartDays->text().toInt()));
        m_harvestStartDays->selectAll();
    }
    if (m_harvestEndDays->text().isEmpty()) {
        QSignalBlocker blocker(m_harvestEndDays);
        m_harvestEndDays->setText("1");
        m_harvestEnd->setDate(m_sowEnd->date().addDays(m_harvestEndDays->text().toInt()));
        m_harvestEndDays->selectAll();
    }

    bool ok = false;
    int days = m_harvestStartDays->text().toInt(&ok);
    if (ok) {
        QSignalBlocker blocker(m_harvestStartDays); // prevent recursion
        QDate newStart = m_plantStart->date().addDays(days);
        m_harvestStart->setDate(newStart);
    }
    else {
        // reset to current date if invalid, validator should prevent this
        m_harvestStart->setDate(m_plantStart->date().addDays(1));
    }
    ok = false;
    days = m_harvestEndDays->text().toInt(&ok);
    if (ok) {
        QSignalBlocker blocker(m_harvestEndDays); // prevent recursion
        QDate newEnd = m_plantEnd->date().addDays(days);
        m_harvestEnd->setDate(newEnd);
    }
    else {
        // reset to current date if invalid, validator should prevent this
        m_harvestEnd->setDate(m_plantEnd->date().addDays(1));
    }

    m_harvestDaysLabel->setText(tr("%1 (%2 -> %3)").arg(m_harvestEndDays->text().toInt() == 1 ? tr("day") : tr("days")).arg(m_harvestStart->date().toString("dd-MM-yyyy")).arg(m_harvestEnd->date().toString("dd-MM-yyyy")));
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
        m_sowStart->setDate(QDate::currentDate());
        m_sowEnd->setDate(QDate::currentDate());
    }

    if (c.plantStart.isValid() || c.plantEnd.isValid()) {
        m_hasPlant->setChecked(true);
        if (c.plantStart.isValid()) m_plantStart->setDate(c.plantStart);
        else m_plantStart->setDate(QDate::currentDate());
        if (c.plantEnd.isValid())   m_plantEnd->setDate(c.plantEnd);
        else m_plantEnd->setDate(QDate::currentDate());

        m_plantStartDays->setText(QString::number(c.sowStart.daysTo(c.plantStart)));
        m_plantEndDays->setText(QString::number(c.sowEnd.daysTo(c.plantEnd)));        
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

        m_harvestStartDays->setText(QString::number(c.plantStart.daysTo(c.harvestStart)));
        m_harvestEndDays->setText(QString::number(c.plantEnd.daysTo(c.harvestEnd)));
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
