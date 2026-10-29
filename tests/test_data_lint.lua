-- Binding diagnostics: every migrated page agrees with its scope and schema,
-- and mistakes are reported with resource, line, node, property and path.
local orca = require "orca"
orca.init()
local fs = require "orca.filesystem"
require("orca.renderer").init(390, 844, true)
local Lint = require "orca.data.lint"
local checks = 0
local function check(value, message) assert(value, message); checks = checks + 1 end

local apps = {
    { "apps/chat", {
        { "Chat/Pages/Inbox", "Chat/Data/Inbox.xml" },
        { "Chat/Pages/Thread", "Chat/Data/Conversation.xml" },
        { "Chat/Pages/Composer", "Chat/Data/Conversation.xml" },
        { "Chat/Pages/Details", "Chat/Data/Details.xml" },
    } },
    { "apps/octopus", {
        { "Octopus/Pages/Home", "Octopus/Data/Feeds.xml" },
        { "Octopus/Pages/Profile", "Octopus/Data/Profile.xml" },
        { "Octopus/Pages/Search", "Octopus/Data/Search.xml" },
        { "Octopus/Pages/Post", "Octopus/Data/Post.xml", { present = { PostDetail = "Post" } } },
        { "Octopus/Pages/Write", "Octopus/Data/Compose.xml" },
        { "Octopus/Pages/ProfileEditor", "Octopus/Data/Me.xml" },
    } },
    { "apps/ledger", { { "Ledger/Pages/Ledger", "Ledger/Data/Ledger.xml" } } },
}
for _, app in ipairs(apps) do
    fs.init(app[1])
    for _, page in ipairs(app[2]) do
        local problems = Lint.page(page[1], page[2], page[3])
        check(#problems == 0, page[1] .. ":\n" .. Lint.format(problems))
    end
end

-- Mistakes name where they are and why.
local problems = Lint.source([[
<StackView>
  <TextBlock Name="Total" Text="{Binding DataContext/Entries/Nope}"/>
  <TextBlock Text="{Binding DataContext/Missing/Items}"/>
  <Button Name="Go" Click="NoSuchHandler"/>
  <Button Click="Entries.Submit"/>
  <ItemsStack ItemsSource="{Binding DataContext/Entries/Items}" ItemTemplate="Ledger/Templates/EntryRow"/>
  <TextBlock Text="{Binding DataContext/NewEntry/Errors/Memo}"/>
  <TextBlock Text="{Binding DataContext/AccountList/Selected/Unknown}"/>
</StackView>]], "Probe.xml", "Ledger/Data/Ledger.xml")
local text = Lint.format(problems)
check(#problems == 5, text)
check(text:find("Probe.xml:2: Total.Text DataContext/Entries/Nope: Query Entries has no state Nope", 1, true), text)
check(text:find("Probe.xml:3: TextBlock.Text DataContext/Missing/Items: the scope has no member Missing", 1, true), text)
check(text:find("Probe.xml:4: Go.Click NoSuchHandler", 1, true), text)
check(text:find("Entries is a Query; Submit applies to Edit members", 1, true), text)
check(text:find("LedgerAccount has no column or relation Unknown", 1, true), text)
print(("PASS: data binding diagnostics (%d checks)"):format(checks))
