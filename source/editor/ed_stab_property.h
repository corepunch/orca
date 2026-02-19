#include "ed_local.h"

void UI_FillOutPropDef(HOBJ, HPROP, LPPROPDEF);

BOOL UI_GetProperty(DWORD dwIndex, LPPROPDEF lpOut) {
//  HPROP property = GetPropertyAtIndex(dwIndex);
//  if (property) {
//    UI_FillOutPropDef(NULL, property, lpOut);
//    return TRUE;
//  } else {
    return FALSE;
//  }
}

void UI_RefreshProperty(DWORD dwIndex) {
//  HPROP property = GetPropertyAtIndex(dwIndex);
//  if (property) {
    // SetPropertyValue(property, property->source_value);
//  }
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
  lpPropDef->lpEnumValues   = p->type == kDataTypeEnum ? p->userdata : NULL;
  lpPropDef->dwFlags        = PROP_GetFlags(p);
  memcpy(lpPropDef->pPrograms, p->programSources, sizeof(p->programSources));
  lpPropDef->bIsUsedInBinding = FALSE;
}

ORCA_API void
PDESC_Print(lpcPropertyDesc_t pdesc, LPSTR buffer, DWORD len, float const* pf)
{
  switch (pdesc->DataType) {
    case kDataTypeBool:
      strcpy(buffer, *(int*)pf ? "true" : "false");
      break;
    case kDataTypeInt:
      snprintf(buffer, len, "%dd", *(int*)pf);
      break;
    case kDataTypeEnum:
      strncpy(buffer, strlistget(*(int*)pf, pdesc->TypeString), len);
      break;
    case kDataTypeFloat:
    case kDataTypeVector2D:
    case kDataTypeVector3D:
    case kDataTypeVector4D:
    case kDataTypeMatrix3D:
    case kDataTypeTransform2D:
    case kDataTypeTransform3D:
    case kDataTypeEdges:
      snprintf(buffer, len, "%g", pf[0]);
      FOR_LOOP(i, (int)pdesc->DataSize/sizeof(float)-1) {
        size_t buflen = strlen(buffer);
        snprintf(buffer + buflen, len - buflen, " %g", pf[i+1]);
      }
      break;
    case kDataTypeFixed:
      strncpy(buffer, (lpcString_t)pf, len);
      break;
    case kDataTypeLongString:
      strncpy(buffer, (lpcString_t)pf, len);
      break;
    case kDataTypeObject:
      if (*(handle_t *)pf) {
        lpcString_t s = "nil";
//        if (pdesc->TypeString && strcmp(pdesc->TypeString, "Object")) {
//          if (CMP_GetObject(*(handle_t *)pf)) {
//            s = OBJ_GetSourceFile(CMP_GetObject(*(handle_t *)pf));
//          }
//        } else {
//          s = OBJ_GetSourceFile(*(handle_t *)pf);
//        }
        if (s) {
          strncpy(buffer, s, len);
        } else {
          *buffer = 0;
        }
      } else {
        strncpy(buffer, "nil", len);
      }
      break;
    case kDataTypeGroup: {
      bool_t add_space = FALSE;
      FOR_LOOP(i, pdesc->NumComponents) {
        lpcPropertyDesc_t p = pdesc+i+1;
        if ((p->DataType) == kDataTypeGroup) continue;
        void* d =(char*)pf + p->Offset - pdesc->Offset;
        if (add_space) {strcat(buffer, " "); buffer++; len--;};
        PDESC_Print(p, buffer, len, d);
        if (strlen(buffer) >= len) {
          buffer[len-1] = 0;
          return;;
        }
        len -= strlen(buffer);
        buffer += strlen(buffer);
        add_space = TRUE;
      }}
      return;
//      if (!strstr(pdesc->typestring, "transform")) {
//        snprintf(buffer, len, "%g", pf[0]);
//        for (int i = 1; i < pdesc->size / sizeof(float); i++) {
//          size_t buflen = strlen(buffer);
//          snprintf(buffer + buflen, len - buflen, " %g", pf[i]);
//        }
//      }
//      break;
    case kDataTypeColor: {
      byte_t r = pf[0] * 255, g = pf[1] * 255, b = pf[2] * 255, a = pf[2] * 255;
      if (pf[3] < 1) {
        snprintf(buffer, len, "#%02x%02x%02x%02x", a, r, g, b);
      } else {
        snprintf(buffer, len, "#%02x%02x%02x", r, g, b);
      }
      break;
    }
    default:
      Con_Error("Unknown type %d in property %s\n", pdesc->DataType, pdesc->id->Name);
      break;
  }
}

void PROP_Print(HPROP p, LPSTR buffer, DWORD len) {
  PDESC_Print(p->pdesc ? p->pdesc: &(struct PropertyDesc){
    .DataType = p->type,
    .DataSize = PROP_GetSize(p),
    .TypeString = p->userdata,
  }, buffer, len, PROP_GetValue(p));
}

static lpcString_t _attrs[ATTR_COUNT] = {
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
  FOR_LOOP(i, ATTR_COUNT) {
    if (prop->programs[i]) {
      xmlChar const *name =  BAD_CAST prop->programSources[i];
      xmlNsPtr ns = xmlFindNs(node, BAD_CAST default_url);
      xmlNodePtr bnd = xmlNewChild(node, ns, XMLSTR("Binding"), name);
      xmlSetProp(bnd, XMLSTR("Property"), BAD_CAST prop->name);
      xmlSetProp(bnd, XMLSTR("Enabled"), BAD_CAST (prop->programs[i]?"true":"false"));
      if (i != ATTR_WHOLE_PROPERTY) {
        xmlSetProp(bnd, XMLSTR("Attribute"), BAD_CAST _attrs[i]);
      }
    }
  }
}
