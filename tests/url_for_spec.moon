test = require "orca.test"

Widget = require "orca.core.widget"
Application = require "orca.core.application"

class Game
  new: (attrs={}) =>
    for k, v in pairs attrs
      @[k] = v

  url_params: (req, ...) =>
    "Adventure", { game: @id }, nil, ...

class Session
  new: (attrs={}) =>
    for k, v in pairs attrs
      @[k] = v

  url_params: (req, ...) =>
    "Adventure", { game: @game_id, session: @id }, nil, ...

build_app = ->
  class App extends Application
    layout: Widget\extend {
      content: ->
        "layout"
    }

    [Adventure: "/adventure/:game(/:session)"]: -> "ok"

  App()

application_url_for_named_route = ->
  app = build_app()
  test.expect_eq app\url_for("Adventure", { game: "zork1" }), "/adventure/zork1", "app:url_for('Adventure', params) should interpolate path params"
  print "PASS: application_url_for_named_route"

application_url_for_model_object = ->
  app = build_app()
  game = Game { id: "zork1" }
  test.expect_eq app\url_for(game), "/adventure/zork1", "app:url_for(game) should use game:url_params()"
  print "PASS: application_url_for_model_object"

application_url_for_session_model_object = ->
  app = build_app()
  session = Session { game_id: "zork1", id: "s1" }
  test.expect_eq app\url_for(session), "/adventure/zork1/s1", "app:url_for(session) should include session path segment"
  print "PASS: application_url_for_session_model_object"

widget_url_for_delegates_to_application = ->
  previous = Application.app
  app = build_app()
  Application.app = app

  widget = Widget()
  game = Game { id: "zork1" }
  session = Session { game_id: "zork1", id: "s1" }

  test.expect_eq widget\url_for("Adventure", { game: "zork1" }), "/adventure/zork1", "widget:url_for should resolve named routes through the current app"
  test.expect_eq widget\url_for(game), "/adventure/zork1", "widget:url_for should resolve model objects through the current app"
  test.expect_eq widget\url_for(session), "/adventure/zork1/s1", "widget:url_for should resolve session model objects through the current app"

  Application.app = previous
  print "PASS: widget_url_for_delegates_to_application"

application_url_for_named_route()
application_url_for_model_object()
application_url_for_session_model_object()
widget_url_for_delegates_to_application()

print "All url_for tests passed."
