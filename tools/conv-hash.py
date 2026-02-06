import sys

def fnv1a32(str):
    hval = 0x811C9DC5
    prime = 0x01000193
    uint32_max = 2 ** 32
    for s in str:
        hval = hval ^ ord(s)
        hval = (hval * prime) % uint32_max
    return hval

args = [
	"LeftMouseDown",
	"RightMouseDown",
	"OtherMouseDown",
	"LeftMouseUp",
	"RightMouseUp",
	"OtherMouseUp",
	"LeftMouseDragged",
	"RightMouseDragged",
	"OtherMouseDragged",
	"MouseMoved",
	"ScrollWheel",
	"KeyDown",
	"KeyUp",
  "IsVisible",
  "DrawBrush",
  "UpdateGeometry",
  "ForegroundContent",
  "ContainsPoint",
  "Awake",
  "WindowPaint",
  "WindowClosed",
  "WindowResized",
  "WindowChangedScreen",
  "PropertyChanged",
  "Timer",
  "Create",
  "Confirm",
]
for arg in args:
	print("0x%08x, // %s" % (fnv1a32(arg), arg))