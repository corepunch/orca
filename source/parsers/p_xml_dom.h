#ifndef ORCA_P_XML_DOM_H
#define ORCA_P_XML_DOM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ORCA_USE_LIBXML2
#include <libxml/parser.h>
#include <libxml/tree.h>
#else

typedef char xmlChar;

#define XML_ELEMENT_NODE 1
#define XML_TEXT_NODE 3
#define XML_PI_NODE 7
#define XML_COMMENT_NODE 8

#define XML_PARSE_NOBLANKS 0
#define XML_PARSE_COMPACT 0

#define BAD_CAST (xmlChar *)

struct _xmlNode;
struct _xmlDoc;
struct _xmlAttr;
struct _xmlNs;
typedef struct _xmlNode xmlNode, *xmlNodePtr;
typedef struct _xmlDoc xmlDoc, *xmlDocPtr, *htmlDocPtr;
typedef struct _xmlAttr xmlAttr, *xmlAttrPtr;
typedef struct _xmlNs xmlNs, *xmlNsPtr;

typedef struct _xmlBuffer xmlBuffer, *xmlBufferPtr;
typedef struct _xmlOutputBuffer xmlOutputBuffer, *xmlOutputBufferPtr;

struct _xmlNs {
  xmlNsPtr next;
  xmlChar *href;
  xmlChar *prefix;
};

struct _xmlAttr {
  xmlChar *name;
  xmlChar *value;
  xmlNsPtr ns;
  xmlAttrPtr next;
  xmlNodePtr parent;
};

struct _xmlDoc {
  int type;
  xmlChar *name;
  xmlChar *URL;
  xmlNodePtr children;
  xmlChar *version;
};

struct _xmlNode {
  void * _private;
  int type;
  xmlChar *name;
  xmlNodePtr children;
  xmlNodePtr last;
  xmlNodePtr parent;
  xmlNodePtr next;
  xmlNodePtr prev;
  xmlDocPtr doc;
  xmlNsPtr ns;
  xmlNsPtr nsDef;
  xmlChar *content;
  xmlAttrPtr properties;
  int extra;
};

struct _xmlBuffer {
  xmlChar *content;
  size_t length;
  size_t capacity;
};

struct _xmlOutputBuffer {
  xmlBufferPtr buffer;
};

xmlDocPtr  xmlNewDoc(const xmlChar *version);
void       xmlFreeDoc(xmlDocPtr doc);
xmlDocPtr  xmlReadMemory(const char *buffer, int size, const char *url, const char *encoding, int options);
xmlNodePtr xmlDocGetRootElement(xmlDocPtr doc);
void       xmlDocSetRootElement(xmlDocPtr doc, xmlNodePtr node);
xmlNodePtr xmlNewNode(xmlNsPtr ns, const xmlChar *name);
xmlNodePtr xmlNewChild(xmlNodePtr parent, xmlNsPtr ns, const xmlChar *name, const xmlChar *content);
xmlNodePtr xmlAddChild(xmlNodePtr parent, xmlNodePtr child);
void       xmlUnlinkNode(xmlNodePtr node);
xmlAttrPtr xmlSetProp(xmlNodePtr node, const xmlChar *name, const xmlChar *value);
int        xmlUnsetProp(xmlNodePtr node, const xmlChar *name);
xmlChar   *xmlGetProp(xmlNodePtr node, const xmlChar *name);
xmlChar   *xmlGetNsProp(xmlNodePtr node, const xmlChar *name, const xmlChar *ns_href);
void       xmlNodeSetContent(xmlNodePtr node, const xmlChar *content);
void       xmlNodeSetName(xmlNodePtr node, const xmlChar *name);
xmlChar   *xmlNodeGetContent(xmlNodePtr node);
int        xmlStrcmp(const xmlChar *a, const xmlChar *b);
size_t     xmlStrlen(const xmlChar *s);
const xmlChar *xmlStrchr(const xmlChar *s, xmlChar ch);
void       xmlFree(void *mem);
xmlNsPtr   xmlNewNs(xmlNodePtr node, const xmlChar *href, const xmlChar *prefix);
xmlNsPtr   xmlFindNs(xmlNodePtr node, const xmlChar *url);
xmlBufferPtr       xmlBufferCreate(void);
void               xmlBufferFree(xmlBufferPtr buf);
const xmlChar     *xmlBufferContent(xmlBufferPtr buf);
size_t             xmlBufferLength(xmlBufferPtr buf);
xmlOutputBufferPtr xmlOutputBufferCreateBuffer(xmlBufferPtr buf, void *enc);
void               xmlNodeDump(xmlBufferPtr buf, xmlDocPtr doc, xmlNodePtr node, int level, int format);
int                xmlSaveFormatFile(const char *filename, xmlDocPtr doc, int format);
int                xmlSaveFormatFileTo(xmlOutputBufferPtr out, xmlDocPtr doc, const char *enc, int fmt);

#endif

#ifdef __cplusplus
}
#endif

#endif
