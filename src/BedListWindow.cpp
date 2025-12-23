#include "BedListWindow.h"
#include "BedModel.h"
#include "BedGanttWidget.h"
#include <QVBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QScrollArea>
#include <QSettings>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCheckBox>
#include <QCollator>

BedListWindow::BedListWindow(QWidget* parent) : QMainWindow(parent) {
    m_model = new BedModel(this);
    // Load persisted bed count (default 10)
    QSettings settings;
    int persistedBeds = settings.value("BedPlanner/BedCount", 10).toInt();
    // Try to load from DB; fallback to defaults if DB not available
    m_model->loadFromDatabase();

    m_gantt = new BedGanttWidget(this);
    m_gantt->setModel(m_model);
    m_gantt->setDayWidth(6);
    m_gantt->setRowCount(persistedBeds);
    m_gantt->setMinimumSize(m_gantt->sizeHint());

    QWidget* central = new QWidget(this);
    auto* lay = new QVBoxLayout(central);
    lay->setContentsMargins(0,0,0,0);
    // Wrap gantt in a scroll area so scrollbars appear when content is larger than viewport
    QScrollArea* scr = new QScrollArea(central);
    scr->setWidgetResizable(false);
    scr->setWidget(m_gantt);
    // Disable horizontal scrollbar, keep vertical as-needed
    scr->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scr->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    lay->addWidget(scr);
    setCentralWidget(central);

    setWindowTitle("Bed Planner");
    setWindowModality(Qt::WindowModal);
    resize(900, 500);

    auto* tb = addToolBar("Actions");

    QAction* add = tb->addAction("Add Crop");
    connect(add, &QAction::triggered, [this](){
        // add at first bed, today, using selected type if any
        BedCrop c;
        QVariant v = m_cropCombo->currentData();
        if (v.isValid() && v.canConvert<int>()) {
            c.name = m_cropCombo->currentText();
            c.lengthDays = v.toInt();
        } else {
            c.name = "New Crop";
            c.lengthDays = 30;
        }
        c.bed = 0;
        c.start = QDate::currentDate();
        m_model->addCrop(c);
    });

    m_cropCombo = new QComboBox(tb);
    m_cropCombo->addItem("(Custom)", QVariant());
    tb->addWidget(m_cropCombo);

    m_includeHidden = new QCheckBox("Include hidden", tb);
    m_includeHidden->setToolTip("Include crops marked hidden in the catalog");
    tb->addWidget(m_includeHidden);

    tb->addSeparator();


    // Add a stretch spacer so subsequent widgets appear at the far right
    QWidget* spacer = new QWidget(tb);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tb->addWidget(spacer);

    // Bed count label + spinbox on the right
    QLabel* bedsLabel = new QLabel("Beds:", tb);
    tb->addWidget(bedsLabel);

    m_bedCount = new QSpinBox(tb);
    m_bedCount->setRange(1, 500);
    m_bedCount->setValue(persistedBeds);
    m_bedCount->setToolTip("Number of beds to display");
    tb->addWidget(m_bedCount);

    // initial load (exclude hidden by default)
    loadCropTypes(false);

    connect(m_cropCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int){
        QVariant v = m_cropCombo->currentData();
        if (v.isValid() && v.canConvert<int>()) {
            m_gantt->setDefaultTemplate(m_cropCombo->currentText(), v.toInt());
        } else {
            m_gantt->setDefaultTemplate(QString(), 30);
        }
    });

    connect(m_includeHidden, &QCheckBox::toggled, [this](bool checked){
        loadCropTypes(checked);
    });

    connect(m_bedCount, QOverload<int>::of(&QSpinBox::valueChanged), [this](int v){
        // change the visible rows in the gantt; do not clear the model
        m_gantt->setRowCount(v);
        // ensure the scroll area will show correct preferred size
        m_gantt->setMinimumSize(m_gantt->sizeHint());
        // persist the value
        QSettings settings;
        settings.setValue("BedPlanner/BedCount", v);
    });


    QAction* clear = tb->addAction("Clear All");
    clear->setToolTip("Remove all crops from the planner");
    connect(clear, &QAction::triggered, [this](){
        auto res = QMessageBox::question(this, "Confirm clear", "Remove all crops? This cannot be undone.",
                                         QMessageBox::Yes | QMessageBox::No);
        if (res == QMessageBox::Yes) {
            if (m_model) m_model->clearAllCrops();
        }
    });
}

BedListWindow::~BedListWindow() {}

void BedListWindow::loadCropTypes(bool includeHidden) {
    m_cropCombo->blockSignals(true);
    m_cropCombo->clear();
    m_cropCombo->addItem("(Custom)", QVariant());

    // Build query string based on includeHidden
    QString sql = "SELECT name, plant_start, harvest_start FROM crops WHERE name IS NOT NULL";
    if (!includeHidden) sql += " AND hide != 'true'";

    QSqlQuery q(sql);
    if (!q.isActive()) {
        qDebug() << "Failed to query crops:" << q.lastError().text();
        m_cropCombo->blockSignals(false);
        return;
    }

    struct Item { QString name; int len; };
    QList<Item> items;
    while (q.next()) {
        QString name = q.value(0).toString();
        QDate plant = q.value(1).toDate();
        QDate harvest = q.value(2).toDate();
        int len = 30;
        if (plant.isValid() && harvest.isValid()) len = plant.daysTo(harvest);
        items.append({name, len});
    }

    // Sort alphabetically using locale-aware collator
    QCollator coll;
    coll.setNumericMode(true);
    std::sort(items.begin(), items.end(), [&](const Item &a, const Item &b){
        return coll.compare(a.name, b.name) < 0;
    });

    for (const auto &it : items) {
        m_cropCombo->addItem(it.name, it.len);
    }
    m_cropCombo->blockSignals(false);
}
