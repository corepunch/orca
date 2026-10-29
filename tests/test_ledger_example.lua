-- The authoring example (apps/ledger): a master-detail screen, paging and a
-- validated form built from a schema, a scope declaration, XML and CSS over
-- a fixture provider. Everything is driven through real control events.
local orca = require "orca"
orca.init()
local fs = require "orca.filesystem"
local core = require "orca.core"
require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
fs.init("apps/ledger")
require("orca.core.startup").load_plugins()
local Lint = require "orca.data.lint"
local savedAsync = orca.async
orca.async = function(fn, ...) return fn(...) end
local checks = 0
local function check(value, message) assert(value, message); checks = checks + 1 end

-- Bindings and declarations agree before anything runs.
local problems = Lint.page("Ledger/Pages/Ledger", "Ledger/Data/Ledger.xml")
check(#problems == 0, Lint.format(problems))

local App = require "Ledger/App"
local app = App {}
local screen = app:dispatch("/").view
local function settle()
    core.flushQueue()
    core.advanceFrame()
    core.runAllPrograms(screen)
    core.flushQueue()
    screen:UpdateLayout(390, 844)
end
local function find(name) return assert(screen:findChild(name, true), "Missing " .. name) end
local function press(node) node:send("Node.LeftButtonUp"); settle() end
settle()

check(find("Accountchecking") and find("Accountcard"), "Accounts load into rows named by record")
check(find("Accountchecking"):findChild("Balance", true).Text == "$1300.00", "Balances are computed fields")
check(find("SelectedAccount").Text == "Checking" and find("Entrye2") and find("Entrye1"),
    "The first account is selected and its entries load")
check(not find("AccountDetail"):findChild("Entrye3", true), "Entries follow the selected account")
press(find("Accountcard"))
check(find("SelectedAccount").Text == "Card" and find("Entrye3") and not screen:findChild("Entrye1", true),
    "Selecting an account loads its entries")
check(find("Entrye3"):findChild("Amount", true).Text == "-$84.50", "Amounts format through the schema")

-- The form validates, keeps input on error and adds an entry on success.
press(find("AddEntry"))
check(find("MemoError").Text == "Say what the entry is for." and find("AmountError").Text ~= "",
    "Validation errors bind to their fields")
find("MemoInput").Text = "Coffee"
find("AmountInput").Text = "abc"
press(find("AddEntry"))
check(find("MemoError").Text == "" and find("AmountError").Text ~= "" and find("MemoInput").Text == "Coffee",
    "Correcting a field clears its error; input stays")
find("AmountInput").Text = "-4.20"
press(find("AddEntry"))
local added = app.store:get("LedgerEntry", "e4")
check(added and added.values.Memo == "Coffee" and find("Entrye4"), "A valid entry is added and shows")
check(find("MemoInput").Text == "" and find("AmountInput").Text == "", "A submitted form clears")
check(find("Accountcard"):findChild("Balance", true).Text == "-$88.70" and find("SelectedBalance").Text == "-$88.70",
    "The add command refreshes balances everywhere the account shows")

-- Paging: five entries a page, then more on request.
for i = 1, 6 do
    find("MemoInput").Text = "Item " .. i
    find("AmountInput").Text = "1.00"
    press(find("AddEntry"))
end
local entries = app.scope:member("Entries")
check(entries.items:count() == 5 and find("MoreEntries").Visible, "Entries come five at a time")
press(find("MoreEntries"))
check(entries.items:count() == 8 and not find("MoreEntries").Visible, "More entries load on request")

orca.async = savedAsync
print(("PASS: ledger authoring example (%d checks)"):format(checks))
