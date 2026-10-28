#!/bin/sh
# Replaces @@BUILD_VERSION@@ in an HTML file with $BUILD_VERSION.
#
#   BUILD_VERSION='any text' sh tools/webgl-version.sh build/webgl/orca.html
#
# The version comes from the environment and is copied literally, so commit
# titles with quotes, slashes, ampersands or backslashes survive; only the
# HTML-special characters are escaped so the page shows the exact text.
set -e
file=$1
awk '
BEGIN {
	v = ENVIRON["BUILD_VERSION"]
	for (i = 1; i <= length(v); i++) {
		c = substr(v, i, 1)
		if (c == "&") c = "&amp;"
		else if (c == "<") c = "&lt;"
		else if (c == ">") c = "&gt;"
		else if (c == "\"") c = "&quot;"
		s = s c
	}
}
{
	line = $0; out = ""
	while ((i = index(line, "@@BUILD_VERSION@@")) > 0) {
		out = out substr(line, 1, i - 1) s
		line = substr(line, i + 17)
	}
	print out line
}' "$file" > "$file.tmp"
mv "$file.tmp" "$file"
