-- Shared character portraits. These become ordinary public profile photos,
-- so every client using profiles.avatarId can display the selected character.
local Avatars = { entries = {} }
local choices = {
    { "orca", "Orca" }, { "octopus", "Octopus" },
    { "whale", "Whale" }, { "dolphin", "Dolphin" },
    { "seal", "Seal" }, { "sea-otter", "Sea otter" },
    { "penguin", "Penguin" }, { "sea-turtle", "Sea turtle" },
    { "manta-ray", "Manta ray" }, { "jellyfish", "Jellyfish" },
    { "crab", "Crab" }, { "pufferfish", "Pufferfish" },
    { "seahorse", "Seahorse" }, { "lobster", "Lobster" },
    { "starfish", "Starfish" }, { "squid", "Squid" },
}
local byId = {}
local root = (rawget(_G, "SHAREDIR") or "share") .. "/avatars/"
for _, choice in ipairs(choices) do
    local entry = { id = choice[1], label = choice[2], path = root .. choice[1] .. ".jpg" }
    Avatars.entries[#Avatars.entries + 1], byId[entry.id] = entry, entry
end

function Avatars.load(id)
    local entry = byId[id]
    if not entry then
        return nil, { code = "validation", message = "Choose one of the available characters." }
    end
    -- SHAREDIR follows the installed/bundled runtime, including on iOS.
    local file = io.open(entry.path, "rb")
    local bytes = file and file:read("*a")
    if file then file:close() end
    if not bytes or bytes:sub(1, 3) ~= "\255\216\255" or bytes:sub(-2) ~= "\255\217" then
        return nil, { code = "file", message = "This character could not be loaded. Please try again." }
    end
    return { jpeg = bytes, width = 362, height = 362,
        byte_count = #bytes, mime_type = "image/jpeg", quality = 0.92 }
end

return Avatars
