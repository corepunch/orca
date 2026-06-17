# Codegen — Usage

```
cgen <module.cgen> <plugin.so> [-o <file>]
```

| Argument | Description |
|---|---|
| `<module.cgen>` | Path to the XML definition file (or `--xml <path>`) |
| `<plugin.so>` | Path to the plugin shared library (or `--plugin <path>`) |
| `-o <file>` | Output file path. Default `"-"` writes to stdout (or `--out`) |

## Exit codes

| Code | Meaning |
|---|---|
| `0` | Success |
| `2` | Usage error |
| `3` | Parse failed |
| `4` | `dlopen` failed |
| `5` | Missing init symbol |
| `6` | Invalid ABI |
| `7` | Emit failed |
