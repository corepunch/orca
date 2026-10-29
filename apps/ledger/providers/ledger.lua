-- The ledger's provider over deterministic fixtures (orca.data.fixtures):
-- the same contract a service-backed provider would offer. Only the domain
-- rules are code: balances, formatting, validation and adding an entry.
local Fixtures = require "orca.data.fixtures"
local Provider = {}

local function money(cents)
    cents = cents or 0
    local sign = cents < 0 and "-" or ""
    cents = math.abs(cents)
    return ("%s$%d.%02d"):format(sign, cents // 100, cents % 100)
end

-- Dollars and cents typed as text ("-12.50") to cents, or nil.
local function cents(text)
    local sign, whole, fraction = (text or ""):match("^%s*([%-+]?)%$?(%d+)%.?(%d?%d?)%s*$")
    if not whole then return nil end
    local value = tonumber(whole) * 100 + tonumber((fraction .. "00"):sub(1, 2))
    return sign == "-" and -value or value
end
Provider.cents = cents

function Provider.new(data)
    data = data or {
        accounts = { { id = "checking", name = "Checking", kind = "asset" }, { id = "card", name = "Card", kind = "liability" } },
        entries = {
            { id = "e1", account_id = "checking", memo = "Salary", amount = 250000, date = "2026-10-01" },
            { id = "e2", account_id = "checking", memo = "Rent", amount = -120000, date = "2026-10-02" },
            { id = "e3", account_id = "card", memo = "Groceries", amount = -8450, date = "2026-10-03" },
        },
    }
    local function newestFirst()
        table.sort(data.entries, function(a, b) return a.date > b.date or (a.date == b.date and a.id > b.id) end)
        return data.entries
    end
    newestFirst()
    local provider = {
        computed = {
            money = function(values) return money(values.Balance) end,
            entryMoney = function(values) return (values.Amount or 0) > 0 and "+" .. money(values.Amount) or money(values.Amount) end,
            tone = function(values) return (values.Amount or 0) < 0 and "debit" or "credit" end,
        },
        validators = {
            entry = function(values)
                local errors
                if not (values.Memo or ""):match("%S") then errors = { Memo = "Say what the entry is for." } end
                local amount = cents(values.Amount)
                if not amount or amount == 0 then
                    errors = errors or {}
                    errors.Amount = "Enter an amount such as 12.50 or -12.50."
                end
                return errors
            end,
        },
        operations = {
            -- Balances are derived from the entries, as a service would.
            accounts = function()
                local rows = {}
                for index, account in ipairs(data.accounts) do
                    local balance = 0
                    for _, entry in ipairs(data.entries) do
                        if entry.account_id == account.id then balance = balance + entry.amount end
                    end
                    rows[index] = { id = account.id, name = account.name, kind = account.kind, balance = balance }
                end
                return rows
            end,
            addEntry = function(params, _, command)
                local account = command.scope:member("AccountList").selected
                if not account then return nil, { code = "no_account", message = "Choose an account first." } end
                data.sequence = (data.sequence or #data.entries) + 1
                data.entries[#data.entries + 1] = { id = "e" .. data.sequence, account_id = account.key,
                    memo = params.draft.Memo, amount = cents(params.draft.Amount), date = data.today or "2026-10-29" }
                newestFirst()
                return true
            end,
        },
    }
    provider.data = data
    return Fixtures.provider({ entries = data.entries }, provider)
end

return Provider
