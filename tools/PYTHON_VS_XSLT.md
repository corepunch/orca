# Comparison: Python vs XSLT for Module Code Generation

## Executive Summary

This document compares the Python-based `conv-module.py` script with the XSLT-based alternative for generating code from ORCA module XML files.

**Bottom Line**: XSLT **can** replace Python for this task, but it's not recommended for production use without significant additional work. The XSLT approach is demonstrated as a proof-of-concept.

## File Generation Comparison

| Output File Type | Python | XSLT | Notes |
|-----------------|--------|------|-------|
| `.h` header files | ✅ Full | ⚠️ Basic | XSLT version works but missing some complex features |
| `_export.c` Lua bindings | ✅ Full | ⚠️ Simplified | XSLT handles basic cases, missing edge cases |
| `_properties.h` | ✅ Full | ✅ Good | XSLT implementation is adequate |
| `orca.dtd` schema | ✅ Full | ⚠️ Basic | XSLT can generate but simplified |
| `index.html` documentation | ✅ Full | ❌ Not implemented | Would require significant XSLT work |

## Approach Comparison

### Python Approach (`conv-module.py`)

**Strengths:**
- Single script execution processes all modules
- Complex logic for type conversions and code generation
- Maintains global state (structs, enums, components across modules)
- Generates rich HTML documentation with syntax highlighting
- Handles edge cases and special scenarios
- Fast execution (single process)
- 1000 lines of well-structured Python code

**Weaknesses:**
- Requires Python runtime
- Mixing XML parsing with code generation logic
- Single large file (harder to maintain different output formats separately)

### XSLT Approach (New)

**Strengths:**
- No Python dependency (uses standard `xsltproc`)
- Declarative XML transformations
- Separation of concerns (one XSL file per output type)
- Easy to understand each transformation in isolation
- Standard W3C XSLT - widely supported
- Can be edited/debugged in XML IDEs

**Weaknesses:**
- XSLT 1.0 limitation: one output file per transformation
  - Requires multiple xsltproc invocations (slower)
  - Shell script needed for orchestration
- More verbose for complex logic
- Limited string manipulation vs Python
- Cannot maintain global state across modules easily
- Current implementation missing key features:
  - HTML documentation generation
  - Complex type conversions
  - Cross-module references
  - Code syntax highlighting

## Technical Differences

### Multi-file Output

**Python:**
```python
# Opens multiple files simultaneously
self.header = open(filename.replace(".xml", ".h"), "w")
self.export = open(filename.replace(".xml", "_export.c"), "w")
self.props = open(filename.replace(".xml", "_properties.h"), "w")
```

**XSLT 1.0:**
- Can only output ONE file per transformation
- Requires separate invocation for each output:
  ```bash
  xsltproc -o file.h generate-header.xsl input.xml
  xsltproc -o file_export.c generate-export.xsl input.xml
  xsltproc -o file_properties.h generate-properties.xsl input.xml
  ```

**XSLT 2.0/3.0 Alternative:**
- Could use `xsl:result-document` to output multiple files
- Requires Saxon instead of xsltproc
- Would be more efficient

### State Management

**Python:**
```python
# Global dictionaries shared across modules
g_structs = {}
g_enums = {}
g_components = {}
g_resources = {}

# Updated as each module is processed
g_structs.update({struct.get('name'): struct for struct in root.findall(f".//struct[@name]")})
```

**XSLT:**
- Stateless transformations
- Each invocation is independent
- Cannot easily share data between modules
- Would need external mechanism (temp files, database, etc.)

### Complex Logic Example

**Python (Method Binding):**
```python
def header_write_method(node, method, file):
    if method.get('const'):
        self_type = f"{lpc(node.get('name'))} self"
    else:
        self_type = f"{lp(node.get('name'))} self"
    
    # Complex logic for different method types, return values, etc.
    # Can use Python's full power: conditionals, loops, functions
```

**XSLT Equivalent:**
```xml
<xsl:template match="method">
    <xsl:choose>
        <xsl:when test="@const = 'true'">
            <xsl:text>const </xsl:text>
        </xsl:when>
    </xsl:choose>
    <!-- More verbose, less flexible -->
</xsl:template>
```

## Performance Comparison

### Python
- Single process execution
- Parses all XMLs once
- Generates all outputs in one pass
- **Estimated time**: 1-2 seconds for all modules

### XSLT
- Multiple xsltproc invocations
- Each XML parsed 4 times (once per output type)
- Shell script overhead
- **Estimated time**: 3-5 seconds for all modules

## Maintenance Comparison

### Adding a New Output Format

**Python:**
1. Add new file handle in `ParserState.__new__`
2. Add generation logic in main `read_xml` function
3. All in one file

**XSLT:**
1. Create new XSL file (e.g., `generate-something.xsl`)
2. Add invocation in `conv-module-xsl.sh`
3. Clearer separation, easier to understand that specific transform

### Modifying Output Format

**Python:**
- Find the relevant section in 1000-line file
- Modify Python code
- Test

**XSLT:**
- Edit the specific XSL file (100-200 lines)
- Modify XSL templates
- Test
- Clearer scope

## Use Case Recommendations

### ✅ Use XSLT When:
- You want to eliminate Python dependency
- Your module XMLs are simple and don't use complex features
- You only need basic code generation (headers, simple bindings)
- You have good XSLT knowledge on the team
- You don't need HTML documentation
- You can accept slower build times

### ✅ Use Python When:
- You need full feature parity with current implementation
- You want HTML documentation generation
- You have complex module definitions
- Build performance matters
- You need to maintain cross-module references
- Python is already available (it usually is)

### 🤔 Hybrid Approach:
- Use XSLT for simple, template-based outputs (DTD, basic headers)
- Use Python for complex logic (Lua bindings, HTML docs, cross-references)

## Migration Effort

To achieve full feature parity with Python using XSLT:

### Estimated Effort: 2-3 weeks full-time

1. **Upgrade to XSLT 2.0/3.0** (1-2 days)
   - Install Saxon
   - Rewrite stylesheets for XSLT 2.0
   - Use `xsl:result-document` for multi-file output

2. **Implement Missing Features** (1-2 weeks)
   - HTML documentation generation
   - Syntax highlighting
   - All method binding variations
   - Type conversion logic
   - Cross-module references
   - Edge case handling

3. **Testing & Validation** (3-5 days)
   - Generate all modules
   - Compare outputs with Python version
   - Fix discrepancies
   - Verify builds work
   - Test all features

4. **Documentation** (2-3 days)
   - Update build documentation
   - Team training
   - Troubleshooting guide

## Conclusion

The XSLT approach is **technically feasible** but requires significant work to match Python's capabilities. The current proof-of-concept demonstrates:

✅ **What Works:**
- Basic code generation for headers, exports, properties
- DTD schema generation
- Modular, maintainable XSL files
- Shell script orchestration

❌ **What's Missing:**
- HTML documentation
- Complex Lua bindings
- Cross-module references
- Full feature parity

**Final Recommendation**: 

**Keep using `conv-module.py`** unless you have a strong requirement to eliminate Python. The Python approach is mature, complete, and well-tested.

If you must use XSLT:
1. Start with XSLT 2.0/3.0 (Saxon)
2. Budget 2-3 weeks for complete implementation
3. Plan for thorough testing
4. Consider hybrid approach for complex features
