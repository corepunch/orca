#ifndef __EVENTS_H__
#define __EVENTS_H__

#define kEventLeftMouseDown 0xfac0b5e7
typedef struct WI_Message* LeftMouseDownEventPtr;

#define kEventRightMouseDown 0x1057ac50
typedef struct WI_Message* RightMouseDownEventPtr;

#define kEventOtherMouseDown 0x9822ca20
typedef struct WI_Message* OtherMouseDownEventPtr;

#define kEventLeftMouseUp 0xf73e019e
typedef struct WI_Message* LeftMouseUpEventPtr;

#define kEventRightMouseUp 0x9160ff69
typedef struct WI_Message* RightMouseUpEventPtr;

#define kEventOtherMouseUp 0x567302d9
typedef struct WI_Message* OtherMouseUpEventPtr;

#define kEventLeftMouseDragged 0x088e1f1b
typedef struct WI_Message* LeftMouseDraggedEventPtr;

#define kEventRightMouseDragged 0x29d4da42
typedef struct WI_Message* RightMouseDraggedEventPtr;

#define kEventOtherMouseDragged 0x0ae3dd32
typedef struct WI_Message* OtherMouseDraggedEventPtr;

#define kEventLeftDoubleClick 0x5a92bc67
typedef struct WI_Message* LeftDoubleClickEventPtr;

#define kEventRightDoubleClick 0xeeebbe60
typedef struct WI_Message* RightDoubleClickEventPtr;

#define kEventOtherDoubleClick 0xf6c60630
typedef struct WI_Message* OtherDoubleClickEventPtr;

#define kEventMouseMoved 0x65db8b6f
typedef struct WI_Message* MouseMovedEventPtr;

#define kEventScrollWheel 0x626f90e3
typedef struct WI_Message* ScrollWheelEventPtr;

#define kEventDragDrop 0x25989e7a
typedef struct WI_Message* DragDropEventPtr;

#define kEventDragEnter 0xc0e97a77
typedef struct WI_Message* DragEnterEventPtr;

#define kEventKeyDown 0x83b19b78
typedef struct WI_Message* KeyDownEventPtr;

#define kEventKeyUp 0xfca37d71
typedef struct WI_Message* KeyUpEventPtr;

#define kEventChar 0x2879e23d
typedef struct WI_Message* CharEventPtr;

#define kEventWindowPaint 0x7ef9e53b
#define kEventWindowClosed 0x7268e69d
#define kEventWindowResized 0xa216e847
#define kEventWindowChangedScreen 0x5fe6b4bf
#define kEventKillFocus 0xa7c0f8d7
typedef void* KillFocusEventPtr;

#define kEventSetFocus 0xc399d265
typedef void* SetFocusEventPtr;

#endif
