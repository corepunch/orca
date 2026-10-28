local ui = require "orca.UIKit"
local C = require "Chat/views/components"
local Viewer = {}
local auto = 0/0

function Viewer.close(app)
    local viewer = app.photoViewer
    if not viewer then return end
    app.photoViewer = nil
    viewer.root:removeFromParent()
    viewer.screen.ClearColor = viewer.background
end

function Viewer.open(app, attachment)
    local session = app.client:getSession()
    if not app.screen or not session or not attachment then return end
    Viewer.close(app)
    local screen, epoch, navigation, room = app.screen, app.sessionEpoch, app.navigationEpoch, app.room
    local viewer = { screen = screen, background = screen.ClearColor, attempt = 0 }
    local root = ui.Grid { Name = "PhotoViewer", Rows = "48px 1fr", Background = "#000000",
        ContentInsetAdjustment = "Always" }
    viewer.root, app.photoViewer = root, viewer
    local function current()
        local active = app.client:getSession()
        return app.photoViewer == viewer and app.screen == screen and app.room == room
            and app.sessionEpoch == epoch and app.navigationEpoch == navigation
            and active and active.user.id == session.user.id
    end
    local bar = ui.Grid { Columns = "48px 1fr" }
    local close = C.iconButton("arrow-left", function() if current() then Viewer.close(app) end end,
        { Name = "ClosePhotoViewer", Width = 48, Height = 48, Foreground = "#ffffff" })
    bar:addChild(close)
    bar:addChild(C.text("photo", { FontSize = 22, Foreground = "#ffffff", TextVerticalAlignment = "Center" }))
    root:addChild(bar)
    local host = ui.Grid { Name = "PhotoViewerContent", Rows = "1fr", Columns = "1fr" }
    root:addChild(host)
    screen.ClearColor = "#000000"
    screen:addChild(root)
    close:setFocus()
    local function clear()
        while host:getFirstChild() do host:getFirstChild():removeFromParent() end
    end
    local function display(path)
        local image = ui.ImageView { Name = "PhotoViewerImage", Src = path, Foreground = "#ffffff",
            Stretch = "Uniform", IgnoreHitTest = true,
            MarginLeft = auto, MarginRight = auto, MarginTop = auto, MarginBottom = auto }
        image:send("Node.LoadView")
        if not image.Source then return false end
        clear()
        viewer.path = path
        host:addChild(image)
        -- ImageView also fits its frame during arrangement. Give auto margins
        -- the final fitted size so centering does not use its intrinsic pixels.
        local ratio = (attachment.width or attachment.thumbnail_width or 4)
            / (attachment.height or attachment.thumbnail_height or 3)
        local availableWidth = "MAX(0, {Screen.Width} - {Screen.SafeAreaLeft} - {Screen.SafeAreaRight})"
        local availableHeight = "MAX(0, {Screen.Height} - {Screen.SafeAreaTop} - MAX({Screen.SafeAreaBottom}, {Screen.KeyboardInsetBottom}) - 48)"
        local width = "MIN(" .. availableWidth .. ", " .. availableHeight .. " * " .. ratio .. ")"
        image:attachPropertyProgram("Node.Width", width, "OneWay", true)
        image:attachPropertyProgram("Node.Height", "(" .. width .. ") / " .. ratio, "OneWay", true)
        return true
    end
    local load
    local function failed(err)
        if not viewer.path then clear() end
        local loading = host:findChild("PhotoViewerLoading", true)
        if loading then loading:removeFromParent() end
        host:addChild(C.stack({ Name = "PhotoViewerError", Spacing = 16, JustifyContent = "Center", Padding = "24",
            MarginTop = viewer.path and auto or 0, Background = "#000000c0" }, {
            C.text(type(err) == "table" and err.message or "Could not load this photo.", {
                WordWrap = true, Foreground = "#ffffff", TextHorizontalAlignment = "Center" }),
            C.button("retry", load, { Name = "RetryPhotoViewer", Foreground = "#ffffff",
                Width = 120, MarginLeft = auto, MarginRight = auto }),
        }))
    end
    load = function()
        if not current() then return end
        viewer.attempt = viewer.attempt + 1
        local attempt = viewer.attempt
        local cached = app.messagePhotos and app.messagePhotos:cached(attachment)
        if cached and display(cached) then return end
        local preview = app.messagePhotos and app.messagePhotos.preview and app.messagePhotos:preview(attachment)
        if viewer.path then display(viewer.path)
        elseif preview then display(preview)
        else clear() end
        host:addChild(ui.ActivityIndicator { Name = "PhotoViewerLoading", Foreground = "#ffffff" })
        app:load(function()
            if not current() or viewer.attempt ~= attempt then return end
            local path, err
            local ok = pcall(function()
                local bytes
                bytes, err = app.client:getPhoto(attachment, false, function(previewBytes)
                    if not current() or viewer.attempt ~= attempt then return false end
                    local media = app:getMedia()
                    if media then
                        local previewPath = media:storePreview(previewBytes,
                            "message-photo-preview-" .. attachment.id .. ":" .. tostring(attachment.updated_at or ""))
                        if previewPath and display(previewPath) then
                            if app.repaint then app:repaint()
                            else screen:post("Window.Paint", require("orca.renderer").getSize()) end
                        end
                    end
                end)
                if not current() or viewer.attempt ~= attempt then return end
                local media = app:getMedia()
                if bytes and media then
                    path, err = media:storePreview(bytes,
                        "message-photo-" .. attachment.id .. ":" .. tostring(attachment.updated_at or ""))
                end
            end)
            if not current() or viewer.attempt ~= attempt then return end
            if not (ok and path and display(path)) then failed(err) end
        end)
    end
    load()
    return root
end

return Viewer
