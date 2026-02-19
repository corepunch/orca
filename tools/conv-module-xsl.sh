#!/bin/bash

# conv-module-xsl.sh
# XSLT-based alternative to conv-module.py
# Generates C headers, exports, properties, and documentation from XML module definitions

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
XSL_DIR="$SCRIPT_DIR/xsl"
SCHEMAS_DIR="$SCRIPT_DIR/schemas"

# Output file for consolidated DTD (updated incrementally)
DTD_OUTPUT="$SCHEMAS_DIR/orca.dtd"

# HTML documentation output
HTML_OUTPUT="../docs/index.html"

# Check if xsltproc is available
if ! command -v xsltproc &> /dev/null; then
    echo "Error: xsltproc is not installed. Please install it:"
    echo "  Ubuntu/Debian: sudo apt-get install xsltproc"
    echo "  Fedora/RHEL: sudo dnf install libxslt"
    echo "  macOS: brew install libxslt"
    exit 1
fi

# Function to process a single XML module file
process_module() {
    local xml_file="$1"
    local base_name="${xml_file%.xml}"
    
    echo "Processing: $xml_file"
    
    # Generate .h header file
    echo "  Generating ${base_name}.h"
    xsltproc -o "${base_name}.h" "$XSL_DIR/generate-header.xsl" "$xml_file"
    
    # Generate _export.c file
    echo "  Generating ${base_name}_export.c"
    xsltproc -o "${base_name}_export.c" "$XSL_DIR/generate-export.xsl" "$xml_file"
    
    # Generate _properties.h file
    echo "  Generating ${base_name}_properties.h"
    xsltproc -o "${base_name}_properties.h" "$XSL_DIR/generate-properties.xsl" "$xml_file"
    
    # Append to DTD file
    echo "  Appending to DTD"
    xsltproc "$XSL_DIR/generate-dtd.xsl" "$xml_file" >> "$DTD_OUTPUT.tmp"
}

# Main script
main() {
    if [ $# -eq 0 ]; then
        echo "Usage: $0 <module1.xml> [module2.xml] ..."
        echo ""
        echo "Example:"
        echo "  $0 ../source/geometry/geom.xml ../source/core/core.xml"
        exit 1
    fi
    
    echo "XSLT-based Module Code Generator"
    echo "================================="
    echo ""
    
    # Initialize DTD output file
    > "$DTD_OUTPUT.tmp"
    
    # Process each XML file provided as argument
    for xml_file in "$@"; do
        if [ ! -f "$xml_file" ]; then
            echo "Warning: File not found: $xml_file"
            continue
        fi
        
        process_module "$xml_file"
        echo ""
    done
    
    # Finalize DTD by appending the standard footer
    if [ -f "$SCHEMAS_DIR/append.dtd" ]; then
        cat "$SCHEMAS_DIR/append.dtd" >> "$DTD_OUTPUT.tmp"
    fi
    
    # Move temporary DTD to final location
    mv "$DTD_OUTPUT.tmp" "$DTD_OUTPUT"
    
    echo "Code generation complete!"
    echo "Generated files:"
    echo "  - Header files (.h)"
    echo "  - Export files (_export.c)"
    echo "  - Properties files (_properties.h)"
    echo "  - DTD schema ($DTD_OUTPUT)"
}

main "$@"
