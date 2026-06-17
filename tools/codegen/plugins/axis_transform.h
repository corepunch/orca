#ifndef CODEGEN_AXIS_TRANSFORM_H
#define CODEGEN_AXIS_TRANSFORM_H

#include <stdio.h>
#include <string.h>

static const char *k_ax[3] = {"Horizontal", "Vertical", "Depth"};
static const char *k_left[3]  = {"Left", "Top", "Front"};
static const char *k_right[3] = {"Right", "Bottom", "Back"};

static void strip_brackets_dots(const char *path, char *out, size_t out_size)
{
    size_t j = 0;
    for (size_t i = 0; path[i] && j < out_size - 1; ++i) {
        char c = path[i];
        if (c != '[' && c != ']' && c != '.') {
            out[j++] = c;
        }
    }
    out[j] = '\0';
}

static void axis_transform(const char *path, char *leaf, size_t leaf_size)
{
    char buf[512];
    snprintf(buf, sizeof(buf), "%s", path);
    leaf[0] = '\0';

    // Rule 1: {name}.Axis[{n}].Left/Right -> {name}{Direction}
    for (int a = 0; a < 3; ++a) {
        char lname[128];
        snprintf(lname, sizeof(lname), ".Axis[%d].%s", a, k_left[a]);
        if (strncmp(buf, lname, strlen(lname)) == 0) {
            char name[256];
            size_t prefix_len = strlen(lname);
            if (prefix_len < strlen(buf)) {
                snprintf(name, sizeof(name), "%.*s", (int)(prefix_len - 6), buf + 1);
            } else {
                name[0] = '\0';
            }
            snprintf(leaf, leaf_size, "%s%s", name, k_right[a]);
            return;
        }
        char rname[128];
        snprintf(rname, sizeof(rname), ".Axis[%d].%s", a, k_right[a]);
        if (strncmp(buf, rname, strlen(rname)) == 0) {
            char name[256];
            size_t prefix_len = strlen(rname);
            if (prefix_len < strlen(buf)) {
                snprintf(name, sizeof(name), "%.*s", (int)(prefix_len - 6), buf + 1);
            } else {
                name[0] = '\0';
            }
            snprintf(leaf, leaf_size, "%s%s", name, k_left[a]);
            return;
        }
    }

    // Rule 2: {name}.Size.Axis[{n}].{sub} -> {Width,Height,Depth}{Requested,Desired,Min,Actual,Scroll}{sub}
    for (int a = 0; a < 3; ++a) {
        char axis_path[128];
        snprintf(axis_path, sizeof(axis_path), ".Size.Axis[%d]", a);
        if (strncmp(buf, axis_path, strlen(axis_path)) == 0) {
            const char *rest = buf + strlen(axis_path);
            const char *suffixes[] = {"Requested", "Desired", "Min", "Actual", "Scroll"};
            for (int s = 0; s < 5; ++s) {
                char suffix_path[64];
                snprintf(suffix_path, sizeof(suffix_path), ".%s", suffixes[s]);
                if (strncmp(rest, suffix_path, strlen(suffix_path)) == 0) {
                    const char *sub = rest + strlen(suffix_path);
                    snprintf(leaf, leaf_size, "%s%s%s%s", k_ax[a], suffixes[s], sub ? sub : "", sub ? sub : "");
                    return;
                }
            }
        }
    }

    // Rule 3: {name}.Layout.Axis[{n}].{sub} -> {Width,Height,Depth}Layout{sub}
    for (int a = 0; a < 3; ++a) {
        char axis_path[128];
        snprintf(axis_path, sizeof(axis_path), ".Layout.Axis[%d]", a);
        if (strncmp(buf, axis_path, strlen(axis_path)) == 0) {
            const char *rest = buf + strlen(axis_path);
            snprintf(leaf, leaf_size, "%sLayout%s", k_ax[a], rest ? rest : "");
            return;
        }
    }

    // Rule 4: {name}.Margin.Axis[{n}].{sub} -> {Width,Height,Depth}Margin{sub}
    for (int a = 0; a < 3; ++a) {
        char axis_path[128];
        snprintf(axis_path, sizeof(axis_path), ".Margin.Axis[%d]", a);
        if (strncmp(buf, axis_path, strlen(axis_path)) == 0) {
            const char *rest = buf + strlen(axis_path);
            snprintf(leaf, leaf_size, "%sMargin%s", k_ax[a], rest ? rest : "");
            return;
        }
    }

    // Rule 5: {name}.Padding.Axis[{n}].{sub} -> {Width,Height,Depth}Padding{sub}
    for (int a = 0; a < 3; ++a) {
        char axis_path[128];
        snprintf(axis_path, sizeof(axis_path), ".Padding.Axis[%d]", a);
        if (strncmp(buf, axis_path, strlen(axis_path)) == 0) {
            const char *rest = buf + strlen(axis_path);
            snprintf(leaf, leaf_size, "%sPadding%s", k_ax[a], rest ? rest : "");
            return;
        }
    }

    // Rule 6: {name}.InsetAxis[{n}].{sub} -> {Width,Height,Depth}Inset{sub}
    for (int a = 0; a < 3; ++a) {
        char axis_path[128];
        snprintf(axis_path, sizeof(axis_path), ".InsetAxis[%d]", a);
        if (strncmp(buf, axis_path, strlen(axis_path)) == 0) {
            const char *rest = buf + strlen(axis_path);
            snprintf(leaf, leaf_size, "%sInset%s", k_ax[a], rest ? rest : "");
            return;
        }
    }

    // Rule 7: {name}.Size.Axis[{n}] -> {Width,Height,Depth}{Size}
    for (int a = 0; a < 3; ++a) {
        char axis_path[128];
        snprintf(axis_path, sizeof(axis_path), ".Size.Axis[%d]", a);
        if (strcmp(buf, axis_path) == 0) {
            snprintf(leaf, leaf_size, "%sSize", k_ax[a]);
            return;
        }
    }

    // Rule 8: {name}.Layout.Axis[{n}] -> {Width,Height,Depth}Layout
    for (int a = 0; a < 3; ++a) {
        char axis_path[128];
        snprintf(axis_path, sizeof(axis_path), ".Layout.Axis[%d]", a);
        if (strcmp(buf, axis_path) == 0) {
            snprintf(leaf, leaf_size, "%sLayout", k_ax[a]);
            return;
        }
    }

    // Rule 9: {name}.Margin.Axis[{n}] -> {Width,Height,Depth}Margin
    for (int a = 0; a < 3; ++a) {
        char axis_path[128];
        snprintf(axis_path, sizeof(axis_path), ".Margin.Axis[%d]", a);
        if (strcmp(buf, axis_path) == 0) {
            snprintf(leaf, leaf_size, "%sMargin", k_ax[a]);
            return;
        }
    }

    // Rule 10: {name}.Padding.Axis[{n}] -> {Width,Height,Depth}Padding
    for (int a = 0; a < 3; ++a) {
        char axis_path[128];
        snprintf(axis_path, sizeof(axis_path), ".Padding.Axis[%d]", a);
        if (strcmp(buf, axis_path) == 0) {
            snprintf(leaf, leaf_size, "%sPadding", k_ax[a]);
            return;
        }
    }

    // Rule 11: {name}.InsetAxis[{n}] -> {Width,Height,Depth}Inset
    for (int a = 0; a < 3; ++a) {
        char axis_path[128];
        snprintf(axis_path, sizeof(axis_path), ".InsetAxis[%d]", a);
        if (strcmp(buf, axis_path) == 0) {
            snprintf(leaf, leaf_size, "%sInset", k_ax[a]);
            return;
        }
    }

    // Rule 12: {name}.Border.Radius.{sub}Radius -> {name}Border{sub}Radius
    if (strncmp(buf, ".Border.Radius.", 15) == 0) {
        const char *rest = buf + 15;
        size_t len = strlen(rest);
        size_t rlen = strlen("Radius");
        if (len > rlen && strcmp(rest + len - rlen, "Radius") == 0) {
            snprintf(leaf, leaf_size, "Border%.*sRadius", (int)(len - rlen), rest);
            return;
        }
    }

    // Rule 13: .Axis[{n}].Left/Right -> {Direction}
    for (int a = 0; a < 3; ++a) {
        char path[128];
        snprintf(path, sizeof(path), ".Axis[%d].%s", a, k_left[a]);
        if (strcmp(buf, path) == 0) {
            snprintf(leaf, leaf_size, "%s", k_left[a]);
            return;
        }
        snprintf(path, sizeof(path), ".Axis[%d].%s", a, k_right[a]);
        if (strcmp(buf, path) == 0) {
            snprintf(leaf, leaf_size, "%s", k_right[a]);
            return;
        }
    }

    // Rule 14: .Axis[{n}] -> {Axis}
    for (int a = 0; a < 3; ++a) {
        char path[64];
        snprintf(path, sizeof(path), ".Axis[%d]", a);
        if (strcmp(buf, path) == 0) {
            snprintf(leaf, leaf_size, "%s", k_ax[a]);
            return;
        }
    }

    // Fallback: strip brackets and dots
    strip_brackets_dots(path, leaf, leaf_size);
}

#endif // CODEGEN_AXIS_TRANSFORM_H
