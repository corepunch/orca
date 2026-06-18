#ifndef CODEGEN_AXIS_TRANSFORM_H
#define CODEGEN_AXIS_TRANSFORM_H

#include <stdio.h>
#include <string.h>

static const char *k_ax[3] = {"Horizontal", "Vertical", "Depth"};
static const char *k_left[3]  = {"Left", "Top", "Front"};
static const char *k_right[3] = {"Right", "Bottom", "Back"};

static const char *k_size_sub[5]     = {"Requested","Desired","Min","Actual","Scroll"};
static const char *k_size_names[3][5] = {
    {"Width","DesiredWidth","MinWidth","ActualWidth","ScrollWidth"},
    {"Height","DesiredHeight","MinHeight","ActualHeight","ScrollHeight"},
    {"Depth","DesiredDepth","MinDepth","ActualDepth","ScrollDepth"},
};

static void strip_brackets_dots(const char *src, char *dst, size_t dsz)
{
    size_t j = 0;
    for (size_t i = 0; src[i] && j + 1 < dsz; ++i) {
        char c = src[i];
        if (c != '[' && c != ']' && c != '.') {
            dst[j++] = c;
        }
    }
    dst[j] = '\0';
}

static void axis_transform(const char *path, char *leaf, size_t leaf_size)
{
    char result[512];
    int n;

    /* --- Size.Axis[N].{Sub} -> {Width,Height,Depth}{Requested,Desired,...} --- */
    for (n = 0; n < 3; n++) {
        char tok[64]; snprintf(tok, sizeof(tok), "Size.Axis[%d].", n);
        if (strncmp(path, tok, strlen(tok)) == 0) {
            char const *sub = path + strlen(tok);
            int k;
            if (strncmp(sub, "Requested", 9) == 0) {
                char const *rest = sub + 9;
                snprintf(leaf, leaf_size, "%s%s", k_size_names[n][0], rest);
                strip_brackets_dots(leaf, leaf, leaf_size);
                return;
            }
            for (k = 1; k < 5; k++) {
                if (strcmp(sub, k_size_sub[k]) == 0) {
                    snprintf(leaf, leaf_size, "%s", k_size_names[n][k]);
                    return;
                }
            }
        }
    }

    /* --- (.+).Axis[N].Left/Right -> {prefix}{Left,Top,Front,Right,Bottom,Back}{suffix} --- */
    for (n = 0; n < 3; n++) {
        char lt[64], rt[64];
        snprintf(lt, sizeof(lt), ".Axis[%d].Left", n);
        snprintf(rt, sizeof(rt), ".Axis[%d].Right", n);
        char const *pos;
        if ((pos = strstr(path, lt)) != NULL) {
            size_t pl = (size_t)(pos - path);
            char prefix[256]; memcpy(prefix, path, pl); prefix[pl] = 0;
            char const *suf = pos + strlen(lt);
            snprintf(result, sizeof(result), "%s%s%s", prefix, k_left[n], suf);
            strip_brackets_dots(result, leaf, leaf_size);
            return;
        }
        if ((pos = strstr(path, rt)) != NULL) {
            size_t pl = (size_t)(pos - path);
            char prefix[256]; memcpy(prefix, path, pl); prefix[pl] = 0;
            char const *suf = pos + strlen(rt);
            snprintf(result, sizeof(result), "%s%s%s", prefix, k_right[n], suf);
            strip_brackets_dots(result, leaf, leaf_size);
            return;
        }
    }

    /* --- (.+).Axis[N] at end -> {Axis}{prefix} --- */
    for (n = 0; n < 3; n++) {
        char tok[32]; snprintf(tok, sizeof(tok), ".Axis[%d]", n);
        size_t plen = strlen(path), tlen = strlen(tok);
        if (plen > tlen && strcmp(path + plen - tlen, tok) == 0) {
            char prefix[256]; memcpy(prefix, path, plen - tlen); prefix[plen - tlen] = 0;
            snprintf(result, sizeof(result), "%s%s", k_ax[n], prefix);
            strip_brackets_dots(result, leaf, leaf_size);
            return;
        }
    }

    /* --- Border.Radius.{sub}Radius -> Border{sub}Radius --- */
    if (strncmp(path, "Border.Radius.", 14) == 0) {
        char const *tail = path + 14;
        size_t len = strlen(tail);
        if (len > 6 && strcmp(tail + len - 6, "Radius") == 0) {
            snprintf(result, sizeof(result), "Border%.*sRadius", (int)(len - 6), tail);
            strip_brackets_dots(result, leaf, leaf_size);
            return;
        }
    }

    /* --- Axis[N].Left/Right without leading segment -> {Direction} --- */
    for (n = 0; n < 3; n++) {
        char lt[64], rt[64];
        snprintf(lt, sizeof(lt), "Axis[%d].Left", n);
        snprintf(rt, sizeof(rt), "Axis[%d].Right", n);
        if (strncmp(path, lt, strlen(lt)) == 0) {
            snprintf(result, sizeof(result), "%s%s", k_left[n], path + strlen(lt));
            strip_brackets_dots(result, leaf, leaf_size);
            return;
        }
        if (strncmp(path, rt, strlen(rt)) == 0) {
            snprintf(result, sizeof(result), "%s%s", k_right[n], path + strlen(rt));
            strip_brackets_dots(result, leaf, leaf_size);
            return;
        }
    }

    /* --- bare Axis[N] anywhere -> {Axis} --- */
    for (n = 0; n < 3; n++) {
        char tok[32]; snprintf(tok, sizeof(tok), "Axis[%d]", n);
        if (strstr(path, tok) != NULL) {
            char const *pos = strstr(path, tok);
            size_t pl = (size_t)(pos - path);
            char prefix[256];
            memcpy(prefix, path, pl);
            prefix[pl] = 0;
            snprintf(result, sizeof(result), "%s%s%s",
                    prefix, k_ax[n], pos + strlen(tok));
            strip_brackets_dots(result, leaf, leaf_size);
            return;
        }
    }

    /* Fallback: strip [,],. */
    strip_brackets_dots(path, leaf, leaf_size);
}

#endif // CODEGEN_AXIS_TRANSFORM_H
