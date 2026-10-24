#!/bin/sh
set -eu
case "$0" in
    */*) script_dir=${0%/*} ;;
    *) script_dir=. ;;
esac
script_dir=$(CDPATH= cd -- "$script_dir" && pwd)
exec /bin/sh "$script_dir/python.sh" "$script_dir/generate.py" "$@"
