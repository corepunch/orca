#include "ed_local.h"

void UI_FillOutPropDef(HOBJ, HPROP, LPPROPDEF);

BOOL UI_GetProperty(DWORD dwIndex, LPPROPDEF lpOut) {
    return FALSE;
}

void UI_RefreshProperty(DWORD dwIndex) {
}

void UI_FillOutPropDef(HOBJ object, HPROP p, LPPROPDEF lpPropDef) {
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
  memcpy(lpPropDef->pPrograms, p->programSources, sizeof(p->programSources));
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
  FOR_LOOP(i, PropertyAttribute_Count) {
    if (prop->programs[i]) {
      xmlChar const *name =  BAD_CAST prop->programSources[i];
      xmlNsPtr ns = xmlFindNs(node, BAD_CAST default_url);
      xmlNodePtr bnd = xmlNewChild(node, ns, XMLSTR("Binding"), name);
      xmlSetProp(bnd, XMLSTR("Property"), BAD_CAST prop->pdesc->Name);
      xmlSetProp(bnd, XMLSTR("Enabled"), BAD_CAST (prop->programs[i]?"true":"false"));
      if (i != kPropertyAttributeWholeProperty) {
        xmlSetProp(bnd, XMLSTR("Attribute"), BAD_CAST _attrs[i]);
      }
    }
  }
}
