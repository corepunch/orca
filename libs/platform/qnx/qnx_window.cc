/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ctype.h>
#include <ft2build.h>
#include <screen/screen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/keycodes.h>
#include <unistd.h>
#include FT_FREETYPE_H
#include "png.h"
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <math.h>

static float   width, height;
static GLuint  background;
static GLfloat vertices[8];
static GLfloat tex_coord[8];
static float   pos_x, pos_y;

struct font_t
{
    unsigned int font_texture;
    float        pt;
    float        advance[128];
    float        width[128];
    float        height[128];
    float        tex_x1[128];
    float        tex_x2[128];
    float        tex_y1[128];
    float        tex_y2[128];
    float        offset_x[128];
    float        offset_y[128];
    int          initialized;
};

typedef struct font_t font_t;

EGLDisplay egl_disp;
EGLSurface egl_surf;

static EGLConfig        egl_conf;
static EGLContext       egl_ctx;
static screen_window_t  screen_win = NULL;
static screen_display_t screen_disp;
static int              nbuffers    = 2;
static int              initialized = 0;
static font_t*          font;

/* Utility function to calculate the dpi based on the display size */
int
calculate_dpi()
{
    int screen_phys_size[2] = { 0, 0 };

    screen_get_display_property_iv(
        screen_disp, SCREEN_PROPERTY_PHYSICAL_SIZE, screen_phys_size
    );

    /* If using a simulator, {0,0} is returned for physical size of the screen,
     so use 170 as the default dpi when this is the case. */
    if ((screen_phys_size[0] == 0) && (screen_phys_size[1] == 0))
    {
        return 170;
    }
    else
    {
        int screen_resolution[2] = { 0, 0 };
        screen_get_display_property_iv(
            screen_disp, SCREEN_PROPERTY_SIZE, screen_resolution
        );

        int diagonal_pixels = sqrt(
            screen_resolution[0] * screen_resolution[0]
            + screen_resolution[1] * screen_resolution[1]
        );
        int diagonal_inches = 0.0393700787
                              * sqrt(
                                  screen_phys_size[0] * screen_phys_size[0]
                                  + screen_phys_size[1] * screen_phys_size[1]
                              );
        return (int)(diagonal_pixels / diagonal_inches);
    }
}

/* Utility function to perform EGL cleanup */
void
egl_cleanup()
{
    /* Typical EGL cleanup */
    if (egl_disp != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(
            egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT
        );
        if (egl_surf != EGL_NO_SURFACE)
        {
            eglDestroySurface(egl_disp, egl_surf);
            egl_surf = EGL_NO_SURFACE;
        }
        if (egl_ctx != EGL_NO_CONTEXT)
        {
            eglDestroyContext(egl_disp, egl_ctx);
            egl_ctx = EGL_NO_CONTEXT;
        }
        if (screen_win != NULL)
        {
            screen_destroy_window(screen_win);
            screen_win = NULL;
        }
        eglTerminate(egl_disp);
        egl_disp = EGL_NO_DISPLAY;
    }
    eglReleaseThread();

    initialized = 0;
}

/* Utility function to initialize and configure a EGL rendering surface */
int
init_egl(screen_context_t screen_ctx)
{
    int    usage;
    int    format   = SCREEN_FORMAT_RGBX8888;
    EGLint interval = 1;
    int    rc, num_configs;

    EGLint attrib_list[] = { EGL_RED_SIZE,
                             8,
                             EGL_GREEN_SIZE,
                             8,
                             EGL_BLUE_SIZE,
                             8,
                             EGL_BLUE_SIZE,
                             8,
                             EGL_SURFACE_TYPE,
                             EGL_WINDOW_BIT,
                             EGL_RENDERABLE_TYPE,
                             EGL_OPENGL_ES_BIT,
                             EGL_NONE };

    /* Assuming GL_ES_1 */
    usage = SCREEN_USAGE_OPENGL_ES1 | SCREEN_USAGE_ROTATION;

    /* Establish a connection to the default display */
    egl_disp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (egl_disp == EGL_NO_DISPLAY)
    {
        egl_cleanup();
        return EXIT_FAILURE;
    }

    /* Initialize EGL on the display */
    rc = eglInitialize(egl_disp, NULL, NULL);
    if (rc != EGL_TRUE)
    {
        egl_cleanup();
        return EXIT_FAILURE;
    }

    /* Calling eglBindAPI() to specify the current rendering API is not
    necessary
     * because OpenGL ES is the default rendering API.
    rc = eglBindAPI(EGL_OPENGL_ES_API);

    if (rc != EGL_TRUE) {
        egl_cleanup();
        return EXIT_FAILURE;
    }*/

    if (!eglChooseConfig(egl_disp, attrib_list, &egl_conf, 1, &num_configs))
    {
        egl_cleanup();
        return EXIT_FAILURE;
    }

    egl_ctx = eglCreateContext(egl_disp, egl_conf, EGL_NO_CONTEXT, NULL);

    if (egl_ctx == EGL_NO_CONTEXT)
    {
        egl_cleanup();
        return EXIT_FAILURE;
    }

    rc = screen_create_window(&screen_win, screen_ctx);
    if (rc)
    {
        perror("screen_create_window");
        egl_cleanup();
        return EXIT_FAILURE;
    }

    rc = screen_set_window_property_iv(
        screen_win, SCREEN_PROPERTY_FORMAT, &format
    );
    if (rc)
    {
        perror("screen_set_window_property_iv(SCREEN_PROPERTY_FORMAT)");
        egl_cleanup();
        return EXIT_FAILURE;
    }

    rc = screen_set_window_property_iv(
        screen_win, SCREEN_PROPERTY_USAGE, &usage
    );
    if (rc)
    {
        perror("screen_set_window_property_iv(SCREEN_PROPERTY_USAGE)");
        egl_cleanup();
        return EXIT_FAILURE;
    }

    rc = screen_get_window_property_pv(
        screen_win, SCREEN_PROPERTY_DISPLAY, (void**)&screen_disp
    );
    if (rc)
    {
        perror("screen_get_window_property_pv");
        egl_cleanup();
        return EXIT_FAILURE;
    }

    int angle = atoi(getenv("ORIENTATION"));

    screen_display_mode_t screen_mode;
    rc = screen_get_display_property_pv(
        screen_disp, SCREEN_PROPERTY_MODE, (void**)&screen_mode
    );
    if (rc)
    {
        perror("screen_get_display_property_pv");
        egl_cleanup();
        return EXIT_FAILURE;
    }

    int size[2];
    rc = screen_get_window_property_iv(
        screen_win, SCREEN_PROPERTY_BUFFER_SIZE, size
    );
    if (rc)
    {
        perror("screen_get_window_property_iv");
        egl_cleanup();
        return EXIT_FAILURE;
    }

    int buffer_size[2] = { size[0], size[1] };

    if ((angle == 0) || (angle == 180))
    {
        if (((screen_mode.width > screen_mode.height) && (size[0] < size[1]))
            || ((screen_mode.width < screen_mode.height) && (size[0] > size[1])
            ))
        {
            buffer_size[1] = size[0];
            buffer_size[0] = size[1];
        }
    }
    else if ((angle == 90) || (angle == 270))
    {
        if (((screen_mode.width > screen_mode.height) && (size[0] > size[1]))
            || ((screen_mode.width < screen_mode.height && size[0] < size[1])))
        {
            buffer_size[1] = size[0];
            buffer_size[0] = size[1];
        }
    }
    else
    {
        fprintf(
            stderr, "Navigator returned an unexpected orientation angle.\n"
        );
        egl_cleanup();
        return EXIT_FAILURE;
    }

    rc = screen_set_window_property_iv(
        screen_win, SCREEN_PROPERTY_BUFFER_SIZE, buffer_size
    );
    if (rc)
    {
        perror("screen_set_window_property_iv");
        egl_cleanup();
        return EXIT_FAILURE;
    }

    rc = screen_set_window_property_iv(
        screen_win, SCREEN_PROPERTY_ROTATION, &angle
    );
    if (rc)
    {
        perror("screen_set_window_property_iv");
        egl_cleanup();
        return EXIT_FAILURE;
    }

    rc = screen_create_window_buffers(screen_win, nbuffers);
    if (rc)
    {
        perror("screen_create_window_buffers");
        egl_cleanup();
        return EXIT_FAILURE;
    }

    egl_surf = eglCreateWindowSurface(egl_disp, egl_conf, screen_win, NULL);
    if (egl_surf == EGL_NO_SURFACE)
    {
        egl_cleanup();
        return EXIT_FAILURE;
    }

    rc = eglMakeCurrent(egl_disp, egl_surf, egl_surf, egl_ctx);
    if (rc != EGL_TRUE)
    {
        egl_cleanup();
        return EXIT_FAILURE;
    }

    rc = eglSwapInterval(egl_disp, interval);
    if (rc != EGL_TRUE)
    {
        egl_cleanup();
        return EXIT_FAILURE;
    }

    initialized = 1;

    return EXIT_SUCCESS;
}

void
measure_text(font_t* font, char const* msg, float* width, float* height)
{
    int i, c;

    if (!msg)
    {
        return;
    }

    if (width)
    {
        /* Width of a text rectangle is a sum advances for every glyph in a
         * string */
        *width = 0.0f;

        for (i = 0; i < strlen(msg); ++i)
        {
            c = msg[i];
            *width += font->advance[c];
        }
    }

    if (height)
    {
        /* Height of a text rectangle is a high of a tallest glyph in a string
         */
        *height = 0.0f;

        for (i = 0; i < strlen(msg); ++i)
        {
            c = msg[i];

            if (*height < font->height[c])
            {
                *height = font->height[c];
            }
        }
    }
}

int
init()
{
    EGLint surface_width, surface_height;

    /* We are going to load MyriadPro-Bold as it looks a little better and
       scale it to fit out bubble nicely. */
    int dpi = calculate_dpi();

    /*font = load_font(
            "/usr/fonts/font_repository/adobe/MyriadPro-Bold.otf", 15, dpi); */
    font = load_font(
        "/usr/fonts/font_repository/monotype/georgiab.ttf", 8, dpi
    );
    if (!font)
    {
        return EXIT_FAILURE;
    }

    /* Load background texture */
    float tex_x, tex_y;
    if (EXIT_SUCCESS
        != load_texture(
            "app/native/HelloWorld_smaller_bubble.png",
            NULL,
            NULL,
            &tex_x,
            &tex_y,
            &background
        ))
    {
        fprintf(stderr, "Unable to load background texture\n");
    }

    /* Query width and height of the window surface created by utility code */
    eglQuerySurface(egl_disp, egl_surf, EGL_WIDTH, &surface_width);
    eglQuerySurface(egl_disp, egl_surf, EGL_HEIGHT, &surface_height);

    EGLint err = eglGetError();
    if (err != 0x3000)
    {
        fprintf(stderr, "Unable to query EGL surface dimensions\n");
        return EXIT_FAILURE;
    }

    width  = (float)surface_width;
    height = (float)surface_height;

    /*	Initialize GL for 2D rendering */
    glViewport(0, 0, (int)width, (int)height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrthof(0.0f, width / height, 0.0f, 1.0f, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Set world coordinates to coincide with screen pixels */
    glScalef(1.0f / height, 1.0f / height, 1.0f);

    float text_width, text_height;
    measure_text(font, "Hello world", &text_width, &text_height);
    pos_x = (width - text_width) / 2;
    pos_y = height / 2;

    /* Setup background polygon */
    vertices[0] = 0.0f;
    vertices[1] = 0.0f;
    vertices[2] = width;
    vertices[3] = 0.0f;
    vertices[4] = 0.0f;
    vertices[5] = height;
    vertices[6] = width;
    vertices[7] = height;

    tex_coord[0] = 0.0f;
    tex_coord[1] = 0.0f;
    tex_coord[2] = tex_x;
    tex_coord[3] = 0.0f;
    tex_coord[4] = 0.0f;
    tex_coord[5] = tex_y;
    tex_coord[6] = tex_x;
    tex_coord[7] = tex_y;

    return EXIT_SUCCESS;
}

void
render_text(font_t* font, char const* msg, float x, float y)
{
    int      i, c;
    GLfloat* vertices;
    GLfloat* texture_coords;
    GLshort* indices;

    float pen_x = 0.0f;

    if (!font)
    {
        fprintf(stderr, "Font must not be null\n");
        return;
    }

    if (!font->initialized)
    {
        fprintf(stderr, "Font has not been loaded\n");
        return;
    }

    if (!msg)
    {
        return;
    }

    int texture_enabled;
    glGetIntegerv(GL_TEXTURE_2D, &texture_enabled);
    if (!texture_enabled)
    {
        glEnable(GL_TEXTURE_2D);
    }

    int blend_enabled;
    glGetIntegerv(GL_BLEND, &blend_enabled);
    if (!blend_enabled)
    {
        glEnable(GL_BLEND);
    }

    int gl_blend_src, gl_blend_dst;
    glGetIntegerv(GL_BLEND_SRC, &gl_blend_src);
    glGetIntegerv(GL_BLEND_DST, &gl_blend_dst);

    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    int vertex_array_enabled;
    glGetIntegerv(GL_VERTEX_ARRAY, &vertex_array_enabled);
    if (!vertex_array_enabled)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
    }

    int texture_array_enabled;
    glGetIntegerv(GL_TEXTURE_COORD_ARRAY, &texture_array_enabled);
    if (!texture_array_enabled)
    {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    vertices       = (GLfloat*)malloc(sizeof(GLfloat) * 8 * strlen(msg));
    texture_coords = (GLfloat*)malloc(sizeof(GLfloat) * 8 * strlen(msg));

    indices = (GLshort*)malloc(sizeof(GLfloat) * 5 * strlen(msg));

    for (i = 0; i < strlen(msg); ++i)
    {
        c = msg[i];

        vertices[8 * i + 0] = x + pen_x + font->offset_x[c];
        vertices[8 * i + 1] = y + font->offset_y[c];
        vertices[8 * i + 2] = vertices[8 * i + 0] + font->width[c];
        vertices[8 * i + 3] = vertices[8 * i + 1];
        vertices[8 * i + 4] = vertices[8 * i + 0];
        vertices[8 * i + 5] = vertices[8 * i + 1] + font->height[c];
        vertices[8 * i + 6] = vertices[8 * i + 2];
        vertices[8 * i + 7] = vertices[8 * i + 5];

        texture_coords[8 * i + 0] = font->tex_x1[c];
        texture_coords[8 * i + 1] = font->tex_y2[c];
        texture_coords[8 * i + 2] = font->tex_x2[c];
        texture_coords[8 * i + 3] = font->tex_y2[c];
        texture_coords[8 * i + 4] = font->tex_x1[c];
        texture_coords[8 * i + 5] = font->tex_y1[c];
        texture_coords[8 * i + 6] = font->tex_x2[c];
        texture_coords[8 * i + 7] = font->tex_y1[c];

        indices[i * 6 + 0] = 4 * i + 0;
        indices[i * 6 + 1] = 4 * i + 1;
        indices[i * 6 + 2] = 4 * i + 2;
        indices[i * 6 + 3] = 4 * i + 2;
        indices[i * 6 + 4] = 4 * i + 1;
        indices[i * 6 + 5] = 4 * i + 3;

        /* Assume we are only working with typewriter fonts */
        pen_x += font->advance[c];
    }

    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, texture_coords);
    glBindTexture(GL_TEXTURE_2D, font->font_texture);

    glDrawElements(GL_TRIANGLES, 6 * strlen(msg), GL_UNSIGNED_SHORT, indices);

    if (!texture_array_enabled)
    {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    if (!vertex_array_enabled)
    {
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    if (!texture_enabled)
    {
        glDisable(GL_TEXTURE_2D);
    }

    glBlendFunc(gl_blend_src, gl_blend_dst);

    if (!blend_enabled)
    {
        glDisable(GL_BLEND);
    }

    free(vertices);
    free(texture_coords);
    free(indices);
}

int
render()
{
    /* Typical rendering pass */
    glClear(GL_COLOR_BUFFER_BIT);

    /* Render background quad first */
    glEnable(GL_TEXTURE_2D);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, tex_coord);
    glBindTexture(GL_TEXTURE_2D, background);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_TEXTURE_2D);

    /* Set color to use for text rendering */
    glColor4f(0.35f, 0.35f, 0.35f, 1.0f);

    /* Render the text onto the screen */
    render_text(font, "Hello world", pos_x, pos_y);

    /* Update the screen; Posting of the new frame requires a call to
     * eglSwapBuffers. For now, this is true even when using single buffering.
     * If an event has occured that invalidates the surface we are currently
     * using, eglSwapBuffers() will return EGL_FALSE and set the error
     * code to EGL_BAD_NATIVE_WINDOW.
     */
    int rc = eglSwapBuffers(egl_disp, egl_surf);

    return rc;
}

int
main(int argc, char** argv)
{
    int            rc;        /* a return code */
    screen_event_t screen_ev; /* a screen event to handle */
    screen_context_t
        screen_ctx; /* a connection to the screen windowing system */
    int vis   = 1;  /* an indicator if our window is visible */
    int pause = 0;  /* an indicator if rendering is frozen */

    int const exit_area_size
        = 20; /* a size of area on the window where a user can
               * contact (using a pointer) to exit this application */

    int size[2] = { 0, 0 }; /* the width and height of your window; will
                             * default to the size of display since the window
                             * property wasn't explicitly set */

    int val;               /* a variable used to set/get window properties */
    int pos[2] = { 0, 0 }; /* the x,y position of your pointer */

    /*Create a screen context that will be the connection to the windowing
     *system; this is used to create an EGL surface to receive libscreen events
     */
    screen_create_context(&screen_ctx, 0);

    /* Initialize EGL for rendering with GL ES 1.1;
     * this initialization includes initializing and configuring EGL as well as
     * creating a native window with the appropriate properties to be used as
     * the EGL rendering surface. */
    if (EXIT_SUCCESS != init_egl(screen_ctx))
    {
        fprintf(stderr, "Unable to initialize EGL\n");
        screen_destroy_context(screen_ctx);
        return 0;
    }

    /* Initialize application data;
     * this initialization includes loading the font and background and
     * initializing the viewport and geometry for your application. */
    if (EXIT_SUCCESS != init())
    {
        fprintf(stderr, "Unable to initialize app logic\n");
        egl_cleanup();
        screen_destroy_context(screen_ctx);
        return 0;
    }

    /* Create a screen event that will be used to retrieve events into so that
     * these events can be handled.*/
    rc = screen_create_event(&screen_ev);
    if (rc)
    {
        fprintf(stderr, "screen_create_event\n");
        egl_cleanup();
        screen_destroy_context(screen_ctx);
        return 0;
    }

    /* This is your main application loop. It keeps on running unless a close
     * event is received from the windowing system or an error occurs. The
     * application loop consists of two parts. The first part processes any
     * events that have been put in your queue. The second part does the
     * rendering. When the window is visible, you don't wait if the event queue
     * is empty; you move on to the rendering part immediately. When the window
     * is not visible we skip the rendering part. */
    while (1)
    {
        /* The first part of the loop is to handle screen events.
         * We start the loop by processing any events that might be in our
         * queue. The only event that is of interest to us are the resize
         * and close events. The timeout variable is set to 0 (no wait) or
         * forever depending if the window is visible or not. */
        while (!screen_get_event(screen_ctx, screen_ev, vis ? 0 : ~0))
        {
            rc = screen_get_event_property_iv(
                screen_ev, SCREEN_PROPERTY_TYPE, &val
            );
            if (rc || val == SCREEN_EVENT_NONE)
            {
                break;
            }
            switch (val)
            {
            case SCREEN_EVENT_CLOSE:
                /* All we have to do when we receive the close event is
                 * to exit the application loop. Because we have a loop
                 * within a loop, a simple break won't work. We'll just
                 * use a goto to take us out of here.*/
                goto end;
            case SCREEN_EVENT_PROPERTY:
                /* We are interested in visibility changes so we can pause
                 * or unpause the rendering. */
                screen_get_event_property_iv(
                    screen_ev, SCREEN_PROPERTY_NAME, &val
                );
                switch (val)
                {
                case SCREEN_PROPERTY_VISIBLE:
                    /* The new visibility status is not included in the
                     * event, so we must get it ourselves. */
                    screen_get_window_property_iv(
                        screen_win, SCREEN_PROPERTY_VISIBLE, &vis
                    );
                    break;
                }
                break;
            case SCREEN_EVENT_POINTER:
                /* To provide a way of gracefully terminating our application,
                 * we will exit if there is a pointer select event in the upper
                 * right corner of our window. This should happen if the
                 * mouse's left button is clicked or if a touch screen display
                 * is pressed. The event will come as a screen pointer event,
                 * with an (x,y) coordinate relative to the window's upper left
                 * corner and a select value. We have to verify ourselves that
                 * the coordinates of the pointer are in the upper right hand
                 * area. */
                screen_get_event_property_iv(
                    screen_ev, SCREEN_PROPERTY_BUTTONS, &val
                );
                if (val)
                {
                    screen_get_event_property_iv(
                        screen_ev, SCREEN_PROPERTY_POSITION, pos
                    );
                    screen_get_window_property_iv(
                        screen_win, SCREEN_PROPERTY_SIZE, size
                    );
                    fprintf(
                        stderr,
                        "window width: %d, window height: %d\n",
                        size[0],
                        size[1]
                    );
                    fprintf(
                        stderr,
                        "pointer x: %d, pointer y: %d\n",
                        pos[0],
                        pos[1]
                    );
                    if (pos[0] >= size[0] - exit_area_size
                        && pos[1] < exit_area_size)
                    {
                        goto end;
                    }
                }
                break;
            }
        }

        /* The second part of the application loop is the rendering. You want
         * to skip the rendering part if your window is not visible. This will
         * leave the CPU and GPU to other applications and make the system a
         * little bit more responsive while you are invisible. */
        if (vis && !pause)
        {
            rc = render();
            if (rc != EGL_TRUE)
                break;
        }
    }

end:
    /* Destroy the font */
    if (font)
    {
        glDeleteTextures(1, &(font->font_texture));
        free(font);
    }

    /* Terminate EGL setup */
    egl_cleanup();

    /* Clean up screen */
    screen_destroy_event(screen_ev);
    screen_destroy_context(screen_ctx);

    return EXIT_SUCCESS;
}
