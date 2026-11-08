-- A public post can travel through the existing text-message protocol. Only
-- the canonical website URL becomes an in-app preview; arbitrary URLs do not.
local Post = {}
local PREFIX = "https://corepunch.github.io/octopus/post.html?id="
local function excerpt(text)
    text = text:gsub("%s+", " ")
    if #text <= 160 then return text end
    local ending = 161
    while ending > 1 and text:byte(ending) >= 128 and text:byte(ending) < 192 do ending = ending - 1 end
    return text:sub(1, ending - 1)
end

function Post.validId(id)
    return type(id) == "string" and #id > 0 and #id <= 36 and id:match("^[%w][%w._%-]*$") ~= nil
end

function Post.url(id)
    return Post.validId(id) and PREFIX .. id or nil
end

function Post.parse(body)
    if type(body) ~= "string" then return nil end
    for candidate in body:gmatch("%S+") do
        if candidate:sub(1, #PREFIX) == PREFIX then
            local id = candidate:sub(#PREFIX + 1)
            if Post.validId(id) then
                local title = body:sub(1, (body:find(candidate, 1, true) or 1) - 1):match("^%s*(.-)%s*$")
                return id, title ~= "" and excerpt(title) or nil
            end
        end
    end
end

function Post.message(post)
    local url = Post.url(post and post.id)
    if not url then return nil end
    local title = post.title or ""
    if not title:match("%S") then title = post.content or "" end
    if not title:match("%S") then title = "Public post" end
    return excerpt(title) .. "\n" .. url
end

return Post
