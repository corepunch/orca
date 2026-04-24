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
  memset(lpPropDef->pPrograms, 0, sizeof(lpPropDef->pPrograms));
  FOR_EACH_LIST(struct property_program const, pp, core_GetPrograms()) {
    if (pp->property == p) {
      lpPropDef->pPrograms[pp->attr] = pp->code;
    }
  }
  lpPropDef->bIsUsedInBinding = FALSE;
}

static lpcString_t _attrs[PropertyAttribute_Count] = {
  "WHOLE_PROPERTY",
  "COLOR_R", // = ATTR_VECTOR_X,
  "COLOR_G", // = ATTR_VECTOR_Y,
  "COLOR_B", // = ATTR_VECTOR_Z,
  "COLOR_A", // = ATTR_VECTOR_W,
  "VECTOR_X",
  "VECTOR_Y",
  "VECTOR_Z",
  "VECTOR_W",
};

#define default_url "http://schemas.corepunch.com/orca/2006/xml/presentation"
xmlNsPtr xmlFindNs(xmlNodePtr node, xmlChar const *url);

void ED_WriteBindings(HPROP prop, xmlNodePtr node) {
  FOR_EACH_LIST(struct property_program const, pp, core_GetPrograms()) {
    if (pp->property != prop) continue;
    xmlChar const *name = BAD_CAST pp->code;
    xmlChar const *enabled = pp->token ? BAD_CAST "true" : BAD_CAST "false";
    xmlNsPtr ns = xmlFindNs(node, BAD_CAST default_url);
    xmlNodePtr bnd = xmlNewChild(node, ns, XMLSTR("Binding"), name);
    xmlSetProp(bnd, XMLSTR("Property"), BAD_CAST prop->pdesc->Name);
    xmlSetProp(bnd, XMLSTR("Enabled"), enabled);
    if (pp->attr != kPropertyAttributeWholeProperty) {
      xmlSetProp(bnd, XMLSTR("Attribute"), BAD_CAST _attrs[pp->attr]);
    }
  }
}
