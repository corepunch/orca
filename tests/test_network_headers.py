"""Regression for native HTTP pooling, compression, concurrency and headers; needs only Python + built ORCA.

Run from any directory:
    python3 tests/test_network_headers.py [--binary build/bin/orca]

The fixture binds only to localhost and makes no external network requests.
"""

import argparse
import gzip
import http.server
import json
import pathlib
import socket
import subprocess
import tempfile
import threading


release_slow = threading.Event()


class Echo(http.server.BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"

    def reply(self, data, content_type, code=200):
        compressed = "gzip" in self.headers.get("Accept-Encoding", "")
        if compressed:
            data = gzip.compress(data)
        self.send_response(code)
        self.send_header("Content-Type", content_type)
        if compressed:
            self.send_header("Content-Encoding", "gzip")
        self.send_header("Set-Cookie", "a_session_test=first; Path=/; HttpOnly")
        self.send_header("Set-Cookie", "other=second; Path=/; HttpOnly")
        self.send_header("X-Mixed-Case", "header-value")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        try:
            self.wfile.write(data)
        except (BrokenPipeError, ConnectionResetError):
            pass

    def do_POST(self):
        data = self.rfile.read(int(self.headers.get("Content-Length", "0")))
        self.reply(data, "application/octet-stream")

    def do_GET(self):
        if self.path == "/binary":
            self.reply(b"\0native\xff\0", "application/octet-stream")
            return
        if self.path == "/drop":
            self.close_connection = True
            self.connection.shutdown(socket.SHUT_RDWR)
            return
        if self.path == "/hold":
            release_slow.wait(timeout=5)
        if self.path == "/release":
            release_slow.set()
        if self.path == "/redirect":
            self.send_response(302)
            self.send_header("Location", "/final")
            self.send_header("X-Intermediate", "must-not-leak")
            self.send_header("Content-Length", "0")
            self.end_headers()
            return
        data = json.dumps({
            "port": self.client_address[1],
            "path": self.path,
            "acceptEncoding": self.headers.get("Accept-Encoding"),
            "authorization": self.headers.get("Authorization"),
            "longValue": self.headers.get("X-Long-Value"),
            "longName": self.headers.get("X-" + "N" * 1500),
            "cookie": self.headers.get("Cookie"),
        }).encode()
        self.reply(data, "application/json", 401 if self.path == "/unauthorized" else 200)

    def log_message(self, *args):
        pass


parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--binary", default="build/bin/orca", help="ORCA binary path relative to repository root")
parser.add_argument("--native-binary", help="Optional C fetch regression executable")
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
assert(data.acceptEncoding:find("gzip", 1, true), "Supported compression was not negotiated")
assert(responseHeaders["content-encoding"][1] == "gzip", "Fixture must return compressed JSON")
local firstPort = data.port
assert(data.authorization == auth, "Authorization JWT was truncated")
assert(data.longValue == longValue, "Header value was truncated")
assert(data.longName == "full-name", "Header name was truncated")
assert(responseHeaders["x-mixed-case"][1] == "header-value", "Response header names must be lowercase")
assert(#responseHeaders["set-cookie"] == 2, "Duplicate Set-Cookie values must stay separate")
assert(responseHeaders["set-cookie"][1] == "a_session_test=first; Path=/; HttpOnly", "Session cookie was truncated")
responseHeaders["set-cookie"][1] = "changed"
assert(data.cookie == nil, "nocookies must not load a shared session")
data = fetch {Authorization="Bearer second", ["X-Long-Value"]="second", [longName]="second"}
assert(data.port == firstPort, "Sequential requests must reuse the HTTP connection")
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
  assert(response:headers()["content-encoding"][1] == "gzip", "Fixture must return compressed binary")
end)
repeat
  local ok, err = coroutine.resume(upload)
  assert(ok, tostring(err))
until coroutine.status(upload) == "dead"
assert(returned == binary, "Binary upload/download must preserve all embedded NUL bytes")
local function start(suffix)
  local result
  local thread = coroutine.create(function()
    result = network.fetch(URL .. suffix, {nocookies=true}):json()
  end)
  assert(coroutine.resume(thread))
  return thread, function() return result end
end
local function complete(thread)
  while coroutine.status(thread) ~= "dead" do
    local ok, err = coroutine.resume(thread)
    assert(ok, tostring(err))
  end
end
local slow, slowResult = start("/hold")
local fast, fastResult = start("/fast")
complete(fast)
assert(fastResult().path == "/fast", "Concurrent response was delivered to the wrong request")
assert(coroutine.status(slow) == "suspended", "A completed fetch must not wait for other requests")
fetch({}, "/release")
complete(slow)
assert(slowResult().path == "/hold", "Polling another request lost this completion")
local failed = coroutine.create(function()
  local ok, err = pcall(network.fetch, URL .. "/drop", {nocookies=true})
  assert(not ok and tostring(err):find("HTTP transport error", 1, true), "Transport failure must be explicit")
end)
complete(failed)
local abandoned = start("/abandoned")
abandoned = nil
collectgarbage("collect")
collectgarbage("collect")
local invalid = coroutine.create(function()
  assert(not pcall(network.fetch, URL, {headers={bad={}}, nocookies=true}))
end)
complete(invalid)
collectgarbage("collect")
data = fetch({}, "/recovered")
assert(data.path == "/recovered", "Failure or abandoned coroutine corrupted the shared pool")
print("PASS: native HTTP reuse, gzip JSON/binary, concurrent completion, failure/GC recovery")
print("PASS: native HTTP headers and binary round-trip (long JWT/header values, request isolation, embedded NUL bytes)")
'''.replace("URL", json.dumps("http://127.0.0.1:" + str(server.server_port))))
        result = subprocess.run([str(binary), "-test=" + str(script)], cwd=root, timeout=20)
        current_stat = cookie_jar.stat() if cookie_jar.exists() else None
        assert current_stat == cookie_stat, "nocookies requests must not write the shared cookie jar"
        if result.returncode:
            raise SystemExit(result.returncode)
        if args.native_binary:
            release_slow.clear()
            native_binary = (root / args.native_binary).resolve()
            result = subprocess.run([str(native_binary), "http://127.0.0.1:" + str(server.server_port)],
                                    cwd=root, timeout=20)
        raise SystemExit(result.returncode)
finally:
    server.shutdown()
    server.server_close()
