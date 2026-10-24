#!/bin/sh
# Run an installed Python directly. Never invoke Apple's /usr/bin/python3 shim.
set -eu

try_python() {
    candidate=$1
    shift
    case "$candidate" in
        /usr/bin/python*|/usr/bin/xcrun) return ;;
        /*) ;;
        *) return ;;
    esac
    if [ -e /usr/bin/python3 ] && [ "$candidate" -ef /usr/bin/python3 ]; then
        return
    fi
    if [ -f "$candidate" ] && [ -x "$candidate" ]; then
        export PYTHONDONTWRITEBYTECODE=1
        exec "$candidate" "$@"
    fi
}

if [ -n "${ORCA_PYTHON:-}" ]; then
    try_python "$ORCA_PYTHON" "$@"
    echo 'ORCA_PYTHON must name an installed Python 3.9+ executable, not /usr/bin/python3.' >&2
    exit 1
fi

for candidate in /opt/homebrew/bin/python3 /usr/local/bin/python3 \
    /Library/Frameworks/Python.framework/Versions/Current/bin/python3; do
    try_python "$candidate" "$@"
done

# Resolve existing installations through filesystem paths, without xcrun or xcode-select.
for developer in "${DEVELOPER_DIR:-/nonexistent}" /var/db/xcode_select_link \
    /Applications/Xcode*.app/Contents/Developer \
    "${HOME:-/nonexistent}"/Applications/Xcode*.app/Contents/Developer; do
    [ -d "$developer" ] || continue
    for candidate in "$developer/Library/Frameworks/Python3.framework/Versions/Current/bin/python3" \
        "$developer"/Library/Frameworks/Python3.framework/Versions/3.*/bin/python3; do
        try_python "$candidate" "$@"
    done
done

echo 'No installed Python 3.9+ runtime found. Generation stopped; nothing was downloaded.' >&2
echo 'Set ORCA_PYTHON to an existing Python executable. Do not use /usr/bin/python3.' >&2
exit 1
