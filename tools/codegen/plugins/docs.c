#include <string.h>
#include "cg_api.h"
#include "outbuf.h"

typedef enum { API_C, API_LUA, API_XML } api_mode;

static api_mode parse_mode(char const **p) {
  if (!*p) return API_C;
  if (!strncmp(*p,"lua:",4)) { *p+=4; return API_LUA; }
  if (!strncmp(*p,"xml:",4)) { *p+=4; return API_XML; }
  if (!strncmp(*p,"c:",  2)) { *p+=2; return API_C;   }
  return API_C;
}

static char const *after_dot(char const *s) { char const *p = s ? strrchr(s,'.') : NULL; return p ? p+1 : (s?s:""); }
static char const *strip_ptr_array(char const *t) { if (!t) return ""; while (*t=='*'||*t==' '||*t=='[') ++t; return t; }

static cg_node const *find_class(cg_model const *m, char const *name) {
  if (!name||!name[0]) return NULL;
  for (size_t i=0; i<m->node_count; ++i)
    if (m->nodes[i].kind==CG_KIND_CLASS && !strcmp(m->nodes[i].name,name)) return &m->nodes[i];
  return NULL;
}
static cg_node const *find_enum_or_struct(cg_model const *m, char const *name) {
  if (!name||!name[0]) return NULL;
  for (size_t i=0; i<m->node_count; ++i) {
    cg_node const *n=&m->nodes[i];
    if ((n->kind==CG_KIND_ENUM||n->kind==CG_KIND_STRUCT) && !strcmp(n->name,name)) return n;
  }
  return NULL;
}

static int collect_names(char const **out, int outsz, char const *list) {
  char tmp[1024]; char *tok; int n=0;
  if (!list||!list[0]) return 0;
  snprintf(tmp,sizeof(tmp),"%s",list);
  for (tok=strtok(tmp,","); tok; tok=strtok(NULL,",")) {
    while (*tok==' '||*tok=='\t') ++tok;
    if (*tok && n<outsz) out[n++]=tok;
  }
  return n;
}

static int emit_type(outbuf *b, cg_node const *p) {
  if ((p->flags&CG_FLAG_POINTER)&&(p->flags&CG_FLAG_ARRAY)) return ob_printf(b,"`%s*[]`",p->type);
  if (p->flags&CG_FLAG_POINTER) return ob_printf(b,"`%s*`",p->type);
  if (p->flags&CG_FLAG_ARRAY)   return ob_printf(b,"`%s[]`",p->type);
  return ob_printf(b,"`%s`",p->type);
}

static int has_any_props(cg_model const *m, cg_node const *cls) {
  cg_foreach(m,cls->id,CG_KIND_PROPERTY,p) if (!(p->flags&CG_FLAG_PRIVATE)) return 1;
  return 0;
}

static int emit_props_for_class(outbuf *b, cg_model const *m, cg_node const *cls,
                char const *section, int sub) {
  int has=0;
  for (cg_node const *p=cg_first(m,cls->id,CG_KIND_PROPERTY); p; p=cg_next(m,p,cls->id,CG_KIND_PROPERTY))
    if (!(p->flags&CG_FLAG_PRIVATE)) { has=1; break; }
  if (!has) return 0;
  if (sub && ob_printf(b,"\n### From %s\n\n",section)<0) return -1;
  if (ob_printf(b,"| Property | Type | Default | Description |\n|----------|------|---------|-------------|\n")<0) return -1;
  cg_foreach(m,cls->id,CG_KIND_PROPERTY,pr) {
    if (pr->flags&CG_FLAG_PRIVATE) continue;
    if (ob_printf(b,"| `%s` | ",pr->name)<0) return -1;
    if (emit_type(b,pr)<0) return -1;
    if (ob_printf(b," | %s | %s |\n",(pr->extra&&pr->extra[0])?pr->extra:"\xe2\x80\x94",pr->doc?pr->doc:"")<0) return -1;
  }
  return 0;
}

static int parents_have_props(cg_model const *m, char const *list) {
  char buf[1024]; char *tok;
  if (!list||!list[0]) return 0;
  snprintf(buf,sizeof(buf),"%s",list);
  for (tok=strtok(buf,","); tok; tok=strtok(NULL,",")) {
    while (*tok==' '||*tok=='\t') ++tok;
    cg_node const *p=find_class(m,tok);
    if (p && has_any_props(m,p)) return 1;
  }
  return 0;
}

static int emit_inherited_props(outbuf *b, cg_model const *m, char const *list) {
  char buf[1024]; char *tok;
  if (!list||!list[0]) return 0;
  snprintf(buf,sizeof(buf),"%s",list);
  for (tok=strtok(buf,","); tok; tok=strtok(NULL,",")) {
    while (*tok==' '||*tok=='\t') ++tok;
    cg_node const *p=find_class(m,tok);
    if (p && emit_props_for_class(b,m,p,p->name,1)<0) return -1;
  }
  return 0;
}

#define MAX_CHAIN 16
static int emit_chain(outbuf *b, cg_model const *m, cg_node const *cls) {
  char const *chain[MAX_CHAIN]; int n=0; cg_node const *cur=cls;
  chain[n++]=cls->name;
  while (cur->type&&cur->type[0]&&n<MAX_CHAIN) {
    if (strchr(cur->type,',')) break;
    cur=find_class(m,cur->type); if (!cur) break;
    chain[n++]=cur->name;
  }
  for (int i=n-1; i>=0; --i) {
    if (i<n-1 && ob_printf(b," \xe2\x86\x92 ")<0) return -1;
    if (ob_printf(b,"%s",chain[i])<0) return -1;
  }
  return 0;
}

static int emit_inheritance(outbuf *b, cg_model const *m, cg_node const *cls) {
  if (!cls->type||!cls->type[0]) return 0;
  if (ob_printf(b,"\n## Inheritance\n\n")<0) return -1;
  if (strchr(cls->type,',')) {
    char buf[1024]; char *tok; int first=1;
    snprintf(buf,sizeof(buf),"%s",cls->type);
    for (tok=strtok(buf,","); tok; tok=strtok(NULL,",")) {
      while (*tok==' '||*tok=='\t') ++tok;
      if (!first && ob_printf(b,", ")<0) return -1;
      if (ob_printf(b,"`%s`",tok)<0) return -1;
      first=0;
    }
    return ob_printf(b," \xe2\x86\x92 `%s`\n",cls->name);
  }
  return emit_chain(b,m,cls)<0 ? -1 : ob_printf(b,"\n");
}

static int emit_handles(outbuf *b, cg_model const *m, cg_node const *cls) {
  int has=0;
  cg_foreach(m,cls->id,CG_KIND_HANDLE,h) { (void)h; has=1; break; }
  if (!has) return 0;
  if (ob_printf(b,"\n## Handled Messages\n\n| Message | Handler |\n|---------|---------|\n")<0) return -1;
  cg_foreach(m,cls->id,CG_KIND_HANDLE,hdl)
    if (ob_printf(b,"| `%s` | `%s_%s` |\n",hdl->name,cls->name,after_dot(hdl->name))<0) return -1;
  return 0;
}

static int emit_see_also(outbuf *b, cg_model const *m, cg_node const *node, cg_kind child_kind, int is_class) {
  char const *parents[64]; int np=collect_names(parents,64,node->type), count=0, first=1;
  int total=0;
  cg_foreach(m,node->id,child_kind,_p) {
    if (_p->flags&CG_FLAG_PRIVATE) continue;
    char const *base=strip_ptr_array(_p->type);
    if (find_enum_or_struct(m,base)||find_class(m,base)) total++;
  }
  for (int i=0; i<np; ++i)
    if (is_class ? find_class(m,parents[i]) : find_enum_or_struct(m,parents[i])) total++;
  if (!total) return 0;
  if (ob_printf(b,"\n## See Also\n\n")<0) return -1;
  cg_foreach(m,node->id,child_kind,pr) {
    if (pr->flags&CG_FLAG_PRIVATE) continue;
    char const *base=strip_ptr_array(pr->type);
    cg_node const *found=find_enum_or_struct(m,base); if (!found) found=find_class(m,base);
    if (!found) continue;
    if (found->kind==CG_KIND_CLASS) {
      if (ob_printf(b,"%s- [%s](classes/%s.md)\n",first?"":"\n",found->name,found->name)<0) return -1;
    } else {
      if (ob_printf(b,"%s- [%s](%s/%s.md) — %s\n",first?"":"\n",found->name,
          found->kind==CG_KIND_ENUM?"enums":"structs",found->name,
          found->doc&&found->doc[0]?found->doc:"")<0) return -1;
    }
    first=0; count++;
  }
  for (int i=0; i<np; ++i) {
    cg_node const *par=is_class?find_class(m,parents[i]):find_enum_or_struct(m,parents[i]);
    if (!par) continue;
    if (ob_printf(b,"%s- [%s](%s/%s.md)\n",first?"":"\n",par->name,
        is_class?"classes":"structs",par->name)<0) return -1;
    first=0; count++;
  }
  return count;
}

/* --- props+inheritance block shared by C/Lua/XML class emitters --- */
static int emit_props_block(outbuf *b, cg_model const *m, cg_node const *cls, char const *header) {
  int own=has_any_props(m,cls), inh=parents_have_props(m,cls->type);
  if (!own&&!inh) return 0;
  if (ob_printf(b,"\n%s\n",header)<0) return -1;
  if (own && emit_props_for_class(b,m,cls,cls->name,1)<0) return -1;
  if (inh && emit_inherited_props(b,m,cls->type)<0) return -1;
  return 0;
}

static int emit_args_list(outbuf *b, cg_model const *m, uint32_t pid) {
  int first=1;
  for (cg_node const *a=cg_first(m,pid,CG_KIND_ARG); a; a=cg_next(m,a,pid,CG_KIND_ARG)) {
    if (!first && ob_printf(b,", ")<0) return -1;
    if (emit_type(b,a)<0) return -1; first=0;
  }
  return first ? ob_printf(b,"\xe2\x80\x94") : 0;
}

static int emit_function_details(outbuf *b, cg_model const *m, cg_node const *fn) {
  cg_node const *ret=cg_first(m,fn->id,CG_KIND_RETURNS); int has=0;
  for (cg_node const *a=cg_first(m,fn->id,CG_KIND_ARG); a; a=cg_next(m,a,fn->id,CG_KIND_ARG)) { has=1; break; }
  if (has) {
    if (ob_printf(b,"\n### Parameters\n\n| Name | Type | Summary |\n|------|------|---------|\n")<0) return -1;
    for (cg_node const *a=cg_first(m,fn->id,CG_KIND_ARG); a; a=cg_next(m,a,fn->id,CG_KIND_ARG)) {
      if (ob_printf(b,"| `%s` | ",a->name)<0) return -1;
      if (emit_type(b,a)<0) return -1;
      if (ob_printf(b," | %s |\n",a->doc?a->doc:"")<0) return -1;
    }
  }
  if (!ret) return 0;
  if (ob_printf(b,"\n*Returns:* ")<0) return -1;
  return emit_type(b,ret)<0 ? -1 : ob_printf(b,"\n");
}

static int emit_method_doc(outbuf *b, cg_model const *m, cg_node const *mt, char const *prefix) {
  cg_node const *ret=cg_first(m,mt->id,CG_KIND_RETURNS);
  if (prefix&&prefix[0]) { if (ob_printf(b,"| `%s%s` | %s | ",prefix,mt->name,mt->doc?mt->doc:"")<0) return -1; }
  else           { if (ob_printf(b,"| `%s` | %s | ",mt->name,mt->doc?mt->doc:"")<0) return -1; }
  if (emit_args_list(b,m,mt->id)<0) return -1;
  if (ret) { if (ob_printf(b," | ")<0) return -1; if (emit_type(b,ret)<0) return -1; }
  else   { if (ob_printf(b," |")<0) return -1; }
  return ob_printf(b," |\n");
}

static int emit_interface_methods(outbuf *b, cg_model const *m, cg_node const *iface, char const *prefix) {
  int has=0;
  cg_foreach(m,iface->id,CG_KIND_METHOD,mt) if (!(mt->flags&CG_FLAG_PRIVATE)) { has=1; break; }
  if (!has) return 0;
  if (ob_printf(b,"\n### Methods\n\n| Method | Description | Args | Type |\n|--------|-------------|------|------|\n")<0) return -1;
  cg_foreach(m,iface->id,CG_KIND_METHOD,mt) {
    if (mt->flags&CG_FLAG_PRIVATE) continue;
    if (emit_method_doc(b,m,mt,prefix)<0) return -1;
  }
  return 0;
}

/* --- Lua helpers --- */
static char const *lua_method_name(cg_node const *mt, char *buf, size_t n) {
  char const *src=mt->name;
  if (mt->extra&&mt->extra[0]) return mt->extra;
  if (!src||!src[0]) return src;
  snprintf(buf,n,"%c%s",(src[0]>='A'&&src[0]<='Z')?(char)(src[0]+32):src[0],src+1);
  return buf;
}

static int emit_lua_args(outbuf *b, cg_model const *m, uint32_t pid) {
  int first=1;
  for (cg_node const *a=cg_first(m,pid,CG_KIND_ARG); a; a=cg_next(m,a,pid,CG_KIND_ARG)) {
    if (!first && ob_printf(b,", ")<0) return -1;
    if (ob_printf(b,"`%s`",a->name)<0) return -1;
    if (a->doc&&a->doc[0] && ob_printf(b," (%s)",a->doc)<0) return -1;
    first=0;
  }
  return first ? ob_printf(b,"\xe2\x80\x94") : 0;
}

static int emit_lua_methods_table(outbuf *b, cg_model const *m, cg_node const *node) {
  int has=0; char nb[256];
  cg_foreach(m,node->id,CG_KIND_METHOD,mt) if (!(mt->flags&CG_FLAG_PRIVATE)) { has=1; break; }
  if (!has) return 0;
  if (ob_printf(b,"\n## Methods\n\n| Method | Description | Parameters |\n|--------|-------------|------------|\n")<0) return -1;
  cg_foreach(m,node->id,CG_KIND_METHOD,mt) {
    if (mt->flags&CG_FLAG_PRIVATE) continue;
    if (ob_printf(b,"| `%s` | %s | ",lua_method_name(mt,nb,sizeof(nb)),mt->doc?mt->doc:"")<0) return -1;
    if (emit_lua_args(b,m,mt->id)<0) return -1;
    if (ob_printf(b," |\n")<0) return -1;
  }
  return 0;
}

/* --- Write helpers --- */
#define WCHECK(host,path,b) do { if ((host)->write_file(path,(b)->s,(b)->n)<0) { (host)->logf("docs: failed write %s",path); free((b)->s); return -1; } free((b)->s); return 0; } while(0)

static char const *module_prefix(cg_model const *m, char *buf, size_t n) {
  if (m->prefix&&m->prefix[0]) return m->prefix;
  snprintf(buf,n,"%s_",m->module_name?m->module_name:"");
  return buf;
}

static int build_path(char *path, size_t psz, char const *base, char const *sub, size_t sublen, char const *name) {
  size_t bl=strlen(base), nl=strlen(name);
  if (bl+1+sublen+nl+4>=psz) return -1;
  memcpy(path,base,bl);
  if (bl>0&&base[bl-1]!='/') path[bl++]='/';
  memcpy(path+bl,sub,sublen); bl+=sublen;
  memcpy(path+bl,name,nl); bl+=nl;
  memcpy(path+bl,".md",4);
  return 0;
}

/* --- Node doc emitters --- */

static int emit_class_docs(cg_host_v1 const *host, cg_model const *m, cg_node const *cls, char const *op) {
  outbuf b={0};
  if (ob_printf(&b,"# %s\n\n**Definition:** `%s`\n",cls->name,m->xml_path)<0) goto err;
  if (cls->aux2&&cls->aux2[0] && ob_printf(&b,"**Namespace:** `%s`\n",cls->aux2)<0) goto err;
  if (cls->doc&&cls->doc[0]   && ob_printf(&b,"\n## Summary\n\n%s\n",cls->doc)<0) goto err;
  if (emit_inheritance(&b,m,cls)<0) goto err;
  if (emit_props_block(&b,m,cls,"## Properties")<0) goto err;
  if (emit_handles(&b,m,cls)<0) goto err;
  if (emit_see_also(&b,m,cls,CG_KIND_PROPERTY,1)<0) goto err;
  if (cls->aux&&cls->aux[0] && ob_printf(&b,"\n## Implementation Details\n\n%s\n",cls->aux)<0) goto err;
  WCHECK(host,op,&b);
err: free(b.s); return -1;
}

static int emit_interface_docs(cg_host_v1 const *host, cg_model const *m, cg_node const *iface, char const *op) {
  outbuf b={0};
  if (ob_printf(&b,"# %s\n\n**Definition:** `%s`\n",iface->name,m->xml_path)<0) goto err;
  if (iface->aux2&&iface->aux2[0] && ob_printf(&b,"**Namespace:** `%s`\n",iface->aux2)<0) goto err;
  if (iface->doc&&iface->doc[0]   && ob_printf(&b,"\n## Summary\n\n%s\n",iface->doc)<0) goto err;
  if (emit_interface_methods(&b,m,iface,NULL)<0) goto err;
  WCHECK(host,op,&b);
err: free(b.s); return -1;
}

static int emit_enum_docs(cg_host_v1 const *host, cg_model const *m, cg_node const *en, char const *op) {
  outbuf b={0}; int has=0;
  if (ob_printf(&b,"# %s\n\n**Definition:** `%s`\n",en->name,m->xml_path)<0) goto err;
  if (en->aux2&&en->aux2[0] && ob_printf(&b,"**Namespace:** `%s`\n",en->aux2)<0) goto err;
  if (en->doc&&en->doc[0]   && ob_printf(&b,"\n## Summary\n\n%s\n",en->doc)<0) goto err;
  cg_foreach(m,en->id,CG_KIND_VALUE,v) { (void)v; has=1; break; }
  if (has) {
    if (ob_printf(&b,"\n### Members\n\n")<0) goto err;
    cg_foreach(m,en->id,CG_KIND_VALUE,v)
      if (ob_printf(&b,"| `%s` | %s |\n",v->name,v->doc?v->doc:"")<0) goto err;
  }
  WCHECK(host,op,&b);
err: free(b.s); return -1;
}

static int emit_struct_docs(cg_host_v1 const *host, cg_model const *m, cg_node const *st, char const *op) {
  outbuf b={0}; int has=0;
  if (ob_printf(&b,"# %s\n\n**Definition:** `%s`\n",st->name,m->xml_path)<0) goto err;
  if (st->aux2&&st->aux2[0] && ob_printf(&b,"**Namespace:** `%s`\n",st->aux2)<0) goto err;
  if (st->doc&&st->doc[0]   && ob_printf(&b,"\n## Summary\n\n%s\n",st->doc)<0) goto err;
  cg_foreach(m,st->id,CG_KIND_FIELD,f) { (void)f; has=1; break; }
  if (has) {
    if (ob_printf(&b,"\n### Fields\n\n")<0) goto err;
    cg_foreach(m,st->id,CG_KIND_FIELD,f) {
      if (ob_printf(&b,"| Field | Type | Description |\n|-------|------|-------------|\n| `%s` | ",f->name)<0) goto err;
      if (emit_type(&b,f)<0) goto err;
      if (ob_printf(&b," | %s |\n",f->doc?f->doc:"")<0) goto err;
    }
  }
  if (emit_interface_methods(&b,m,st,st->type)<0) goto err;
  if (emit_see_also(&b,m,st,CG_KIND_FIELD,0)<0) goto err;
  WCHECK(host,op,&b);
err: free(b.s); return -1;
}

static int emit_lua_class_docs(cg_host_v1 const *host, cg_model const *m, cg_node const *node, char const *op) {
  outbuf b={0};
  if (ob_printf(&b,"# %s\n\n**Definition:** `%s`\n",node->name,m->xml_path)<0) goto err;
  if (node->doc&&node->doc[0] && ob_printf(&b,"\n## Summary\n\n%s\n",node->doc)<0) goto err;
  if (emit_inheritance(&b,m,node)<0) goto err;
  if (emit_props_block(&b,m,node,"## Properties")<0) goto err;
  if (emit_lua_methods_table(&b,m,node)<0) goto err;
  WCHECK(host,op,&b);
err: free(b.s); return -1;
}

static int emit_xml_class_docs(cg_host_v1 const *host, cg_model const *m, cg_node const *cls, char const *op) {
  outbuf b={0};
  char const *xn=(cls->aux2&&cls->aux2[0])?cls->aux2:cls->name;
  if (ob_printf(&b,"# %s\n\n**XML element:** `<%s>`\n**Definition:** `%s`\n",cls->name,xn,m->xml_path)<0) goto err;
  if (cls->doc&&cls->doc[0] && ob_printf(&b,"\n## Summary\n\n%s\n",cls->doc)<0) goto err;
  if (emit_inheritance(&b,m,cls)<0) goto err;
  {
    int own=has_any_props(m,cls), inh=parents_have_props(m,cls->type);
    if (own||inh) {
      if (ob_printf(&b,"\n## Attributes\n\n> These properties are set as XML attributes on `<%s>`.\n",xn)<0) goto err;
      if (own && emit_props_for_class(&b,m,cls,cls->name,1)<0) goto err;
      if (inh && emit_inherited_props(&b,m,cls->type)<0) goto err;
    }
  }
  {
    int has=0;
    cg_foreach(m,cls->id,CG_KIND_MESSAGE,msg) { (void)msg; has=1; break; }
    if (has) {
      if (ob_printf(&b,"\n## Messages\n\n| Message | Routing | Description |\n|---------|---------|-------------|\n")<0) goto err;
      cg_foreach(m,cls->id,CG_KIND_MESSAGE,msg) {
        char const *r=(msg->type&&msg->type[0])?msg->type:"Direct";
        if (ob_printf(&b,"| `%s` | %s | %s |\n",msg->name,r,msg->doc?msg->doc:"")<0) goto err;
      }
    }
  }
  if (emit_handles(&b,m,cls)<0) goto err;
  WCHECK(host,op,&b);
err: free(b.s); return -1;
}

static int emit_c_function_docs(cg_host_v1 const *host, cg_model const *m, cg_node const *fn, char const *op) {
  outbuf b={0}; char pb[128]; char const *pfx=module_prefix(m,pb,sizeof(pb));
  if (ob_printf(&b,"# %s%s\n\n**C name:** `%s%s`\n**Definition:** `%s`\n",pfx,fn->name,pfx,fn->name,m->xml_path)<0) goto err;
  if (fn->doc&&fn->doc[0] && ob_printf(&b,"\n## Summary\n\n%s\n",fn->doc)<0) goto err;
  if (emit_function_details(&b,m,fn)<0) goto err;
  WCHECK(host,op,&b);
err: free(b.s); return -1;
}

/* --- Overview --- */
static int emit_overview(outbuf *b, cg_model const *model, api_mode mode,
             uint32_t ne, uint32_t ni, uint32_t ns, uint32_t nc) {
  char const *label=mode==API_LUA?"Lua API":mode==API_XML?"XML API":"C API";
  if (ob_printf(b,"# %s — %s\n\n- **source:** `%s`\n- **enums:** %u\n- **interfaces:** %u\n- **structs:** %u\n- **classes:** %u\n\n",
      model->module_name,label,model->xml_path,ne,ni,ns,nc)<0) return -1;

  if (mode==API_LUA) {
    if (ni>0) {
      if (ob_printf(b,"\n## Interfaces\n\n| Name | Summary |\n|------|---------|\n")<0) return -1;
      for (size_t i=0; i<model->node_count; ++i) { cg_node const *c=&model->nodes[i];
        if (c->kind!=CG_KIND_INTERFACE) continue;
        if (ob_printf(b,"| [%s](interfaces/%s.md) | %s |\n",c->name,c->name,c->doc?c->doc:"")<0) return -1; }
    }
    if (nc>0) {
      if (ob_printf(b,"\n## Classes\n\n| Name | Summary |\n|------|---------|\n")<0) return -1;
      for (size_t i=0; i<model->node_count; ++i) { cg_node const *c=&model->nodes[i];
        if (c->kind!=CG_KIND_CLASS) continue;
        if (ob_printf(b,"| [%s](classes/%s.md) | %s |\n",c->name,c->name,c->doc?c->doc:"")<0) return -1; }
    }
    return 0;
  }
  if (mode==API_XML) {
    if (ob_printf(b,"\n## Elements\n\n| Element | XML tag | Summary |\n|---------|---------|--------|\n")<0) return -1;
    for (size_t i=0; i<model->node_count; ++i) { cg_node const *c=&model->nodes[i];
      if (c->kind!=CG_KIND_CLASS) continue;
      char const *xn=(c->aux2&&c->aux2[0])?c->aux2:c->name;
      if (ob_printf(b,"| [%s](classes/%s.md) | `<%s>` | %s |\n",c->name,c->name,xn,c->doc?c->doc:"")<0) return -1; }
    return 0;
  }
  /* C mode */
  if (ne>0) {
    if (ob_printf(b,"\n## Enums\n\n| Name | Summary |\n|------|---------|\n")<0) return -1;
    for (size_t i=0; i<model->node_count; ++i) if (model->nodes[i].kind==CG_KIND_ENUM)
      if (ob_printf(b,"| [%s](enums/%s.md) | %s |\n",model->nodes[i].name,model->nodes[i].name,model->nodes[i].doc?model->nodes[i].doc:"")<0) return -1;
  }
  if (ns>0) {
    if (ob_printf(b,"\n## Structs\n\n| Name | Summary |\n|------|---------|\n")<0) return -1;
    for (size_t i=0; i<model->node_count; ++i) if (model->nodes[i].kind==CG_KIND_STRUCT)
      if (ob_printf(b,"| [%s](structs/%s.md) | %s |\n",model->nodes[i].name,model->nodes[i].name,model->nodes[i].doc?model->nodes[i].doc:"")<0) return -1;
  }
  if (ni>0) {
    if (ob_printf(b,"\n## Interfaces\n\n| Name | Summary |\n|------|---------|\n")<0) return -1;
    for (size_t i=0; i<model->node_count; ++i) if (model->nodes[i].kind==CG_KIND_INTERFACE)
      if (ob_printf(b,"| [%s](interfaces/%s.md) | %s |\n",model->nodes[i].name,model->nodes[i].name,model->nodes[i].doc?model->nodes[i].doc:"")<0) return -1;
  }
  {
    char pb[128]; char const *pfx=module_prefix(model,pb,sizeof(pb)); uint32_t nf=0;
    for (size_t i=0; i<model->node_count; ++i)
      if (model->nodes[i].kind==CG_KIND_METHOD&&model->nodes[i].parent==0&&!(model->nodes[i].flags&CG_FLAG_PRIVATE)) nf++;
    if (nf>0) {
      if (ob_printf(b,"\n## Functions\n\n| Name | Summary |\n|------|---------|\n")<0) return -1;
      for (size_t i=0; i<model->node_count; ++i) { cg_node const *fn=&model->nodes[i];
        if (fn->kind!=CG_KIND_METHOD||fn->parent!=0||(fn->flags&CG_FLAG_PRIVATE)) continue;
        if (ob_printf(b,"| [%s%s](functions/%s%s.md) | %s |\n",pfx,fn->name,pfx,fn->name,fn->doc?fn->doc:"")<0) return -1; }
    }
  }
  if (ob_printf(b,"\n## Classes\n\n| Name | Summary |\n|------|---------|\n")<0) return -1;
  for (size_t i=0; i<model->node_count; ++i) if (model->nodes[i].kind==CG_KIND_CLASS)
    if (ob_printf(b,"| [%s](classes/%s.md) | %s |\n",model->nodes[i].name,model->nodes[i].name,model->nodes[i].doc?model->nodes[i].doc:"")<0) return -1;
  return 0;
}

/* --- Stdout inline emitters (shared logic) --- */
static int emit_class_inline(outbuf *b, cg_model const *m, cg_node const *cls) {
  if (ob_printf(b,"\n---\n\n# %s\n\n**Definition:** `%s`\n",cls->name,m->xml_path)<0) return -1;
  if (cls->aux2&&cls->aux2[0] && ob_printf(b,"**Namespace:** `%s`\n",cls->aux2)<0) return -1;
  if (cls->doc&&cls->doc[0]   && ob_printf(b,"\n## Summary\n\n%s\n",cls->doc)<0) return -1;
  if (emit_inheritance(b,m,cls)<0) return -1;
  if (emit_props_block(b,m,cls,"## Properties")<0) return -1;
  if (emit_handles(b,m,cls)<0) return -1;
  if (emit_see_also(b,m,cls,CG_KIND_PROPERTY,1)<0) return -1;
  if (cls->aux&&cls->aux[0] && ob_printf(b,"\n## Implementation Details\n\n%s\n",cls->aux)<0) return -1;
  return 0;
}

/* --- Main entry --- */
static int emit_docs(cg_host_v1 const *host, cg_model const *model, char const *output) {
  outbuf b={0}; size_t i;
  uint32_t ne=0,ni=0,ns=0,nc=0;
  api_mode mode=parse_mode(&output);
  int is_stdout=output&&!strcmp(output,"-");
  char path[512];

  for (i=0; i<model->node_count; ++i) {
    ne+=model->nodes[i].kind==CG_KIND_ENUM;
    ni+=model->nodes[i].kind==CG_KIND_INTERFACE;
    ns+=model->nodes[i].kind==CG_KIND_STRUCT;
    nc+=model->nodes[i].kind==CG_KIND_CLASS;
  }
  if (emit_overview(&b,model,mode,ne,ni,ns,nc)<0) { free(b.s); return -1; }

  if (is_stdout) {
    if (mode==API_LUA) {
      cg_foreach(model,0,CG_KIND_INTERFACE,iface) {
        if (ob_printf(&b,"\n---\n\n")<0) { free(b.s); return -1; }
        /* reuse lua inline: header+props+methods */
        if (ob_printf(&b,"# %s\n\n**Definition:** `%s`\n",iface->name,model->xml_path)<0) { free(b.s); return -1; }
        if (iface->doc&&iface->doc[0] && ob_printf(&b,"\n## Summary\n\n%s\n",iface->doc)<0) { free(b.s); return -1; }
        if (emit_inheritance(&b,model,iface)<0) { free(b.s); return -1; }
        if (emit_props_block(&b,model,iface,"## Properties")<0) { free(b.s); return -1; }
        if (emit_lua_methods_table(&b,model,iface)<0) { free(b.s); return -1; }
      }
      cg_foreach(model,0,CG_KIND_CLASS,cls) {
        if (ob_printf(&b,"\n---\n\n")<0) { free(b.s); return -1; }
        if (ob_printf(&b,"# %s\n\n**Definition:** `%s`\n",cls->name,model->xml_path)<0) { free(b.s); return -1; }
        if (cls->doc&&cls->doc[0] && ob_printf(&b,"\n## Summary\n\n%s\n",cls->doc)<0) { free(b.s); return -1; }
        if (emit_inheritance(&b,model,cls)<0) { free(b.s); return -1; }
        if (emit_props_block(&b,model,cls,"## Properties")<0) { free(b.s); return -1; }
        if (emit_lua_methods_table(&b,model,cls)<0) { free(b.s); return -1; }
      }
    } else if (mode==API_XML) {
      cg_foreach(model,0,CG_KIND_CLASS,cls) {
        char const *xn=(cls->aux2&&cls->aux2[0])?cls->aux2:cls->name;
        if (ob_printf(&b,"\n---\n\n# %s\n\n**XML element:** `<%s>`\n**Definition:** `%s`\n",cls->name,xn,model->xml_path)<0) { free(b.s); return -1; }
        if (cls->doc&&cls->doc[0] && ob_printf(&b,"\n## Summary\n\n%s\n",cls->doc)<0) { free(b.s); return -1; }
        if (emit_inheritance(&b,model,cls)<0) { free(b.s); return -1; }
        {
          int own=has_any_props(model,cls), inh=parents_have_props(model,cls->type);
          if (own||inh) {
            if (ob_printf(&b,"\n## Attributes\n\n> These properties are set as XML attributes on `<%s>`.\n",xn)<0) { free(b.s); return -1; }
            if (own && emit_props_for_class(&b,model,cls,cls->name,1)<0) { free(b.s); return -1; }
            if (inh && emit_inherited_props(&b,model,cls->type)<0) { free(b.s); return -1; }
          }
        }
        {
          int has=0;
          cg_foreach(model,cls->id,CG_KIND_MESSAGE,msg) { (void)msg; has=1; break; }
          if (has) {
            if (ob_printf(&b,"\n## Messages\n\n| Message | Routing | Description |\n|---------|---------|-------------|\n")<0) { free(b.s); return -1; }
            cg_foreach(model,cls->id,CG_KIND_MESSAGE,msg) {
              char const *r=(msg->type&&msg->type[0])?msg->type:"Direct";
              if (ob_printf(&b,"| `%s` | %s | %s |\n",msg->name,r,msg->doc?msg->doc:"")<0) { free(b.s); return -1; }
            }
          }
        }
        if (emit_handles(&b,model,cls)<0) { free(b.s); return -1; }
      }
    } else {
      cg_foreach(model,0,CG_KIND_CLASS,cls)   if (emit_class_inline(&b,model,cls)<0) { free(b.s); return -1; }
      cg_foreach(model,0,CG_KIND_ENUM,en) {
        if (ob_printf(&b,"\n---\n\n# %s\n\n**Definition:** `%s`\n",en->name,model->xml_path)<0) { free(b.s); return -1; }
        if (en->aux2&&en->aux2[0] && ob_printf(&b,"**Namespace:** `%s`\n",en->aux2)<0) { free(b.s); return -1; }
        if (en->doc&&en->doc[0]   && ob_printf(&b,"\n## Summary\n\n%s\n",en->doc)<0) { free(b.s); return -1; }
        { int has=0; cg_foreach(model,en->id,CG_KIND_VALUE,v){(void)v;has=1;break;}
          if (has) { if (ob_printf(&b,"\n### Members\n\n")<0) { free(b.s); return -1; }
          cg_foreach(model,en->id,CG_KIND_VALUE,v) if (ob_printf(&b,"| `%s` | %s |\n",v->name,v->doc?v->doc:"")<0) { free(b.s); return -1; } } }
      }
      cg_foreach(model,0,CG_KIND_STRUCT,st) {
        if (ob_printf(&b,"\n---\n\n# %s\n\n**Definition:** `%s`\n",st->name,model->xml_path)<0) { free(b.s); return -1; }
        if (st->aux2&&st->aux2[0] && ob_printf(&b,"**Namespace:** `%s`\n",st->aux2)<0) { free(b.s); return -1; }
        if (st->doc&&st->doc[0]   && ob_printf(&b,"\n## Summary\n\n%s\n",st->doc)<0) { free(b.s); return -1; }
        { int has=0; cg_foreach(model,st->id,CG_KIND_FIELD,f){(void)f;has=1;break;}
          if (has) { if (ob_printf(&b,"\n### Fields\n\n")<0) { free(b.s); return -1; }
          cg_foreach(model,st->id,CG_KIND_FIELD,f) {
            if (ob_printf(&b,"| Field | Type | Description |\n|-------|------|-------------|\n| `%s` | ",f->name)<0) { free(b.s); return -1; }
            if (emit_type(&b,f)<0) { free(b.s); return -1; }
            if (ob_printf(&b," | %s |\n",f->doc?f->doc:"")<0) { free(b.s); return -1; } } } }
        if (emit_interface_methods(&b,model,st,st->type)<0) { free(b.s); return -1; }
      }
      cg_foreach(model,0,CG_KIND_INTERFACE,iface) {
        if (ob_printf(&b,"\n---\n\n# %s\n\n**Definition:** `%s`\n",iface->name,model->xml_path)<0) { free(b.s); return -1; }
        if (iface->aux2&&iface->aux2[0] && ob_printf(&b,"**Namespace:** `%s`\n",iface->aux2)<0) { free(b.s); return -1; }
        if (iface->doc&&iface->doc[0]   && ob_printf(&b,"\n## Summary\n\n%s\n",iface->doc)<0) { free(b.s); return -1; }
        if (emit_interface_methods(&b,model,iface,NULL)<0) { free(b.s); return -1; }
      }
      for (i=0; i<model->node_count; ++i) {
        cg_node const *fn=&model->nodes[i]; char pb[128];
        char const *pfx=module_prefix(model,pb,sizeof(pb));
        if (fn->kind!=CG_KIND_METHOD||fn->parent!=0||(fn->flags&CG_FLAG_PRIVATE)) continue;
        if (ob_printf(&b,"\n---\n\n# %s%s\n\n**C name:** `%s%s`\n**Definition:** `%s`\n",pfx,fn->name,pfx,fn->name,model->xml_path)<0) { free(b.s); return -1; }
        if (fn->doc&&fn->doc[0] && ob_printf(&b,"\n## Summary\n\n%s\n",fn->doc)<0) { free(b.s); return -1; }
        if (emit_function_details(&b,model,fn)<0) { free(b.s); return -1; }
      }
    }
    if (host->write_file(output,b.s,b.n)<0) { host->logf("docs: failed write %s",output); free(b.s); return -1; }
    free(b.s); return 0;
  }

  /* Directory mode: write overview.md */
  {
    size_t bl=strlen(output);
    if (bl+12>=sizeof(path)) { host->logf("docs: path too long for overview"); free(b.s); return -1; }
    memcpy(path,output,bl);
    if (bl>0&&output[bl-1]!='/') path[bl++]='/';
    strcat(path+bl,"overview.md");
    if (host->write_file(path,b.s,b.n)<0) { host->logf("docs: failed write %s",path); free(b.s); return -1; }
  }
  free(b.s);

  if (mode==API_LUA) {
    cg_foreach(model,0,CG_KIND_INTERFACE,iface) {
      if (build_path(path,sizeof(path),output,"interfaces/",11,iface->name)<0) { host->logf("docs: path too long for %s",iface->name); continue; }
      if (emit_lua_class_docs(host,model,iface,path)<0) return -1;
    }
    cg_foreach(model,0,CG_KIND_CLASS,cls) {
      if (build_path(path,sizeof(path),output,"classes/",8,cls->name)<0) { host->logf("docs: path too long for %s",cls->name); continue; }
      if (emit_lua_class_docs(host,model,cls,path)<0) return -1;
    }
    return 0;
  }
  if (mode==API_XML) {
    cg_foreach(model,0,CG_KIND_CLASS,cls) {
      if (build_path(path,sizeof(path),output,"classes/",8,cls->name)<0) { host->logf("docs: path too long for %s",cls->name); continue; }
      if (emit_xml_class_docs(host,model,cls,path)<0) return -1;
    }
    return 0;
  }
  cg_foreach(model,0,CG_KIND_ENUM,en) {
    if (build_path(path,sizeof(path),output,"enums/",6,en->name)<0) { host->logf("docs: path too long for %s",en->name); continue; }
    if (emit_enum_docs(host,model,en,path)<0) return -1;
  }
  cg_foreach(model,0,CG_KIND_STRUCT,st) {
    if (build_path(path,sizeof(path),output,"structs/",8,st->name)<0) { host->logf("docs: path too long for %s",st->name); continue; }
    if (emit_struct_docs(host,model,st,path)<0) return -1;
  }
  cg_foreach(model,0,CG_KIND_INTERFACE,iface) {
    if (build_path(path,sizeof(path),output,"interfaces/",11,iface->name)<0) { host->logf("docs: path too long for %s",iface->name); continue; }
    if (emit_interface_docs(host,model,iface,path)<0) return -1;
  }
  cg_foreach(model,0,CG_KIND_CLASS,cls) {
    if (build_path(path,sizeof(path),output,"classes/",8,cls->name)<0) { host->logf("docs: path too long for %s",cls->name); continue; }
    if (emit_class_docs(host,model,cls,path)<0) return -1;
  }
  {
    char pb[128]; char const *pfx=module_prefix(model,pb,sizeof(pb));
    for (i=0; i<model->node_count; ++i) {
      char fn_name[256]; cg_node const *fn=&model->nodes[i];
      if (fn->kind!=CG_KIND_METHOD||fn->parent!=0||(fn->flags&CG_FLAG_PRIVATE)) continue;
      snprintf(fn_name,sizeof(fn_name),"%s%s",pfx,fn->name);
      if (build_path(path,sizeof(path),output,"functions/",10,fn_name)<0) { host->logf("docs: path too long for %s",fn_name); continue; }
      if (emit_c_function_docs(host,model,fn,path)<0) return -1;
    }
  }
  return 0;
}

static cg_plugin_v1 const plugin={ .abi=CG_API_VERSION, .name="docs", .emit=emit_docs };
cg_plugin_v1 const *cg_plugin_init_v1(void) { return &plugin; }