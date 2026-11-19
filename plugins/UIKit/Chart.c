// Chart, LineSeries and ColumnSeries: a cartesian chart after the WPF Toolkit
// and the Windows Phone toolkit charts. The chart owns the axes (a value axis
// with round intervals, category labels, gridlines and the legend); each
// series is an authored child laid over the chart that reads its ValuePath
// from the chart's records and draws itself in its own Foreground.
#include "metro.h"
#include <include/renderer.h>

#define CHART_MAX_POINTS     48
#define CHART_LABEL_SIZE     12.0f
#define CHART_LABEL_GAP      6.0f
#define CHART_LEGEND_SWATCH  10.0f
#define CHART_DEFAULT_HEIGHT 160.0f
#define CHART_GRID_OPACITY   0.2f

// BOX3_FromRect is a macro; a compound literal's commas would split it.
static struct box3
Chart_Box(float x, float y, float width, float height)
{
  struct rect r = { x, y, width, height };
  return BOX3_FromRect(r);
}

// Up to CHART_MAX_POINTS DataObject records of the chart's ItemsSource.
static int
Chart_Records(struct Chart const *chart, struct Object **records)
{
  struct Object *source = chart->ItemsSource ? CMP_GetObject(chart->ItemsSource) : NULL;
  int count = 0;
  if (source) FOR_EACH_OBJECT(record, source) {
    if (count == CHART_MAX_POINTS) break;
    if (GetDataObject(record)) records[count++] = record;
  }
  return count;
}

static struct Property *
Chart_Field(struct Object *record, lpcString_t path)
{
  if (!record || !path || !*path) return NULL;
  return OBJ_FindShortProperty(record, fnv1a32(path));
}

// A record's number at `path`; FALSE leaves a gap in the series.
static bool_t
Chart_Number(struct Object *record, lpcString_t path, float *out)
{
  struct Property *property = Chart_Field(record, path);
  if (!property || !PROP_GetValue(property)) return FALSE;
  switch (PROP_GetType(property)) {
    case kDataTypeFloat: *out = *(float const *)PROP_GetValue(property); return !isnan(*out);
    case kDataTypeInt: *out = (float)*(int const *)PROP_GetValue(property); return TRUE;
    default: return FALSE;
  }
}

static void
Chart_Text(struct Object *record, lpcString_t path, LPSTR buf, size_t size)
{
  struct Property *property = Chart_Field(record, path);
  float number;
  *buf = 0;
  if (!property) return;
  if (PROP_GetType(property) == kDataTypeString) {
    lpcString_t text = *(lpcString_t const *)PROP_GetValue(property);
    snprintf(buf, size, "%s", text ? text : "");
  } else if (Chart_Number(record, path, &number)) {
    snprintf(buf, size, "%g", number);
  }
}

// The value with as many decimals as the step needs, without trailing
// zeros: 4, 4.5, 5 rather than 4.0, 4.5, 5.0.
static void
Chart_FormatValue(float value, float step, LPSTR buf, size_t size)
{
  if (fabsf(value) < step * 1e-3f) value = 0;
  int decimals = step >= 1 ? 0 : step >= 0.1f ? 1 : 2;
  snprintf(buf, size, "%.*f", decimals, value);
  if (decimals) {
    LPSTR end = buf + strlen(buf) - 1;
    while (end > buf && *end == '0') *end-- = 0;
    if (end > buf && *end == '.') *end = 0;
  }
}

// 1, 2 or 5 times a power of ten, so that `range` spans about `lines` steps.
static float
Chart_NiceStep(float range, int lines)
{
  if (!(range > 0)) return 1;
  float raw = range / (float)lines, power = powf(10, floorf(log10f(raw))), unit = raw / power;
  return (unit <= 1 ? 1 : unit <= 2 ? 2 : unit <= 5 ? 5 : 10) * power;
}

// A child series that takes part: shown, so hidden series neither scale the
// axis nor appear in the legend.
static struct ChartSeries *
Chart_Series(struct Object *child)
{
  struct ChartSeries *series = GetChartSeries(child);
  return series && GetNode(child)->Visible ? series : NULL;
}

static struct Chart *
Chart_Of(struct Object *series)
{
  struct Object *parent = OBJ_GetParent(series);
  return parent ? GetChart(parent) : NULL;
}

static void
Chart_Observe(struct Chart *chart)
{
  struct Object *hObject = CMP_GetObject(chart);
  struct Object *source = chart->ItemsSource ? CMP_GetObject(chart->ItemsSource) : NULL;
  if (source == chart->_observedSource) return;
  if (source) OBJ_AddRef(source);
  if (chart->_observedSource) {
    core_UnobserveCollection(chart->_observedSource, hObject);
    OBJ_ReleaseRef(chart->_observedSource);
  }
  chart->_observedSource = source;
  if (source) core_ObserveCollection(source, hObject);
}

static struct Object *
Chart_Label(struct Object *hObject, struct Object **slot)
{
  if (!*slot) {
    *slot = Metro_CreateText(hObject, ID_TextBlock, "chart-label", CHART_LABEL_SIZE, FALSE);
    if (*slot) GetNode2D(*slot)->IgnoreHitTest = TRUE;
  }
  return *slot;
}

static void
Chart_HideFrom(struct Object **labels, int from, int count)
{
  for (int i = from; i < count; i++) if (labels[i]) Metro_Show(labels[i], FALSE, 1);
}

// The value range every series needs: their values, zero under columns,
// then the authored Minimum, Maximum and Interval.
static void
Chart_Scale(struct Object *hObject, struct Chart *chart, struct Object **records, int count)
{
  float low = INFINITY, high = -INFINITY, value;
  FOR_EACH_OBJECT(child, hObject) {
    struct ChartSeries *series = Chart_Series(child);
    if (!series) continue;
    for (int i = 0; i < count; i++) {
      if (!Chart_Number(records[i], series->ValuePath, &value)) continue;
      low = fminf(low, value);
      high = fmaxf(high, value);
    }
    if (GetColumnSeries(child)) { low = fminf(low, 0); high = fmaxf(high, 0); }
  }
  if (!isfinite(low)) low = 0, high = 1;
  if (!isnan(chart->Minimum)) low = chart->Minimum;
  if (!isnan(chart->Maximum)) high = chart->Maximum;
  if (high <= low) high = low + 1;
  float step = chart->Interval > 0 ? chart->Interval : Chart_NiceStep(high - low, 4);
  chart->_step = step;
  chart->_min = isnan(chart->Minimum) ? floorf(low / step + 1e-4f) * step : low;
  chart->_max = isnan(chart->Maximum) ? ceilf(high / step - 1e-4f) * step : high;
  if (chart->_max <= chart->_min) chart->_max = chart->_min + step;
}

HANDLER(Chart, Object, Start)
{
  pChart->_started = TRUE;
  Chart_Observe(pChart);
  OBJ_SetDirty(hObject);
  return FALSE;
}

// Only the chart's own settings relayout it: layout itself changes other
// properties, and reacting to those would loop.
HANDLER(Chart, Object, PropertyChanged)
{
  if (!pPropertyChanged->Property) return FALSE;
  switch (PROP_GetLongIdentifier(pPropertyChanged->Property)) {
    case ID_Chart_ItemsSource:
      if (pChart->_started) Chart_Observe(pChart);
      OBJ_SetDirty(hObject);
      return FALSE;
    case ID_Chart_CategoryPath: case ID_Chart_Minimum: case ID_Chart_Maximum: case ID_Chart_Interval:
    case ID_Chart_LabelInterval: case ID_Chart_ShowGridLines:
      OBJ_SetDirty(hObject);
      return FALSE;
    default:
      return FALSE;
  }
}

HANDLER(Chart, DataObject, CollectionChanged)
{
  if (pCollectionChanged->Source != pChart->_observedSource) return FALSE;
  OBJ_SetDirty(hObject);
  return TRUE;
}

HANDLER(Chart, Object, Destroy)
{
  if (pChart->_observedSource) {
    core_UnobserveCollection(pChart->_observedSource, hObject);
    OBJ_ReleaseRef(pChart->_observedSource);
    pChart->_observedSource = NULL;
  }
  struct Object **groups[] = { pChart->_valueLabels, pChart->_categoryLabels, pChart->_legendLabels };
  int sizes[] = { 16, CHART_MAX_POINTS, 8 };
  for (int g = 0; g < 3; g++)
    for (int i = 0; i < sizes[g]; i++)
      if (groups[g][i]) { OBJ_ReleaseRef(groups[g][i]); groups[g][i] = NULL; }
  return FALSE;
}

HANDLER(Chart, Node2D, MeasureOverride)
{
  float height = isnan(GetNode(hObject)->Size.Axis[1].Requested) ? CHART_DEFAULT_HEIGHT : pMeasureOverride->Height;
  return Layout_Return(pMeasureOverride, pMeasureOverride->Width, fminf(height, pMeasureOverride->Height));
}

HANDLER(Chart, Node2D, ArrangeOverride)
{
  float x = pArrangeOverride->X, y = pArrangeOverride->Y;
  float w = pArrangeOverride->Width, h = pArrangeOverride->Height;
  struct Object *records[CHART_MAX_POINTS];
  int count = Chart_Records(pChart, records);
  pChart->_count = count;
  Chart_Scale(hObject, pChart, records, count);

  // Legend: one line of swatch + title per titled series.
  float legendHeight = 0, legendX = x;
  int titled = 0;
  FOR_EACH_OBJECT(child, hObject) {
    struct ChartSeries *series = Chart_Series(child);
    if (!series || !series->Title || !*series->Title || titled == 8) continue;
    struct Object *label = Chart_Label(hObject, &pChart->_legendLabels[titled++]);
    Metro_SetText(label, series->Title, NULL);
    Metro_Show(label, TRUE, 1);
    struct Size size = Metro_Measure(label, w, h);
    Metro_Arrange(label, legendX + CHART_LEGEND_SWATCH + 4, y, size.width, size.height);
    pChart->_legendX[titled - 1] = legendX - x;
    pChart->_legendY[titled - 1] = (size.height - CHART_LEGEND_SWATCH) / 2;
    legendX += CHART_LEGEND_SWATCH + 4 + size.width + 12;
    legendHeight = fmaxf(legendHeight, size.height + 6);
  }
  Chart_HideFrom(pChart->_legendLabels, titled, 8);

  // Value labels on the right, one per gridline, measured first for width.
  char text[64];
  int lines = (int)roundf((pChart->_max - pChart->_min) / pChart->_step);
  lines = MIN(MAX(lines, 1), 15);
  float valueWidth = 0, labelHeight = CHART_LABEL_SIZE + 4;
  struct Size sizes[16];
  for (int i = 0; i <= lines; i++) {
    struct Object *label = Chart_Label(hObject, &pChart->_valueLabels[i]);
    Chart_FormatValue(pChart->_max - pChart->_step * i, pChart->_step, text, sizeof(text));
    Metro_SetText(label, text, NULL);
    Metro_Show(label, TRUE, 1);
    sizes[i] = Metro_Measure(label, w, h);
    valueWidth = fmaxf(valueWidth, sizes[i].width);
    labelHeight = sizes[i].height;
  }
  Chart_HideFrom(pChart->_valueLabels, lines + 1, 16);

  float top = legendHeight + labelHeight / 2;
  float plotWidth = fmaxf(w - valueWidth - CHART_LABEL_GAP, 1);
  float plotHeight = fmaxf(h - top - labelHeight - CHART_LABEL_GAP, 1);
  pChart->_plotLeft = 0;
  pChart->_plotTop = top;
  pChart->_plotWidth = plotWidth;
  pChart->_plotHeight = plotHeight;
  for (int i = 0; i <= lines; i++) {
    float lineY = y + top + plotHeight * i / lines;
    Metro_Arrange(pChart->_valueLabels[i], x + plotWidth + CHART_LABEL_GAP, lineY - sizes[i].height / 2,
                  sizes[i].width, sizes[i].height);
  }

  // Category labels, thinned so neighbours never overlap.
  float slot = count ? plotWidth / count : plotWidth, widest = 0;
  struct Size categorySizes[CHART_MAX_POINTS];
  for (int i = 0; i < count; i++) {
    struct Object *label = Chart_Label(hObject, &pChart->_categoryLabels[i]);
    Chart_Text(records[i], pChart->CategoryPath, text, sizeof(text));
    Metro_SetText(label, text, NULL);
    categorySizes[i] = Metro_Measure(label, w, h);
    widest = fmaxf(widest, categorySizes[i].width);
  }
  int every = pChart->LabelInterval > 0 ? pChart->LabelInterval : MAX(1, (int)ceilf((widest + 8) / fmaxf(slot, 1)));
  for (int i = 0; i < count; i++) {
    bool_t shown = i % every == 0 && pChart->CategoryPath && *pChart->CategoryPath;
    Metro_Show(pChart->_categoryLabels[i], shown, 1);
    float centre = x + slot * (i + 0.5f);
    float labelX = fminf(fmaxf(centre - categorySizes[i].width / 2, x), x + plotWidth - categorySizes[i].width);
    Metro_Arrange(pChart->_categoryLabels[i], labelX, y + top + plotHeight + CHART_LABEL_GAP,
                  categorySizes[i].width, categorySizes[i].height);
  }
  Chart_HideFrom(pChart->_categoryLabels, count, CHART_MAX_POINTS);

  // Series cover the chart and draw within its plot area.
  FOR_EACH_OBJECT(child, hObject) {
    if (GetChartSeries(child)) Metro_Arrange(child, x, y, w, h);
  }
  return Layout_Return(pArrangeOverride, w, h);
}

HANDLER(Chart, Node2D, ForegroundContent)
{
  return pChart->ShowGridLines;
}

HANDLER(Chart, Node2D, DrawForeground)
{
  if (!pChart->ShowGridLines) return FALSE;
  struct Node2D *view = GetNode2D(hObject);
  struct rect bounds = view->_rect;
  struct ViewEntity entity;
  Node2D_GetViewEntity(view, &entity, NULL, &pDrawForeground->brush);
  entity.radius = (struct vec4){ 0 };
  entity.material.opacity = view->_opacity * CHART_GRID_OPACITY;
  int lines = MIN(MAX((int)roundf((pChart->_max - pChart->_min) / pChart->_step), 1), 15);
  for (int i = 0; i <= lines; i++) {
    float lineY = bounds.y + pChart->_plotTop + pChart->_plotHeight * i / lines;
    entity.bbox = Chart_Box(bounds.x + pChart->_plotLeft, lineY - 0.5f, pChart->_plotWidth, 1);
    R_DrawEntity(pDrawForeground->viewdef, &entity);
  }
  return TRUE;
}

HANDLER(ChartSeries, Object, Create)
{
  GetNode2D(hObject)->IgnoreHitTest = TRUE;
  return FALSE;
}

HANDLER(ChartSeries, Object, PropertyChanged)
{
  if (!pPropertyChanged->Property) return FALSE;
  switch (PROP_GetLongIdentifier(pPropertyChanged->Property)) {
    case ID_ChartSeries_ValuePath: case ID_ChartSeries_Title: case ID_Node_Visible:
    case ID_LineSeries_StrokeThickness: case ID_LineSeries_ShowMarkers: case ID_ColumnSeries_ColumnWidth: {
      struct Object *parent = OBJ_GetParent(hObject);
      if (parent && GetChart(parent)) OBJ_SetDirty(parent);
      OBJ_SetDirty(hObject);
      return FALSE;
    }
    default:
      return FALSE;
  }
}

// The plot position of each record's value, with `valid` FALSE for a gap.
// Returns the number of records, or 0 outside a Chart.
static int
ChartSeries_Points(struct Object *hObject, struct vec2 *points, bool_t *valid, float *baseline, float *slot)
{
  struct Chart *chart = Chart_Of(hObject);
  struct ChartSeries *series = GetChartSeries(hObject);
  if (!chart || !series) return 0;
  struct Object *records[CHART_MAX_POINTS];
  int count = Chart_Records(chart, records);
  struct rect bounds = GetNode2D(hObject)->_rect;
  float left = bounds.x + chart->_plotLeft, top = bounds.y + chart->_plotTop;
  float range = chart->_max - chart->_min;
  *slot = count ? chart->_plotWidth / count : chart->_plotWidth;
  float base = fminf(fmaxf(0, chart->_min), chart->_max);
  *baseline = top + chart->_plotHeight * (1 - (base - chart->_min) / range);
  for (int i = 0; i < count; i++) {
    float value;
    valid[i] = Chart_Number(records[i], series->ValuePath, &value);
    if (!valid[i]) continue;
    value = fminf(fmaxf(value, chart->_min), chart->_max);
    points[i] = (struct vec2){ left + *slot * (i + 0.5f), top + chart->_plotHeight * (1 - (value - chart->_min) / range) };
  }
  return count;
}

// A segment as a rounded bar rotated from a to b, so strokes keep their
// width at any angle on every renderer (GL line widths are not portable).
static void
LineSeries_Segment(struct ViewDef const *viewdef, struct ViewEntity *entity, struct mat4 const *matrix,
                   struct vec2 a, struct vec2 b, float thickness)
{
  float dx = b.x - a.x, dy = b.y - a.y, length = sqrtf(dx * dx + dy * dy);
  struct mat4 m = *matrix;
  MAT4_Translate(&m, &(struct vec3){ a.x, a.y, 0 });
  MAT4_Rotate(&m, &(struct vec3){ 0, 0, atan2f(dy, dx) * 180.0f / 3.14159265f }, kRotationOrderXYZ);
  entity->matrix = m;
  entity->bbox = Chart_Box(-thickness / 2, -thickness / 2, length + thickness, thickness);
  entity->radius = (struct vec4){ thickness / 2, thickness / 2, thickness / 2, thickness / 2 };
  R_DrawEntity(viewdef, entity);
}

// The legend swatch beside this series' title, when it has one.
static void
ChartSeries_DrawSwatch(struct Object *hObject, struct ViewDef const *viewdef, struct ViewEntity *entity,
                       struct mat4 const *matrix)
{
  struct Chart *chart = Chart_Of(hObject);
  struct Object *parent = OBJ_GetParent(hObject);
  int index = 0;
  FOR_EACH_OBJECT(child, parent) {
    struct ChartSeries *series = Chart_Series(child);
    if (!series || !series->Title || !*series->Title) continue;
    if (child == hObject) break;
    index++;
  }
  struct ChartSeries *self = GetChartSeries(hObject);
  if (!self->Title || !*self->Title || index >= 8 || !chart->_legendLabels[index]) return;
  // The series is laid out exactly over the chart, so the chart's offsets apply.
  struct rect bounds = GetNode2D(hObject)->_rect;
  entity->matrix = *matrix;
  entity->radius = (struct vec4){ 0 };
  entity->bbox = Chart_Box(bounds.x + chart->_legendX[index], bounds.y + chart->_legendY[index],
                           CHART_LEGEND_SWATCH, CHART_LEGEND_SWATCH);
  R_DrawEntity(viewdef, entity);
}

HANDLER(LineSeries, Node2D, ForegroundContent)
{
  return Chart_Of(hObject) != NULL;
}

HANDLER(LineSeries, Node2D, DrawForeground)
{
  struct vec2 points[CHART_MAX_POINTS];
  bool_t valid[CHART_MAX_POINTS];
  float baseline, slot;
  int count = ChartSeries_Points(hObject, points, valid, &baseline, &slot);
  if (!count) return FALSE;
  struct Node2D *view = GetNode2D(hObject);
  struct ViewEntity entity;
  Node2D_GetViewEntity(view, &entity, NULL, &pDrawForeground->brush);
  struct mat4 matrix = entity.matrix;
  float thickness = fmaxf(pLineSeries->StrokeThickness, 0.5f);
  for (int i = 1; i < count; i++) {
    if (valid[i - 1] && valid[i])
      LineSeries_Segment(pDrawForeground->viewdef, &entity, &matrix, points[i - 1], points[i], thickness);
  }
  if (pLineSeries->ShowMarkers) {
    float d = thickness * 2.5f;
    entity.matrix = matrix;
    entity.radius = (struct vec4){ d / 2, d / 2, d / 2, d / 2 };
    for (int i = 0; i < count; i++) {
      if (!valid[i]) continue;
      entity.bbox = Chart_Box(points[i].x - d / 2, points[i].y - d / 2, d, d);
      R_DrawEntity(pDrawForeground->viewdef, &entity);
    }
  }
  ChartSeries_DrawSwatch(hObject, pDrawForeground->viewdef, &entity, &matrix);
  return TRUE;
}

HANDLER(ColumnSeries, Node2D, ForegroundContent)
{
  return Chart_Of(hObject) != NULL;
}

HANDLER(ColumnSeries, Node2D, DrawForeground)
{
  struct vec2 points[CHART_MAX_POINTS];
  bool_t valid[CHART_MAX_POINTS];
  float baseline, slot;
  int count = ChartSeries_Points(hObject, points, valid, &baseline, &slot);
  if (!count) return FALSE;
  struct Node2D *view = GetNode2D(hObject);
  struct ViewEntity entity;
  Node2D_GetViewEntity(view, &entity, NULL, &pDrawForeground->brush);
  struct mat4 matrix = entity.matrix;
  entity.radius = (struct vec4){ 0 };
  float width = slot * fminf(fmaxf(pColumnSeries->ColumnWidth, 0.05f), 1);
  for (int i = 0; i < count; i++) {
    if (!valid[i]) continue;
    float top = fminf(points[i].y, baseline), height = fabsf(baseline - points[i].y);
    if (height < 0.5f) continue;
    entity.bbox = Chart_Box(points[i].x - width / 2, top, width, height);
    R_DrawEntity(pDrawForeground->viewdef, &entity);
  }
  ChartSeries_DrawSwatch(hObject, pDrawForeground->viewdef, &entity, &matrix);
  return TRUE;
}
