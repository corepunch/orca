# ORCA Tests

This directory contains test files for the ORCA engine.

## Test Files

- **test1.lua**: Manual integration tests for core functionality
- **generated_tests.lua**: Auto-generated unit tests from XML specifications (regenerated on each build)

## Auto-Generated Tests

The `generated_tests.lua` file is automatically created from test specifications embedded in the XML API definition files (e.g., `source/core/core.xml`).

### Regenerating Tests

To regenerate the test file after modifying XML specifications:

```bash
make modules
```

### Adding New Tests

To add new unit tests:

1. Open the relevant XML file (e.g., `source/core/core.xml`)
2. Find the method you want to test
3. Add a `<test>` element inside the `<method>` element:

```xml
<method name="YourMethod">
  <summary>Method description</summary>
  <!-- ... args, returns, etc ... -->
  
  <test name="descriptive_test_name">
    <description>What this test validates</description>
    <code><![CDATA[
local obj = orca.ui.TextBlock { Name = "Test" }
assert(obj:yourMethod() == expected, "Error message")
obj:removeFromParent()
    ]]></code>
  </test>
</method>
```

4. Run `make modules` to regenerate the test file
5. Build and run tests to verify

See [docs/UNIT_TESTS.md](../docs/UNIT_TESTS.md) for detailed documentation.

## Running Tests

After building the project:

```bash
# Run generated tests
./build/bin/orca tests/generated_tests.lua

# Run manual tests
./build/bin/orca tests/test1.lua

# Or use the make target
make test
```

## Test Output

Tests use Lua's `assert()` function for validation. The generated test runner:
- Catches assertion failures and prints detailed error messages
- Displays a summary of passed/failed tests
- Exits with code 0 on success, 1 on failure (suitable for CI/CD)

Example output:
```
✓ PASSED: Object.FindChild.find_immediate_child
✗ FAILED: Object.SetName.set_object_name
  Error: assertion failed: Name should be updated
  
Tests: 4 passed, 1 failed, 5 total
```
