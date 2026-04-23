test = require "orca.test"
core = require "orca.core"
ui = require "orca.UIKit"

-- Event callback test using lowercase callback alias mapped to LoadView property.

test_imageview_start_and_property_changed = ->
  img = ui.ImageView { Src: "images/start.png" }
  calls = 0
  last = nil

  img.loadView = (self, src) ->
    calls += 1
    last = src

  img\send "Object.Start"
  core.flushQueue!
  test.expect_eq calls, 1, "Object.Start should post Node.LoadView once for initial Src"
  test.expect_eq last, "images/start.png", "loadView should receive initial Src"

  img.Src = "images/changed.png"
  core.flushQueue!
  test.expect_eq calls, 2, "Changing Src should trigger another load"
  test.expect_eq last, "images/changed.png", "loadView should receive updated Src"

  img.Src = "images/changed.png"
  core.flushQueue!
  test.expect_eq calls, 2, "Setting same Src should not trigger duplicate callback"

  print "PASS: test_imageview_start_and_property_changed"

test_imageview_start_and_property_changed!
