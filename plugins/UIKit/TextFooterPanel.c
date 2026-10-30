#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <UIKit/UIKit.h>

// The text, its footer and where the footer goes for one available width.
struct footer_layout
{
  struct Object *text, *footer;
  struct Size textSize, footerSize, size;
  bool_t shared; // footer sits on the text's last line
};

// Where the text's last line ends, including its leading margin and padding.
static float
_LastLineEnd(struct Object *hText, float width)
{
  struct TextRun *run = GetTextRun(hText);
  if (!run) return width;
  struct Node2D *n = GetNode2D(hText);
  return MARGIN_TOP(n, 0) + PADDING_TOP(n, 0) + run->_textinfo.txLastLineWidth;
}

static struct footer_layout
_FooterLayout(struct Object *hObject, struct TextFooterPanel *pPanel, float width, float height)
{
  struct footer_layout l = { 0 };
  FOR_EACH_LAYOUTABLE(c, hObject) { if (l.footer && !l.text) l.text = l.footer; l.footer = c; }
  if (!l.footer) return l;
  LRESULT s = _SendMessage(l.footer, Node2D, Measure, width, height);
  l.footerSize = (struct Size){ LOWORD(s), HIWORD(s) };
  if (!l.text) return l.size = l.footerSize, l;
  s = _SendMessage(l.text, Node2D, Measure, width, height);
  l.textSize = (struct Size){ LOWORD(s), HIWORD(s) };
  float const sharedWidth = _LastLineEnd(l.text, l.textSize.width) + pPanel->Spacing + l.footerSize.width;
  l.shared = sharedWidth <= width;
  l.size = l.shared
    ? (struct Size){ MAX(l.textSize.width, sharedWidth), MAX(l.textSize.height, l.footerSize.height) }
    : (struct Size){ MAX(l.textSize.width, l.footerSize.width), l.textSize.height + l.footerSize.height };
  return l;
}

HANDLER(TextFooterPanel, Node2D, MeasureOverride)
{
  struct footer_layout l = _FooterLayout(hObject, pTextFooterPanel, pMeasureOverride->Width, pMeasureOverride->Height);
  return MAKEDWORD(l.size.width, l.size.height);
}

HANDLER(TextFooterPanel, Node2D, ArrangeOverride)
{
  Node2D_ArrangeOverrideMsg_t r = *pArrangeOverride;
  // Measure again at the final width: a wider panel can rewrap its text.
  struct footer_layout l = _FooterLayout(hObject, pTextFooterPanel, r.Width, r.Height);
  if (l.text) {
    float const textHeight = l.shared ? r.Height : MAX(0, r.Height - l.footerSize.height);
    _SendMessage(l.text, Node2D, Arrange, r.X, r.Y, r.Width, MIN(l.textSize.height, textHeight));
  }
  if (l.footer) {
    _SendMessage(l.footer, Node2D, Arrange,
                 r.X + r.Width - l.footerSize.width, r.Y + r.Height - l.footerSize.height,
                 l.footerSize.width, l.footerSize.height);
  }
  return MAKEDWORD(r.Width, r.Height);
}
