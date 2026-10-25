"""Regression for complete native HTTP headers; needs only Python + built ORCA.

Run from any directory:
    python3 tests/test_network_headers.py [--binary build/bin/orca]

The fixture binds only to localhost and makes no external network requests.
"""

import argparse
import http.server
import json
import pathlib
import subprocess
import tempfile
import threading


class Echo(http.server.BaseHTTPRequestHandler):
    def do_POST(self):
        data = self.rfile.read(int(self.headers.get("Content-Length", "0")))
        self.send_response(200)
        self.send_header("Content-Type", "application/octet-stream")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        if self.path == "/redirect":
            self.send_response(302)
            self.send_header("Location", "/final")
            self.send_header("X-Intermediate", "must-not-leak")
            self.send_header("Content-Length", "0")
            self.end_headers()
            return
        data = json.dumps({
            "authorization": self.headers.get("Authorization"),
            "longValue": self.headers.get("X-Long-Value"),
            "longName": self.headers.get("X-" + "N" * 1500),
            "cookie": self.headers.get("Cookie"),
        }).encode()
        self.send_response(401 if self.path == "/unauthorized" else 200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Set-Cookie", "a_session_test=first; Path=/; HttpOnly")
        self.send_header("Set-Cookie", "other=second; Path=/; HttpOnly")
        self.send_header("X-Mixed-Case", "header-value")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def log_message(self, *args):
        pass


parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--binary", default="build/bin/orca", help="ORCA binary path relative to repository root")
args = parser.parse_args()
root = pathlib.Path(__file__).resolve().parent.parent
binary = (root / args.binary).resolve()
if not binary.is_file():
    parser.error("Build ORCA first or pass --binary with its executable path")

server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), Echo)
threading.Thread(target=server.serve_forever, daemon=True).start()
cookie_jar = pathlib.Path("/tmp/cookies.txt")
cookie_stat = cookie_jar.stat() if cookie_jar.exists() else None
try:
    with tempfile.TemporaryDirectory(prefix="orca-network-headers-") as directory:
        script = pathlib.Path(directory) / "test.lua"
        script.write_text('''
local network = require "orca.network"
local auth = "Bearer " .. string.rep("jwt", 3000)
local longValue = string.rep("value", 1200)
local longName = "X-" .. string.rep("N", 1500)
local function fetch(headers, suffix)
  local result, responseHeaders
  local thread = coroutine.create(function()
    local _, response = pcall(network.fetch, URL .. (suffix or ""), {headers=headers, nocookies=true})
    result = response:json()
    responseHeaders = response:headers()
  end)
  repeat
    local ok, err = coroutine.resume(thread)
    assert(ok, tostring(err))
  until coroutine.status(thread) == "dead"
  return result, responseHeaders
end
local data, responseHeaders = fetch {Authorization=auth, ["X-Long-Value"]=longValue, [longName]="full-name"}
assert(data.authorization == auth, "Authorization JWT was truncated")
assert(data.longValue == longValue, "Header value was truncated")
assert(data.longName == "full-name", "Header name was truncated")
assert(responseHeaders["x-mixed-case"][1] == "header-value", "Response header names must be lowercase")
assert(#responseHeaders["set-cookie"] == 2, "Duplicate Set-Cookie values must stay separate")
assert(responseHeaders["set-cookie"][1] == "a_session_test=first; Path=/; HttpOnly", "Session cookie was truncated")
responseHeaders["set-cookie"][1] = "changed"
assert(data.cookie == nil, "nocookies must not load a shared session")
data = fetch {Authorization="Bearer second", ["X-Long-Value"]="second", [longName]="second"}
assert(data.authorization == "Bearer second" and data.longValue == "second" and data.longName == "second", "Headers leaked across requests")
assert(data.cookie == nil, "nocookies must not reuse a prior response cookie")
data, responseHeaders = fetch({}, "/redirect")
assert(responseHeaders["x-intermediate"] == nil, "Intermediate response headers leaked after redirect")
assert(responseHeaders["set-cookie"][1] == "a_session_test=first; Path=/; HttpOnly", "Final response cookies were lost")
data, responseHeaders = fetch({}, "/unauthorized")
assert(responseHeaders["x-mixed-case"][1] == "header-value", "HTTP errors must preserve response headers")
local binary = string.char(255,216,0,17,0,255) .. string.rep("binary", 2000) .. string.char(0,255,217)
local returned
local upload = coroutine.create(function()
  local response = network.fetch(URL, {method="POST",body=binary,nocookies=true,
    headers={["Content-Type"]="application/octet-stream"}})
  returned = response:text()
end)
repeat
  local ok, err = coroutine.resume(upload)
  assert(ok, tostring(err))
until coroutine.status(upload) == "dead"
assert(returned == binary, "Binary upload/download must preserve all embedded NUL bytes")
print("PASS: native HTTP headers and binary round-trip (long JWT/header values, request isolation, embedded NUL bytes)")
'''.replace("URL", json.dumps("http://127.0.0.1:" + str(server.server_port))))
        result = subprocess.run([str(binary), "-test=" + str(script)], cwd=root, timeout=20)
        current_stat = cookie_jar.stat() if cookie_jar.exists() else None
        assert current_stat == cookie_stat, "nocookies requests must not write the shared cookie jar"
        raise SystemExit(result.returncode)
finally:
    server.shutdown()
    server.server_close()
