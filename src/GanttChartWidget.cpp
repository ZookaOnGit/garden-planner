
#include "GanttChartWidget.h"
#include <QPainter>
#include <QFontMetrics>
#include <QPen>
#include <QMouseEvent>
#include <QToolTip>
#include "Theme.h"
#include <algorithm>
#include <limits>

GanttChartWidget::GanttChartWidget(QWidget* parent) : QWidget(parent) {
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Theme::WindowBackground);
    setPalette(pal);
    setMouseTracking(true);
}

void GanttChartWidget::setLeftMargin(int pixels) {
    m_leftWidthOverride = std::max(0, pixels);
    updateGeometry();
    setMinimumSize(sizeHint());
    update();
}

void GanttChartWidget::setTopMargin(int pixels) {
    m_topMargin = std::max(0, pixels);
    updateGeometry();
    setMinimumSize(sizeHint());
    update();
}

int GanttChartWidget::topMargin() const {
    return m_topMargin;
}

void GanttChartWidget::setItems(QVector<CropWindow> items) {
    m_items = std::move(items);
    computeRange();
    updateGeometry();
    // Ensure the widget reports its content size as a minimum size so a
    // parent QScrollArea (when not setWidgetResizable) can show scrollbars
    // based on the content dimension.
    setMinimumSize(sizeHint());
    update();
}

void GanttChartWidget::setDayWidth(int pixelsPerDay) {
    m_dayWidth = std::max(1, pixelsPerDay);
    updateGeometry();
    setMinimumSize(sizeHint());
    update();
}

void GanttChartWidget::showTodayLine(bool on) {
    m_showToday = on;
    update();
}

void GanttChartWidget::showSubrowLabels(bool on) {
    m_showSubLabels = on;
    update();
}

int GanttChartWidget::blockHeight() const {
    return 3 * m_subRowHeight + 2 * m_subRowGap;
}

int GanttChartWidget::laneTopY(int cropIndex, int laneIndex) const {
    int blockTop = m_topMargin + cropIndex * (blockHeight() + m_blockGap);
    int laneOffset = laneIndex * (m_subRowHeight + m_subRowGap);
    return blockTop + laneOffset;
}

QSize GanttChartWidget::sizeHint() const {
    int height = m_topMargin + m_items.size() * (blockHeight() + m_blockGap) + 40;
    int width = totalDays() * m_dayWidth + m_rightMargin;
    return { std::max(width, 800), std::max(height, 400) };
}

int GanttChartWidget::totalDays() const {
    if (!m_minDate.isValid() || !m_maxDate.isValid()) return 365;
    qint64 td = m_minDate.daysTo(m_maxDate) + 1; // qint64 in Qt6
    if (td < 1) td = 1;
    if (td > std::numeric_limits<int>::max()) td = std::numeric_limits<int>::max();
    return static_cast<int>(td);
}

int GanttChartWidget::dateToX(const QDate& d) const {
    if (!d.isValid() || !m_minDate.isValid()) return m_leftMargin;
    qint64 days64 = m_minDate.daysTo(d);
    qint64 lo = 0;
    qint64 hi = static_cast<qint64>(totalDays() - 1);
    if (days64 < lo) days64 = lo;
    if (days64 > hi) days64 = hi;
    int days = static_cast<int>(days64);

    return days * m_dayWidth;
}

int GanttChartWidget::xForDate(const QDate& d) const {
    return dateToX(d);
}

void GanttChartWidget::computeRange() {
    bool first = true;
    QDate minD, maxD;
    auto consider = [&](const QDate& a, const QDate& b){
        if (!isValidRange(a,b)) return;
        if (first) { minD = a; maxD = b; first = false; }
        else {
            if (a < minD) minD = a;
            if (b > maxD) maxD = b;
        }
    };

    for (const auto& c : m_items) {
        consider(c.sowStart, c.sowEnd);
        consider(c.plantStart, c.plantEnd);
        consider(c.harvestStart, c.harvestEnd);
    }

    if (first) {
        QDate today = QDate::currentDate();
        m_minDate = QDate(today.year(), 1, 1);
        m_maxDate = QDate(today.year(), 12, 31);
    } else {
        m_minDate = QDate(minD.year(), minD.month(), 1);
        QDate maxMonthEnd(maxD.year(), maxD.month(), 1);
        maxMonthEnd = maxMonthEnd.addMonths(1).addDays(-1);
        m_maxDate = maxMonthEnd;
    }
}

void GanttChartWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    drawGrid(p);
    drawHeader(p);
    drawBars(p);
    drawWeekLines(p);
}

void GanttChartWidget::drawHeader(QPainter& p) {
    QFont bold = p.font(); bold.setBold(true); p.setFont(bold);
    const int y = m_topMargin - 20;

    p.setPen(Theme::TextPrimary);

    QDate d = QDate(m_minDate.year(), m_minDate.month(), 1);
    while (d <= m_maxDate) {
        int x = dateToX(d);
        QString label = d.toString("MMM yyyy");
        p.drawText(x+4, y, label);

        p.setPen(Theme::GridLine);
        p.drawLine(x, m_topMargin, x, height());
        p.setPen(Theme::TextPrimary);
        d = d.addMonths(1);
    }

    // Draw week ticks: find the first Monday on/after m_minDate and draw a
    // short vertical tick for each week boundary. This visually delineates
    // weeks inside the month grid without drawing full-height grid lines.
    QDate wk = m_minDate;
    // In Qt, dayOfWeek(): 1 = Monday, 7 = Sunday.
    while (wk <= m_maxDate && wk.dayOfWeek() != 1) wk = wk.addDays(1);
    p.setPen(Theme::WeekLine);
    const int tickTop = m_topMargin - 12;
    const int tickBottom = m_topMargin - 2;
    while (wk <= m_maxDate) {
        int tx = dateToX(wk);
        p.drawLine(tx, tickTop, tx, tickBottom);
        wk = wk.addDays(7);
    }

    p.setPen(Theme::HeaderSeparator);
    p.drawLine(0, m_topMargin-8, dateToX(m_maxDate), m_topMargin-8);

    if (m_showToday) {
        QDate today = QDate::currentDate();
        if (today >= m_minDate && today <= m_maxDate) {
            int tx = dateToX(today);
            QPen pen(Theme::TodayLine);
            pen.setWidth(2);
            pen.setStyle(Qt::DashLine);
            p.setPen(pen);
            p.drawLine(tx, m_topMargin, tx, height());
            p.setPen(Theme::TodayLine);
            //p.drawText(tx-15, m_topMargin-40, "Today");
        }
    }
}

void GanttChartWidget::drawGrid(QPainter& p) {
    // (Week lines are drawn later so they appear above the task bars.)

    for (int i = 0; i < m_items.size(); ++i) {
        for (int lane = 0; lane < 3; ++lane) {
            int top = laneTopY(i, lane);
            QColor bg = ((i + lane) % 2 == 0) ? Theme::RowBg1 : Theme::RowBg2;
            p.fillRect(0, top, width(), m_subRowHeight, bg);
            p.setPen(Theme::LightGrid);
            p.drawLine(0, top + m_subRowHeight, width(), top + m_subRowHeight);
        }
        int afterBlockY = laneTopY(i, 2) + m_subRowHeight;
        p.setPen(Theme::Separator);
        p.drawLine(0, afterBlockY + m_blockGap, width(), afterBlockY + m_blockGap);
    }
}

void GanttChartWidget::drawLegend(QPainter& p) {
    p.save();
    int x = m_leftMargin;
    int y = 8;
    const int box = 12;
    const int spacing = 30;

    auto chip = [&](const QColor& color, const QString& text){
        p.setPen(QPen(Theme::BarOutline, 1));
        p.setBrush(QBrush(color, Qt::SolidPattern));
        p.drawRect(x, y, box, box);
        p.setPen(Theme::TextPrimary);
        p.setBrush(Qt::NoBrush);
        p.drawText(x + box + 6, y + box - 2, text);
        x += p.fontMetrics().horizontalAdvance(text) + box + spacing;
    };

    chip(Theme::Sow,  QStringLiteral("Sow"));
    chip(Theme::Plant,   QStringLiteral("Plant"));
    chip(Theme::Harvest,   QStringLiteral("Harvest"));
    p.restore();
}

void GanttChartWidget::drawBars(QPainter& p) {
    QFontMetrics fm(p.font());

    for (int i = 0; i < m_items.size(); ++i) {
        const auto& c = m_items[i];

        auto drawRangeLane = [&](const QDate& a, const QDate& b, int lane, const QColor& col){
            if (!isValidRange(a,b)) return;
            int laneTop = laneTopY(i, lane);
            int baseline = laneTop + m_subRowHeight / 2;
            int x1 = dateToX(a);
            int x2 = dateToX(b.addDays(1));
            int h = std::max(10, m_subRowHeight - 6);
            QRect r(x1, baseline - h/2, x2 - x1, h);
            p.setPen(Theme::BarOutline);
            p.setBrush(col);
            p.drawRoundedRect(r, 3, 3);
        };

        drawRangeLane(c.sowStart,     c.sowEnd,     0, Theme::Sow);
        drawRangeLane(c.plantStart,   c.plantEnd,   1, Theme::Plant);
        drawRangeLane(c.harvestStart, c.harvestEnd, 2, Theme::Harvest);
    }

    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(Theme::TextDark));
}

void GanttChartWidget::drawWeekLines(QPainter& p) {
    QDate wk = m_minDate;
    while (wk <= m_maxDate && wk.isValid() && wk.dayOfWeek() != 1) wk = wk.addDays(1);
    if (wk.isValid() && wk <= m_maxDate) {
        QPen weekPen(Theme::WeekLine);
        weekPen.setStyle(Qt::DashLine);
        weekPen.setWidth(1);
        p.setPen(weekPen);
        int top = m_topMargin;
        int bottom = height();
        while (wk <= m_maxDate) {
            int x = dateToX(wk);
            p.drawLine(x, top, x, bottom);
            wk = wk.addDays(7);
        }
    }
}

void GanttChartWidget::mouseMoveEvent(QMouseEvent* ev) {
    QPoint pos = ev->pos();
    // Check each bar to see if mouse is over it
    for (int i = 0; i < m_items.size(); ++i) {
        const auto& c = m_items[i];
        for (int lane = 0; lane < 3; ++lane) {
            QDate a, b;
            QString taskName;
            if (lane == 0) { a = c.sowStart; b = c.sowEnd; taskName = "Sow"; }
            else if (lane == 1) { a = c.plantStart; b = c.plantEnd; taskName = "Plant"; }
            else { a = c.harvestStart; b = c.harvestEnd; taskName = "Harvest"; }

            if (!isValidRange(a,b)) continue;
            int laneTop = laneTopY(i, lane);
            int baseline = laneTop + m_subRowHeight / 2;
            int x1 = dateToX(a);
            int x2 = dateToX(b.addDays(1));
            int h = std::max(10, m_subRowHeight - 6);
            QRect r(x1, baseline - h/2, x2 - x1, h);
            if (r.contains(pos)) {
                if (m_lastHover.first == i && m_lastHover.second == lane) return; // already shown
                m_lastHover = qMakePair(i, lane);
                QString tip = QString("%1\n%2: %3 - %4")
                    .arg(c.name)
                    .arg(taskName)
                    .arg(a.toString("dd MMM yyyy"))
                    .arg(b.toString("dd MMM yyyy"));
                QToolTip::showText(ev->globalPosition().toPoint(), tip, this, r);
                return;
            }
        }
    }
    // not over any bar
    if (m_lastHover.first != -1 || m_lastHover.second != -1) {
        m_lastHover = qMakePair(-1, -1);
        QToolTip::hideText();
    }
}

void GanttChartWidget::leaveEvent(QEvent* ev) {
    QWidget::leaveEvent(ev);
    m_lastHover = qMakePair(-1, -1);
    QToolTip::hideText();
}

bool GanttChartWidget::isValidRange(const QDate& a, const QDate& b) {
    return a.isValid() && b.isValid() && (a <= b);
}
