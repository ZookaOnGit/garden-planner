#pragma once

#include <QWidget>
#include <QVector>
#include <QDate>
#include "CropWindow.h"

class LeftColumnWidget : public QWidget {
    Q_OBJECT
public:
    explicit LeftColumnWidget(QWidget* parent = nullptr);

    void setItems(const QVector<CropWindow>& items);
    QSize sizeHint() const override;
    // Adjust the width of the left column (in pixels)
    void setColumnWidth(int w);
    // Return a preferred width based on longest crop name and lane labels
    int preferredWidth() const;

protected:
    void paintEvent(QPaintEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent* ev) override;
    void contextMenuEvent(QContextMenuEvent* ev) override;
    void mousePressEvent(QMouseEvent* ev) override;

signals:
    // Emitted when the user double-clicks a crop row in the left column.
    // The int is the crop index in the current items vector.
    void cropDoubleClicked(int index);
    // Emitted when the user requests deletion of a crop (right-click -> Delete)
    void cropDeleteRequested(int index);
    // Emitted when user clicks the small + in the header to add a crop
    void addCropRequested();

private:
    QVector<CropWindow> m_items;

    // Keep layout constants in sync with GanttChartWidget
    int m_leftMargin  = 160;
    int m_topMargin   = 60;
    int m_subRowHeight = 20;
    int m_subRowGap    = 4;
    int m_blockGap     = 10;

    int blockHeight() const { return 3 * m_subRowHeight + 2 * m_subRowGap; }
    int laneTopY(int cropIndex, int laneIndex) const {
        int blockTop = m_topMargin + cropIndex * (blockHeight() + m_blockGap);
        int laneOffset = laneIndex * (m_subRowHeight + m_subRowGap);
        return blockTop + laneOffset;
    }
    QRect m_addBtnRect;
    
};
