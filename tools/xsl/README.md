# XSLT-based Code Generation for ORCA Modules

This directory contains XSLT stylesheets and a shell script that provide an alternative to the Python-based `conv-module.py` script for generating C code and documentation from XML module definitions.

## Overview

The XSLT approach replaces the Python script with:
- **Multiple XSL stylesheets** - One for each output format
- **Shell script orchestrator** - `conv-module-xsl.sh` to manage the transformation pipeline
- **Standard XSLT processor** - Uses `xsltproc` (available on most systems)

## Files

### XSL Stylesheets
- `xsl/generate-header.xsl` - Generates C header files (`.h`)
- `xsl/generate-export.xsl` - Generates Lua binding exports (`_export.c`)
- `xsl/generate-properties.xsl` - Generates component properties (`_properties.h`)
- `xsl/generate-dtd.xsl` - Generates DTD schema fragments

### Shell Script
- `conv-module-xsl.sh` - Main orchestration script that:
  - Processes multiple XML module files
  - Invokes xsltproc for each transformation
  - Combines DTD outputs
  - Generates all required files

## Usage

### Basic Usage

```bash
cd tools
./conv-module-xsl.sh <module1.xml> [module2.xml] ...
```

### Example

```bash
cd tools
./conv-module-xsl.sh \
    ../source/geometry/geom.xml \
    ../source/core/core.xml \
    ../source/renderer/api/renderer.xml \
    ../source/UIKit/UIKit.xml \
    ../source/SceneKit/SceneKit.xml \
    ../source/filesystem/filesystem.xml
```

**⚠️ Important Note**: The XSLT-generated files are different from the Python-generated files. The repository uses Python-generated files by default. Only use the XSLT approach for testing or if you're migrating away from Python. Do not commit XSLT-generated files unless they fully match the Python output.

## Prerequisites

Install `xsltproc` (XSLT processor):

```bash
# Ubuntu/Debian
sudo apt-get install xsltproc

# Fedora/RHEL
sudo dnf install libxslt

# macOS
brew install libxslt
```

## Advantages of XSLT Approach

1. **No Python dependency** - Uses standard XML tools available on most systems
2. **Declarative transformations** - XSL is designed specifically for XML transformations
3. **Separation of concerns** - Each output format has its own dedicated stylesheet
4. **Easier to extend** - Add new output formats by creating new XSL files
5. **Standards-based** - Uses W3C XSLT standard
6. **Better tooling** - Many IDEs have built-in XSLT debugging and validation

## Disadvantages and Limitations

1. **XSLT 1.0 limitations**:
   - Only one output file per transformation (requires multiple invocations)
   - More verbose than Python for complex logic
   - Limited string manipulation capabilities

2. **Complexity**:
   - The Python script has ~1000 lines with complex logic
   - Some features are difficult to replicate in XSLT:
     - Global state management (g_structs, g_enums, etc.)
     - Code syntax highlighting
     - Complex HTML documentation generation
     - Inter-module references

3. **Performance**:
   - Multiple xsltproc invocations (one per file type per module)
   - Slower than single Python script execution

4. **Feature gaps in current implementation**:
   - HTML documentation generation not yet implemented
   - Simplified Lua bindings (doesn't handle all edge cases)
   - Missing some of the complex Python logic for:
     - Method signature generation
     - Type conversions
     - Cross-referencing between modules

## Current Status

This is a **proof of concept** demonstrating that XSLT can generate the basic file structures:
- ✅ Header files (.h) - Basic structure working
- ✅ Export files (_export.c) - Simplified implementation
- ✅ Properties files (_properties.h) - Basic implementation
- ✅ DTD schema - Basic implementation
- ❌ HTML documentation - Not yet implemented
- ❌ Full Python feature parity - Not achieved

## Recommendations

### For Simple Use Cases
If your module definitions are straightforward and you want to avoid Python dependencies, the XSLT approach can work.

### For Complex Use Cases
The Python script (`conv-module.py`) is recommended because:
1. Better suited for complex code generation logic
2. Handles edge cases and type conversions
3. Generates comprehensive HTML documentation
4. Maintains global state across modules
5. Single execution is faster than multiple XSLT invocations

### Hybrid Approach
Consider using XSLT for specific outputs:
- Use Python for complex logic (Lua bindings, HTML docs)
- Use XSLT for simple, template-based outputs (DTD, simple headers)

## Migration Path

To fully replace Python with XSLT, you would need:

1. **XSLT 2.0 or 3.0 support**:
   - Install Saxon instead of xsltproc
   - Enables `xsl:result-document` for multiple outputs
   - Better string functions and grouping

2. **Complete all transformations**:
   - Implement HTML documentation generation
   - Handle all type conversions and edge cases
   - Support code syntax highlighting
   - Manage cross-module references

3. **Testing**:
   - Compare outputs byte-for-byte with Python version
   - Test with all module XML files
   - Verify generated code compiles and works

## Updating the Makefile

To use the XSLT approach in your build process:

```makefile
modules:
	cd tools && \
	./conv-module-xsl.sh \
	../source/geometry/geom.xml \
	../source/core/core.xml \
	../source/renderer/api/renderer.xml \
	../source/UIKit/UIKit.xml \
	../source/SceneKit/SceneKit.xml \
	../source/filesystem/filesystem.xml && cd ..
```

## Conclusion

While XSLT **can** replace Python for this use case, it requires significant effort to achieve feature parity. The current implementation demonstrates the feasibility but is not production-ready.

**Recommendation**: Keep using `conv-module.py` unless you have a strong requirement to eliminate Python dependencies. If you do proceed with XSLT, consider using XSLT 2.0/3.0 processors like Saxon for better capabilities.
