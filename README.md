# Garden Planner

This program provides:
- a planner for different crops with times for sowing, planting and harvesting periods
- a database to add your own crops
- a simple easy to use interface that shows what tasks can be performed today

## Build
Requires; qt6, cmake, ninja.  Linux only (atm).

```bash
./build.sh
```

## Run

```bash
./build/garden-planner
```

# Developer Info

## Database Structure
```
name TEXT,
sow_start DATE,
sow_end DATE,
plant_start DATE,
plant_end DATE,
harvest_start DATE,
harvest_end DATE
notes TEXT
```

## Theme tokens

This project centralizes colours in `src/Theme.h`. Use these tokens when painting UI so colours remain consistent across widgets.

Recommended mappings (short):
- `Theme::WindowBackground` - used for top-level widget backgrounds (`GanttChartWidget`, `LeftColumnWidget`).
- `Theme::TextPrimary`, `Theme::TextDark`, `Theme::TextMuted` - text colours for headers, primary labels, and secondary labels.
- `Theme::GridLine`, `Theme::LightGrid`, `Theme::Separator`, `Theme::HeaderSeparator` - grid and separator lines.
- `Theme::RowBg1`, `Theme::RowBg2` - alternating row backgrounds.
- `Theme::Sow`, `Theme::Plant`, `Theme::Harvest` - task bar fill colours.
- `Theme::PlantPrePost`, `Theme::HarvestPrePost` - task bar pre and post range colours
- `Theme::WeekLine`, `Theme::TodayLine` - week tick/line and today's indicator.
- `Theme::BarOutline` - outlines used for task bars and legend chips.

When adding UI, prefer reusing an existing token; add a new token to `Theme.h` only if the colour is semantically different.
