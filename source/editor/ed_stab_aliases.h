#include "ed_local.h"

#define default_url "http://schemas.corepunch.com/orca/2006/xml/presentation"
xmlNsPtr xmlFindNs(xmlNodePtr node, xmlChar const *url);

bool_t
UI_EnumObjectAliases(lpObject_t object, EnumAliasProc proc, void* args)
{
  FOR_LOOP(i, GetNode(object) ? GetNode(object)->NumResources : 0) {
    proc(GetNode(object)->Resources[i].Key, GetNode(object)->Resources[i].Value, args);
  }
  return TRUE;
}
//
//void ED_WriteAliases(struct alias *it, xmlNodePtr node) {
//  if (it->next) {
//    ED_WriteAliases(it->next, node);
//  }
//  xmlNsPtr ns = xmlFindNs(node, BAD_CAST default_url);
//  xmlNodePtr xml = xmlNewChild(node, ns, XMLSTR("Resource"), BAD_CAST it->path);
//  xmlSetProp(xml, XMLSTR("id"), BAD_CAST it->name);
//}
//
