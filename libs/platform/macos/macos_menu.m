#import <AppKit/AppKit.h>

#include "macos_local.h"

typedef struct lua_State lua_State;

@interface                  MenuCommand : NSObject
@property lua_State        *state;
@property(retain) NSString *callback;
@end


@implementation MenuCommand
- (id)initWithState:(lua_State *)state withCallback:(NSString *)name
{
    self.state    = state;
    self.callback = name;
    return self;
}
- (void)perform:(id)sender
{
    // lua_getglobal(self.state, "PerformCommand");
    // lua_pushstring(self.state, [self.callback UTF8String]);
    // if (lua_pcall(self.state, 1, LUA_MULTRET, 0) != LUA_OK)
    // {
    //     fprintf(stderr, "%s\n", lua_tostring(self.state, -1));
    //     lua_pop(self.state, 1);
    // }
}
//- (bool)validateMenuItem:(NSMenuItem *)menuItem
//{
//    lua_getglobal(self.state, "ValidateCommand");
//    lua_pushstring(self.state, [self.callback UTF8String]);
//    if (lua_pcall(self.state, 1, LUA_MULTRET, 0) != LUA_OK)
//    {
//        fprintf(stderr, "%s\n", lua_tostring(self.state, -1));
//        lua_pop(self.state, 1);
//        return NO;
//    }
//    else
//    {
//        return lua_toboolean(self.state, -1);
//    }
//}
@end

int API_AddMenu(lua_State *L) {
    //    static int menuInitialized = 0;
    //
    //    NSMenu *mainMenu = nil;
    //    NSString *cmd = [[NSString alloc]
    //    initWithUTF8String:luaL_checkstring(L, 1)]; NSString* stroke =
    //    [[NSString alloc] initWithUTF8String:luaL_checkstring(L, 2)]; NSArray
    //    *cmdTokens = [cmd componentsSeparatedByString:@":"]; NSArray*
    //    strokeTokens = [stroke componentsSeparatedByString:@"+"];;
    //
    //    if ([cmdTokens count] != 2) {
    //        return 0;
    //    }
    //
    //    if (!menuInitialized) {
    //        mainMenu = [[NSMenu new] autorelease];
    //        NSMenu *menu = [[[NSMenu alloc] initWithTitle:@"AppKit"]
    //        autorelease]; NSMenuItem *menuItem = [[NSMenuItem new]
    //        autorelease];
    //
    //        [menuItem setSubmenu:menu];
    //        [menuItem setTitle:@"AppKit"];
    //        [mainMenu addItem:menuItem];
    //
    //        [NSApp setMainMenu:mainMenu];
    //
    //        menuInitialized = 1;
    //    } else {
    //        mainMenu = [NSApp mainMenu];
    //    }
    //
    //    NSString *menuNameString = [cmdTokens objectAtIndex:0];
    //    NSString *itemNameString = [cmdTokens objectAtIndex:1];
    //    NSMenuItem *commandMenuItem = [[NSMenuItem alloc]
    //    initWithTitle:itemNameString action:@selector(terminate:)
    //    keyEquivalent:@"q"]; NSEventModifierFlags modflags = 0;
    //
    //    for (int i = 0; i < [strokeTokens count]; i++) {
    //        if ([[strokeTokens objectAtIndex:i] isEqualTo:@"shift"]) {
    //            modflags |= NSEventModifierFlagShift;
    //        }
    //        if ([[strokeTokens objectAtIndex:i] isEqualTo:@"alt"]) {
    //            modflags |= NSEventModifierFlagOption;
    //        }
    //        if ([[strokeTokens objectAtIndex:i] isEqualTo:@"ctrl"]) {
    //            modflags |= NSEventModifierFlagCommand;
    //        }
    //        if ([[strokeTokens objectAtIndex:i] isEqualTo:@"cmd"]) {
    //            modflags |= NSEventModifierFlagControl;
    //        }
    //    }
    //
    //    MenuCommand* target = [[[MenuCommand alloc] initWithState:L
    //    withCallback:cmd] autorelease];
    //
    //    [commandMenuItem setKeyEquivalent:[strokeTokens lastObject]];
    //    [commandMenuItem setKeyEquivalentModifierMask: modflags];
    //    [commandMenuItem setAction:@selector(perform:)];
    //    [commandMenuItem setTarget:target];
    //
    //    for (int i = 0; i < [mainMenu numberOfItems]; i++) {
    //        NSMenuItem *menuItem = [mainMenu itemAtIndex:i];
    //        NSString *menuTitle = [menuItem title];
    //        if ([menuTitle isEqualTo: menuNameString] && [menuItem submenu])
    //        {
    //            [[menuItem submenu] addItem:commandMenuItem];
    //            return 0;
    //        }
    //    }
    //
    //    NSMenu *menu = [[[NSMenu alloc] initWithTitle:menuNameString]
    //    autorelease]; NSMenuItem *menuItem = [[NSMenuItem new] autorelease];
    //
    //    [menu addItem:commandMenuItem];
    //    [menuItem setSubmenu:menu];
    //    [menuItem setTitle:menuNameString];
    //    [mainMenu addItem:menuItem];

    return 0;
}
