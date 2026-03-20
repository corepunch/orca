#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#include <mach-o/dyld.h>

#include "macos_local.h"

#include <TargetConditionals.h>

extern bool_t g_refreshAssets;
NSString *appName = @"CorePunch";

static char documents[1024] = { 0 };
static char local[1024] = { 0 };
static char share[1024] = { 0 };

//void Menu_Command(uint32_t eventType);
//void INP_NullEvent(void);

bool_t __openFile = FALSE;
char const *__openFileType = NULL;
void (*__openProjectFile)(char const *);

void WI_OpenFile(char const *ext, void (*callback)(char const *)) {
	__openFile = TRUE;
	__openFileType = ext;
	__openProjectFile = callback;
}

void WI_OpenFolder(void (*callback)(char const *)) {
	__openFile = TRUE;
	__openFileType = NULL;
	__openProjectFile = callback;
}

char const *WI_DocumentsDirectory(void) {
    if (documents[0] == 0) {
        NSFileManager *fileManager = [NSFileManager defaultManager];
        NSArray<NSURL*>* urls = [fileManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask];
        NSString *path = [urls[0] path];
        memcpy(documents, [path UTF8String], MIN([path length] + 1, sizeof(documents)));
    }
	return documents;
}

char const *WI_SettingsDirectory(void) {
    if (local[0] == 0) {
        NSFileManager *fileManager = [NSFileManager defaultManager];
        NSArray<NSURL*>* urls = [fileManager URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask];
        NSString *applicationSupportDirectory = [urls[0] path];
        NSString *path = [applicationSupportDirectory stringByAppendingPathComponent:appName];
        NSError *errorw;
        
        [fileManager createDirectoryAtPath:path
               withIntermediateDirectories:YES
                                attributes:nil
                                     error:&errorw];

        memcpy(local, [path UTF8String], MIN([path length] + 1, sizeof(local)));
    }
	return local;
}


char const *WI_ShareDirectory(void) {
    if (share[0] == 0) {
        unsigned size = sizeof(share);
        _NSGetExecutablePath(share, &size);
        char *c = share + strlen(share) - 1;
#if !TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR
        char const *resource = "/Resources/";
        for (int i = 1;; c--) {
            if (*c == '/') {
                if (i > 0){
                    i--;
                } else {
                    break;
                }
            }
        }
        strcpy(c, resource);
#else
        for (; *c != '/'; c--);
        *(c + 1) = 0;
#endif
    }
    return share;
}

char const *WI_LibDirectory(void) {
    return WI_ShareDirectory();
}

void WI_SetDragContents(char const *contents) {
}

char const *WI_GetPlatform(void) {
    return "macos";
}

//void WI_GetOpenFileName(void (*callback)(char const *)) {
//    // Create and configure the open panel
//    NSOpenPanel *panel = [NSOpenPanel openPanel];
//    [panel setCanChooseFiles:NO];
//    [panel setCanChooseDirectories:YES];
//    [panel setAllowsMultipleSelection:NO];
//    [panel setResolvesAliases:YES];
//    [panel setTitle:@"Select a Folder"];
//    [panel setMessage:@"Please select a folder."];
//    
//    // Display the panel and process the result
//    [panel beginWithCompletionHandler:^(NSModalResponse result) {
//        if (result == NSModalResponseOK) {
//            NSURL *selectedFolder = [[panel URLs] firstObject];
//            NSLog(@"Selected folder: %@", [selectedFolder path]);
//            
//            // Convert the folder path to a C string
//            char const *folderPath = [[selectedFolder path] UTF8String];
//            
//            // Call the C callback with the selected folder path
//            if (callback) {
//                callback(folderPath);
//            }
//        }
//    }];
//}

bool_t WI_GetOpenFileName(struct _WI_OpenFileName const *ofn) {
  @autoreleasepool {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.canChooseFiles = YES;
    panel.canChooseDirectories = NO;
    panel.allowsMultipleSelection = NO;
    NSInteger result = [panel runModal];
    if (result == NSModalResponseOK) {
      NSString *path = [[panel URL] path];
      strncpy(ofn->lpstrFile, [path UTF8String], ofn->nMaxFile);
      ofn->lpstrFile[ofn->nMaxFile - 1] = '\0';
      return TRUE;
    }
    return FALSE;
  }
}

bool_t WI_GetSaveFileName(struct _WI_OpenFileName const *ofn) {
  @autoreleasepool {
    NSSavePanel *panel = [NSSavePanel savePanel];
    NSInteger result = [panel runModal];
    if (result == NSModalResponseOK) {
      NSString *path = [[panel URL] path];
      strncpy(ofn->lpstrFile, [path UTF8String], ofn->nMaxFile);
      ofn->lpstrFile[ofn->nMaxFile - 1] = '\0';
      return TRUE;
    }
    return FALSE;
  }
}

bool_t WI_GetFolderName(struct _WI_OpenFileName const *ofn) {
  @autoreleasepool {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.canChooseFiles = NO;
    panel.canChooseDirectories = YES;
    panel.allowsMultipleSelection = NO;
    NSInteger result = [panel runModal];
    if (result == NSModalResponseOK) {
      NSString *path = [[panel URL] path];
      strncpy(ofn->lpstrFile, [path UTF8String], ofn->nMaxFile);
      ofn->lpstrFile[ofn->nMaxFile - 1] = '\0';
      return TRUE;
    }
    return FALSE;
  }
}

bool_t WI_IsDarkTheme(void) {
  NSAppearance *appearance = [NSApp effectiveAppearance];
  NSAppearanceName bestMatch = [appearance bestMatchFromAppearancesWithNames:@[NSAppearanceNameDarkAqua, NSAppearanceNameAqua]];
  return [bestMatch isEqualToString:NSAppearanceNameDarkAqua] ? TRUE : FALSE;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int WI_ExecuteProcess(char const *process) {
#if 0
    pid_t pid;
    
    // Fork the process
    pid = fork();
    if (pid < 0) {
        // Fork failed
        perror("fork");
        return 1;
    }
    
    if (pid == 0) { // Child process
                    // Define the command and arguments
        char *command = "./orca";
        char *args[] = {
            "./orca",
            "-data=/Users/igor/Developer/ui-framework/projects/cluster/Cluster.xml",
            "-start=Cluster/Screens/InstrumentCluster",
            NULL // NULL-terminate the argument list
        };
        
        // Execute the command
        execvp(command, args);
        
        // If execvp fails
        perror("execvp");
        exit(1);
    } else { // Parent process
             // Do not wait for the child process
        fprintf(stderr, "Child process (PID %d) started in parallel\n", pid);
        
        // Optionally, you can perform other tasks here in the parent process
        
        // Return 0 to indicate success
        return 0;
    }
#endif
	return 0;
}

void WI_Init(void) {
  //    id menubar     = [[NSMenu new] autorelease];
  //    id appMenuItem = [[NSMenuItem new] autorelease];
  //    [menubar addItem:appMenuItem];
  //
  //    id appMenu      = [[NSMenu new] autorelease];
  //    id appName      = [[NSProcessInfo processInfo] processName];
  //    id quitTitle    = [@"Quit " stringByAppendingString:appName];
  //    id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle
  //                                                  action:@selector(terminate:)
  //                                           keyEquivalent:@"q"] autorelease];
  //    [appMenu addItem:quitMenuItem];
  //    [appMenuItem setSubmenu:appMenu];
  
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
  [NSApp finishLaunching];
  //    [NSApp setMainMenu:menubar];
  
  NSAppearance *appearance = [NSApp effectiveAppearance];
  NSAppearanceName bestMatch = [appearance bestMatchFromAppearancesWithNames:@[NSAppearanceNameDarkAqua, NSAppearanceNameAqua]];
  
  if ([bestMatch isEqualToString:NSAppearanceNameDarkAqua]) {
    NSLog(@"App started in Dark Mode");
  } else {
    NSLog(@"App started in Light Mode");
  }
}

