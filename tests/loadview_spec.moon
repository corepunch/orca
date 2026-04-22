test = require "orca.test"
-- Headless tests for LoadView self-loading behavior in PrefabView/ImageView.
-- Covers Object.Start, Object.PropertyChanged, and loadPrefabs compatibility path.
--
-- Run via: $(TARGET) -test=tests/loadview_spec.moon

core = require "orca.core"
ui = require "orca.UIKit"
sk = require "orca.SceneKit"

test_prefabview2d_start_and_property_changed = ->
  view = ui.PrefabView2D { SCA: "ui/start.sca" }
  calls = 0
  last = nil

  view.loadSCA = (self, sca) ->
    calls += 1
    last = sca

  view\send "Object.Start"
  core.flushQueue!
  test.expect_eq calls, 1, "Object.Start should post Node.LoadView once for initial SCA"
  test.expect_eq last, "ui/start.sca", "loadSCA should receive initial SCA"

  view.SCA = "ui/changed.sca"
  core.flushQueue!
  test.expect_eq calls, 2, "Changing SCA should trigger another load"
  test.expect_eq last, "ui/changed.sca", "loadSCA should receive updated SCA"

  view.SCA = "ui/changed.sca"
  core.flushQueue!
  test.expect_eq calls, 2, "Setting same SCA should not trigger duplicate callback"

  print "PASS: test_prefabview2d_start_and_property_changed"


test_imageview_start_and_property_changed = ->
  img = ui.ImageView { Src: "images/start.png" }
  calls = 0
  last = nil

  img.loadImageAsync = (self, src) ->
    calls += 1
    last = src

  img\send "Object.Start"
  core.flushQueue!
  test.expect_eq calls, 1, "Object.Start should post Node.LoadView once for initial Src"
  test.expect_eq last, "images/start.png", "loadImageAsync should receive initial Src"

  img.Src = "images/changed.png"
  core.flushQueue!
  test.expect_eq calls, 2, "Changing Src should trigger another load"
  test.expect_eq last, "images/changed.png", "loadImageAsync should receive updated Src"

  img.Src = "images/changed.png"
  core.flushQueue!
  test.expect_eq calls, 2, "Setting same Src should not trigger duplicate callback"

  print "PASS: test_imageview_start_and_property_changed"


test_prefabview3d_start_and_property_changed = ->
  view3d = sk.PrefabView3D { Prefab: "scenes/start.prefab" }
  calls = 0
  last = nil

  view3d.loadView = (self, path) ->
    calls += 1
    last = path

  view3d\send "Object.Start"
  core.flushQueue!
  test.expect_eq calls, 1, "Object.Start should post Node.LoadView once for initial Prefab"
  test.expect_eq last, "scenes/start.prefab", "loadView should receive initial Prefab"

  view3d.Prefab = "scenes/changed.prefab"
  core.flushQueue!
  test.expect_eq calls, 2, "Changing Prefab should trigger another load"
  test.expect_eq last, "scenes/changed.prefab", "loadView should receive updated Prefab"

  view3d.Prefab = "scenes/changed.prefab"
  core.flushQueue!
  test.expect_eq calls, 2, "Setting same Prefab should not trigger duplicate callback"

  print "PASS: test_prefabview3d_start_and_property_changed"


test_loadprefabs_compatibility_wrapper = ->
  root = ui.Node2D!
  child = root + ui.PrefabView2D { Prefab: "prefabs/legacy.xml" }
  calls = 0
  last = nil

  child.loadView = (self, prefab_path) ->
    calls += 1
    last = prefab_path

  root\loadPrefabs!

  test.expect_eq calls, 1, "loadPrefabs should traverse subtree and trigger PrefabView load"
  test.expect_eq last, "prefabs/legacy.xml", "loadView should receive Prefab path"

  print "PASS: test_loadprefabs_compatibility_wrapper"


-- Run all
test_prefabview2d_start_and_property_changed!
test_imageview_start_and_property_changed!
test_prefabview3d_start_and_property_changed!
test_loadprefabs_compatibility_wrapper!
