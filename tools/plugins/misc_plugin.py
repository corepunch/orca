from codegen_c import w, hash, lpc, lp, write_struct_fwd_def, write_push_check

def include_parse(_, include, parser, ctx):
	w(parser.header, f"#include <{include.get('file')}>")

def external_parse(_, external, parser, ctx):
	sname = external.get('struct')
	write_struct_fwd_def(sname, sname, parser.header)

def resource_parse(_, resource, parser, ctx):
	name = resource.get('type')
	write_struct_fwd_def(name, name, parser.header)
	if resource.get('no-lua'):
		return
	write_push_check(name, parser.header)
	w(parser.export, f"void luaX_push{name}(lua_State *L, {lpc(name)} {name}) {{")
	w(parser.export, f"\tlua_pushlightuserdata(L, ({lp(name)}){name});\n}}")	
	w(parser.export, f"{lp(name)} luaX_check{name}(lua_State *L, int idx) {{")
	w(parser.export, f"\treturn lua_touserdata(L, idx);\n}}")	

def event_parse(_, event, parser, ctx):
	ename, etype = event.get('name'), event.get('type')
	w(parser.header, f"#define kEvent{ename} " + hash(ename))	
	if etype == "void":
		w(parser.header, f"typedef void* {ename}EventPtr;\n")
	elif etype:
		w(parser.header, f"typedef struct {etype}* {ename}EventPtr;\n")

def register(parsers):
	parsers['include'] = include_parse
	parsers['external'] = external_parse
	parsers['resource'] = resource_parse
	parsers['event'] = event_parse
