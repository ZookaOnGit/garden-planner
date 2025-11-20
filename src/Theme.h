#pragma once

#include <QColor>

// Centralized color theme for the app.
// Keep short comments here to describe where each color is used so future
// contributors can quickly find and reuse the correct token.
namespace Theme {
    // General window / background
    // Used by: top-level widgets as background (e.g. `GanttChartWidget` palette)
    inline const QColor WindowBackground = QColor(250,250,250);

    // Primary text colors
    // Used by: header text, primary labels in `GanttChartWidget` and `LeftColumnWidget`
    inline const QColor TextPrimary = QColor(60,60,60);
    // Slightly darker text for outlines and emphasized small text (bar outline fallback)
    // Used by: bar outline fallback, some dense labels
    inline const QColor TextDark = QColor(50,50,50);
    // Muted text for secondary/subrow labels
    // Used by: subrow labels (Sow/Plant/Harvest), legend descriptions
    inline const QColor TextMuted = QColor(90,90,90);

    // Grid and separators
    // Used by: month vertical separators, light grid dashes in `GanttChartWidget::drawHeader`/`drawGrid`
    inline const QColor GridLine = QColor(210,210,210);
    // Slightly lighter grid color used for subrow separators
    inline const QColor LightGrid = QColor(230,230,230);
    // General separators between blocks/rows
    // Used by: inter-row divider lines
    inline const QColor Separator = QColor(235,235,235);
    // Header underline separator
    inline const QColor HeaderSeparator = QColor(180,180,180);

    // Row background alternation
    // Used by: alternating row backgrounds in `GanttChartWidget::drawGrid`
    inline const QColor RowBg1 = QColor(255,255,255);
    inline const QColor RowBg2 = QColor(248,248,248);

    // Task bar fill colors
    // Used by: task bars in `GanttChartWidget::drawBars`
    inline const QColor Sow = QColor(66,133,244);
    inline const QColor Plant = QColor(52,168,83);
    inline const QColor PlantPrePost = QColor(52,168,83,100);
    inline const QColor Harvest = QColor(244,180,0);
    inline const QColor HarvestPrePost = QColor(244,180,0, 100);

    // Week tick/line and "today" indicator
    // Used by: weekly tick marks, dashed week separators drawn on top of bars
    inline const QColor WeekLine = QColor(220,220,220);
    // Used by: the 'Today' marker/line in the header
    inline const QColor TodayLine = QColor(220,0,0);

    // Bar outline / chip border
    // Used by: small outlines around task bars and legend chips
    inline const QColor BarOutline = QColor(70,70,70,120);
}
