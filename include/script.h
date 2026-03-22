#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include "orcadef.h"

/*
 * ScriptContext — opaque handle for the active scripting-runtime state.
 *
 * For the built-in Lua back-end this is a lua_State *; a future scripting
 * language plug-in would supply its own runtime pointer here.  Code outside
 * the scripting layer must treat this value as opaque and must not
 * dereference or inspect it directly.
 */
typedef void *ScriptContext;

/*
 * ScriptMessageProc — signature for a scripting-aware message handler.
 *
 * Handlers are registered with SV_RegisterMessageProc() and are called by
 * the engine backend for every platform/engine message.
 *
 *   ctx  – the scripting context supplied at registration time.
 *   msg  – the platform message to process.
 *
 * Returns TRUE if the message was fully consumed (no further dispatch),
 * FALSE to pass the message on to the next registered handler.
 */
struct WI_Message;
typedef LRESULT (*ScriptMessageProc)(ScriptContext ctx, struct WI_Message *msg);

#endif /* __SCRIPT_H__ */
