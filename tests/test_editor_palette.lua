-- Appearance changes restyle retained inspector controls and keep text legible.
local test = require 'orca.test'
local orca = require 'orca'
local core = require 'orca.core'
local ui = require 'orca.UIKit'
local fs = require 'orca.filesystem'
local geometry = require 'orca.geometry'
orca.init()
fs.init('apps/Editor')
require('orca.core.startup').load_plugins()

local function luminance(color)
    local function linear(channel)
        return channel <= 0.04045 and channel / 12.92 or ((channel + 0.055) / 1.055) ^ 2.4
    end
    return 0.2126 * linear(color.R) + 0.7152 * linear(color.G) + 0.0722 * linear(color.B)
end
local function contrast(foreground, background)
    local a, b = luminance(foreground), luminance(background)
    return (math.max(a, b) + 0.05) / (math.min(a, b) + 0.05)
end
local function distance(a, b)
    return math.abs(a.R - b.R) + math.abs(a.G - b.G) + math.abs(a.B - b.B)
end
local function readable(node, background, label)
    test.expect(contrast(node.ForegroundColor, background) >= 4.5, label .. ' retains readable text contrast')
end

local screen = ui.Screen {
    Width = 720, Height = 540, ResizeMode = 'NoResize', Appearance = 'Light',
    StyleSheet = ui.loadObjectFromCss('apps/Editor/Styles/editor.css'), class = 'editor',
}
local panel = screen + ui.Grid { class = 'editor-panel' }
local label = panel + ui.TextBlock { Text = 'Padding', class = 'property-label' }
local readonly = panel + ui.TextBlock { Text = 'Read-only value', class = 'property-readonly' }
local input = panel + ui.Input { Text = 'Uncommitted draft', class = 'property-input' }
local choice = panel + ui.Button { Text = 'Stretch', class = 'property-enum' }
local resource = panel + ui.Button { Text = 'Inter Regular', class = 'property-reference' }
local records = core.DataObject { Name = 'Records' }
records:addChild(core.DataObject { Name = 'First' })
records:addChild(core.DataObject { Name = 'Second' })
local template = ui.Grid { Height = 28, class = 'property-row property-bound' }
template:addChild(ui.VectorView { Name = 'Bound', class = 'bind-icon' })
template:addChild(ui.TextBlock { Name = 'Label', Text = 'Selected property', class = 'property-label' })
local list = panel + ui.ListBox { ItemsSource = records, ItemTemplate = template }
core.flushQueue()
local row = assert(list:getFirstChild(), 'Native list creates the property row')
local bound = assert(row:findChild('Bound'))
local selectedLabel = assert(row:findChild('Label'))
local accent = geometry.Color.parse(fs.getThemeValue('$ui-accent'))
local lightSurface

for _, appearance in ipairs { 'Light', 'Dark', 'Light' } do
    screen.Appearance = appearance
    core.flushQueue()
    screen:send('StyleController.ThemeChanged', { recursive = true })
    core.flushQueue()
    local surface = panel.BackgroundColor
    readable(panel, surface, appearance .. ' panel')
    readable(label, surface, appearance .. ' property label')
    readable(readonly, surface, appearance .. ' read-only value')
    readable(input, input.BackgroundColor, appearance .. ' text input')
    readable(choice, choice.BackgroundColor, appearance .. ' dropdown')
    readable(resource, resource.BackgroundColor, appearance .. ' resource link')
    test.expect(distance(input.BackgroundColor, choice.BackgroundColor) > 0.01,
        appearance .. ' dropdown has a distinct filled appearance')
    test.expect(distance(input.BackgroundColor, resource.BackgroundColor) > 0.01,
        appearance .. ' resource link is visually distinct from a text field')
    test.expect(resource.BorderColor.A > 0.9, appearance .. ' resource link has a visible outline')
    test.expect(distance(bound.ForegroundColor, accent) < 0.02,
        appearance .. ' binding marker keeps its accent colour')
    local selected = row.BackgroundColor
    test.expect(distance(selected, surface) > 0.01, appearance .. ' selected row remains visible')
    readable(selectedLabel, selected, appearance .. ' selected property label')
    row:setHover()
    core.flushQueue()
    test.expect(distance(row.BackgroundColor, selected) < 0.02,
        appearance .. ' hovering a selected property keeps the selection highlight')
    test.expect_eq(input.Text, 'Uncommitted draft', 'Changing appearance preserves the input draft')
    test.expect(list:getFirstChild() == row, 'Changing appearance retains the native list row')
    if appearance == 'Dark' then
        test.expect(luminance(surface) < lightSurface, 'Dark appearance uses a darker panel surface')
    else
        lightSurface = luminance(surface)
    end
end
screen:clear()
print('PASS: editor appearance palettes and retained inspector controls')
