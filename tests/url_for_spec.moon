test = require "orca.test"

Widget = require "orca.core.widget"
Application = require "orca.core.application"

class Game
  new: (attrs={}) =>
    for k, v in pairs attrs
      @[k] = v

  url_params: (req, ...) =>
    "Adventure", nil, { game: @id }, ...

build_app = ->
  class App extends Application
    layout: Widget\extend {
      content: ->
        "layout"
    }

    [Adventure: "/adventure"]: -> "ok"

  App()

application_url_for_named_route = ->
  app = build_app()
  test.expect_eq app\url_for("Adventure"), "/adventure", "app:url_for('Adventure') should return the named route path"
  print "PASS: application_url_for_named_route"

application_url_for_model_object = ->
  app = build_app()
  game = Game { id: "zork1" }
  test.expect_eq app\url_for(game), "/adventure?game=zork1", "app:url_for(game) should use game:url_params()"
  print "PASS: application_url_for_model_object"

widget_url_for_delegates_to_application = ->
  previous = Application.app
  app = build_app()
  Application.app = app

  widget = Widget()
  game = Game { id: "zork1" }

  test.expect_eq widget\url_for("Adventure"), "/adventure", "widget:url_for should resolve named routes through the current app"
  test.expect_eq widget\url_for(game), "/adventure?game=zork1", "widget:url_for should resolve model objects through the current app"

  Application.app = previous
  print "PASS: widget_url_for_delegates_to_application"

application_url_for_named_route()
application_url_for_model_object()
widget_url_for_delegates_to_application()

print "All url_for tests passed."
