#include "BedGanttWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QInputDialog>
#include <QColor>
#include <QRandomGenerator>
#include <QWheelEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QMenu>
#include <QAction>
#include <QLocale>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "Theme.h"
#include <QPixmap>

// Helper: query catalog planting range for a crop name and return whether the crop's start
// date is outside that range. Optionally returns a formatted plantingRange HTML string.
static bool isStartOutsidePlantingRange(const BedCrop &c, QString &outPlantingRange) {
    outPlantingRange.clear();
    if (c.name.isEmpty()) return false;
    QSqlQuery q;
    q.prepare("SELECT plant_start, plant_end FROM crops WHERE name = :name LIMIT 1");
    q.bindValue(":name", c.name);
    if (q.exec() && q.next()) {
        QString ps = q.value(0).toString();
        QString pe = q.value(1).toString();
        QDate pstart = QDate::fromString(ps, Qt::ISODate);
        QDate pend = QDate::fromString(pe, Qt::ISODate);
        // fallback to QDate value if ISO parsing didn't work
        if (!pstart.isValid()) pstart = q.value(0).toDate();
        if (!pend.isValid()) pend = q.value(1).toDate();
        if (pstart.isValid() && pend.isValid()) {
            QLocale loc = QLocale::system();
            QString pstartStr = loc.toString(pstart, QLocale::ShortFormat);
            QString pendStr = loc.toString(pend, QLocale::ShortFormat);
            outPlantingRange = QString("<i>Recommended: %1 - %2</i>").arg(pstartStr.toHtmlEscaped(), pendStr.toHtmlEscaped());
            return (c.start < pstart || c.start > pend);
        }
    } else if (q.lastError().isValid()) {
        qDebug() << "Failed to query crop planting range:" << q.lastError().text();
    }
    return false;
}

BedGanttWidget::BedGanttWidget(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(400);
    setMouseTracking(true);
}

void BedGanttWidget::setModel(BedModel* model) {
    if (m_model) disconnect(m_model, &BedModel::modelChanged, this, QOverload<>::of(&BedGanttWidget::update));
    m_model = model;
    if (m_model) connect(m_model, &BedModel::modelChanged, this, [this](){ updateGeometry(); update(); });
}

int BedGanttWidget::xForDate(const QDate& d) const {
    int days = m_refDate.daysTo(d);
    return m_leftMargin + days * m_dayWidth;
}

void BedGanttWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), Theme::WindowBackground);

    const int rows = m_rows; // configurable number of beds
    int h = m_rowHeight;
    // draw rows with alternating background
    for (int i = 0; i < rows; ++i) {
        int y = i * h;
        QRect rowRect(0, y, width(), h);
        QColor rowBg = ((i) % 2 == 0) ? Theme::RowBg1 : Theme::RowBg2;
        p.fillRect(rowRect, rowBg);
        p.setPen(Theme::LightGrid);
        p.drawLine(0, y + h - 1, width(), y + h - 1);
    // bed labels are now drawn in a frozen left-column widget; the gantt no longer paints them
    }

    // draw timeline ticks (every 7 days) with localized date + year
    int daysToShow = (width() - m_leftMargin) / m_dayWidth;
    for (int d = 0; d < daysToShow; d += 7) {
        int x = m_leftMargin + d * m_dayWidth;
        QPen weekPen(Theme::WeekLine);
        weekPen.setStyle(Qt::DashLine);
        weekPen.setWidth(1);
        p.setPen(weekPen);
        p.drawLine(x, 0, x, rows * h);
        //QDate date = m_refDate.addDays(d);
        //p.setPen(Theme::TextPrimary);
        //QLocale loc = QLocale::system();
        //QString dateText = loc.toString(date, QLocale::ShortFormat);
    // dates are shown in the fixed header; omit bottom labels here
    }

    if (!m_model) return;

    // draw crops
    auto colorForName = [](const QString &name)->QColor {
        // deterministic mapping from name to hue
        if (name.isEmpty()) return QColor(120,200,140);
        uint h = uint(qHash(name)) % 360;
        // Use medium-high saturation and mid lightness for vivid but readable colors
        int s = 200;
        int l = 150;
        return QColor::fromHsl(int(h), s, l);
    };

    auto contrastingTextColor = [](const QColor &c)->QColor {
        // Use perceived luminance to decide black/white
        double lum = 0.299 * c.redF() + 0.587 * c.greenF() + 0.114 * c.blueF();
        return lum > 0.6 ? Qt::black : Qt::white;
    };

    for (const auto& c : m_model->crops()) {
        int r = c.bed;
        if (r < 0 || r >= rows) continue;
        int x = xForDate(c.start);
        int w = std::max(4, c.lengthDays * m_dayWidth);
        int y = r * h + 4;
        QRect rct(x, y, w, h - 8);
        // Check planting range and whether this crop's start is out-of-range
        QString plantingRangeHtml;
        bool outOfRange = isStartOutsidePlantingRange(c, plantingRangeHtml);
    // Determine a stable color per crop name so identical crops share a color
    QColor fill = c.name.isEmpty() ? c.color : colorForName(c.name);
    if (!fill.isValid()) fill = c.color;
    p.setBrush(fill);
    // use Theme outline for a subtle border
    p.setPen(Theme::BarOutline);
    p.drawRect(rct);
    QColor textColor = contrastingTextColor(fill);
    p.setPen(textColor);
    QLocale loc = QLocale::system();
    QString startStr = loc.toString(c.start, QLocale::ShortFormat);
    QDate endDate = c.start.addDays(c.lengthDays);
    QString endStr = loc.toString(endDate, QLocale::ShortFormat);
    QString label = QString("%1 (%2 - %3)").arg(c.name).arg(startStr).arg(endStr);
    // If the text is too wide for the rect, elide it
    QFontMetrics fm(font());
    QString elided = fm.elidedText(label, Qt::ElideRight, rct.width() - 8);
        p.drawText(rct.adjusted(4,0,-4,0), Qt::AlignVCenter|Qt::AlignLeft, elided);

        // draw small red flag pennant at the right edge of the crop bar if out of recommended planting range
        if (outOfRange) {
            // Attempt to load a user-provided red flag image from the working directory.
            // Cache the pixmap in a static so we don't reload it every paint.
            static QPixmap s_flagPix;
            if (s_flagPix.isNull()) {
                // Load from Qt resource (embedded). Resource path: :/icons/red-flag.png
                s_flagPix.load(":/icons/red-flag.png");
            }

            int pad = 4;
            int poleH = qMax(8, rct.height() * 6 / 10);
            int flagW = poleH; // make icon roughly square and proportional to bar height
            int flagH = poleH;
            int flagX = rct.right() - pad - flagW;
            int flagY = rct.top() + (rct.height() - flagH) / 2;

            if (!s_flagPix.isNull()) {
                // draw scaled pixmap preserving aspect ratio and filling the area
                QPixmap scaled = s_flagPix.scaled(flagW, flagH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                // center within the allocated rectangle
                int drawX = flagX + (flagW - scaled.width())/2;
                int drawY = flagY + (flagH - scaled.height())/2;
                p.drawPixmap(drawX, drawY, scaled);
            }
        }
    }
}

static int hitTestCrop(const BedModel* model, int x, int y, int leftMargin, int rowHeight, const QDate& refDate, int dayWidth, int rows) {
    if (!model) return -1;
    int row = y / rowHeight;
    if (row < 0 || row >= rows) return -1;
    for (const auto& c : model->crops()) {
        if (c.bed != row) continue;
        int cx = leftMargin + refDate.daysTo(c.start) * dayWidth;
        int cw = std::max(4, c.lengthDays * dayWidth);
        QRect r(cx, row * rowHeight + 4, cw, rowHeight - 8);
        if (r.contains(x, y)) return c.id;
    }
    return -1;
}

void BedGanttWidget::mousePressEvent(QMouseEvent* evt) {
    if (!m_model) return;
    QPoint pos = evt->position().toPoint();
    int id = hitTestCrop(m_model, pos.x(), pos.y(), m_leftMargin, m_rowHeight, m_refDate, m_dayWidth, m_rows);
    // middle button -> pan
    if (evt->button() == Qt::MiddleButton) {
        m_panning = true;
        m_panStart = pos;
        setCursor(Qt::ClosedHandCursor);
        return;
    }

    if (id >= 0 && evt->button() == Qt::LeftButton) {
        // determine if near left/right edge for resizing
        for (const auto& c : m_model->crops()) {
            if (c.id != id) continue;
            int cx = m_leftMargin + m_refDate.daysTo(c.start) * m_dayWidth;
            int cw = std::max(4, c.lengthDays * m_dayWidth);
            QRect r(cx, c.bed * m_rowHeight + 4, cw, m_rowHeight - 8);
            const int handle = 6;
            if (QRect(r.left(), r.top(), handle, r.height()).contains(pos)) {
                m_resizingId = id;
                m_resizingSide = Left;
            } else if (QRect(r.right()-handle+1, r.top(), handle, r.height()).contains(pos)) {
                m_resizingId = id;
                m_resizingSide = Right;
            } else {
                m_draggingId = id;
            }
            m_dragStartPos = pos;
            // store original
            m_originalBed = c.bed;
            m_originalStart = c.start;
            break;
        }
    }
    // right-click -> context menu: Delete when over an existing crop; Add when over empty row
    if (evt->button() == Qt::RightButton) {
        if (id >= 0) {
            // show delete for that crop
            for (const auto& c : m_model->crops()) {
                if (c.id != id) continue;
                QMenu m(this);
                QAction* del = m.addAction("Delete crop");
                QAction* act = m.exec(mapToGlobal(pos));
                if (act == del) {
                    m_model->removeCropById(id);
                }
                break;
            }
        } else {
            // clicked on empty space: offer to add a crop at this position
            int row = pos.y() / m_rowHeight;
            if (row < 0 || row >= m_rows) return;
            QMenu m(this);
            QAction* add = m.addAction("Add crop here");
            QAction* act = m.exec(mapToGlobal(pos));
            if (act == add) {
                // use template if present, otherwise prompt
                QString name;
                int len = 30;
                if (!m_templateName.isEmpty()) {
                    name = m_templateName;
                    len = m_templateLength;
                } else {
                    bool ok;
                    name = QInputDialog::getText(this, "Add crop", "Crop name:", QLineEdit::Normal, "New Crop", &ok);
                    if (!ok || name.isEmpty()) return;
                    len = QInputDialog::getInt(this, "Crop length (days)", "Days until harvest:", 30, 1, 365, 1, &ok);
                    if (!ok) return;
                }
                int dx = pos.x() - m_leftMargin;
                int days = dx / m_dayWidth;
                QDate start = m_refDate.addDays(days);
                BedCrop c;
                c.name = name;
                c.bed = row;
                c.start = start;
                c.lengthDays = len;
                c.color = QColor::fromHsl(QRandomGenerator::global()->bounded(360), 180, 140);
                m_model->addCrop(c);
            }
        }
    }
}

void BedGanttWidget::mouseMoveEvent(QMouseEvent* evt) {
    QPoint pos = evt->position().toPoint();
    // Hover feedback: change cursor when over resize handles or crop body
    if (!m_panning && m_resizingId == -1 && m_draggingId == -1 && evt->buttons() == Qt::NoButton) {
        bool overAny = false;
        const int handle = 6;
    int row = pos.y() / m_rowHeight;
    if (row >= 0 && row < m_rows && m_model) {
            QLocale loc = QLocale::system();
            for (const auto& c : m_model->crops()) {
                if (c.bed != row) continue;
                int cx = m_leftMargin + m_refDate.daysTo(c.start) * m_dayWidth;
                int cw = std::max(4, c.lengthDays * m_dayWidth);
                QRect r(cx, row * m_rowHeight + 4, cw, m_rowHeight - 8);
                if (r.contains(pos)) {
                    overAny = true;
                    if (QRect(r.left(), r.top(), handle, r.height()).contains(pos) ||
                        QRect(r.right()-handle+1, r.top(), handle, r.height()).contains(pos)) {
                        setCursor(Qt::SizeHorCursor);
                    } else {
                        setCursor(Qt::OpenHandCursor);
                    }
                    // set a tooltip with full details, including recommended planting range if available
                    QDate endDate = c.start.addDays(c.lengthDays);
                    QString startStr = loc.toString(c.start, QLocale::ShortFormat);
                    QString endStr = loc.toString(endDate, QLocale::ShortFormat);

                    QString plantingRange;
                    bool outOfRange = false;
                    if (!c.name.isEmpty()) {
                        outOfRange = isStartOutsidePlantingRange(c, plantingRange);
                    }

                    QString tip = QString("<b>%1</b><br/>%2 - %3<br/>Length: %4 days<br/>Bed: %5<br/>%6")
                                      .arg(c.name.toHtmlEscaped())
                                      .arg(startStr.toHtmlEscaped())
                                      .arg(endStr.toHtmlEscaped())
                                      .arg(c.lengthDays)
                                      .arg(c.bed + 1)
                                      .arg(plantingRange);
                    if (outOfRange) {
                        tip += QString("<br/><b style='color:#b22222'>Warning: Start date outside recommended planting range</b>");
                    }
                    setToolTip(tip);
                    break;
                }
            }
        }
        if (!overAny) {
            setCursor(Qt::ArrowCursor);
            setToolTip(QString());
        }
    }
    if (m_panning) {
        // Decide whether the user intends vertical scroll (scroll area) or horizontal pan
        int dx = pos.x() - m_panStart.x();
        int dy = pos.y() - m_panStart.y();
        QWidget* w = parentWidget();
        QScrollArea* sa = nullptr;
        while (w) {
            sa = qobject_cast<QScrollArea*>(w);
            if (sa) break;
            w = w->parentWidget();
        }
        // If a scroll area exists and vertical motion dominates, scroll vertically.
        if (sa && std::abs(dy) >= std::abs(dx)) {
            QScrollBar* vb = sa->verticalScrollBar();
            vb->setValue(vb->value() - dy);
            m_panStart = pos;
            return;
        }

        // Otherwise treat as horizontal pan (move timeline)
        panBy(dx);
        m_panStart = pos;
        return;
    }

    if (m_resizingId != -1) {
        // resizing
        for (auto c : m_model->crops()) {
            if (c.id != m_resizingId) continue;
            int mouseDays = m_refDate.daysTo(QDate::currentDate()); // placeholder
            // compute days at mouse x
            int dx = pos.x() - m_leftMargin;
            int d = dx / m_dayWidth;
            if (m_resizingSide == Left) {
                QDate newStart = m_refDate.addDays(d);
                // ensure newStart is before end
                int newLength = newStart.daysTo(c.start.addDays(c.lengthDays));
                if (newLength < 1) newLength = 1;
                c.lengthDays = newLength;
                c.start = newStart;
            } else if (m_resizingSide == Right) {
                int startDays = m_refDate.daysTo(c.start);
                int newLen = d - startDays;
                if (newLen < 1) newLen = 1;
                c.lengthDays = newLen;
            }
            m_model->updateCropById(c.id, c);
            break;
        }
        return;
    }

    if (m_draggingId < 0) return;
    QPoint delta = pos - m_dragStartPos;
    int dayShift = qRound((double)delta.x() / m_dayWidth);
    int bedShift = delta.y() / m_rowHeight;
    // find crop
    for (auto c : m_model->crops()) {
        if (c.id != m_draggingId) continue;
        c.start = m_originalStart.addDays(dayShift);
        c.bed = qBound(0, m_originalBed + bedShift, m_rows - 1);
        m_model->updateCropById(c.id, c);
        break;
    }
}

void BedGanttWidget::mouseReleaseEvent(QMouseEvent* evt) {
    Q_UNUSED(evt);
    m_draggingId = -1;
    m_resizingId = -1;
    m_resizingSide = None;
    if (m_panning) {
        m_panning = false;
        setCursor(Qt::ArrowCursor);
    }
}

void BedGanttWidget::mouseDoubleClickEvent(QMouseEvent* evt) {
    if (!m_model) return;
    QPoint pos = evt->position().toPoint();
    int id = hitTestCrop(m_model, pos.x(), pos.y(), m_leftMargin, m_rowHeight, m_refDate, m_dayWidth, m_rows);
    // If double-clicked on an existing crop, allow renaming
    if (id >= 0) {
        for (auto c : m_model->crops()) {
            if (c.id != id) continue;
            bool ok;
            QString newName = QInputDialog::getText(this, "Edit crop name", "Name:", QLineEdit::Normal, c.name, &ok);
            if (ok && !newName.isEmpty()) {
                c.name = newName;
                m_model->updateCropById(c.id, c);
            }
            break;
        }
        return;
    }

    // otherwise double-click in empty space acts as add (left button)
    if (evt->button() == Qt::LeftButton) {
        int row = pos.y() / m_rowHeight;
            if (row < 0 || row >= m_rows) return;
        bool ok;
        QString name;
        int len;
        if (!m_templateName.isEmpty()) {
            name = m_templateName;
            len = m_templateLength;
        } else {
            name = QInputDialog::getText(this, "Add crop", "Crop name:", QLineEdit::Normal, "New Crop", &ok);
            if (!ok || name.isEmpty()) return;
            len = QInputDialog::getInt(this, "Crop length (days)", "Days until harvest:", 30, 1, 365, 1, &ok);
            if (!ok) return;
        }
        // compute date under mouse
        int dx = pos.x() - m_leftMargin;
        int days = dx / m_dayWidth;
        QDate start = m_refDate.addDays(days);
        BedCrop c;
        c.name = name;
        c.bed = row;
        c.start = start;
        c.lengthDays = len;
        c.color = QColor::fromHsl(QRandomGenerator::global()->bounded(360), 180, 140);
        m_model->addCrop(c);
    }
}

void BedGanttWidget::wheelEvent(QWheelEvent* evt) {
    // Ctrl + wheel => zoom; otherwise let parent (scroll area) handle vertical scrolling
    if (evt->modifiers() & Qt::ControlModifier) {
        int num = evt->angleDelta().y();
        if (num == 0) return;
        double factor = (num > 0) ? 1.15 : (1.0/1.15);
        zoomByFactor(factor, evt->position().toPoint().x());
    } else {
        // don't consume the event; allow the scroll area to scroll vertically
        evt->ignore();
    }
}

void BedGanttWidget::zoomByFactor(double f, int centerX) {
    int oldW = m_dayWidth;
    int newW = qBound(2, int(std::round(m_dayWidth * f)), 60);
    // If rounding made no change, nudge by 1 in the requested direction so zooming remains responsive
    if (newW == oldW) {
        if (f > 1.0 && oldW < 60) newW = oldW + 1;
        else if (f < 1.0 && oldW > 2) newW = oldW - 1;
    }
    if (newW == oldW) return;

    // adjust refDate so that the date under centerX remains the same
    double centerDaysF = double(centerX - m_leftMargin) / double(oldW);
    int centerDays = qRound(centerDaysF);
    QDate centerDate = m_refDate.addDays(centerDays);
    m_dayWidth = newW;
    double newCenterDaysF = double(centerX - m_leftMargin) / double(newW);
    int newCenterDays = qRound(newCenterDaysF);
    m_refDate = centerDate.addDays(-newCenterDays);
    updateGeometry();
    update();
    emit viewChanged();
}

void BedGanttWidget::panBy(int dx) {
    int dayShift = qRound((double)dx / m_dayWidth);
    m_refDate = m_refDate.addDays(-dayShift);
    update();
    emit viewChanged();
}
