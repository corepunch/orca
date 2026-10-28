local fs = require "orca.filesystem"
local ui = require "orca.UIKit"
local Items = require "orca.UIKit.ItemsView"
local C = require "Chat/views/components"
local colors = C.colors
local auto = 0/0
local MessageItems = {}
local messageColumns = {
    EntryName = "string", HolderName = "string", BubbleName = "string", ActionsName = "string", TimeName = "string",
    Date = "string", EmptyTitle = "string", EmptyHint = "string", Sender = "string", Body = "string", Timestamp = "string",
    Alignment = "string", Tone = "string", Ink = "string", Meta = "string", Dim = "string", Muted = "string", Accent = "string",
    ShowDate = "bool", Empty = "bool", HasMessage = "bool", ShowSender = "bool", HasPhotos = "bool", ShowCaption = "bool",
    HasBody = "bool", CanAct = "bool", IncomingTail = "bool", OutgoingTail = "bool",
    Gap = "float", LeftInset = "float", RightInset = "float", BubbleWidth = "float", Inset = "float",
    VerticalInset = "float", PhotoInset = "float", PhotoVerticalInset = "float", SectionGap = "float",
}
local photoColumns = { FrameName = "string", ContentName = "string", TimeName = "string", Timestamp = "string",
    Soft = "string", Muted = "string", ShowTime = "bool", Height = "float" }

function MessageItems.new(app, node)
    local room, epoch, navigation = app.room, app.sessionEpoch, app.navigationEpoch
    local user = app.client:getSession().user.id
    local function current()
        local session = app.client:getSession()
        return app.room == room and app.messageList == node and app.sessionEpoch == epoch
            and app.navigationEpoch == navigation and session and session.user.id == user
    end
    local photoTemplate = assert(fs.loadObject("Chat/Templates/Photo"))
    return Items.new {
        node = node, template = assert(fs.loadObject("Chat/Templates/Message")),
        record = Items.record("ChatMessagePresentation", messageColumns),
        key = function(row) return row.key end,
        fields = function(row) return row.fields end,
        update = function(entry, row, created)
            if created then
                entry.holder = entry.node:findChild("MessageHolder", true)
                entry.bubble = entry.node:findChild("MessageBubble", true)
                entry.button = entry.node:findChild("MessageActions", true)
                entry.time = entry.node:findChild("MessageTime", true)
                entry.photos = Items.new {
                    node = entry.node:findChild("Photos", true), template = photoTemplate,
                    record = Items.record("ChatPhotoPresentation", photoColumns),
                    key = function(photo) return photo.attachment.id end,
                    fields = function(photo) return photo.fields end,
                    update = function(item, photo, fresh)
                        if fresh then
                            item.time = item.node:findChild("PhotoTime", true)
                            item.node:getFirstChild().Name = photo.fields.ContentName
                            item.open = ui.Button { Name = "OpenPhoto" .. photo.attachment.id,
                                Background = "#00000000", BorderWidth = "0" }
                            item.node:addChild(item.open)
                        end
                        item.node.Name, item.time.Name = photo.fields.FrameName, photo.fields.TimeName
                        local attachment = photo.attachment
                        item.open.Click = function() if current() then app:openPhoto(attachment) end end
                        local revision = attachment.updated_at or ""
                        if not fresh and item.revision == revision and not item.failed then return end
                        item.revision, item.failed = revision, false
                        item.open.IgnoreHitTest = false
                        local content = item.node:getFirstChild()
                        while content:getFirstChild() do content:getFirstChild():removeFromParent() end
                        if photo.path then
                            content:addChild(ui.ImageView { Name = "Photo" .. attachment.id, Src = photo.path,
                                Foreground = "#ffffff", Stretch = "UniformToFill", IgnoreHitTest = true })
                        else
                            content:addChild(C.text("Loading photo…", { FontSize = 12, Foreground = colors.muted,
                                TextHorizontalAlignment = "Center", TextVerticalAlignment = "Center" }))
                            app.messagePhotos:watch(content, attachment, function(path, err)
                                if not current() or item.revision ~= revision then return end
                                item.failed = not path and err ~= nil
                                item.open.IgnoreHitTest = item.failed
                                local active = app.timelineSnapshot
                                if path and active then active.photos[attachment.id] = { updated_at = attachment.updated_at, path = path } end
                                if active then
                                    active.failedPhotos[attachment.id] = item.failed or nil
                                    active.retryPhotos = next(active.failedPhotos) ~= nil
                                end
                            end)
                        end
                    end,
                }
            end
            entry.node.Name, entry.holder.Name = row.fields.EntryName, row.fields.HolderName
            entry.bubble.Name, entry.button.Name = row.fields.BubbleName, row.fields.ActionsName
            entry.time.Name = row.fields.TimeName
            entry.photos:refresh(row.photos)
            -- The template owns the hit target. Its callback reads this refresh's
            -- message and permissions rather than a stale service record.
            if entry.button then
                -- Keep the photo's tap target clear; message commands remain
                -- available through a compact menu button over its corner.
                local photos = #row.photos > 0
                entry.button.Width, entry.button.Height = photos and 44 or auto, photos and 44 or auto
                entry.button.MarginLeft, entry.button.MarginBottom = photos and auto or 0, photos and auto or 0
                entry.button.Background = photos and "#00000099" or "#00000000"
                entry.button.Foreground = "#ffffff"
                entry.button.TextHorizontalAlignment, entry.button.TextVerticalAlignment = "Center", "Center"
                entry.button.FontSize = 22
                entry.button:setTextContent(photos and "…" or "")
                entry.button.Click = function()
                    if current() and (row.canEdit or row.canDelete) then app:openMessageActions(row.message, row.canEdit, row.canDelete) end
                end
            end
        end,
    }
end

return MessageItems
