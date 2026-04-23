# Lapis App Development Guide
*How to write pages, views, models, and everything else in a Lapis + MoonScript app*

---

## Project Structure

Start with this layout. Lapis expects certain conventions by default, though most are configurable.

```
my_app/
├── app.moon              ← your Application class (entry point)
├── config.moon           ← environment settings
├── models.moon           ← hub that exports all models
├── migrations.moon       ← database schema migrations
│
├── models/               ← one file per table
│   ├── users.moon
│   └── posts.moon
│
├── views/                ← HTML widgets, one per page
│   ├── layout.moon       ← wraps every page
│   ├── index.moon
│   └── user_profile.moon
│
├── flows/                ← business logic, grouped by domain
│   └── accounts.moon
│
├── widgets/              ← reusable UI pieces (not full pages)
│   └── post_card.moon
│
└── static/               ← CSS, JS, images (served by nginx directly)
```

---

## 1. Config (`config.moon`)

Set environment-specific options here. The DSL uses function calls as key-value pairs:

```moonscript
import config from require "lapis.config"

config "development", ->
  port 8080
  secret "change_me_in_production"
  session_name "myapp"
  postgres ->
    database "myapp_dev"
    user "postgres"

config "production", ->
  port 80
  secret os.getenv "APP_SECRET"
  num_workers 4
  postgres ->
    host os.getenv "DB_HOST"
    database "myapp"
    user "myapp"
    password os.getenv "DB_PASS"
```

Access config anywhere:

```moonscript
import get_config from require "lapis.config"
config = get_config require "lapis.environment"
print config.secret
```

---

## 2. The App (`app.moon`)

Your app is a class. Routes, before-filters, and settings all live here. For larger apps, split routes into sub-applications and `include` them.

```moonscript
lapis = require "lapis"
import respond_to, capture_errors from require "lapis.application"

class App extends lapis.Application
  -- Layout widget to wrap every rendered page
  layout: require "views.layout"

  -- Runs before every action
  @before_filter =>
    if @session.user_id
      @current_user = (require "models").Users\find @session.user_id

  -- Simple route: return a string directly
  "/": =>
    render: true   -- loads views/index.moon

  -- Named route with URL parameter
  [user_profile: "/user/:username"]: =>
    @user = (require "models").Users\find username: @params.username
    return status: 404, "Not found" unless @user
    render: true   -- loads views/user_profile.moon

  -- Handle GET and POST separately
  [login: "/login"]: respond_to {
    GET: => render: true

    POST: =>
      @flow("accounts")\login!
  }

  -- JSON endpoint (layout disabled automatically)
  [api_users: "/api/users"]: =>
    users = (require "models").Users\select "order by id desc limit 20"
    json: users

  -- 404 page
  handle_404: =>
    status: 404, render: "errors.not_found"
```

### Sub-applications

Split a large app into logical sections:

```moonscript
-- applications/admin.moon
import respond_to from require "lapis.application"

class AdminApp extends lapis.Application
  @before_filter =>
    unless @current_user and @current_user.is_admin
      @write redirect_to: @url_for "home"

  [admin_users: "/users"]: =>
    @users = (require "models").Users\select "order by id desc"
    render: true
```

```moonscript
-- app.moon
class App extends lapis.Application
  @include (require "applications.admin"), path: "/admin", name: "admin."
  -- Now /admin/users is accessible as route "admin.admin_users"
```

---

## 3. Writing Pages (Views)

Views are widget classes. The `content` method writes HTML using builder syntax — every unknown name is treated as an HTML tag.

### Basic View

```moonscript
-- views/index.moon
import Widget from require "lapis.html"

class Index extends Widget
  content: =>
    div class: "hero", ->
      h1 "Welcome to My App"
      p "Sign up today."
      a href: @url_for("register"), class: "btn", "Get Started"
```

### Using Data from the Action

Any `@field` set in the action is available in the view:

```moonscript
-- In action:
[user_profile: "/user/:username"]: =>
  @user = Users\find username: @params.username
  @posts = @user\get_posts!
  @page_title = "#{@user.username}'s Profile"
  render: true

-- In views/user_profile.moon:
class UserProfile extends Widget
  content: =>
    div class: "profile", ->
      h1 @user.username
      p "Member since: #{@user.created_at}"

      h2 "Recent Posts"
      if #@posts == 0
        p "No posts yet."
      else
        ul ->
          for post in *@posts
            li ->
              a href: @url_for("post", post), post.title
```

### Conditional Classes

```moonscript
div class: {"active", is_new: @item.is_new, featured: @item.featured}, ->
  -- generates class="active featured" if is_new=false, featured=true
  text @item.title
```

### Forms

```moonscript
class NewPostForm extends Widget
  content: =>
    form action: @url_for("create_post"), method: "POST", ->
      input type: "hidden", name: "csrf_token", value: @csrf_token
      div class: "field", ->
        label "Title"
        input type: "text", name: "post[title]", value: @params.post and @params.post.title
      div class: "field", ->
        label "Body"
        textarea name: "post[body]", ->
          if @params.post
            text @params.post.body
      button type: "submit", "Publish"
```

### Rendering Sub-Widgets

Break complex pages into composable pieces:

```moonscript
-- widgets/post_card.moon
import Widget from require "lapis.html"

class PostCard extends Widget
  content: =>
    div class: "card", ->
      h3 @post.title
      p @post.excerpt
      a href: @url_for("post", @post), "Read more"
```

```moonscript
-- views/index.moon
import Widget from require "lapis.html"
import PostCard from require "widgets.post_card"

class Index extends Widget
  content: =>
    h1 "Latest Posts"
    for post in *@posts
      widget PostCard post: post
```

---

## 4. The Layout

The layout wraps every page. `@content_for "inner"` is where the current view's output goes.

```moonscript
-- views/layout.moon
import Widget from require "lapis.html"

class Layout extends Widget
  content: =>
    html_5 ->
      head ->
        meta charset: "utf-8"
        title @title or "My App"
        link rel: "stylesheet", href: "/static/app.css"
        -- Views can inject extra <head> content:
        @content_for "head"
      body ->
        div id: "nav", ->
          a href: "/", "Home"
          if @current_user
            span "Hello, #{@current_user.username}"
            a href: @url_for("logout"), "Log out"
          else
            a href: @url_for("login"), "Log in"
        div id: "main", ->
          @content_for "inner"   -- the page view goes here
        div id: "footer", "© My App 2026"
      script src: "/static/app.js"
```

### Sending Data from View to Layout

A view can push content into named slots in the layout:

```moonscript
class MyPage extends Widget
  content: =>
    -- Set the page title
    @content_for "title", "My Page Title"
    
    -- Inject a stylesheet into <head>
    @content_for "head", ->
      link rel: "stylesheet", href: "/static/special.css"
    
    -- The main content:
    h1 "Hello"
```

The layout reads it with `@content_for "title"` (no second arg).

### Disabling the Layout

```moonscript
-- Globally for a route:
"/plain": =>
  layout: false, "plain text"

-- For JSON (layout is auto-disabled):
"/api": =>
  json: { ok: true }
```

---

## 5. Models

Each model class maps to a database table. The class name determines the table name: `Users` → `users`, `UserPosts` → `user_posts`.

### Defining a Model

```moonscript
-- models/users.moon
import Model, enum from require "lapis.db.model"

class Users extends Model
  @timestamp: true           -- auto created_at / updated_at

  @primary_key: "id"         -- default; can be "email" or {"a","b"}

  @constraints: {
    username: (value) =>
      if #value < 3
        "Username too short"
      elseif #value > 30
        "Username too long"
  }

  @relations: {
    {"posts", has_many: "Posts"}
    {"profile", has_one: "UserProfiles"}
  }

  -- Instance methods
  display_name: =>
    @name or @username

  is_admin: =>
    @role == 2

  -- For url_for to work with model instances
  url_params: (req, ...) =>
    "user_profile", { username: @username }, ...

  -- Class method (called as Users\find_active!)
  @find_active: =>
    @select "where active = true order by created_at desc"
```

### The Models Hub (`models.moon`)

This pattern (used by streak.club) lets models reference each other by name without circular requires:

```moonscript
-- models.moon
import Users from require "models.users"
import Posts from require "models.posts"
import Tags from require "models.tags"

{ :Users, :Posts, :Tags }
```

In `relations`, Lapis calls `require("models")["Users"]` by default. So always reference other models as strings in relations: `belongs_to: "Users"`.

### CRUD Operations

```moonscript
-- Find one
user = Users\find 42
user = Users\find email: "igor@example.com"

-- Find many
users = Users\select "where active = true order by username"
users = Users\select "where created_at > ? limit ?", some_date, 20

-- Create
user = Users\create {
  username: "igor"
  email: "igor@example.com"
}

-- Update
user\update { username: "igor_new" }

-- Also valid:
user.username = "igor_new"
user\update "username"

-- Delete
if user\delete!
  -- deletion happened; safe to run side effects

-- Count
n = Users\count "active = true"
```

### Relations in Practice

```moonscript
class Posts extends Model
  @relations: {
    -- FK: posts.user_id → users.id
    {"user", belongs_to: "Users"}

    -- FK: tags.post_id → posts.id
    {"tags", has_many: "Tags"}

    -- FK: posts.category_id → categories.id, stored as @category
    {"category", belongs_to: "Categories"}
  }
```

```moonscript
-- In an action or flow:
post = Posts\find 1
user = post\get_user!    -- SELECT * FROM users WHERE id = post.user_id
tags = post\get_tags!    -- SELECT * FROM tags WHERE post_id = post.id
```

### Preloading Relations (Avoid N+1)

```moonscript
import preload from require "lapis.db.model"

posts = Posts\select "order by id desc limit 20"

-- Load user and tags for all posts in 2 queries total
preload posts, "user", "tags"

-- Load nested: posts → user → user's profile
preload posts, user: "profile"

-- Now iterate without any additional queries:
for post in *posts
  print post.user.username
  for tag in *post.tags
    print tag.name
```

### Pagination

```moonscript
-- Offset-based (page numbers)
pager = Posts\paginated "where user_id = ? order by id desc", user_id, per_page: 10

page1 = pager\get_page 1
total_pages = pager\num_pages!
total_items = pager\total_items!

-- Cursor-based (efficient for large datasets, no page numbers)
import OrderedPaginator from require "lapis.db.pagination"

pager = OrderedPaginator Posts, "id", "where user_id = ?", user_id, {
  per_page: 20
  order: "desc"
}

results, next_cursor = pager\get_page!
results_p2 = pager\get_page next_cursor
```

---

## 6. Flows (Business Logic)

Flows keep your action functions small and your code testable. They wrap the request object, proxying fields and methods transparently.

```moonscript
-- flows/accounts.moon
import Flow from require "lapis.flow"
import capture_errors, yield_error from require "lapis.application"

class AccountsFlow extends Flow
  -- These assignments will appear on the request object too
  @expose_assigns: {"current_user"}

  login: =>
    yield_error "Email required" unless @params.email
    yield_error "Password required" unless @params.password

    user = (require "models").Users\find email: @params.email
    yield_error "Invalid credentials" unless user
    yield_error "Invalid credentials" unless user\check_password @params.password

    @session.user_id = user.id
    redirect_to: @url_for "dashboard"

  logout: =>
    @session.user_id = nil
    redirect_to: @url_for "home"

  register: =>
    yield_error "Username required" unless @params.username
    yield_error "Password required" unless @params.password

    existing = (require "models").Users\find username: @params.username
    yield_error "Username taken" if existing

    user = (require "models").Users\create {
      username: @params.username
      email: @params.email
      password: hash_password @params.password
    }

    @session.user_id = user.id
    redirect_to: @url_for "dashboard"
```

```moonscript
-- In app.moon:
import capture_errors from require "lapis.application"

class App extends lapis.Application
  [login: "/login"]: capture_errors respond_to {
    on_error: => render: "errors.form", status: 422
    GET: => render: true
    POST: => @flow("accounts")\login!
  }

  [register: "/register"]: capture_errors respond_to {
    on_error: => render: "errors.form", status: 422
    GET: => render: true
    POST: => @flow("accounts")\register!
  }

  [logout: "/logout"]: =>
    @flow("accounts")\logout!
```

---

## 7. Input Validation

Use `assert_valid` or `yield_error` for request parameter checking:

```moonscript
import assert_valid from require "lapis.validate"
import capture_errors, yield_error from require "lapis.application"

class App extends lapis.Application
  [create_post: "/posts"]: capture_errors respond_to {
    on_error: =>
      -- @errors is an array of error strings
      render: true, status: 422

    POST: =>
      assert_valid @params, {
        {"title",   exists: true, min_length: 1, max_length: 200}
        {"body",    exists: true, min_length: 10}
        {"tags",    optional: true, max_length: 500}
      }

      post = (require "models").Posts\create {
        user_id: @current_user.id
        title:   @params.title
        body:    @params.body
      }
      redirect_to: @url_for "post", post
  }
```

---

## 8. Sessions and Cookies

Sessions are signed JSON cookies — safe from tampering:

```moonscript
-- Store something
@session.user_id = 42
@session.preferences = { theme: "dark" }

-- Read it
user_id = @session.user_id

-- Clear it
@session.user_id = nil
```

Raw cookies (unsigned strings):

```moonscript
-- Read
last_seen = @cookies.last_seen

-- Write
@cookies.last_seen = tostring os.time!
```

---

## 9. Error Pages

```moonscript
-- views/errors/not_found.moon
import Widget from require "lapis.html"

class NotFound extends Widget
  content: =>
    div class: "error-page", ->
      h1 "404 — Page Not Found"
      p "The page you're looking for doesn't exist."
      a href: @url_for("home"), "← Go home"
```

```moonscript
-- In app.moon:
class App extends lapis.Application
  handle_404: =>
    status: 404, render: "errors.not_found"

  handle_error: (err, trace) =>
    ngx.log ngx.ERR, "Error: #{err}"
    status: 500, render: "errors.server_error"
```

---

## 10. URL Generation

```moonscript
-- In any action or widget:
@url_for "home"
@url_for "user_profile", username: "igor"
@url_for "post", id: 42
@url_for some_user_instance      -- calls some_user:url_params(@)

-- With query params:
@url_for "search", nil, q: "lapis", page: 2
-- → /search?q=lapis&page=2

-- Absolute URL:
@build_url @url_for("home")
-- → http://myapp.com/
```

---

## 11. JSON APIs

```moonscript
class App extends lapis.Application
  [api_posts: "/api/posts"]: =>
    posts = (require "models").Posts\select "order by id desc limit 20"
    json: posts

  [api_post: "/api/posts/:id"]: =>
    post = (require "models").Posts\find @params.id
    return json: { error: "not found" }, status: 404 unless post
    json: post

  [api_create: "/api/posts"]: respond_to {
    POST: =>
      -- parse JSON body
      import json_params from require "lapis.application"
      @params = json_params @
      post = (require "models").Posts\create @params.post
      json: post, status: 201
  }
```

---

## 12. Migrations (`migrations.moon`)

Database schema changes are tracked as timestamped entries:

```moonscript
import create_table, add_column, create_index, drop_column, types from require "lapis.db.schema"

{
  [1700000001]: =>
    create_table "users", {
      {"id",         types.serial}
      {"username",   types.varchar}
      {"email",      types.varchar}
      {"password",   types.varchar}
      {"is_admin",   types.boolean, default: false, null: false}
      {"created_at", types.time, null: false}
      {"updated_at", types.time, null: false}
      "PRIMARY KEY (id)"
    }
    create_index "users", "username", unique: true
    create_index "users", "email",    unique: true

  [1700000002]: =>
    create_table "posts", {
      {"id",         types.serial}
      {"user_id",    types.foreign_key, null: false}
      {"title",      types.varchar}
      {"body",       types.text}
      {"published",  types.boolean, default: false, null: false}
      {"created_at", types.time, null: false}
      {"updated_at", types.time, null: false}
      "PRIMARY KEY (id)"
    }
    create_index "posts", "user_id"

  [1700000003]: =>
    add_column "users", "display_name", types.varchar
}
```

Run migrations with `lapis migrate`.

---

## Common Patterns from streak.club

These patterns appear throughout the streak.club codebase and represent real-world Lapis idioms:

### Auth Guard Before Filter

```moonscript
class App extends lapis.Application
  @before_filter =>
    if @session.user_id
      @current_user = (require "models").Users\find @session.user_id

  -- Protect specific routes in a sub-app:
  -- applications/dashboard.moon
  @before_filter =>
    unless @current_user
      @write redirect_to: @url_for "login"
```

### Flash Messages

```moonscript
-- Set a flash (survives one redirect)
@session.flash = "Your post was published!"

-- In the layout, read and clear it:
if msg = @session.flash
  @session.flash = nil
  div class: "flash", msg
```

### Nested Params from Forms

HTML forms can send nested params using `field[subfield]` naming:

```html
<input name="post[title]" />
<input name="post[body]" />
```

In the action:

```moonscript
POST: =>
  -- @params.post is { title: "...", body: "..." }
  Posts\create @params.post
```

### Widgets with Helpers (Mixin Pattern)

```moonscript
-- helpers/html_helpers.moon
class HtmlHelpers
  format_date: (date) =>
    -- format it however you like
    date\fmt "%B %d, %Y"

  truncate: (str, len=100) =>
    if #str > len
      str\sub(1, len) .. "…"
    else
      str
```

```moonscript
-- views/post_list.moon
import Widget from require "lapis.html"
import HtmlHelpers from require "helpers.html_helpers"

class PostList extends Widget
  @include HtmlHelpers

  content: =>
    for post in *@posts
      div class: "post", ->
        h2 post.title
        p @truncate post.body, 200
        span class: "date", @format_date post.created_at
```

### Reusable Pagination Widget

```moonscript
-- widgets/pager.moon
import Widget from require "lapis.html"

class Pager extends Widget
  content: =>
    return unless @pager
    total = @pager\num_pages!
    return unless total > 1

    current = @page or 1
    div class: "pagination", ->
      if current > 1
        a href: @url_for(@route_name, nil, page: current - 1), "← Prev"
      for i = 1, total
        if i == current
          span class: "current", i
        else
          a href: @url_for(@route_name, nil, page: i), i
      if current < total
        a href: @url_for(@route_name, nil, page: current + 1), "Next →"
```

---

## Quick Reference

### Action Return Values

| Return | Effect |
|--------|--------|
| `"string"` | Render string, wrapped in layout |
| `render: true` | Load view named after route |
| `render: "view_name"` | Load `views/view_name.moon` |
| `json: table` | JSON response, no layout |
| `redirect_to: url` | 302 redirect |
| `status: 404` | Set status code |
| `layout: false` | Disable layout wrapping |
| `layout: "other"` | Use different layout |

### Model Quick Reference

| Operation | Code |
|-----------|------|
| Find by PK | `Model\find 42` |
| Find by column | `Model\find email: "a@b.com"` |
| Select many | `Model\select "where active = true"` |
| Create | `Model\create { field: value }` |
| Update instance | `record\update { field: value }` |
| Delete instance | `record\delete!` |
| Count | `Model\count "active = true"` |
| Paginate | `Model\paginated "where ...", per_page: 20` |
| Preload | `preload records, "relation"` |

### HTML Builder Quick Reference

| MoonScript | HTML Output |
|------------|-------------|
| `div "text"` | `<div>text</div>` |
| `div class: "x", -> p "y"` | `<div class="x"><p>y</p></div>` |
| `input type: "text"` | `<input type="text" />` |
| `text "str"` | escaped text node |
| `raw "<br/>"` | unescaped HTML |
| `widget SomeWidget x: 1` | render sub-widget with params |
| `@content_for "slot", -> ...` | push to layout slot |
