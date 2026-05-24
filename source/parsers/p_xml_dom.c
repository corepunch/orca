#include "p_xml_dom.h"

#include <expat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static xmlChar *xml_strdup(const xmlChar *s)
{
  return s ? (xmlChar *)strdup((const char *)s) : NULL;
}

void xmlFree(void *mem)
{
  free(mem);
}

int xmlStrcmp(const xmlChar *a, const xmlChar *b)
{
  if (a == b) return 0;
  if (!a) return -1;
  if (!b) return 1;
  return strcmp((const char *)a, (const char *)b);
}

size_t xmlStrlen(const xmlChar *s)
{
  return s ? strlen((const char *)s) : 0;
}

const xmlChar *xmlStrchr(const xmlChar *s, xmlChar ch)
{
  return s ? (const xmlChar *)strchr((const char *)s, ch) : NULL;
}

static void set_doc_tree(xmlNodePtr node, xmlDocPtr doc)
{
  for (; node; node = node->next) {
    node->doc = doc;
    if (node->children) set_doc_tree(node->children, doc);
  }
}

static xmlNodePtr first_sibling(xmlNodePtr node)
{
  while (node && node->prev) node = node->prev;
  return node;
}

static void free_ns_list(xmlNsPtr ns)
{
  while (ns) {
    xmlNsPtr next = ns->next;
    free(ns->href);
    free(ns->prefix);
    free(ns);
    ns = next;
  }
}

static void free_attr_list(xmlAttrPtr attr)
{
  while (attr) {
    xmlAttrPtr next = attr->next;
    free(attr->name);
    free(attr->value);
    free(attr);
    attr = next;
  }
}

static void free_node_chain(xmlNodePtr node)
{
  while (node) {
    xmlNodePtr next = node->next;
    if (node->children) free_node_chain(node->children);
    free_attr_list(node->properties);
    free_ns_list(node->nsDef);
    free(node->name);
    free(node->content);
    free(node);
    node = next;
  }
}

xmlDocPtr xmlNewDoc(const xmlChar *version)
{
  xmlDocPtr doc = calloc(1, sizeof(*doc));
  if (!doc) return NULL;
  doc->type = 9;
  doc->name = xml_strdup(BAD_CAST "document");
  doc->version = xml_strdup(version ? version : BAD_CAST "1.0");
  return doc;
}

void xmlFreeDoc(xmlDocPtr doc)
{
  if (!doc) return;
  if (doc->children) free_node_chain(first_sibling(doc->children));
  free(doc->name);
  free(doc->URL);
  free(doc->version);
  free(doc);
}

xmlNodePtr xmlDocGetRootElement(xmlDocPtr doc)
{
  return doc ? doc->children : NULL;
}

void xmlDocSetRootElement(xmlDocPtr doc, xmlNodePtr node)
{
  if (!doc) return;
  doc->children = node;
  if (!node) return;
  node->parent = NULL;
  set_doc_tree(first_sibling(node), doc);
}

xmlNodePtr xmlNewNode(xmlNsPtr ns, const xmlChar *name)
{
  xmlNodePtr node = calloc(1, sizeof(*node));
  if (!node) return NULL;
  node->type = XML_ELEMENT_NODE;
  node->name = xml_strdup(name);
  node->ns = ns;
  node->extra = 0;
  return node;
}

static xmlNodePtr xmlNewTextNode(const xmlChar *content)
{
  xmlNodePtr node = calloc(1, sizeof(*node));
  if (!node) return NULL;
  node->type = XML_TEXT_NODE;
  node->name = xml_strdup(BAD_CAST "#text");
  node->content = xml_strdup(content ? content : BAD_CAST "");
  return node;
}

static xmlNodePtr xmlNewSpecialNode(int type, const xmlChar *name, const xmlChar *content)
{
  xmlNodePtr node = calloc(1, sizeof(*node));
  if (!node) return NULL;
  node->type = type;
  node->name = xml_strdup(name);
  node->content = xml_strdup(content ? content : BAD_CAST "");
  return node;
}

void xmlUnlinkNode(xmlNodePtr node)
{
  if (!node) return;
  if (node->parent) {
    if (node->parent->children == node) node->parent->children = node->next;
    if (node->parent->last == node) node->parent->last = node->prev;
  } else if (node->doc && node->doc->children == node) {
    node->doc->children = NULL;
  }
  if (node->prev) node->prev->next = node->next;
  if (node->next) node->next->prev = node->prev;
  node->parent = NULL;
  node->next = NULL;
  node->prev = NULL;
  node->doc = NULL;
}

xmlNodePtr xmlAddChild(xmlNodePtr parent, xmlNodePtr child)
{
  if (!parent || !child) return child;
  if (child->parent || child->prev || child->next) xmlUnlinkNode(child);
  child->parent = parent;
  child->doc = parent->doc;
  child->prev = parent->last;
  child->next = NULL;
  if (parent->last) parent->last->next = child;
  else parent->children = child;
  parent->last = child;
  set_doc_tree(child, parent->doc);
  return child;
}

xmlNodePtr xmlNewChild(xmlNodePtr parent, xmlNsPtr ns, const xmlChar *name, const xmlChar *content)
{
  xmlNodePtr child = xmlNewNode(ns, name);
  if (!child) return NULL;
  if (content) xmlNodeSetContent(child, content);
  return xmlAddChild(parent, child);
}

static void clear_children(xmlNodePtr node)
{
  if (!node || !node->children) return;
  free_node_chain(node->children);
  node->children = NULL;
  node->last = NULL;
}

void xmlNodeSetContent(xmlNodePtr node, const xmlChar *content)
{
  if (!node) return;
  if (node->type == XML_ELEMENT_NODE) {
    clear_children(node);
    if (content && *content) xmlAddChild(node, xmlNewTextNode(content));
    return;
  }
  free(node->content);
  node->content = xml_strdup(content ? content : BAD_CAST "");
}

void xmlNodeSetName(xmlNodePtr node, const xmlChar *name)
{
  if (!node) return;
  free(node->name);
  node->name = xml_strdup(name);
}

static int buffer_reserve(xmlBufferPtr buf, size_t extra)
{
  if (!buf) return 0;
  size_t need = buf->length + extra + 1;
  if (need <= buf->capacity) return 1;
  size_t cap = buf->capacity ? buf->capacity : 128;
  while (cap < need) cap *= 2;
  xmlChar *new_content = realloc(buf->content, cap);
  if (!new_content) return 0;
  buf->content = new_content;
  buf->capacity = cap;
  return 1;
}

static int buffer_append_len(xmlBufferPtr buf, const char *text, size_t len)
{
  if (!buffer_reserve(buf, len)) return 0;
  memcpy(buf->content + buf->length, text, len);
  buf->length += len;
  buf->content[buf->length] = '\0';
  return 1;
}

static int buffer_append(xmlBufferPtr buf, const char *text)
{
  return buffer_append_len(buf, text, text ? strlen(text) : 0);
}

xmlBufferPtr xmlBufferCreate(void)
{
  xmlBufferPtr buf = calloc(1, sizeof(*buf));
  if (!buf) return NULL;
  if (!buffer_reserve(buf, 0)) {
    free(buf);
    return NULL;
  }
  buf->content[0] = '\0';
  return buf;
}

void xmlBufferFree(xmlBufferPtr buf)
{
  if (!buf) return;
  free(buf->content);
  free(buf);
}

const xmlChar *xmlBufferContent(xmlBufferPtr buf)
{
  return buf ? buf->content : NULL;
}

size_t xmlBufferLength(xmlBufferPtr buf)
{
  return buf ? buf->length : 0;
}

xmlOutputBufferPtr xmlOutputBufferCreateBuffer(xmlBufferPtr buf, void *enc)
{
  (void)enc;
  xmlOutputBufferPtr out = calloc(1, sizeof(*out));
  if (!out) return NULL;
  out->buffer = buf;
  return out;
}

xmlNsPtr xmlNewNs(xmlNodePtr node, const xmlChar *href, const xmlChar *prefix)
{
  xmlNsPtr ns = calloc(1, sizeof(*ns));
  if (!ns) return NULL;
  ns->href = xml_strdup(href);
  ns->prefix = xml_strdup(prefix);
  if (node) {
    ns->next = node->nsDef;
    node->nsDef = ns;
  }
  return ns;
}

static xmlNsPtr find_ns_by_prefix(xmlNodePtr node, const xmlChar *prefix)
{
  for (xmlNodePtr cur = node; cur; cur = cur->parent) {
    for (xmlNsPtr ns = cur->nsDef; ns; ns = ns->next) {
      if ((!prefix && !ns->prefix) || (prefix && ns->prefix && !xmlStrcmp(ns->prefix, prefix))) return ns;
    }
  }
  return NULL;
}

xmlNsPtr xmlFindNs(xmlNodePtr node, const xmlChar *url)
{
  for (xmlNodePtr cur = node; cur; cur = cur->parent) {
    for (xmlNsPtr ns = cur->nsDef; ns; ns = ns->next) {
      if (!xmlStrcmp(ns->href, url)) return ns;
    }
  }
  return NULL;
}

static void split_name(const xmlChar *name, xmlChar **prefix, const xmlChar **local)
{
  const xmlChar *colon = xmlStrchr(name, ':');
  if (colon) {
    size_t len = (size_t)(colon - name);
    *prefix = calloc(len + 1, 1);
    if (*prefix) memcpy(*prefix, name, len);
    *local = colon + 1;
  } else {
    *prefix = NULL;
    *local = name;
  }
}

static xmlAttrPtr find_attr(xmlNodePtr node, const xmlChar *name, const xmlChar *ns_href)
{
  if (!node) return NULL;
  for (xmlAttrPtr attr = node->properties; attr; attr = attr->next) {
    if (xmlStrcmp(attr->name, name)) continue;
    if (!ns_href && !attr->ns) return attr;
    if (ns_href && attr->ns && !xmlStrcmp(attr->ns->href, ns_href)) return attr;
  }
  return NULL;
}

xmlAttrPtr xmlSetProp(xmlNodePtr node, const xmlChar *name, const xmlChar *value)
{
  if (!node || !name) return NULL;
  xmlChar *prefix = NULL;
  const xmlChar *local = NULL;
  split_name(name, &prefix, &local);
  xmlNsPtr ns = prefix ? find_ns_by_prefix(node, prefix) : NULL;
  xmlAttrPtr attr = find_attr(node, local, ns ? ns->href : NULL);
  if (!attr) {
    attr = calloc(1, sizeof(*attr));
    if (!attr) {
      free(prefix);
      return NULL;
    }
    attr->name = xml_strdup(local);
    attr->ns = ns;
    attr->parent = node;
    attr->next = node->properties;
    node->properties = attr;
  }
  free(attr->value);
  attr->value = xml_strdup(value ? value : BAD_CAST "");
  free(prefix);
  return attr;
}

int xmlUnsetProp(xmlNodePtr node, const xmlChar *name)
{
  if (!node || !name) return -1;
  xmlChar *prefix = NULL;
  const xmlChar *local = NULL;
  split_name(name, &prefix, &local);
  xmlNsPtr ns = prefix ? find_ns_by_prefix(node, prefix) : NULL;
  xmlAttrPtr prev = NULL;
  for (xmlAttrPtr attr = node->properties; attr; prev = attr, attr = attr->next) {
    if (xmlStrcmp(attr->name, local)) continue;
    if ((ns && attr->ns != ns) || (!ns && attr->ns)) continue;
    if (prev) prev->next = attr->next;
    else node->properties = attr->next;
    free(attr->name);
    free(attr->value);
    free(attr);
    free(prefix);
    return 0;
  }
  free(prefix);
  return -1;
}

xmlChar *xmlGetProp(xmlNodePtr node, const xmlChar *name)
{
  xmlAttrPtr attr = find_attr(node, name, NULL);
  return attr && attr->value ? xml_strdup(attr->value) : NULL;
}

xmlChar *xmlGetNsProp(xmlNodePtr node, const xmlChar *name, const xmlChar *ns_href)
{
  xmlAttrPtr attr = find_attr(node, name, ns_href);
  return attr && attr->value ? xml_strdup(attr->value) : NULL;
}

static void node_content_append(xmlBufferPtr buf, xmlNodePtr node)
{
  for (; node; node = node->next) {
    if (node->type == XML_TEXT_NODE || node->type == XML_PI_NODE) {
      if (node->content) buffer_append(buf, (const char *)node->content);
    } else if (node->children) {
      node_content_append(buf, node->children);
    }
  }
}

xmlChar *xmlNodeGetContent(xmlNodePtr node)
{
  if (!node) return NULL;
  if (node->type != XML_ELEMENT_NODE) return xml_strdup(node->content ? node->content : BAD_CAST "");
  xmlBufferPtr buf = xmlBufferCreate();
  if (!buf) return NULL;
  node_content_append(buf, node->children);
  xmlChar *content = xml_strdup(xmlBufferContent(buf));
  xmlBufferFree(buf);
  return content;
}

static void buffer_indent(xmlBufferPtr buf, int level)
{
  for (int i = 0; i < level; ++i) buffer_append(buf, "  ");
}

static void buffer_escape(xmlBufferPtr buf, const xmlChar *text, int attr)
{
  for (const xmlChar *s = text; s && *s; ++s) {
    switch (*s) {
      case '&': buffer_append(buf, "&amp;"); break;
      case '<': buffer_append(buf, "&lt;"); break;
      case '>': buffer_append(buf, "&gt;"); break;
      case '"': buffer_append(buf, attr ? "&quot;" : "\""); break;
      default: buffer_append_len(buf, (const char *)s, 1); break;
    }
  }
}

static int has_element_child(xmlNodePtr node)
{
  for (xmlNodePtr child = node ? node->children : NULL; child; child = child->next) {
    if (child->type == XML_ELEMENT_NODE || child->type == XML_COMMENT_NODE || child->type == XML_PI_NODE) return 1;
  }
  return 0;
}

static int is_whitespace_only(const xmlChar *text)
{
  if (!text) return 1;
  for (const xmlChar *s = text; *s; ++s) {
    if (!strchr(" \t\r\n", *s)) return 0;
  }
  return 1;
}

static void serialize_node(xmlBufferPtr buf, xmlNodePtr node, int level, int format)
{
  if (!node) return;
  switch (node->type) {
    case XML_TEXT_NODE: {
      int complex_parent = has_element_child(node->parent);
      if (format && complex_parent && is_whitespace_only(node->content)) return;
      if (format && complex_parent) buffer_indent(buf, level);
      buffer_escape(buf, node->content ? node->content : BAD_CAST "", 0);
      if (format && complex_parent) buffer_append(buf, "\n");
      return;
    }
    case XML_COMMENT_NODE:
      if (format) buffer_indent(buf, level);
      buffer_append(buf, "<!--");
      buffer_append(buf, (const char *)(node->content ? node->content : BAD_CAST ""));
      buffer_append(buf, "-->");
      if (format) buffer_append(buf, "\n");
      return;
    case XML_PI_NODE:
      if (format) buffer_indent(buf, level);
      buffer_append(buf, "<?");
      buffer_append(buf, (const char *)(node->name ? node->name : BAD_CAST "pi"));
      if (node->content && *node->content) {
        buffer_append(buf, " ");
        buffer_append(buf, (const char *)node->content);
      }
      buffer_append(buf, "?>");
      if (format) buffer_append(buf, "\n");
      return;
    default:
      break;
  }
  if (format) buffer_indent(buf, level);
  buffer_append(buf, "<");
  if (node->ns && node->ns->prefix && *node->ns->prefix) {
    buffer_append(buf, (const char *)node->ns->prefix);
    buffer_append(buf, ":");
  }
  buffer_append(buf, (const char *)(node->name ? node->name : BAD_CAST "node"));
  for (xmlNsPtr ns = node->nsDef; ns; ns = ns->next) {
    buffer_append(buf, ns->prefix ? " xmlns:" : " xmlns");
    if (ns->prefix) buffer_append(buf, (const char *)ns->prefix);
    buffer_append(buf, "=\"");
    buffer_escape(buf, ns->href ? ns->href : BAD_CAST "", 1);
    buffer_append(buf, "\"");
  }
  for (xmlAttrPtr attr = node->properties; attr; attr = attr->next) {
    buffer_append(buf, " ");
    if (attr->ns && attr->ns->prefix && *attr->ns->prefix) {
      buffer_append(buf, (const char *)attr->ns->prefix);
      buffer_append(buf, ":");
    }
    buffer_append(buf, (const char *)attr->name);
    buffer_append(buf, "=\"");
    buffer_escape(buf, attr->value ? attr->value : BAD_CAST "", 1);
    buffer_append(buf, "\"");
  }
  if (!node->children) {
    buffer_append(buf, "/>");
    if (format) buffer_append(buf, "\n");
    return;
  }
  int complex = has_element_child(node);
  buffer_append(buf, ">");
  if (format && complex) buffer_append(buf, "\n");
  for (xmlNodePtr child = node->children; child; child = child->next) {
    serialize_node(buf, child, level + (complex ? 1 : 0), format);
  }
  if (format && complex) buffer_indent(buf, level);
  buffer_append(buf, "</");
  if (node->ns && node->ns->prefix && *node->ns->prefix) {
    buffer_append(buf, (const char *)node->ns->prefix);
    buffer_append(buf, ":");
  }
  buffer_append(buf, (const char *)node->name);
  buffer_append(buf, ">");
  if (format) buffer_append(buf, "\n");
}

void xmlNodeDump(xmlBufferPtr buf, xmlDocPtr doc, xmlNodePtr node, int level, int format)
{
  (void)doc;
  serialize_node(buf, node, level, format);
}

int xmlSaveFormatFile(const char *filename, xmlDocPtr doc, int format)
{
  xmlBufferPtr buf = xmlBufferCreate();
  if (!buf) return -1;
  xmlOutputBufferPtr out = xmlOutputBufferCreateBuffer(buf, NULL);
  if (!out) {
    xmlBufferFree(buf);
    return -1;
  }
  if (xmlSaveFormatFileTo(out, doc, NULL, format) < 0) {
    xmlBufferFree(buf);
    return -1;
  }
  FILE *fp = fopen(filename, "wb");
  if (!fp) {
    xmlBufferFree(buf);
    return -1;
  }
  size_t written = fwrite(xmlBufferContent(buf), 1, xmlBufferLength(buf), fp);
  fclose(fp);
  xmlBufferFree(buf);
  return (int)written;
}

int xmlSaveFormatFileTo(xmlOutputBufferPtr out, xmlDocPtr doc, const char *enc, int fmt)
{
  (void)enc;
  if (!out || !out->buffer || !doc) {
    free(out);
    return -1;
  }
  if (doc->version && *doc->version) {
    buffer_append(out->buffer, "<?xml version=\"");
    buffer_append(out->buffer, (const char *)doc->version);
    buffer_append(out->buffer, "\"?>");
    if (fmt) buffer_append(out->buffer, "\n");
  }
  for (xmlNodePtr node = first_sibling(doc->children); node; node = node->next) {
    serialize_node(out->buffer, node, 0, fmt);
  }
  int len = (int)xmlBufferLength(out->buffer);
  free(out);
  return len;
}

struct parse_ctx {
  xmlDocPtr doc;
  xmlNodePtr current;
  xmlNodePtr root;
  xmlNodePtr top_last;
};

static void append_top_level(struct parse_ctx *ctx, xmlNodePtr node)
{
  node->doc = ctx->doc;
  node->parent = NULL;
  node->prev = ctx->top_last;
  node->next = NULL;
  if (ctx->top_last) ctx->top_last->next = node;
  ctx->top_last = node;
  if (!ctx->root && node->type == XML_ELEMENT_NODE) {
    ctx->root = node;
    ctx->doc->children = node;
  }
}

static void append_text(xmlNodePtr parent, const XML_Char *s, int len)
{
  if (!parent || len <= 0) return;
  xmlNodePtr text = parent->last;
  if (text && text->type == XML_TEXT_NODE) {
    size_t old_len = xmlStrlen(text->content);
    xmlChar *merged = realloc(text->content, old_len + (size_t)len + 1);
    if (!merged) return;
    memcpy(merged + old_len, s, (size_t)len);
    merged[old_len + (size_t)len] = '\0';
    text->content = merged;
    return;
  }
  text = xmlNewSpecialNode(XML_TEXT_NODE, BAD_CAST "#text", BAD_CAST "");
  if (!text) return;
  free(text->content);
  text->content = calloc((size_t)len + 1, 1);
  if (!text->content) {
    free(text->name);
    free(text);
    return;
  }
  memcpy(text->content, s, (size_t)len);
  xmlAddChild(parent, text);
}

static void parse_start(void *ud, const XML_Char *name, const XML_Char **atts)
{
  struct parse_ctx *ctx = ud;
  xmlChar *prefix = NULL;
  const xmlChar *local = NULL;
  const xmlChar *qname = (const xmlChar *)name;
  split_name(qname, &prefix, &local);
  xmlNodePtr node = xmlNewNode(NULL, local);
  free(prefix);
  if (!node) return;
  if (ctx->current) xmlAddChild(ctx->current, node);
  else append_top_level(ctx, node);
  for (int i = 0; atts && atts[i]; i += 2) {
    if (!strcmp(atts[i], "xmlns")) xmlNewNs(node, BAD_CAST atts[i + 1], NULL);
    else if (!strncmp(atts[i], "xmlns:", 6)) xmlNewNs(node, BAD_CAST atts[i + 1], BAD_CAST(atts[i] + 6));
  }
  split_name(qname, &prefix, &local);
  if (prefix) node->ns = find_ns_by_prefix(node, prefix);
  free(prefix);
  for (int i = 0; atts && atts[i]; i += 2) {
    if (!strcmp(atts[i], "xmlns") || !strncmp(atts[i], "xmlns:", 6)) continue;
    xmlSetProp(node, BAD_CAST atts[i], BAD_CAST atts[i + 1]);
  }
  ctx->current = node;
}

static void parse_end(void *ud, const XML_Char *name)
{
  (void)name;
  struct parse_ctx *ctx = ud;
  if (ctx->current) ctx->current = ctx->current->parent;
}

static void parse_chars(void *ud, const XML_Char *s, int len)
{
  struct parse_ctx *ctx = ud;
  if (ctx->current) append_text(ctx->current, s, len);
}

static void parse_pi(void *ud, const XML_Char *target, const XML_Char *data)
{
  struct parse_ctx *ctx = ud;
  xmlNodePtr node = xmlNewSpecialNode(XML_PI_NODE, BAD_CAST target, BAD_CAST(data ? data : ""));
  if (!node) return;
  if (ctx->current) xmlAddChild(ctx->current, node);
  else append_top_level(ctx, node);
}

static void parse_comment(void *ud, const XML_Char *data)
{
  struct parse_ctx *ctx = ud;
  xmlNodePtr node = xmlNewSpecialNode(XML_COMMENT_NODE, BAD_CAST "#comment", BAD_CAST(data ? data : ""));
  if (!node) return;
  if (ctx->current) xmlAddChild(ctx->current, node);
  else append_top_level(ctx, node);
}

xmlDocPtr xmlReadMemory(const char *buffer, int size, const char *url, const char *encoding, int options)
{
  (void)encoding;
  (void)options;
  if (!buffer || size < 0) return NULL;
  xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
  if (!doc) return NULL;
  free(doc->URL);
  free(doc->name);
  doc->URL = xml_strdup((const xmlChar *)url);
  doc->name = xml_strdup((const xmlChar *)(url ? url : "document"));
  struct parse_ctx ctx = { .doc = doc };
  XML_Parser parser = XML_ParserCreate(NULL);
  if (!parser) {
    xmlFreeDoc(doc);
    return NULL;
  }
  XML_SetUserData(parser, &ctx);
  XML_SetElementHandler(parser, parse_start, parse_end);
  XML_SetCharacterDataHandler(parser, parse_chars);
  XML_SetProcessingInstructionHandler(parser, parse_pi);
  XML_SetCommentHandler(parser, parse_comment);
  if (XML_Parse(parser, buffer, size, 1) == XML_STATUS_ERROR) {
    XML_ParserFree(parser);
    xmlFreeDoc(doc);
    return NULL;
  }
  XML_ParserFree(parser);
  return doc;
}
