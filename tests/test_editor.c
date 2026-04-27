/*
 * test_editor.c — C unit tests for the EditorKit editor toggle and paint
 * dispatch.
 *
 * Tests cover:
 *   - Initial visibility state is FALSE
 *   - Pressing backtick (`) via kEventKeyDown makes ED_IsVisible() TRUE
 *   - Pressing backtick a second time makes ED_IsVisible() FALSE (toggle off)
 *   - When visible, dispatching ID_Window_Paint calls ED_Draw (verified via
 *     the ED_SetDrawHook test hook — no OpenGL context required)
 *   - When not visible, dispatching ID_Window_Paint is forwarded (returns FALSE)
 *     so the normal screen renderer can handle it
 *
 * Compiled via the `test-editor` Makefile target (depends on `buildplugins`).
 *
 * The test binary links against liborca.so (for core engine symbols) and the
 * EditorKit plugin object file (for ED_* symbols). The draw hook mechanism
 * (ED_SetDrawHook) bypasses the real renderer so tests run headless.
 */

#include <include/editor.h>
#include <libs/platform/events.h>
#include <source/renderer/api/renderer_properties.h>
#include <source/renderer/api/renderer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Minimal test harness                                                */
/* ------------------------------------------------------------------ */

static int s_tests_run    = 0;
static int s_tests_failed = 0;
static const char* s_current_test = NULL;

#define EXPECT(...)                                                        \
    if (!(__VA_ARGS__)) {                                                  \
        fprintf(stderr, "  FAIL [%s]: %s (line %d)\n",                   \
                s_current_test, #__VA_ARGS__, __LINE__);                  \
        s_tests_failed++;                                                  \
        break;                                                             \
    }

#define RUN(name, block)                                                   \
    do {                                                                   \
        s_current_test = name;                                             \
        s_tests_run++;                                                     \
        printf("Running %s...\n", name);                                   \
        do { block } while (0);                                            \
    } while (0)

/* ------------------------------------------------------------------ */
/* Draw hook state                                                     */
/* ------------------------------------------------------------------ */

static int s_draw_called = 0;

static void test_draw_hook(void) {
    s_draw_called++;
}

/* ------------------------------------------------------------------ */
/* Helper: send a kEventKeyDown for the given character               */
/* ------------------------------------------------------------------ */

static LRESULT dispatch_key(uint32_t ch) {
    /* wParam low byte carries the ASCII character for the key press. */
    return ED_DispatchMessage(kEventKeyDown, ch & 0xff, NULL);
}

/* ------------------------------------------------------------------ */
/* Tests                                                               */
/* ------------------------------------------------------------------ */

static void test_initial_state(void)
{
    RUN("initial_visibility_is_false", {
        EXPECT(ED_IsVisible() == FALSE);
    });
}

static void test_backtick_enables_editor(void)
{
    RUN("backtick_keydown_enables_editor", {
        /* Make sure we start invisible. */
        if (ED_IsVisible()) dispatch_key('`');
        EXPECT(ED_IsVisible() == FALSE);

        LRESULT r = dispatch_key('`');
        EXPECT(r == TRUE);           /* event consumed */
        EXPECT(ED_IsVisible() == TRUE);

        /* Restore state for subsequent tests. */
        dispatch_key('`');
    });
}

static void test_backtick_toggles_off(void)
{
    RUN("second_backtick_disables_editor", {
        /* Ensure starting from visible. */
        if (!ED_IsVisible()) dispatch_key('`');
        EXPECT(ED_IsVisible() == TRUE);

        LRESULT r = dispatch_key('`');
        EXPECT(r == TRUE);
        EXPECT(ED_IsVisible() == FALSE);
    });
}

static void test_paint_invokes_draw_when_visible(void)
{
    struct Window_PaintEventArgs paint = { .WindowWidth = 800, .WindowHeight = 600 };
    RUN("id_window_paint_calls_draw_when_visible", {
        /* Ensure editor is visible. */
        if (!ED_IsVisible()) dispatch_key('`');
        EXPECT(ED_IsVisible() == TRUE);

        ED_SetDrawHook(test_draw_hook);
        s_draw_called = 0;

        LRESULT r = ED_DispatchMessage(ID_Window_Paint, 0, &paint);

        ED_SetDrawHook(NULL);

        EXPECT(r == TRUE);          /* event was handled */
        EXPECT(s_draw_called == 1); /* draw hook was invoked exactly once */

        /* Restore state. */
        dispatch_key('`');
    });
}

static void test_paint_not_consumed_when_hidden(void)
{
    struct Window_PaintEventArgs paint = { .WindowWidth = 800, .WindowHeight = 600 };
    RUN("id_window_paint_forwarded_when_hidden", {
        /* Ensure editor is hidden. */
        if (ED_IsVisible()) dispatch_key('`');
        EXPECT(ED_IsVisible() == FALSE);

        ED_SetDrawHook(test_draw_hook);
        s_draw_called = 0;

        LRESULT r = ED_DispatchMessage(ID_Window_Paint, 0, &paint);

        ED_SetDrawHook(NULL);

        EXPECT(r == FALSE);         /* event not consumed — screen renderer handles it */
        EXPECT(s_draw_called == 0); /* draw hook must NOT have been called */
    });
}

/* ------------------------------------------------------------------ */
/* Entry point                                                         */
/* ------------------------------------------------------------------ */

int main(void)
{
    /* Initialise only the filesystem hash tables — the lightweight portion of
     * the editor startup that does NOT touch OpenGL.  Calling the full
     * ED_Init() would reach ED_InitLayout() → CanvasView EVT_CREATE →
     * RenderTexture_Create() → glGetIntegerv(), which crashes on macOS when
     * there is no OpenGL context (Mesa silently tolerates it on Linux). */
    FS_InitHash();

    test_initial_state();
    test_backtick_enables_editor();
    test_backtick_toggles_off();
    test_paint_invokes_draw_when_visible();
    test_paint_not_consumed_when_hidden();

    FS_ShutdownHash();

    printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
    return s_tests_failed == 0 ? 0 : 1;
}