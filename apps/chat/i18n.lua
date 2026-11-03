local Localization = require "orca.core.localization"

local strings = Localization.new {
    fallback = "en",
    catalogs = { en = require "Chat/Locales/en", ru = require "Chat/Locales/ru" },
}

-- These nodes are authored page chrome. Never translate conversation or profile
-- names by matching their content against the catalog.
local chrome = {
    InboxContent = { Inbox = { Title = "Messenger" }, FilterAll = { Header = "All" },
        FilterUnread = { Header = "Unread" }, FilterPinned = { Header = "Pinned" } },
    ContactsPage = { ContactsPanorama = { Title = "contacts" }, ContactsRecent = { Header = "recent" },
        ContactsAll = { Header = "all" } },
    AvatarPicker = { AvatarPicker = { Title = "Messenger" } },
}
local apply = strings.apply
function strings:apply(root)
    apply(self, root)
    for name, properties in pairs(chrome[root.Name] or {}) do
        local node = root.Name == name and root or root:findChild(name, true)
        if node then for property, key in pairs(properties) do node[property] = self:t(key) end end
    end
    if root.Name == "AvatarPicker" then root:getFirstChild().Header = self:t("characters") end
    return root
end

function strings:date(iso)
    local y, m, d = (iso or ""):match("^(%d%d%d%d)%-(%d%d)%-(%d%d)")
    if not y then return self:t("CONVERSATION") end
    local stamp = os.time { year = tonumber(y), month = tonumber(m), day = tonumber(d), hour = 12 }
    local weekday = tonumber(os.date("%w", stamp)) + 1
    return self:t("date.weekday." .. weekday) .. ", " .. d .. " " .. self:t("date.month." .. tonumber(m))
end

function strings:initials(name)
    local function letter(value)
        local first = (value or ""):match(utf8.charpattern)
        if not first then return "" end
        local code = utf8.codepoint(first)
        if code >= 0x430 and code <= 0x44f then return utf8.char(code - 32) end
        if code == 0x451 then return utf8.char(0x401) end
        return first:upper()
    end
    local first, last = (name or ""):match("^%s*(%S+)"), (name or ""):match("%s+(%S+)%s*$")
    return (letter(first) .. letter(last)):gsub("^$", "C")
end

return strings
