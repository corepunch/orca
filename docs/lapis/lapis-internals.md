# Lapis Internals: How It Works
*A deep-dive for anyone recreating Lapis-style architecture in their own framework, with MoonScript throughout*

---

## What Lapis Is

Lapis is a Lua/MoonScript web framework that runs on top of **OpenResty** (Nginx + LuaJIT) or **lua-http**. Its core job is to map incoming HTTP requests to action functions, run those functions against a request object, and turn the return value into an HTTP response — including rendered HTML.

The entire framework is written in MoonScript (compiled to Lua). Understanding it means understanding MoonScript class patterns, Lua's module system, and the specific lifecycle Lapis enforces on every request.

---

## MoonScript: The Language Primer

Before diving into Lapis architecture, here is how MoonScript code is structured, since every example in this doc uses it.

### Classes

MoonScript uses a class syntax that compiles to Lua metatables. There is no magic — just `__index` chains:

```moonscript
class Animal
  -- class-level field (stored on the class table, not instances)
  @sound: "..."

  -- constructor
  new: (name) =>
    @name = name   -- @ is self

  -- instance method
  speak: =>
    "#{@name} says #{@@sound}"   -- @@ is the class itself

class Dog extends Animal
  @sound: "woof"
```

The key conventions:

- `=>` defines a method that receives `self` as first arg, aliased as `@`
- `@field` is `self.field`
- `@@field` is `self.__class.field` (the class object)
- `@field!` is `self:field()` — calling a method on self
- `class Foo extends Bar` sets up the prototype chain automatically
- `super` calls the parent class's version of the current method
- `@before_filter` inside a class body means `Foo.before_filter = ...` (a class method call)

### Imports and Modules

```moonscript
-- Lua-style require
lapis = require "lapis"

-- Destructuring import (sugar for require + field extraction)
import Widget from require "lapis.html"
import Model, enum from require "lapis.db.model"
import respond_to, capture_errors from require "lapis.application"
```

### Tables and Function Calls

MoonScript drops parentheses and uses indentation/commas freely:

```moonscript
-- these are equivalent
foo(a, b, c)
foo a, b, c

-- table literal
config = {
  port: 8080
  secret: "abc123"
}

-- method call shorthand
user\update name: "Igor"   -- user:update({ name = "Igor" })
```

### Arrow Functions

```moonscript
fn = -> "no args"
fn = (x) -> x * 2
method = => "self is #{@name}"  -- bound to self
```

### String Interpolation

```moonscript
name = "Igor"
msg = "Hello #{name}, you are #{1 + 1} years old"
```

### Comprehensions and Loops

```moonscript
-- array comprehension
doubled = [x * 2 for x in *list]

-- hash comprehension  
mapping = {k, v for k, v in pairs(tbl)}

-- iteration over array (MoonScript's * operator unpacks)
for item in *items
  print item.name
```

### `unless` and `with`

```moonscript
unless user
  error "no user"

-- with block: sets self to the expression
with user
  .name = "Alice"
  \save!
```

---

## The Request Lifecycle

Every request follows this exact sequence:

```
Nginx/HTTP server receives request
    ↓
lapis.serve("app_module") -- the entry point in nginx.conf
    ↓
Application:dispatch(request_obj)
    ↓
1. Route matching (find action by URL pattern)
    ↓
2. Before filters run (in order, can short-circuit)
    ↓
3. Action function runs (returns render options or string)
    ↓
4. Return value processed by request:write()
    ↓
5. Render phase: view widget rendered, wrapped in layout
    ↓
6. Response written to client
```

---

## Routing

### How Routes Are Stored

An `Application` is a class. Routes are stored as an ordered list inside the class itself. When you subclass `lapis.Application` and write:

```moonscript
class App extends lapis.Application
  "/": => "hello"
  [profile: "/user/:name"]: => render: true
```

MoonScript iterates the class body as key-value pairs. Lapis intercepts these via an `__inherited` callback or the class's `__newindex`. Each key that looks like a route pattern (string starting with `/`, or a table `{name: pattern}`) gets registered via `app:match(name, pattern, action)`.

### Pattern Syntax

Lapis uses its own pattern matcher (not Lua's `string.find`). The syntax:

| Pattern | Matches |
|---------|---------|
| `/hello` | exact literal |
| `/user/:name` | captures `name`, no slashes |
| `/files/*` | wildcard `splat`, including slashes |
| `/post/:id[%d]` | `:id` only digits |
| `/user(/:page)` | optional segment |

Internally Lapis compiles these to Lua patterns and stores them in a priority list. Literal routes have highest priority, then single-variable routes, then multi-variable, then splats.

### Route Resolution

When a request arrives, Lapis iterates the route list and tries each pattern in priority order. The first match wins. Captured values are placed into `self.params`.

### Named Routes and `url_for`

```moonscript
-- Declare with a name
[user_profile: "/user/:username"]: =>
  "Hello #{@params.username}"

-- Generate URL from anywhere that has a request object
@url_for "user_profile", username: "igor"
-- returns: /user/igor

-- With query params
@url_for "user_profile", {username: "igor"}, page: 2
-- returns: /user/igor?page=2
```

The magic: models can implement `url_params` to be passed directly to `url_for`:

```moonscript
class Users extends Model
  url_params: (req, ...) =>
    "user_profile", { username: @username }, ...

-- Then in a view or action:
@url_for some_user   -- calls some_user:url_params(@) internally
```

---

## The Application Class

### Structure

```moonscript
lapis = require "lapis"
import respond_to from require "lapis.application"

class App extends lapis.Application
  -- Application-level settings (fields on the class, not instances)
  layout: require "views.layout"      -- default layout widget
  views_prefix: "views"               -- prepended to view names
  actions_prefix: "actions"           -- for lazy-loaded action modules
  flows_prefix: "flows"               -- for flow modules

  -- Before filter: runs before every action
  @before_filter =>
    if @session.user_id
      @current_user = Users\find @session.user_id

  -- Simple routes
  "/": => "Hello world"

  -- Named route
  [about: "/about"]: =>
    render: true   -- loads views/about.moon

  -- HTTP-verb specific
  [create_user: "/users/new"]: respond_to {
    GET: => render: true
    POST: =>
      user = Users\create @params.user
      redirect_to: @url_for "user_profile", user
  }

  -- 404 handler
  handle_404: =>
    status: 404, render: "errors.not_found"
```

### Before Filters

Before filters run in the order they were declared. If a before filter calls `@write(...)`, the action is cancelled and no further filters or action functions run:

```moonscript
class App extends lapis.Application
  @before_filter =>
    unless @current_user
      @write redirect_to: @url_for "login"
      -- action will NOT run after this

  [dashboard: "/dashboard"]: =>
    render: true
```

### Application Inheritance and `include`

Sub-applications can be composed:

```moonscript
class AdminApp extends lapis.Application
  @before_filter => error "not admin" unless @current_user.is_admin
  "/admin/users": => render: true

class MainApp extends lapis.Application
  @include AdminApp, path: "/admin", name: "admin."
  -- now /admin/admin/users is available as "admin.admin_users"
```

---

## The Request Object

When an action function runs, `self` (i.e. `@`) **is the request object**, not the application. The application is at `@app`.

Key fields on `@` inside an action:

| Field | Description |
|-------|-------------|
| `@params` | Merged URL params + POST body + query string |
| `@GET` | Only query string params |
| `@POST` | Only POST body params |
| `@session` | Signed JSON cookie (read/write) |
| `@cookies` | Raw cookie access |
| `@req` | Raw server request (headers, parsed URL, etc.) |
| `@app` | The Application instance |
| `@options` | Accumulated render options |
| `@route_name` | Name of the matched route |

Methods on `@`:

```moonscript
@url_for "route_name", params       -- generate URL
@build_url "path", host: "x.com"   -- absolute URL
@flow "flow_name"                   -- load a Flow for this request
@html -> div "markup"               -- inline HTML builder
@write things...                    -- accumulate render options
```

---

## Render Options (What Actions Return)

An action returns either a string (rendered directly) or a table of options:

```moonscript
-- String: sent to browser as-is, with layout
"/plain": => "just a string"

-- Table: render options
"/page": =>
  status: 200
  render: true          -- load views/{route_name}.moon
  layout: false         -- disable layout wrapping

-- Multiple return values: merged
"/multi": =>
  "some content", render: "my_view", status: 201

-- JSON: disables layout, sets content-type
"/api/data": =>
  json: { users: Users\select! }

-- Redirect
"/old": =>
  redirect_to: @url_for "new_page"

-- Full control: render named view
"/custom": =>
  render: "errors.not_found", status: 404

-- Skip Lapis render entirely (for streaming)
"/stream": =>
  ngx.print "streaming..."
  skip_render: true
```

---

## View Rendering

### The Widget System

Views are classes inheriting `lapis.html.Widget`. Lapis loads them by module name using `{views_prefix}.{view_name}`. So `render: "index"` with the default `views_prefix` loads the module `views.index`.

The `content` method is called to render HTML. Inside it, the environment is special: any unknown identifier is treated as an HTML tag generator.

```moonscript
-- views/index.moon
import Widget from require "lapis.html"

class Index extends Widget
  content: =>
    h1 class: "title", "Welcome"
    div class: "body", ->
      p "Hello #{@page_title}"  -- @page_title came from the action via self
      ul ->
        for item in *@items
          li item.name
```

The HTML builder works like this:
- `div "text"` → `<div>text</div>`
- `div class: "foo", -> ...` → `<div class="foo">...</div>` (the function is the children)
- `text "unsafe"` → escaped text node
- `raw "<br/>"` → unescaped output
- `widget SomeWidget label: "x"` → renders another widget inline
- `@content_for "title", -> h1 "My Title"` → sends a block to the layout

### Layouts

A layout is also a `Widget`. It wraps every action's rendered output. The inner content is accessed via `@content_for "inner"`:

```moonscript
-- views/layout.moon
import Widget from require "lapis.html"

class Layout extends Widget
  content: =>
    html_5 ->
      head ->
        title @title or "My App"
        link rel: "stylesheet", href: "/static/app.css"
      body ->
        div id: "header", ->
          a href: "/", "Home"
        div id: "main", ->
          @content_for "inner"    -- the view content goes here
        div id: "footer", "Footer"
```

Setting a custom layout:

```moonscript
class App extends lapis.Application
  layout: require "views.layout"

  -- Override per-action
  "/special": =>
    layout: "mobile_layout", render: true
```

### Passing Data to Views

Any field you set on `@` in the action is available in the view as `@`:

```moonscript
-- In action:
[user_page: "/user/:id"]: =>
  @user = Users\find @params.id
  @page_title = "#{@user.name}'s Profile"
  render: true

-- In views/user_page.moon:
class UserPage extends Widget
  content: =>
    h1 @page_title
    p "Joined: #{@user.created_at}"
```

---

## Configuration

Configuration is in `config.moon`. It uses a DSL that maps method calls to table fields:

```moonscript
import config from require "lapis.config"

config "development", ->
  port 8080
  postgres ->
    host "127.0.0.1"
    database "myapp_dev"
    user "postgres"
  session_name "myapp_session"
  secret "dev_secret_change_this"
  num_workers 1

config "production", ->
  port 80
  postgres ->
    host (os.getenv "DB_HOST") or "localhost"
    database "myapp"
    user "myapp_user"
    password os.getenv "DB_PASS"
  secret os.getenv "APP_SECRET"
  num_workers 4
```

To read config in your app:

```moonscript
import get_config from require "lapis.config"
config = get_config require "lapis.environment"
print config.secret
```

The config DSL translates function calls to key-value pairs. Nested calls become nested tables. The `include` keyword merges another config block in. Calling `set "include", "value"` lets you assign reserved keyword names.

---

## Models

Models represent database tables. Each class is one table, each instance is one row.

### Defining Models

```moonscript
import Model from require "lapis.db.model"

class Users extends Model
  -- class-level settings
  @timestamp: true    -- auto-manage created_at / updated_at
  @primary_key: "id"  -- default, can be "login" or {"user_id", "post_id"}

  -- class-level constraints
  @constraints: {
    username: (value) =>
      if #value < 3
        "Username must be at least 3 characters"
  }

  -- relations
  @relations: {
    {"posts", has_many: "Posts"}
    {"profile", has_one: "UserProfiles"}
  }

  -- instance methods
  display_name: =>
    @name or @username

  -- url_params lets models be passed to url_for
  url_params: (req, ...) =>
    "user_profile", { id: @id }, ...
```

### Querying

```moonscript
-- Find by primary key
user = Users\find 42

-- Find by arbitrary column
user = Users\find email: "igor@example.com"

-- Select multiple (returns array of model instances)
users = Users\select "where active = ? order by created_at desc", true

-- Count
n = Users\count "active = true"

-- Paginate
pager = Users\paginated "where active = ? order by id asc", true, per_page: 20
page = pager\get_page 1
total = pager\total_items!
```

### Creating and Updating

```moonscript
-- Create (INSERT + RETURNING)
user = Users\create {
  username: "igor"
  email: "igor@example.com"
}

-- Update (two forms)
user\update { username: "igor2" }    -- form 1: pass a table
user.username = "igor2"
user\update "username"               -- form 2: list fields to sync

-- Delete
if user\delete!
  -- only runs if row was actually deleted (race-safe)
  do_cleanup!
```

### Relations

```moonscript
class Posts extends Model
  @relations: {
    {"user", belongs_to: "Users"}          -- adds get_user()
    {"tags", has_many: "Tags"}             -- adds get_tags(), get_tags_paginated()
    {"author", belongs_to: "Users", key: "author_id"}  -- custom FK
  }

post = Posts\find 1
user = post\get_user!
tags = post\get_tags!
```

### Preloading (Avoiding N+1)

```moonscript
import preload from require "lapis.db.model"

posts = Posts\select "order by id desc limit 20"
preload posts, "user", "tags"   -- single query per relation

-- Nested preloading: load user, then load each user's profile
preload posts, user: "profile"
```

### Enums

```moonscript
import Model, enum from require "lapis.db.model"

class Posts extends Model
  @statuses: enum {
    draft: 1
    published: 2
    archived: 3
  }

-- Usage
post = Posts\find 1
print Posts.statuses[post.status]     -- "draft"
Posts.statuses\for_db "published"     -- 2
Posts.statuses\to_name 3              -- "archived"
```

---

## Flows

Flows are the Lapis answer to "service objects" or "form objects". A Flow wraps a contained object (usually the request) and proxies field reads/writes to it, while adding its own methods.

### Defining a Flow

```moonscript
import Flow from require "lapis.flow"

class AccountsFlow extends Flow
  -- Expose specific assignments back to the request object
  @expose_assigns: {"current_user"}

  load_user: =>
    @current_user = Users\find @session.user_id  -- sets on request too

  require_login: =>
    unless @session.user_id
      @write redirect_to: @url_for "login"

  register: =>
    -- @params comes from the request via proxy
    user = Users\create {
      username: @params.username
      password: hash @params.password
    }
    @session.user_id = user.id
    redirect_to: @url_for "dashboard"
```

### Using Flows in Actions

```moonscript
import capture_errors from require "lapis.application"

class App extends lapis.Application
  [register: "/register"]: capture_errors respond_to {
    GET: => render: true
    POST: => @flow("accounts")\register!
  }

  [dashboard: "/dashboard"]: =>
    @flow("accounts")\require_login!
    @flow("accounts")\load_user!
    render: true
```

`@flow("accounts")` loads `flows/accounts.moon`, instantiates it with `@` (the request), and returns a cached instance for that request.

Flows are automatically cached per-request: calling `@flow("accounts")` twice returns the same instance.

---

## Error Handling

### `capture_errors`

Wraps an action so that `error()` calls are caught and rendered gracefully instead of crashing:

```moonscript
import capture_errors, yield_error from require "lapis.application"

class App extends lapis.Application
  [create_post: "/posts/new"]: capture_errors {
    on_error: =>
      -- @errors is an array of error strings
      render: "errors.form", status: 422

    respond_to {
      GET: => render: true
      POST: =>
        yield_error "Title is required" unless @params.title
        post = Posts\create { title: @params.title }
        redirect_to: @url_for "post", post
    }
  }
```

`yield_error "message"` throws an error that `capture_errors` catches. `@errors` will be the array of such messages.

### Global Error Handler

```moonscript
class App extends lapis.Application
  handle_error: (err, trace) =>
    -- Log it
    ngx.log ngx.ERR, "Error: #{err}\n#{trace}"
    -- Render custom page
    render: "errors.server_error", status: 500
```

---

## File & Module Layout

A typical Lapis app looks like this. This mirrors the streak.club structure:

```
app.moon              ← main Application class, entry point
config.moon           ← environment configs (dev/prod)
models.moon           ← re-exports all models (require "models").SomeModel
migrations.moon       ← database schema migrations

models/
  users.moon
  posts.moon
  tags.moon

views/
  layout.moon         ← default layout widget
  index.moon
  user_profile.moon
  errors/
    not_found.moon
    server_error.moon

flows/
  accounts.moon
  posts.moon

applications/         ← sub-apps (included into main app)
  admin.moon
  api.moon

widgets/              ← reusable sub-widgets (non-page widgets)
  post_card.moon
  pagination.moon

helpers/              ← utility modules
  markdown.moon
  uploads.moon

static/               ← served directly by nginx
  app.css
  app.js
```

### The Models Re-export Pattern

streak.club uses a `models.moon` at the root that re-exports all models from `models/`:

```moonscript
-- models.moon
import Users from require "models.users"
import Posts from require "models.posts"
import Tags from require "models.tags"

{ :Users, :Posts, :Tags }
```

Then in any model's relations, you reference by string (`"Users"`) and Lapis calls `require("models")["Users"]` via the `get_relation_model` default implementation. This means all models are lazily cross-referenced through one hub module.

---

## Input Validation

```moonscript
import capture_errors from require "lapis.application"
import assert_valid from require "lapis.validate"

class App extends lapis.Application
  [create_user: "/users"]: capture_errors respond_to {
    POST: =>
      assert_valid @params, {
        {"username", exists: true, min_length: 3, max_length: 30}
        {"email", exists: true, matches_pattern: ".+@.+"}
        {"password", exists: true, min_length: 8}
      }
      -- if assert_valid fails, capture_errors catches the yielded error
      user = Users\create @params
      redirect_to: @url_for "profile", user
  }
```

---

## Key Architectural Patterns to Replicate

If you are building something similar, here are the core decisions Lapis makes:

**1. Request object as `self` in actions.** Don't pass a request separately — make the action function's first argument (self/this) the request. This makes `@params`, `@session`, `@url_for` feel like first-class language features.

**2. Return-value driven rendering.** Actions don't write to a response directly. They return a table of options (`render`, `status`, `json`, `redirect_to`). This lets before-filters short-circuit cleanly via `write`.

**3. Widgets over templates.** HTML generated by composable class instances gives you inheritance, mixins, and `content_for` slots — things template engines struggle with.

**4. Models know their URLs.** Implement `url_params` on models so they can be passed to `url_for`. This prevents hardcoded URLs throughout your codebase.

**5. Flows as service objects.** Rather than fat controllers or fat models, business logic lives in Flow classes that proxy to the request.

**6. Lazy module loading for actions and views.** An action can be a string `"user_profile"` which loads `actions.user_profile` on first hit. This keeps startup time fast and modules small.

**7. Class-based config DSL.** The MoonScript config builder (function calls → table keys) is much nicer than JSON or INI files for multi-environment config with logic.
