#!/usr/bin/env python3
from logging import root
import argparse, subprocess
from pathlib import Path
import xml.etree.ElementTree as ET

# Dict of optional libraries
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
	return default if not a else a in ("y","yes")

def package_xml(name, flags):
	root = ET.Element("Project", {
		"Name": name,
		"StartupScreen": f"{name}/App",
		"Width": "400",
		"Height": "800",
	})
	refs = ET.SubElement(root, "ProjectReferenceLibrary")
	# core refs
	for k,v in {"applications":"applications","assets":"assets","routing":"lib/routing","model":"model","config":"config"}.items():
		ET.SubElement(refs, "ProjectReferenceItem", {"Name": k}).text = v
	# optional libs
	for k in flags:
		if flags[k]:
			ET.SubElement(refs, "ProjectReferenceItem", {"Name": k}).text = f"lib/{k}"
	ET.SubElement(root, "FontLibrary", {"Name":"fonts"})
	msgs = ET.SubElement(root, "MessageLibrary", {"Name":"Messages"})
	for attrs,text in [({"Message":"KeyDown","Key":"q"},"return"),
					   ({"Message":"WindowClosed"},"return"),
					   ({"Message":"RequestReload"},"window:refresh()")]:
		ET.SubElement(msgs, "SystemMessage", attrs).text = text
	ET.indent(root, space="  ")
	return ET.ElementTree(root)

def main():
	p = argparse.ArgumentParser(prog="orca")
	p.add_argument("name", nargs="?")
	for k in LIBS:
		p.add_argument(f"--{k}", action="store_true")
	args = p.parse_args()

	name = args.name or input("Project name: ").strip()
	flags = {k: getattr(args,k) or ask(f"Include {k}?", k=="tailwind") for k in LIBS}

	root = Path(name)
	root.mkdir(exist_ok=False)
	# basic folders
	for d in ["layouts","pages","components","assets","lib","config","model","fonts"]:
		(root/d).mkdir(parents=True, exist_ok=True)
	# App = root/"App.moon"
	# for k, _ in LIBS.items(): 
	# 	if flags[k]: 
	# 		App.write_text(f'{k} = require "{k}"\n', append=True)
	# App.write_text("return {}\n", append=True)

	if flags["routing"]:
		with open(root / "App.moon", "w") as f:
			for k, _ in LIBS.items(): 
				if flags[k]: 
					f.write(f'require "{k}" -- initialize {k}\n')
			f.write(f"\nimport Home, Help from require \"{name}.pages\"")
			f.write("\nimport Application from require \"routing\"\n")
			f.write("\nclass App extends Application\n")
			f.write("\t@stylesheet require \"tailwind\"\n" if flags["tailwind"] else "")
			f.write("\t@stylesheet \"assets/globals.css\"\n\n")
			f.write("\t\"/\": => Home!\n")
			f.write("\t\"/help\": => Help\n")
	else:
		(root/"App.moon").write_text(
			"ui = require \"orca.ui\"\n"
			"class App extends ui.Screen\n"
			"\t@stylesheet \"assets/globals.css\"\n"
			"\tbody: =>\n\t\tui.TextBlock \"Hello World!\"\n")
		
	(root/"pages/Home.moon").write_text(
		"ui = require \"orca.ui\"\n\n"
		"class Home extends ui.Node2D\n"
		"\tbody: =>\n\t\tui.TextBlock \"Hello World!\"\n")

	(root/"pages/Help.moon").write_text(
		"ui = require \"orca.ui\"\n\n"
		"class Help extends ui.Node2D\n"
		"\tbody: =>\n\t\tui.TextBlock \"Help\"\n")

	(root/"pages/init.moon").write_text(
		"return {\n"
		f"\tHome: require \"{name}.pages.Home\"\n"
		f"\tHelp: require \"{name}.pages.Help\"\n"
		"}\n")

	(root/"assets/globals.css").write_text("body {\n\tbackground-color: black;\n\tcolor: white;\n}\n")

	# git init
	# subprocess.run(["git","init"], cwd=root)

	# add selected libs as submodules
	# for k,url in LIBS.items():
	#	 if flags[k]:
	#		 subprocess.run(["git","submodule","add",url,f"lib/{k}"], cwd=root)

	# write package.xml
	package_xml(name, flags).write(root/"package.xml", encoding="utf-8", xml_declaration=True)

	print(f"Project '{name}' created with libs: {', '.join([k for k,v in flags.items() if v])}")

if __name__=="__main__":
	main()
