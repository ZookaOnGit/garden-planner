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
#include <QScrollBar>
#include <QSettings>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCheckBox>
#include <QCollator>
#include <QPainter>
#include <QMouseEvent>
#include <QPixmap>
#include "Theme.h"

// Small header widget that draws the timeline dates and stays fixed at top.
class DateHeader : public QWidget {
public:
    DateHeader(BedGanttWidget* target, QWidget* parent = nullptr) : QWidget(parent), m_target(target) {
        setFixedHeight(28);
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.fillRect(rect(), Theme::WindowBackground);
        if (!m_target) return;
        int left = m_target->leftMargin();
        int dayW = m_target->dayWidth();
        QDate ref = m_target->refDate();
        int widthAvail = width();
        int daysToShow = (widthAvail - left) / dayW;
        p.setPen(Theme::TextPrimary);
        // use bold labels
        QFont f = font();
        f.setBold(true);
        p.setFont(f);
        QLocale loc = QLocale::system();

        // Determine a sensible step (in days) so labels don't overlap.
        // Measure a representative label width and ensure spacing >= width + padding.
        QFontMetrics fm(p.font());
        QDate sampleDate = ref;
        QString sampleText = loc.toString(sampleDate, QLocale::ShortFormat);
        int sampleW = fm.horizontalAdvance(sampleText) + 12; // padding
        int dayStep = 1;
        if (dayW > 0) dayStep = qMax(1, int(std::ceil(double(sampleW) / double(dayW))));

        // Prefer week-aligned steps when possible (1,7,14,28)
        if (dayStep > 1) {
            if (dayStep <= 7) dayStep = 7;
            else if (dayStep <= 14) dayStep = 14;
            else if (dayStep <= 28) dayStep = 28;
            // else keep the computed dayStep (covers very narrow displays)
        }

        for (int d = 0; d < daysToShow; d += dayStep) {
            int x = left + d * dayW;
            QDate date = ref.addDays(d);
            QString dateText = loc.toString(date, QLocale::ShortFormat);
            p.drawText(x + 2, height()/2 + fm.ascent()/2 + 2, dateText);
        }
    }

    void mousePressEvent(QMouseEvent* evt) override {
        if (!m_target) return;
        if (evt->button() == Qt::LeftButton) {
            m_dragging = true;
            m_lastPos = evt->pos();
            setCursor(Qt::ClosedHandCursor);
        }
    }

    void mouseMoveEvent(QMouseEvent* evt) override {
        if (!m_target) return;
        if (!m_dragging) return;
        QPoint pos = evt->pos();
        int dx = pos.x() - m_lastPos.x();
        if (dx != 0) {
            // use the public facade to pan the gantt by pixels
            m_target->panByPixels(dx);
            m_lastPos = pos;
        }
    }

    void mouseReleaseEvent(QMouseEvent* evt) override {
        Q_UNUSED(evt);
        if (m_dragging) {
            m_dragging = false;
            setCursor(Qt::ArrowCursor);
        }
    }

private:
    BedGanttWidget* m_target{nullptr};
    bool m_dragging{false};
    QPoint m_lastPos;
};

// Frozen left-column widget that draws bed labels and stays fixed while the gantt scrolls horizontally.
class BedLabelsWidget : public QWidget {
public:
    BedLabelsWidget(BedGanttWidget* target, QScrollArea* scr, QWidget* parent = nullptr)
        : QWidget(parent), m_target(target), m_scr(scr) {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        if (m_target) setFixedWidth(m_target->leftMargin());
        if (m_scr && m_scr->verticalScrollBar()) {
            connect(m_scr->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int v){ m_scroll = v; update(); });
        }
        if (m_target) connect(m_target, &BedGanttWidget::viewChanged, this, [this](){ if (m_target) setFixedWidth(m_target->leftMargin()); update(); });
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.fillRect(rect(), Theme::WindowBackground);
        if (!m_target) return;
        int rows = m_target->rowCount();
        int rowH = m_target->rowHeight();
        // bold labels
        QFont f = font(); f.setBold(true); p.setFont(f);
        QFontMetrics fm(p.font());
        for (int i = 0; i < rows; ++i) {
            int y = i * rowH - m_scroll;
            // skip rows outside viewport to reduce overdraw
            if (y + rowH < 0) continue;
            if (y > height()) break;
            // separator line
            p.setPen(Theme::LightGrid);
            p.drawLine(0, y + rowH - 1, width(), y + rowH - 1);
            p.setPen(Theme::TextPrimary);
            // draw label with a small left padding
            int textX = 6;
            p.drawText(textX, y + (rowH/2) + fm.ascent()/2 - fm.descent()/2, QString("Bed %1").arg(i+1));

            // check crops in this row and draw a small red flag on the right if any crop starts outside the recommended planting range
            bool showFlag = false;
            if (m_target && m_target->model()) {
                BedModel* model = m_target->model();
                for (const auto &c : model->crops()) {
                    if (c.bed != i) continue;
                    if (c.name.isEmpty()) continue;
                    QSqlQuery q;
                    q.prepare("SELECT plant_start, plant_end FROM crops WHERE name = :name LIMIT 1");
                    q.bindValue(":name", c.name);
                    if (q.exec() && q.next()) {
                        QString ps = q.value(0).toString();
                        QString pe = q.value(1).toString();
                        QDate pstart = QDate::fromString(ps, Qt::ISODate);
                        QDate pend = QDate::fromString(pe, Qt::ISODate);
                        if (!pstart.isValid()) pstart = q.value(0).toDate();
                        if (!pend.isValid()) pend = q.value(1).toDate();
                        if (pstart.isValid() && pend.isValid()) {
                            if (c.start < pstart || c.start > pend) { showFlag = true; break; }
                        }
                    }
                }
            }

            if (showFlag) {
                // draw embedded resource flag scaled to fit
                static QPixmap s_labelFlag;
                if (s_labelFlag.isNull()) s_labelFlag.load(":/icons/red-flag.png");
                int pad = 6;
                int flagH = qMax(8, rowH * 6 / 10);
                int flagW = flagH; // square area
                int flagX = width() - pad - flagW;
                int flagY = y + (rowH - flagH)/2;
                if (!s_labelFlag.isNull()) {
                    QPixmap scaled = s_labelFlag.scaled(flagW, flagH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    int drawX = flagX + (flagW - scaled.width())/2;
                    int drawY = flagY + (flagH - scaled.height())/2;
                    p.drawPixmap(drawX, drawY, scaled);
                } else {
                    // fallback pennant
                    int poleWidth = 2;
                    int cx = width() - pad - poleWidth;
                    int cy = y + rowH/2 - flagH/2;
                    p.setPen(Qt::NoPen);
                    p.setBrush(Theme::TextDark);
                    p.drawRect(cx, cy, poleWidth, flagH);
                    QPolygon poly;
                    poly << QPoint(cx - 1, cy)
                         << QPoint(cx - 1 - 8, cy + flagH/2)
                         << QPoint(cx - 1, cy + flagH);
                    p.setBrush(QColor(220,50,50));
                    p.drawPolygon(poly);
                }
            }
        }
    }

private:
    BedGanttWidget* m_target{nullptr};
    QScrollArea* m_scr{nullptr};
    int m_scroll{0};
};

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
    // fixed header that shows dates and stays visible while rows scroll
    DateHeader* header = new DateHeader(m_gantt, central);
    lay->addWidget(header);
    // Wrap gantt in a scroll area so vertical scrollbar appears when content is larger than viewport
    QScrollArea* scr = new QScrollArea(central);
    scr->setWidgetResizable(false);
    scr->setWidget(m_gantt);
    // Disable horizontal scrollbar, keep vertical as-needed
    scr->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scr->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // Create a horizontal row containing a frozen labels column and the gantt scroll area.
    QWidget* contentRow = new QWidget(central);
    auto* hLay = new QHBoxLayout(contentRow);
    hLay->setContentsMargins(0,0,0,0);
    hLay->setSpacing(0);

    BedLabelsWidget* labels = new BedLabelsWidget(m_gantt, scr, contentRow);
    hLay->addWidget(labels);
    hLay->addWidget(scr);

    lay->addWidget(contentRow);
    setCentralWidget(central);

    // keep header in sync with view changes (pan/zoom/dayWidth)
    connect(m_gantt, &BedGanttWidget::viewChanged, header, QOverload<>::of(&QWidget::update));

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
