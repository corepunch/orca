#include "ed_local.h"

void UI_FillOutPropDef(struct Object *, HPROP, LPPROPDEF);

BOOL UI_GetProperty(DWORD dwIndex, LPPROPDEF lpOut) {
    return FALSE;
}

void UI_RefreshProperty(DWORD dwIndex) {
}

void UI_FillOutPropDef(struct Object *object, HPROP p, LPPROPDEF lpPropDef) {
#ifdef EDITOR_LIB
  lpPropDef->szName = PROP_GetName(p);
#endif
  lpPropDef->dwHandleType   = 0;
  lpPropDef->Type           = PROP_GetType(p);
  lpPropDef->dwSize         = PROP_GetSize(p);
  lpPropDef->lpEditorValue  = (void*)PROP_GetValue(p);
  lpPropDef->lpRuntimeValue = (void*)PROP_GetValue(p);
  lpPropDef->bHasBinding    = PROP_HasProgram(p);
  lpPropDef->lpEnumValues   = PROP_GetUserData(p);
  lpPropDef->lpEnumArray    = p->pdesc->EnumValues;
  lpPropDef->dwFlags        = PROP_GetFlags(p);
  lpPropDef->pProgram = p->binding ? "" : NULL;
  lpPropDef->bIsUsedInBinding = FALSE;
}

#define default_url "http://schemas.corepunch.com/orca/2006/xml/presentation"
xmlNsPtr xmlFindNs(xmlNodePtr node, xmlChar const *url);

void ED_WriteBindings(HPROP prop, xmlNodePtr node) {
  if (prop->binding) {
    struct Binding const *pp = prop->binding;
    xmlChar const *name = BAD_CAST "";
    xmlChar const *enabled = pp->token ? BAD_CAST "true" : BAD_CAST "false";
    xmlNsPtr ns = xmlFindNs(node, BAD_CAST default_url);
    xmlNodePtr bnd = xmlNewChild(node, ns, XMLSTR("Binding"), name);
    xmlSetProp(bnd, XMLSTR("Property"), BAD_CAST prop->pdesc->Name);
    xmlSetProp(bnd, XMLSTR("Enabled"), enabled);
  }
}
