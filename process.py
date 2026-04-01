import os
import glob
import xml.etree.ElementTree as ET

def iter_xml_files(root_dir):
    return glob.glob(os.path.join(root_dir, "**", "*.xml"), recursive=True)

def build_global_message_map(xml_files):
    """
    Builds mapping:
        SomeName -> ParentName.SomeName
    from ALL XML files combined.
    """
    mapping = {}

    for xml_path in xml_files:
        try:
            tree = ET.parse(xml_path)
            root = tree.getroot()
        except ET.ParseError:
            continue

        for parent in root.iter():
            if parent.tag not in ("class", "interface"):
                continue

            parent_name = parent.attrib.get("name")
            if not parent_name:
                continue

            for msg in parent.findall(".//message"):
                msg_name = msg.attrib.get("name")
                if not msg_name:
                    continue

                # keep first seen (avoid random overwrites)
                mapping.setdefault(msg_name, f"{parent_name}.{msg_name}")

    return mapping

def update_handles_in_file(xml_path, mapping):
    try:
        tree = ET.parse(xml_path)
        root = tree.getroot()
    except ET.ParseError as e:
        print(f"Parse error in {xml_path}: {e}")
        return False

    changed = False

    for handles in root.iter("handles"):
        msg = handles.attrib.get("message")
        if not msg:
            continue

        if "." in msg:
            continue

        full = mapping.get(msg)
        if full:
            handles.set("message", full)
            changed = True

    if changed:
        tree.write(xml_path, encoding="utf-8", xml_declaration=True)

    return changed

def process_workspace(root_dir):
    xml_files = iter_xml_files(root_dir)

    print(f"Found {len(xml_files)} XML files")

    mapping = build_global_message_map(xml_files)
    print(f"Known messages: {len(mapping)}")

    updated = 0
    for xml_path in xml_files:
        if update_handles_in_file(xml_path, mapping):
            updated += 1
            print(f"Updated: {xml_path}")

    print(f"Done. Updated {updated} files.")

if __name__ == "__main__":
    process_workspace(".")  # set workspace root here