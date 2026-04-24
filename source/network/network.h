#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <include/orcadef.h>
#include <stddef.h>

/*
 * C-callable asynchronous HTTP/HTTPS fetch API.
 *
 * Usage pattern (from an event-driven HANDLER or a system message proc):
 *
 *   fetch_handle_t h = NET_FetchBegin("https://example.com/image.png");
 *   // ... on each frame / poll event:
 *   if (!NET_FetchPoll(h)) {
 *       void  *data; size_t size;
 *       long   code = NET_FetchFinish(h, &data, &size);
 *       // use data[0..size-1], then free(data)
 *   }
 *   // to cancel:  NET_FetchCancel(h);
 */

/* Initialise the network subsystem (idempotent).
   Called automatically by luaopen_orca_network; also safe to call from
   C-only paths (e.g. UIKit) before using NET_FetchBegin. */
ORCA_API void NET_Init(void);

/* Shut down the network subsystem (idempotent). */
ORCA_API void NET_Shutdown(void);

/* Opaque handle for an in-flight HTTP GET request. */
typedef void *fetch_handle_t;

/* Start an asynchronous HTTP/HTTPS GET for `url'.
   Returns NULL if initialisation fails. */
ORCA_API fetch_handle_t NET_FetchBegin(const char *url);

/* Poll the fetch status.
   Returns TRUE (non-zero) while the download is still in progress.
   Returns FALSE (zero) when it is complete (success or failure). */
ORCA_API bool_t NET_FetchPoll(fetch_handle_t handle);

/* Retrieve the completed response.
   Sets *data_out to a malloc'd buffer (caller must free it) and *size_out to
   its byte count.  Returns the HTTP response code (200, 404, …), or -1 on a
   transport error.  The handle is freed; do not use it afterwards. */
ORCA_API long NET_FetchFinish(fetch_handle_t handle,
                               void        **data_out,
                               size_t       *size_out);

/* Cancel an in-flight (or completed) fetch and free its resources.
   Safe to call with NULL. */
ORCA_API void NET_FetchCancel(fetch_handle_t handle);

#endif /* __NETWORK_H__ */
