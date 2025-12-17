#include "LeftColumnWidget.h"
#include "Theme.h"
#include <QPainter>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <algorithm>

LeftColumnWidget::LeftColumnWidget(QWidget* parent) : QWidget(parent) {
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Theme::WindowBackground);
    setPalette(pal);
}

void LeftColumnWidget::setItems(const QVector<CropWindow>& items) {
    m_items = items;
    m_itemsDefault = items;  // Store the original order
    setSortMode(m_sortMode);
    updateGeometry();
    setMinimumSize(sizeHint());
    update();
}

void LeftColumnWidget::setColumnWidth(int w) {
    m_leftMargin = std::max(0, w);
    updateGeometry();
    setMinimumSize(sizeHint());
    update();
}

QSize LeftColumnWidget::sizeHint() const {
    int height = m_topMargin + m_items.size() * (blockHeight() + m_blockGap) + 40;
    int width = m_leftMargin;
    return { std::max(width, 120), std::max(height, 400) };
}

int LeftColumnWidget::preferredWidth() const {
    QFont f; // default font
    QFontMetrics fm(f);
    int maxw = 0;
    // Consider crop names
    for (const auto &c : m_items) {
        maxw = std::max(maxw, fm.horizontalAdvance(c.name));
    }
    // Consider lane labels (Sow/Plant/Harvest)
    maxw = std::max(maxw, fm.horizontalAdvance(QStringLiteral("Harvest")));
    // Add space for left padding and margin and small reserve
    int padding = 12 + 40; // margin + space for lane labels
    int w = std::max(m_leftMargin, maxw + padding);
    return std::max(w, 100);
}

void LeftColumnWidget::setSortMode(SortMode mode) {
    if (m_sortMode == mode) return;
    m_sortMode = mode;

    if (mode == Alphabetical) {
        // Sort items alphabetically by crop name
        std::sort(m_items.begin(), m_items.end(), [](const CropWindow& a, const CropWindow& b) {
            return a.name.toLower() < b.name.toLower();
        });
    } else if (mode == Default) {
        // Restore to original database order
        m_items = m_itemsDefault;
    }

    updateGeometry();
    setMinimumSize(sizeHint());
    update();

    // Emit signal with the reordered items so the chart and main data can be updated
    emit itemsReordered(m_items);
}

void LeftColumnWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    // Draw header in bold, but keep the painter font unchanged for the
    // rest of the painting so only the header and crop name are bold.
    QFont prevFont = p.font();
    QFont headerFont = prevFont; headerFont.setBold(true);
    const int y = m_topMargin - 20;
    p.setFont(headerFont);
    p.setPen(Theme::TextPrimary);
    const QString headerText = QStringLiteral("Crop");
    p.drawText(8, y, headerText);
    // Draw a small '+' button near the right edge of the left column.
    const int btnSize = 16;
    const int btnPadding = 8;
    int bx = m_leftMargin - btnSize - btnPadding;
    int by = y - btnSize/2 - 2; // center vertically near the header baseline
    m_addBtnRect = QRect(bx, by, btnSize, btnSize);
    p.setPen(QPen(Theme::BarOutline, 1));
    p.setBrush(QBrush(Theme::WindowBackground));
    p.drawRoundedRect(m_addBtnRect, 3, 3);
    // Draw plus sign
    p.setPen(Theme::TextPrimary);
    int cx = bx + btnSize/2;
    int cy = by + btnSize/2;
    p.drawLine(cx - 4, cy, cx + 4, cy);
    p.drawLine(cx, cy - 4, cx, cy + 4);
    p.setFont(prevFont);

    QFontMetrics fm(prevFont);

    for (int i = 0; i < m_items.size(); ++i) {
        const auto& c = m_items[i];
        p.setPen(Theme::TextDark);
        // Draw crop name in bold only for the name itself.
        QFont nameFont = prevFont; nameFont.setBold(true);
        QFontMetrics nameFm(nameFont);
        QString name = nameFm.elidedText(c.name, Qt::ElideRight, m_leftMargin - 12);
        int nameBaseline = laneTopY(i, 0) + m_subRowHeight - 4;
        p.setFont(nameFont);
        p.drawText(8, nameBaseline, name);
        p.setFont(prevFont);

        // subrow labels: draw right-aligned within the left column
        auto drawLaneLabel = [&](int lane, const QString& text){
            p.setPen(Theme::TextMuted);
            QFont f = prevFont; f.setBold(false); f.setPointSizeF(std::max(6.0, f.pointSizeF() - 1));
            p.setFont(f);
            int y = laneTopY(i, lane) + m_subRowHeight - 4;
            // compute right-aligned X inside the left column with an 8px padding
            int rightEdge = m_leftMargin - 8;
            int textWidth = QFontMetrics(f).horizontalAdvance(text);
            int x = rightEdge - textWidth;
            p.drawText(x, y, text);
            p.setFont(prevFont);
        };

            drawLaneLabel(0, "Sow");
            drawLaneLabel(1, "Plant");
            drawLaneLabel(2, "Harvest");
    }
}

void LeftColumnWidget::mouseDoubleClickEvent(QMouseEvent* ev) {
    // Determine which crop row (if any) was double-clicked based on Y
    int y = ev->pos().y();
    if (y < m_topMargin) return;
    int block = blockHeight() + m_blockGap;
    int idx = (y - m_topMargin) / block;
    if (idx >= 0 && idx < m_items.size()) {
        emit cropDoubleClicked(idx);
    }
}

void LeftColumnWidget::contextMenuEvent(QContextMenuEvent* ev) {
    int y = ev->pos().y();
    // If the click is above the content area (in the header), show a
    // header-only menu with Add and Sort options.
    if (y < m_topMargin) {
        QMenu menu(this);
        QAction* addAct = menu.addAction("Add");
        menu.addSeparator();
        QAction* sortDefaultAct = menu.addAction("Sort: Default");
        QAction* sortAlphaAct = menu.addAction("Sort: Alphabetical");

        // Mark current sort mode
        if (m_sortMode == Default) {
            sortDefaultAct->setText("✓ Sort: Default");
        } else {
            sortAlphaAct->setText("✓ Sort: Alphabetical");
        }

        QAction* chosen = menu.exec(ev->globalPos());
        if (!chosen) return;
        if (chosen == addAct) {
            emit addCropRequested();
        } else if (chosen == sortDefaultAct) {
            setSortMode(Default);
        } else if (chosen == sortAlphaAct) {
            setSortMode(Alphabetical);
        }
        return;
    }

    int block = blockHeight() + m_blockGap;
    int idx = (y - m_topMargin) / block;
    if (idx < 0 || idx >= m_items.size()) return;

    QMenu menu(this);
    QAction* addAct = menu.addAction("Add");
    QAction* editAct = menu.addAction("Edit");
    QAction* delAct = menu.addAction("Delete");
    menu.addSeparator();
    QAction* hideAct = menu.addAction("Hide");
    QAction* unhideAct = menu.addAction("Unhide Crops");
    menu.addSeparator();
    QAction* sortDefaultAct = menu.addAction("Sort: Default");
    QAction* sortAlphaAct = menu.addAction("Sort: Alphabetical");

    // Mark current sort mode
    if (m_sortMode == Default) {
        sortDefaultAct->setText("✓ Sort: Default");
    } else {
        sortAlphaAct->setText("✓ Sort: Alphabetical");
    }

    QAction* chosen = menu.exec(ev->globalPos());
    if (!chosen) return;
    if (chosen == addAct) {
        emit addCropRequested();
    } else if (chosen == editAct) {
        emit cropDoubleClicked(idx);
    } else if (chosen == delAct) {
        emit cropDeleteRequested(idx);
    } else if (chosen == hideAct) {
        emit cropHideRequested(idx);
    } else if (chosen == unhideAct) {
        emit unhideCropsRequested();
    } else if (chosen == sortDefaultAct) {
        setSortMode(Default);
    } else if (chosen == sortAlphaAct) {
        setSortMode(Alphabetical);
    }
}

void LeftColumnWidget::mousePressEvent(QMouseEvent* ev) {
    if (ev->button() == Qt::LeftButton) {
        if (m_addBtnRect.contains(ev->pos())) {
            emit addCropRequested();
            return;
        }
    }
    QWidget::mousePressEvent(ev);
}
