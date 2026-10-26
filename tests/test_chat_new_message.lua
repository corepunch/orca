-- Controller-only checks. Native layout/interaction remains in test_chat_app.lua.
local Node = {}
Node.__index = Node
function Node:addChild(child) self.children[#self.children + 1] = child; child.parent = self end
function Node:getFirstChild() return self.children[1] end
function Node:removeFromParent()
    for i, child in ipairs(self.parent.children) do
        if child == self then table.remove(self.parent.children, i); break end
    end
end
function Node:setTextContent(text) self.Text = text end
function Node:getTextContent() return self.Text end
function Node:setFocus() self.focused = true end
function Node:send() if self.Click then self.Click() end end
function Node:findChild(name)
    for _, child in ipairs(self.children) do
        if child.Name == name then return child end
        local found = child:findChild(name)
        if found then return found end
    end
end
local function make(props, text)
    props = props or {}
    props.children, props.Text = {}, text or props.Text or ""
    if props.Visible == nil then props.Visible = true end
    return setmetatable(props, Node)
end
package.preload['orca.UIKit'] = function() return setmetatable({}, { __index = function() return make end }) end
package.preload['orca.system'] = function() return { is_dark_theme = function() return true end } end
package.preload['Chat/theme'] = function() return dofile('apps/chat/theme.lua') end
package.preload['Chat/views/components'] = function() return dofile('apps/chat/views/components.lua') end
local Conversations = dofile('apps/chat/views/conversations.lua')
local app = { navigationEpoch = 1, sessionEpoch = 1, models = { Rooms = {} } }
local creates, searches = {}, 0
local createError, onFind
app.client = {
    getSession = function() return { user = { id = 'me' } } end,
    listContacts = function() return { { id = 'alex', display_name = 'Alex', username = 'alex' } } end,
    findPeople = function(_, query)
        searches = searches + 1
        if onFind then onFind() end
        if query == '@alex' then return { { id = 'alex', display_name = 'Alex', username = 'alex' } } end
        return {}
    end,
    findChannels = function() return { { id = 'news', name = 'News' } } end,
    joinPublicChannel = function() return { id = 'news', kind = 'channel' } end,
    joinInvite = function(_, code)
        if code ~= 'valid' then return nil, { message = 'Invalid invitation' } end
        return { id = 'invited', kind = 'group' }
    end,
}
function app.models.Rooms:create(values)
    creates[#creates + 1] = values
    if createError then return nil, { message = 'Please retry' } end
    return { id = 'room', kind = values.kind, peer_id = values.member_ids[1] }
end
function app:avatarPhoto() end
function app:frame(page) self.screen = page; return page end
function app:openRoom(room) self.opened = room end
local screen = Conversations.newRoom(app)
local function node(name) return assert(screen:findChild(name), name) end
local function click(name) node(name):send() end
local function text(name, value) node(name).Text = value end
assert(node('NewChatActions').Visible and node('Contacts').Visible)
assert(not node('SelectedPeople').Visible and not node('CreateConversation').parent.Visible)
assert(not node('InvitationSetup').Visible and not node('ConversationVisibility').Visible)
click('Contactalex')
assert(app.opened.peer_id == 'alex' and creates[1].kind == 'direct')
assert(#creates[1].member_ids == 1 and creates[1].visibility == 'private')
createError, app.opened = true, nil
click('Contactalex')
assert(not app.opened and node('NewChatStatus').Text == 'Please retry')
createError = false
text('FindPerson', '   '); click('FindPeople'); assert(searches == 0)
text('FindPerson', 'missing'); click('FindPeople')
assert(node('NoPeopleResults').Text:find('public username', 1, true))
assert(node('NoPeopleResults').Text:find("don't need to add a contact", 1, true))
text('FindPerson', '@alex'); node('FindPerson').Submit()
assert(node('FoundPersonalex'))
click('FoundPersonalex'); assert(app.opened.kind == 'direct')
click('KindGroup')
assert(not node('NewChatActions').Visible and node('SelectedPeople').Visible)
assert(not node('ConversationSetup').Visible)
click('Contactalex'); assert(app.newSelection.alex)
click('Contactalex'); assert(not app.newSelection.alex)
click('Contactalex')
text('FindPerson', '@alex'); click('FindPeople')
assert(node('FoundPersonalex').Background == node('Contactalex').Background, 'Selected members stay highlighted in search')
click('GroupNext')
assert(node('ConversationSetup').Visible and not node('PeopleSearch').Visible)
assert(not node('ConversationVisibility').Visible, 'Groups must always remain private')
text('RoomName', 'Draft group'); click('NewChatBack')
assert(node('Contacts').Visible and app.newSelection.alex and node('RoomName').Text == 'Draft group')
click('GroupNext'); text('RoomName', '')
local count = #creates
click('CreateConversation'); assert(#creates == count and node('RoomName').focused)
text('RoomName', 'Our group'); createError = true; click('CreateConversation')
assert(app.newSelection.alex and node('RoomName').Text == 'Our group', 'Failed creation retains draft')
createError = false; click('CreateConversation')
assert(creates[#creates].kind == 'group' and creates[#creates].member_ids[1] == 'alex')
assert(node('NewChatActions').Visible and not next(app.newSelection))
click('KindChannel')
assert(node('ConversationSetup').Visible and not node('Contacts').Visible)
assert(node('ConversationVisibility').Visible and not node('SelectedPeople').Visible)
click('ConversationVisibility'); text('RoomName', 'News'); click('CreateConversation')
assert(creates[#creates].kind == 'channel' and creates[#creates].visibility == 'public')
assert(#creates[#creates].member_ids == 0, 'Channel creation must not subscribe arbitrary contacts')
click('SearchChannels'); text('FindPerson', 'News'); node('FindPerson').Submit()
click('JoinChannelnews'); assert(app.opened.id == 'news')
click('NewChatBack'); click('ShowInvite'); text('InviteCode', 'bad'); click('JoinInvite')
assert(node('NewChatStatus').Text == 'Invalid invitation')
text('InviteCode', 'valid'); click('JoinInvite'); assert(app.opened.id == 'invited')
-- Changing flows or signing out while a lookup yields must discard its response.
onFind = function() text('FindPerson', 'edited while waiting') end
text('FindPerson', '@alex'); click('FindPeople')
assert(not screen:findChild('FoundPersonalex'))
onFind = function() click('KindChannel') end
text('FindPerson', '@alex'); click('FindPeople')
assert(not screen:findChild('FoundPersonalex'))
click('NewChatBack')
onFind = function() app.sessionEpoch = 2 end
text('FindPerson', '@alex'); click('FindPeople')
assert(not screen:findChild('FoundPersonalex'))
print('PASS: new message controller (direct, group, channel, invite, failures, stale responses)')
