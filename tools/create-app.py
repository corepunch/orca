#!/usr/bin/env python3
import argparse
from pathlib import Path
import xml.etree.ElementTree as ET

LANGUAGES = ["moonscript", "lua", "xml"]

# Dict of optional libraries (only relevant for moonscript/lua projects)
LIBS = {
	"routing": "https://github.com/corepunch/orca-routing",
	"tailwind": "https://github.com/corepunch/orca-tailwind",
	"html": "https://github.com/corepunch/orca-html",
	"appwrite": "https://github.com/corepunch/orca-appwrite",
	"openai": "https://github.com/corepunch/orca-openai",
}

def ask(q, default=False):
	s = " [Y/n]: " if default else " [y/N]: "
	a = input(q + s).strip().lower()
	return default if not a else a in ("y", "yes")

def ask_choice(q, choices, default=None):
	display = "/".join(
		c.upper() if c == default else c for c in choices
	)
	while True:
		a = input(f"{q} [{display}]: ").strip().lower()
		if not a and default:
			return default
		if a in choices:
			return a
		print(f"  Please enter one of: {', '.join(choices)}")

# ---------------------------------------------------------------------------
# package.lua generators
#
# The runtime loads only <project>/package.lua. It is a declarative Lua chunk
# whose global assignments set properties of the Project object (see the
# Project class in source/filesystem/filesystem.cgen).
# ---------------------------------------------------------------------------

SYSTEM_MESSAGES = [
	{"Message": "KeyDown", "Key": "q", "Command": "return"},
	{"Message": "WindowClosed", "Command": "return"},
	{"Message": "RequestReload", "Command": "window:refresh()"},
]

def _lua_value(v):
	if isinstance(v, bool):
		return "true" if v else "false"
	if isinstance(v, int):
		return str(v)
	s = str(v).replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n")
	return f'"{s}"'

def _lua_table(fields):
	return "{ " + ", ".join(f"{k} = {_lua_value(v)}" for k, v in fields.items()) + " }"

def _lua_array(name, items):
	lines = [f"{name} = {{"]
	lines += [f"\t{_lua_table(item)}," for item in items]
	lines.append("}")
	return lines

def package_lua_script(name, flags, width, height):
	"""package.lua for moonscript/lua projects (ProjectReferences style)."""
	refs = {
		"applications": "applications",
		"assets": "assets",
		"routing": "lib/routing",
		"model": "model",
		"config": "config",
	}
	for k in flags:
		if flags[k]:
			refs[k] = f"lib/{k}"
	lines = [
		f"Name = {_lua_value(name)}",
		f"StartupScreen = {_lua_value(f'{name}/App')}",
		f"WindowWidth = {_lua_value(width)}",
		f"WindowHeight = {_lua_value(height)}",
	]
	lines += _lua_array("ProjectReferences", [{"Name": k, "Path": v} for k, v in refs.items()])
	lines.append(f"FontLibrary = {_lua_table({'Name': 'fonts'})}")
	lines += _lua_array("SystemMessages", SYSTEM_MESSAGES)
	return "\n".join(lines) + "\n"

def package_lua_xml(name, width, height):
	"""package.lua for xml projects (ScreenLibrary style, like Example/Adventure)."""
	lines = [
		f"Name = {_lua_value(name)}",
		f"StartupScreen = {_lua_value(f'{name}/Screens/App')}",
		f"WindowWidth = {_lua_value(width)}",
		f"WindowHeight = {_lua_value(height)}",
		f"ScreenLibrary = {_lua_table({'IsExternal': True})}",
		f"FontLibrary = {_lua_table({'Name': 'fonts'})}",
	]
	lines += _lua_array("SystemMessages", SYSTEM_MESSAGES)
	lines += _lua_array("EnginePlugins", [{"Name": "orca.UIKit"}])
	return "\n".join(lines) + "\n"

# ---------------------------------------------------------------------------
# File generators: moonscript
# ---------------------------------------------------------------------------

def create_moonscript(root, name, flags):
	for d in ["layouts", "pages", "components", "assets", "lib", "config", "model", "fonts"]:
		(root / d).mkdir(parents=True, exist_ok=True)

	if flags["routing"]:
		with open(root / "App.moon", "w") as f:
			for k in LIBS:
				if flags[k]:
					f.write(f'require "{k}" -- initialize {k}\n')
			f.write(f'\nimport Home, Help from require "{name}.pages"')
			f.write('\nimport Application from require "routing"\n')
			f.write('\nclass App extends Application\n')
			if flags["tailwind"]:
				f.write('\t@stylesheet require "tailwind"\n')
			f.write('\t@stylesheet "assets/globals.css"\n\n')
			f.write('\t"/": => Home!\n')
			f.write('\t"/help": => Help\n')
	else:
		(root / "App.moon").write_text(
			'ui = require "orca.ui"\n'
			"class App extends ui.Screen\n"
			'\t@stylesheet "assets/globals.css"\n'
			"\tbody: =>\n\t\tui.TextBlock \"Hello World!\"\n")

	(root / "pages/Home.moon").write_text(
		'ui = require "orca.ui"\n\n'
		"class Home extends ui.Node2D\n"
		"\tbody: =>\n\t\tui.TextBlock \"Hello World!\"\n")

	(root / "pages/Help.moon").write_text(
		'ui = require "orca.ui"\n\n'
		"class Help extends ui.Node2D\n"
		"\tbody: =>\n\t\tui.TextBlock \"Help\"\n")

	(root / "pages/init.moon").write_text(
		"return {\n"
		f'\tHome: require "{name}.pages.Home"\n'
		f'\tHelp: require "{name}.pages.Help"\n'
		"}\n")

	(root / "assets/globals.css").write_text(
		"body {\n\tbackground-color: black;\n\tcolor: white;\n}\n")

# ---------------------------------------------------------------------------
# File generators: lua
# ---------------------------------------------------------------------------

def create_lua(root, name, flags):
	for d in ["layouts", "pages", "components", "assets", "lib", "config", "model", "fonts"]:
		(root / d).mkdir(parents=True, exist_ok=True)

	if flags["routing"]:
		with open(root / "App.lua", "w") as f:
			for k in LIBS:
				if flags[k]:
					f.write(f'require "{k}" -- initialize {k}\n')
			f.write(f'\nlocal pages = require "{name}.pages"\n')
			f.write('local Application = require("routing").Application\n')
			if flags["tailwind"]:
				f.write('local tailwind = require "tailwind"\n')
			f.write('\nlocal App = Application:extend {\n')
			if flags["tailwind"]:
				f.write('\tstylesheet = tailwind,\n')
			f.write('\t["/"] = function(self) return pages.Home() end,\n')
			f.write('\t["/help"] = function(self) return pages.Help() end,\n')
			f.write('}\n\nreturn App\n')
	else:
		(root / "App.lua").write_text(
			'local ui = require "orca.ui"\n\n'
			"local App = ui.Screen:extend {\n"
			'\tstylesheet = "assets/globals.css",\n'
			"\tbody = function(self)\n"
			'\t\tui.TextBlock "Hello World!"\n'
			"\tend,\n"
			"}\n\nreturn App\n")

	(root / "pages/Home.lua").write_text(
		'local ui = require "orca.ui"\n\n'
		"local Home = ui.Node2D:extend {\n"
		"\tbody = function(self)\n"
		'\t\tui.TextBlock "Hello World!"\n'
		"\tend,\n"
		"}\n\nreturn Home\n")

	(root / "pages/Help.lua").write_text(
		'local ui = require "orca.ui"\n\n'
		"local Help = ui.Node2D:extend {\n"
		"\tbody = function(self)\n"
		'\t\tui.TextBlock "Help"\n'
		"\tend,\n"
		"}\n\nreturn Help\n")

	(root / "pages/init.lua").write_text(
		"return {\n"
		f'\tHome = require "{name}.pages.Home",\n'
		f'\tHelp = require "{name}.pages.Help",\n'
		"}\n")

	(root / "assets/globals.css").write_text(
		"body {\n\tbackground-color: black;\n\tcolor: white;\n}\n")

# ---------------------------------------------------------------------------
# File generators: xml
# ---------------------------------------------------------------------------

def create_xml(root, name, width, height):
	for d in ["Screens", "fonts"]:
		(root / d).mkdir(parents=True, exist_ok=True)

	screen = ET.Element("Screen", {
		"Name": "App",
		"Width": str(width),
		"Height": str(height),
		"ClearColor": "#0B0B12",
	})
	stack = ET.SubElement(screen, "StackView", {
		"Name": "Root",
		"Direction": "Vertical",
		"Spacing": "16",
		"Padding": "40",
		"BackgroundColor": "#0B0B12",
		"AlignItems": "Center",
		"JustifyContent": "Center",
	})
	ET.SubElement(stack, "TextBlock", {
		"Name": "Title",
		"Text": name,
		"FontSize": "32",
		"ForegroundColor": "#8B5CF6",
	})
	ET.SubElement(stack, "TextBlock", {
		"Name": "Subtitle",
		"Text": "Hello World!",
		"FontSize": "16",
		"ForegroundColor": "#94A3B8",
	})
	ET.indent(screen, space="\t")
	tree = ET.ElementTree(screen)

	app_xml = root / "Screens" / "App.xml"
	with open(app_xml, "w", encoding="utf-8") as fh:
		fh.write('<?xml version="1.0" encoding="UTF-8"?>\n')
		fh.write('<!DOCTYPE Screen SYSTEM "https://corepunch.github.io/orca/schemas/orca.dtd">\n')
		tree.write(fh, encoding="unicode", xml_declaration=False)

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
	p = argparse.ArgumentParser(
		prog="orca",
		description="Create a new ORCA project.",
	)
	p.add_argument("name", nargs="?", help="Project name")
	p.add_argument(
		"--language", "--lang",
		choices=LANGUAGES,
		dest="language",
		help="Scripting language / project type (moonscript, lua, xml)",
	)
	p.add_argument("--width", type=int, default=None, help="Window width (default 400)")
	p.add_argument("--height", type=int, default=None, help="Window height (default 800)")
	for k in LIBS:
		p.add_argument(f"--{k}", action="store_true", help=f"Include {k} library")
	args = p.parse_args()

	name = args.name or input("Project name: ").strip()
	if not name:
		p.error("Project name cannot be empty.")

	language = args.language or ask_choice(
		"Language/type", LANGUAGES, default="moonscript"
	)

	is_xml = language == "xml"

	# Window dimensions
	default_width, default_height = 400, 800
	width = args.width if args.width is not None else (
		int(input(f"Window width [{default_width}]: ").strip() or default_width)
		if args.language is None else default_width
	)
	height = args.height if args.height is not None else (
		int(input(f"Window height [{default_height}]: ").strip() or default_height)
		if args.language is None else default_height
	)

	# Library flags (only for script-based projects)
	if is_xml:
		flags = {k: False for k in LIBS}
	else:
		flags = {k: getattr(args, k) or ask(f"Include {k}?", k == "tailwind") for k in LIBS}

	root_dir = Path(name)
	root_dir.mkdir(exist_ok=False)

	if language == "moonscript":
		create_moonscript(root_dir, name, flags)
		package = package_lua_script(name, flags, width, height)
	elif language == "lua":
		create_lua(root_dir, name, flags)
		package = package_lua_script(name, flags, width, height)
	elif language == "xml":
		create_xml(root_dir, name, width, height)
		package = package_lua_xml(name, width, height)
	(root_dir / "package.lua").write_text(package, encoding="utf-8")

	active_libs = [k for k, v in flags.items() if v]
	summary = f"language={language}"
	if active_libs:
		summary += f", libs={', '.join(active_libs)}"
	print(f"Project '{name}' created ({summary})")

if __name__ == "__main__":
	main()
