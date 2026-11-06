-- Exercise startup policy without creating a native window or GL context.
local startupPath = (rawget(_G, 'SHAREDIR') or 'share') .. '/orca/core/startup.lua'

local function check(server, controller)
    local calls, kitLoads, screenLoads = {}, 0, 0
    local screen = {}
    local project = { WindowWidth = 640, WindowHeight = 480 }
    if controller then
        project.StartupViewController = 'Fixture/Controller'
        project.StartupRoute = '/preview'
    else
        project.StartupScreen = 'Fixture/Main'
    end
    local appClass = setmetatable({}, { __call = function() return {} end })
    local modules = {
        orca = { config = {} },
        ['orca.core'] = {},
        ['orca.filesystem'] = {
            init = function(path)
                assert(path == 'fixture-project', 'Startup opens the requested project')
                return project
            end,
            loadObject = function(path)
                assert(path == 'Fixture/Main', 'Screen startup preserves the project screen path')
                screenLoads = screenLoads + 1
                return screen
            end,
        },
        ['orca.renderer'] = {
            init = function(width, height, offscreen)
                calls[#calls + 1] = { width = width, height = height, offscreen = offscreen }
            end,
        },
        ['orca.system'] = { list_dir = function() return function() return nil end end },
        ['orca.UIKit'] = { Screen = function() error('The declared project screen must be loaded') end },
        ['Fixture/Controller'] = function()
            return {
                activate_route = function(self, route)
                    assert(route == '/preview', 'Controller startup preserves the project route')
                    self.route = route
                end,
            }
        end,
    }
    local env = setmetatable({ SERVER = server, SHAREDIR = 'fixture-share' }, { __index = _G })
    env._G = env
    env.require = function(name)
        if name == 'orca.EditorKit' then
            kitLoads = kitLoads + 1
            assert(#calls == 1, 'The renderer is initialized before editor runtime endpoints load')
            return {}
        end
        return assert(modules[name], 'Unexpected startup dependency: ' .. name)
    end
    local Startup = assert(loadfile(startupPath, 't', env))()
    local app = assert(Startup.open(appClass, 'fixture-project'))
    assert(#calls == 1 and calls[1].width == 640 and calls[1].height == 480,
        'Startup initializes one renderer at the project dimensions')
    assert(calls[1].offscreen == (server == true),
        server and 'Editor server startup renders offscreen instead of creating a window'
            or 'Ordinary interactive startup retains its visible renderer window')
    assert(kitLoads == (server and 1 or 0), 'Only editor runtime startup loads editor request endpoints')
    assert(appClass.app == app, 'Startup publishes the initialized application')
    if controller then
        assert(app.route == '/preview' and screenLoads == 0, 'Offscreen mode preserves controller startup')
    else
        assert(app.screen == screen and app.controller.view == screen and screenLoads == 1,
            'Offscreen mode preserves screen startup for Preview rendering')
    end
end

for _, controller in ipairs { false, true } do
    check(nil, controller)
    check(false, controller)
    check(true, controller)
end
print('PASS: server startup renders offscreen; interactive screen/controller startup remains visible')
