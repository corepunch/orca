/* String ownership regressions. Run with the allocation tracker normally and
 * with ASan/UBSan (without allocator interposition) to detect invalid accesses. */
#include <include/api.h>
#include <include/orca.h>
#include <core/core.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int s_tests_run, s_tests_failed;
static const char *s_current_test;
#include "mem_tracker.h"

/* Binding compilation is an exported private seam; no private headers needed. */
extern bool_t PROP_SetBinding(struct Property *, const char *, eBindingMode_t, bool_t);

struct StringMemory {
    const char *Left, *Right, *Result, *Saved, *Inherited;
    int Count, EnumA, EnumB;
    float PairA[2], PairB[2];
    struct Object *ObjectA, *ObjectB;
};

#define STRING_FIELD(name) { .Name = #name, .Key = #name, .DataType = kDataTypeString, \
    .DataSize = sizeof(char *), .Offset = offsetof(struct StringMemory, name) }
static const char *enum_a[] = { "first", "second", NULL };
static const char *enum_b[] = { "off", "on", NULL };
static struct PropertyType properties[] = {
    STRING_FIELD(Left), STRING_FIELD(Right), STRING_FIELD(Result), STRING_FIELD(Saved),
    { .Name = "Inherited", .Key = "Inherited", .DataType = kDataTypeString,
      .DataSize = sizeof(char *), .Offset = offsetof(struct StringMemory, Inherited), .IsInherited = TRUE },
    { .Name = "Count", .Key = "Count", .DataType = kDataTypeInt,
      .DataSize = sizeof(int), .Offset = offsetof(struct StringMemory, Count) },
    { .Name = "EnumA", .Key = "EnumA", .DataType = kDataTypeEnum, .DataSize = sizeof(int),
      .Offset = offsetof(struct StringMemory, EnumA), .EnumValues = enum_a },
    { .Name = "EnumB", .Key = "EnumB", .DataType = kDataTypeEnum, .DataSize = sizeof(int),
      .Offset = offsetof(struct StringMemory, EnumB), .EnumValues = enum_b },
    { .Name = "PairA", .Key = "PairA", .DataType = kDataTypeStruct, .DataSize = sizeof(float[2]),
      .Offset = offsetof(struct StringMemory, PairA), .TypeString = "Vector2D" },
    { .Name = "PairB", .Key = "PairB", .DataType = kDataTypeStruct, .DataSize = sizeof(float[2]),
      .Offset = offsetof(struct StringMemory, PairB), .TypeString = "bounds" },
    { .Name = "ObjectA", .Key = "ObjectA", .DataType = kDataTypeObject, .DataSize = sizeof(void *),
      .Offset = offsetof(struct StringMemory, ObjectA), .TypeString = "DataContext" },
    { .Name = "ObjectB", .Key = "ObjectB", .DataType = kDataTypeObject, .DataSize = sizeof(void *),
      .Offset = offsetof(struct StringMemory, ObjectB), .TypeString = "Texture" },
};
static int destroyed_components;
static LRESULT fixture_proc(struct Object *o, void *component, uint32_t message, wParam_t w, lParam_t l)
{
    if (message == ID_Object_Destroy) destroyed_components++;
    return FALSE;
}
static struct ClassDesc fixture = {
    .ObjProc = fixture_proc, .ClassName = "StringMemory", .DefaultName = "string-memory", .ClassSize = sizeof(struct StringMemory),
    .Properties = properties, .NumProperties = ARRAY_SIZE(properties),
};

struct TinyPropertyOwner { int Value; };
static struct PropertyType tiny_property = {
    .Name = "TinyValue", .Key = "TinyValue", .DataType = kDataTypeInt,
    .DataSize = sizeof(int), .Offset = offsetof(struct TinyPropertyOwner, Value),
};
static struct ClassDesc tiny_class = {
    .ClassName = "TinyPropertyOwner", .DefaultName = "tiny-owner",
    .ClassSize = sizeof(struct TinyPropertyOwner), .Properties = &tiny_property, .NumProperties = 1,
};

static struct Object *object(void) { return OBJ_Create(fixture.ClassID); }
static struct Property *property(struct Object *obj, const char *name)
{
    struct Property *p = OBJ_FindShortProperty(obj, fnv1a32(name));
    assert(p);
    return p;
}
static const char *text(struct Property *p)
{
    const char *const *value = PROP_GetValue(p);
    return value ? *value : NULL;
}
static void expect_text(struct Property *p, const char *expected)
{
    const char *actual = text(p);
    if (!actual || strcmp(actual, expected)) {
        fprintf(stderr, "%s: expected %.100s, got %.100s\n", s_current_test,
                expected, actual ? actual : "(null)");
        abort();
    }
}
static void bind(struct Property *p, const char *expression)
{
    assert(PROP_SetBinding(p, expression, kBindingModeExpression, TRUE));
    assert(PROP_HasProgram(p));
}
static void update(struct Property *p)
{
    core_AdvanceFrame();
    assert(PROP_Update(p));
    assert(PROP_HasProgram(p));
}
static char *pattern(size_t length, char fill)
{
    char *value = malloc(length + 1);
    assert(value);
    memset(value, fill, length);
    value[length] = 0;
    return value;
}

static void destruction_delivers_component_cleanup_message(void)
{
    int before = destroyed_components;
    struct Object *parent = object(), *child = object();
    OBJ_AddChild(parent, child);
    PROP_SetStringValue(property(parent, "Left"), "parent allocation");
    PROP_SetStringValue(property(child, "Left"), "child allocation");
    OBJ_ReleaseRef(child);
    OBJ_ReleaseRef(parent);
    assert(destroyed_components == before + 2);
}

static void mutable_stack_is_copied(void)
{
    struct Object *o = object();
    struct Property *p = property(o, "Left");
    char source[] = "stack-owned value";
    PROP_SetStringValue(p, source);
    assert(text(p) != source);
    memset(source, '!', sizeof(source) - 1);
    expect_text(p, "stack-owned value");
    OBJ_ReleaseRef(o);
}

static void heap_source_can_be_freed(void)
{
    struct Object *o = object();
    struct Property *p = property(o, "Left");
    char *source = strdup("caller-owned allocation");
    PROP_SetValue(p, &source);
    assert(text(p) != source);
    memset(source, '?', strlen(source));
    free(source);
    expect_text(p, "caller-owned allocation");
    OBJ_ReleaseRef(o);
}

static void properties_own_independent_copies(void)
{
    struct Object *source = object(), *target = object();
    struct Property *a = property(source, "Left"), *b = property(target, "Right");
    PROP_SetStringValue(a, "independent lifetime");
    PROP_SetValue(b, PROP_GetValue(a));
    assert(text(a) != text(b));
    PROP_Clear(a);
    OBJ_ReleaseRef(source);
    expect_text(b, "independent lifetime");
    OBJ_ReleaseRef(target);
}

static void equal_value_and_exact_alias_are_safe(void)
{
    struct Object *o = object();
    struct Property *p = property(o, "Left");
    PROP_SetStringValue(p, "unchanged");
    long before = MEM_SNAPSHOT();
    for (int i = 0; i < 2000; i++) {
        char equal[] = "unchanged";
        PROP_SetStringValue(p, equal);
        PROP_SetValue(p, PROP_GetValue(p));
        PROP_SetStringValue(p, text(p));
        expect_text(p, "unchanged");
    }
    assert(MEM_SNAPSHOT() == before);
    OBJ_ReleaseRef(o);
}

static void interior_alias_string_setter(void)
{
    struct Object *o = object();
    struct Property *p = property(o, "Left");
    PROP_SetStringValue(p, "prefix/retained suffix");
    PROP_SetStringValue(p, text(p) + 7);
    expect_text(p, "retained suffix");
    OBJ_ReleaseRef(o);
}

static void interior_alias_generic_setter(void)
{
    struct Object *o = object();
    struct Property *p = property(o, "Left");
    PROP_SetStringValue(p, "prefix/another suffix");
    const char *suffix = text(p) + 7;
    PROP_SetValue(p, &suffix);
    expect_text(p, "another suffix");
    OBJ_ReleaseRef(o);
}

static void alias_at_terminator_becomes_empty(void)
{
    struct Object *o = object();
    struct Property *p = property(o, "Left");
    PROP_SetStringValue(p, "short");
    PROP_SetStringValue(p, text(p) + strlen(text(p)));
    expect_text(p, "");
    assert(!PROP_IsNull(p));
    OBJ_ReleaseRef(o);
}

static void long_and_unicode_values_are_owned(void)
{
    const size_t lengths[] = { 0, 1, 15, 16, 255, 256, 257, 4095, 4096, 65535, 262144 };
    struct Object *o = object();
    struct Property *p = property(o, "Left");
    for (size_t i = 0; i < ARRAY_SIZE(lengths); i++) {
        char *source = pattern(lengths[i], 'A' + i);
        PROP_SetStringValue(p, source);
        assert(strlen(text(p)) == lengths[i]);
        assert(!memcmp(text(p), source, lengths[i] + 1));
        memset(source, '?', lengths[i]);
        free(source);
        assert(strlen(text(p)) == lengths[i]);
    }
    char unicode[] = "密碼🙂 café Привет\nline\tend";
    PROP_SetStringValue(p, unicode);
    memset(unicode, 0, sizeof(unicode));
    expect_text(p, "密碼🙂 café Привет\nline\tend");
    OBJ_ReleaseRef(o);
}

static void replacement_clear_and_reuse_stress(void)
{
    struct Object *o = object();
    struct Property *p = property(o, "Left");
    PROP_SetStringValue(p, "warmup");
    long baseline = MEM_SNAPSHOT();
    for (int i = 0; i < 2000; i++) {
        char value[96];
        snprintf(value, sizeof(value), "value %d / %s", i, i % 2 ? "a" : "a much longer replacement");
        PROP_SetStringValue(p, value);
        expect_text(p, value);
        PROP_Clear(p);
        PROP_Clear(p);
        assert(PROP_IsNull(p) && text(p) == NULL);
        PROP_SetStringValue(p, "");
        assert(!PROP_IsNull(p));
        assert(MEM_SNAPSHOT() == baseline);
    }
    OBJ_ReleaseRef(o);
}

static void lua_replacements_release_conversion_storage(void)
{
    lua_State *L = luaL_newstate();
    assert(L);
    struct Object *o = object();
    struct Property *p = property(o, "Left");
    for (int i = 0; i < 2000; i++) {
        char value[96];
        snprintf(value, sizeof(value), "Lua replacement %d / 漢字🙂", i);
        lua_pushstring(L, value);
        luaX_readProperty(L, -1, p);
        lua_pop(L, 1);
        expect_text(p, value);
        if (i % 20 == 0) lua_gc(L, LUA_GCCOLLECT, 0);
    }
    lua_close(L);
    expect_text(p, "Lua replacement 1999 / 漢字🙂");
    OBJ_ReleaseRef(o);
}

static void lua_snapshots_outlive_native_property(void)
{
    lua_State *L = luaL_newstate();
    assert(L);
    struct Object *o = object();
    struct Property *p = property(o, "Left");
    PROP_SetStringValue(p, "native original");
    luaX_pushProperty(L, p);
    PROP_SetStringValue(p, "replacement");
    PROP_Clear(p);
    OBJ_ReleaseRef(o);
    lua_gc(L, LUA_GCCOLLECT, 0);
    assert(!strcmp(lua_tostring(L, -1), "native original"));
    lua_close(L);
}

static void lua_nil_empty_and_reuse_release_owned_strings(void)
{
    lua_State *L = luaL_newstate();
    assert(L);
    struct Object *o = object();
    struct Property *p = property(o, "Left");
    for (int i = 0; i < 500; i++) {
        lua_pushliteral(L, "owned value");
        luaX_readProperty(L, -1, p);
        lua_pop(L, 1);
        lua_pushnil(L);
        luaX_readProperty(L, -1, p);
        lua_pop(L, 1);
        assert(PROP_IsNull(p) && text(p) == NULL);
        lua_pushliteral(L, "");
        luaX_readProperty(L, -1, p);
        lua_pop(L, 1);
        expect_text(p, "");
        assert(!PROP_IsNull(p));
    }
    OBJ_ReleaseRef(o);
    lua_close(L);
}

static void lua_large_source_and_native_copy_have_separate_lifetimes(void)
{
    lua_State *L = luaL_newstate();
    assert(L);
    struct Object *o = object();
    struct Property *p = property(o, "Left");
    char *source = pattern(100000, 'L');
    lua_pushlstring(L, source, 100000);
    memset(source, '?', 100000);
    free(source);
    luaX_readProperty(L, -1, p);
    lua_close(L);
    assert(strlen(text(p)) == 100000);
    for (size_t i = 0; i < 100000; i++) assert(text(p)[i] == 'L');
    OBJ_ReleaseRef(o);
}

static void specialization_preserves_owned_base(void)
{
    struct Object *o = object();
    struct Property *p = property(o, "Left");
    PROP_SetStringValue(p, "base string");
    for (int i = 0; i < 500; i++) {
        PROP_SetFlag(p, PF_SPECIALIZED);
        PROP_SetFlag(p, PF_SPECIALIZED);
        PROP_SetStringValue(p, "hover");
        PROP_SetStringValue(p, "focused");
        PROP_ClearSpecialized(p);
        expect_text(p, "base string");
    }
    PROP_SetFlag(p, PF_SPECIALIZED);
    PROP_SetStringValue(p, "destroy while overridden");
    OBJ_ReleaseRef(o);
}

static void specialization_of_unset_string(void)
{
    struct Object *o = object();
    struct Property *p = property(o, "Left");
    PROP_SetFlag(p, PF_SPECIALIZED);
    PROP_SetStringValue(p, "temporary override");
    PROP_ClearSpecialized(p);
    assert(PROP_IsNull(p) && text(p) == NULL);
    OBJ_ReleaseRef(o);
}

static void inherited_string_override_owns_copy(void)
{
    struct Object *parent = object(), *child = object();
    OBJ_AddChild(parent, child);
    struct Property *a = property(parent, "Inherited"), *b = property(child, "Inherited");
    PROP_SetStringValue(a, "inherited base");
    expect_text(b, "inherited base");
    PROP_SetStringValue(b, text(b));
    assert(text(a) != text(b));
    PROP_SetStringValue(a, "changed parent");
    expect_text(b, "inherited base");
    PROP_Clear(b);
    expect_text(b, "changed parent");
    OBJ_ReleaseRef(child);
    OBJ_ReleaseRef(parent);
}

static void inherited_string_reparent_and_parent_release(void)
{
    struct Object *a = object(), *b = object(), *child = object();
    PROP_SetStringValue(property(a, "Inherited"), "first parent");
    PROP_SetStringValue(property(b, "Inherited"), "second parent");
    struct Property *p = property(child, "Inherited");
    OBJ_AddChild(a, child);
    expect_text(p, "first parent");
    OBJ_AddChild(b, child);
    expect_text(p, "second parent");
    OBJ_ReleaseRef(a);
    OBJ_RemoveFromParent(child);
    assert(PROP_GetValue(p) == NULL);
    OBJ_ReleaseRef(b);
    PROP_SetStringValue(p, "now locally owned");
    expect_text(p, "now locally owned");
    OBJ_ReleaseRef(child);
}

static void binding_expression_is_copied(void)
{
    struct Object *o = object();
    struct Property *p = property(o, "Result");
    char *expression = strdup("\"temporary expression storage\"");
    bind(p, expression);
    memset(expression, '!', strlen(expression));
    free(expression);
    assert(!strcmp(PROP_GetBindingExpression(p), "\"temporary expression storage\""));
    update(p);
    expect_text(p, "temporary expression storage");
    OBJ_ReleaseRef(o);
}

static void binding_expression_can_replace_itself(void)
{
    struct Object *o = object();
    struct Property *p = property(o, "Result");
    bind(p, "\"stable expression\"");
    for (int i = 0; i < 200; i++) {
        const char *expression = PROP_GetBindingExpression(p);
        bind(p, expression);
        assert(!strcmp(PROP_GetBindingExpression(p), "\"stable expression\""));
        update(p);
        expect_text(p, "stable expression");
    }
    OBJ_ReleaseRef(o);
}

static void public_attach_expression_alias_is_owned(void)
{
    struct Object *o = object();
    struct Property *p = property(o, "Result");
    assert(OBJ_AttachPropertyProgram(o, "Result", "\"public binding\"", kBindingModeExpression, TRUE));
    for (int i = 0; i < 200; i++) {
        const char *expression = PROP_GetBindingExpression(p);
        assert(OBJ_AttachPropertyProgram(o, "Result", expression, kBindingModeExpression, TRUE));
        assert(!strcmp(PROP_GetBindingExpression(p), "\"public binding\""));
        update(p);
        expect_text(p, "public binding");
    }
    OBJ_ReleaseRef(o);
}

static void public_attach_rejects_missing_target_without_leaking(void)
{
    struct Object *o = object();
    struct Property *p = property(o, "Result");
    bind(p, "\"keep this binding\"");
    for (int i = 0; i < 100; i++)
        assert(!OBJ_AttachPropertyProgram(o, "MissingStringProperty", "\"unused token\"", kBindingModeExpression, TRUE));
    update(p);
    expect_text(p, "keep this binding");
    OBJ_ReleaseRef(o);
}

static void rejected_binding_preserves_existing_ownership(void)
{
    struct Object *o = object();
    struct Property *p = property(o, "Result");
    bind(p, "\"still valid\"");
    assert(!PROP_SetBinding(p, "", kBindingModeExpression, TRUE));
    assert(!PROP_SetBinding(p, NULL, kBindingModeExpression, TRUE));
    update(p);
    expect_text(p, "still valid");
    assert(!strcmp(PROP_GetBindingExpression(p), "\"still valid\""));
    OBJ_ReleaseRef(o);
}

static void binding_clear_and_recompute(void)
{
    struct Object *o = object();
    struct Property *a = property(o, "Left"), *p = property(o, "Result");
    bind(p, "{./Left} + \"!\"");
    for (int i = 0; i < 500; i++) {
        PROP_SetStringValue(a, i % 2 ? "alpha" : "beta");
        update(p);
        expect_text(p, i % 2 ? "alpha!" : "beta!");
        PROP_Clear(p);
        assert(text(p) == NULL && PROP_HasProgram(p));
        update(p);
        expect_text(p, i % 2 ? "alpha!" : "beta!");
    }
    OBJ_ReleaseRef(o);
}

static void replacement_of_binding_and_disabled_binding(void)
{
    struct Object *o = object();
    struct Property *p = property(o, "Result");
    for (int i = 0; i < 500; i++) {
        char expression[100], expected[80];
        snprintf(expected, sizeof(expected), "replacement-%d", i);
        snprintf(expression, sizeof(expression), "\"%s\"", expected);
        bind(p, expression);
        update(p);
        expect_text(p, expected);
        assert(PROP_SetBinding(p, "\"disabled\"", kBindingModeExpression, FALSE));
        assert(!PROP_HasProgram(p));
        expect_text(p, expected);
        assert(!strcmp(PROP_GetBindingExpression(p), "\"disabled\""));
    }
    OBJ_ReleaseRef(o);
}

static void nested_conditional_binding_stress(void)
{
    struct Object *o = object();
    struct Property *left = property(o, "Left"), *right = property(o, "Right");
    struct Property *count = property(o, "Count"), *p = property(o, "Result");
    PROP_SetStringValue(left, "密碼🙂");
    PROP_SetStringValue(right, "fallback");
    bind(p, "IF({./Count}, ({./Left} + \":\") + STRING({./Count}), {./Right} + \"!\")");
    for (int i = 0; i < 1000; i++) {
        int n = i % 2 ? i : 0;
        PROP_SetValue(count, &n);
        update(p);
        char expected[96];
        if (n) snprintf(expected, sizeof(expected), "密碼🙂:%d", n);
        else snprintf(expected, sizeof(expected), "fallback!");
        expect_text(p, expected);
    }
    OBJ_ReleaseRef(o);
}

static void binding_chain_preserves_snapshots(void)
{
    struct Object *o = object();
    struct Property *a = property(o, "Left"), *b = property(o, "Right");
    struct Property *c = property(o, "Result"), *saved = property(o, "Saved");
    bind(b, "\"[\" + {./Left} + \"]\"");
    bind(c, "{./Right} + \"|\" + {./Left}");
    for (int i = 0; i < 500; i++) {
        char value[48], expected[100];
        snprintf(value, sizeof(value), "source-%d", i);
        PROP_SetStringValue(a, value);
        core_AdvanceFrame();
        PROP_RunAllPrograms(o);
        snprintf(expected, sizeof(expected), "[%s]|%s", value, value);
        expect_text(c, expected);
        PROP_SetValue(saved, PROP_GetValue(c));
        PROP_Clear(a);
        PROP_Clear(b);
        PROP_Clear(c);
        expect_text(saved, expected);
    }
    OBJ_ReleaseRef(o);
}

static void binding_fanout_and_source_destruction(void)
{
    struct Object *parent = object(), *children[32];
    PROP_SetStringValue(property(parent, "Left"), "shared source");
    for (int i = 0; i < 32; i++) {
        children[i] = object();
        OBJ_AddChild(parent, children[i]);
        bind(property(children[i], "Result"), "{../Left} + \"!\"");
    }
    core_AdvanceFrame();
    PROP_RunAllPrograms(parent);
    for (int i = 0; i < 32; i++) {
        OBJ_RemoveFromParent(children[i]);
        expect_text(property(children[i], "Result"), "shared source!");
    }
    OBJ_ReleaseRef(parent);
    for (int i = 31; i >= 0; i--) {
        expect_text(property(children[i], "Result"), "shared source!");
        OBJ_ReleaseRef(children[i]);
    }
}

static void specialization_preserves_binding_ownership(void)
{
    struct Object *o = object();
    struct Property *a = property(o, "Left"), *p = property(o, "Result");
    PROP_SetStringValue(a, "base");
    bind(p, "{./Left} + \"!\"");
    update(p);
    for (int i = 0; i < 200; i++) {
        PROP_SetFlag(p, PF_SPECIALIZED);
        bind(p, "\"state binding\"");
        update(p);
        expect_text(p, "state binding");
        PROP_ClearSpecialized(p);
        update(p);
        expect_text(p, "base!");
    }
    PROP_SetFlag(p, PF_SPECIALIZED);
    bind(p, "\"destroy specialized binding\"");
    update(p);
    OBJ_ReleaseRef(o);
}

static void bound_result_survives_vm_temporary_reuse(void)
{
    struct Object *o = object();
    struct Property *a = property(o, "Left"), *p = property(o, "Result"), *saved = property(o, "Saved");
    char *source = pattern(5000, 'S');
    PROP_SetStringValue(a, source);
    bind(p, "{./Left}");
    update(p);
    PROP_SetValue(saved, PROP_GetValue(p));
    expect_text(saved, source);
    for (int i = 0; i < 200; i++) {
        char *next = pattern(5000, 'a' + i % 26);
        PROP_SetStringValue(a, next);
        free(next);
        update(p);
        expect_text(saved, source);
    }
    free(source);
    OBJ_ReleaseRef(o);
}

static void long_nested_binding_keeps_live_operands(void)
{
    struct Object *o = object();
    struct Property *a = property(o, "Left"), *b = property(o, "Right"), *p = property(o, "Result");
    char *left = pattern(4096, 'A'), *right = pattern(4096, 'B');
    PROP_SetStringValue(a, left);
    PROP_SetStringValue(b, right);
    free(left); free(right);
    char expression[1024] = "{./Left} + (";
    for (int i = 0; i < 20; i++) strcat(expression, "{./Right} + (");
    strcat(expression, "\"end\"");
    for (int i = 0; i < 21; i++) strcat(expression, ")");
    bind(p, expression);
    update(p);
    char *expected = pattern(MAX_PROPERTY_STRING - 1, 'A');
    expect_text(p, expected);
    free(expected);
    OBJ_ReleaseRef(o);
}

static void nested_program_overflow_keeps_outer_operands(void)
{
    struct Object *o = object();
    char *a = pattern(4096, 'A'), *b = pattern(4096, 'B');
    PROP_SetStringValue(property(o, "Left"), a);
    PROP_SetStringValue(property(o, "Right"), b);
    free(a); free(b);
    char expression[1024] = "";
    for (int i = 0; i < 20; i++) strcat(expression, "{./Right} + (");
    strcat(expression, "\"end\"");
    for (int i = 0; i < 20; i++) strcat(expression, ")");
    bind(property(o, "Saved"), expression);
    struct Property *p = property(o, "Result");
    bind(p, "{./Left} + {./Saved}");
    char *expected = pattern(MAX_PROPERTY_STRING - 1, 'A');
    for (int i = 0; i < 40; i++) {
        update(p);
        expect_text(p, expected);
    }
    free(expected);
    OBJ_ReleaseRef(o);
}

static void failed_overflow_evaluation_releases_scratch(void)
{
    struct Object *o = object();
    char *source = pattern(4096, 'R');
    PROP_SetStringValue(property(o, "Left"), source);
    free(source);
    struct Property *p = property(o, "Result");
    char expression[1024] = "";
    for (int i = 0; i < 20; i++) strcat(expression, "{./Left} + (");
    strcat(expression, "{./MissingAfterOverflow}");
    for (int i = 0; i < 20; i++) strcat(expression, ")");
    PROP_SetStringValue(p, "last good value");
    for (int i = 0; i < 30; i++) {
        bind(p, expression);
        core_AdvanceFrame();
        assert(PROP_Update(p));
        assert(!PROP_HasProgram(p));
        expect_text(p, "last good value");
        bind(p, "\"recovered\"");
        update(p);
        expect_text(p, "recovered");
        PROP_SetStringValue(p, "last good value");
    }
    OBJ_ReleaseRef(o);
}

static void binding_failure_releases_expression_and_token(void)
{
    struct Object *o = object();
    struct Property *p = property(o, "Result");
    PROP_SetStringValue(p, "last good value");
    bind(p, "{./MissingBindingSource}");
    core_AdvanceFrame();
    assert(PROP_Update(p));
    assert(!PROP_HasProgram(p) && PROP_GetBindingExpression(p) == NULL);
    expect_text(p, "last good value");
    OBJ_ReleaseRef(o);
}

static void writable_binding_rejects_readonly_and_mismatched_sources(void)
{
    struct Object *o = object();
    struct Property *source = property(o, "Left"), *target = property(o, "Result");
    PROP_SetStringValue(source, "readonly source");
    bind(target, "\"previous binding\"");
    properties[0].IsReadOnly = TRUE;
    assert(!PROP_SetBinding(target, "{./Left}", kBindingModeTwoWay, TRUE));
    assert(!PROP_SetBinding(target, "{./Left}", kBindingModeOneWayToSource, TRUE));
    properties[0].IsReadOnly = FALSE;
    update(target);
    expect_text(target, "previous binding");
    assert(!PROP_SetBinding(target, "{./Count}", kBindingModeTwoWay, TRUE));
    update(target);
    expect_text(target, "previous binding");
    assert(PROP_SetBinding(target, "{./Left}", kBindingModeTwoWay, TRUE));
    update(target);
    expect_text(target, "readonly source");
    PROP_SetStringValue(target, "edited draft");
    expect_text(source, "edited draft");
    OBJ_ReleaseRef(o);
}

static void writable_binding_rejects_incompatible_compound_types(void)
{
    struct Object *o = object();
    assert(!PROP_SetBinding(property(o, "EnumA"), "{./EnumB}", kBindingModeTwoWay, TRUE));
    assert(!PROP_SetBinding(property(o, "PairA"), "{./PairB}", kBindingModeTwoWay, TRUE));
    assert(!PROP_SetBinding(property(o, "ObjectA"), "{./ObjectB}", kBindingModeOneWayToSource, TRUE));
    assert(!PROP_HasProgram(property(o, "EnumA")) && !PROP_HasProgram(property(o, "PairA")) &&
           !PROP_HasProgram(property(o, "ObjectA")));
    OBJ_ReleaseRef(o);
}

static void small_component_property_cache_is_aligned(void)
{
    struct Object *o = OBJ_Create(tiny_class.ClassID);
    struct Property *p = property(o, "TinyValue");
    int value = 42;
    PROP_SetValue(p, &value);
    assert(OBJ_GetPropertyAtIndex(o, tiny_class.ClassID, sizeof(struct TinyPropertyOwner), 0) == p);
    assert(*(const int *)PROP_GetValue(p) == 42);
    OBJ_ReleaseRef(o);
}

static void destruction_order_stress(void)
{
    for (int iteration = 0; iteration < 200; iteration++) {
        struct Object *parent = object(), *child = object();
        OBJ_AddChild(parent, child);
        PROP_SetStringValue(property(parent, "Left"), "parent bytes");
        struct Property *p = property(child, "Result");
        bind(p, "\"child: \" + {../Left}");
        update(p);
        expect_text(p, "child: parent bytes");
        if (iteration % 2) {
            OBJ_ReleaseRef(child);
            OBJ_ReleaseRef(parent);
        } else {
            OBJ_RemoveFromParent(child);
            OBJ_ReleaseRef(parent);
            expect_text(p, "child: parent bytes");
            OBJ_ReleaseRef(child);
        }
    }
}

#define TEST(name) { #name, name }
int main(int argc, char **argv)
{
    puts("ORCA string property memory tests");
    fixture.ClassID = fnv1a32(fixture.ClassName);
    for (size_t i = 0; i < ARRAY_SIZE(properties); i++) {
        properties[i].ShortIdentifier = fnv1a32(properties[i].Name);
        /* Runtime-only fixtures use the VM's unqualified property paths. */
        properties[i].FullIdentifier = properties[i].ShortIdentifier;
    }
    OBJ_RegisterClass(&fixture);
    tiny_class.ClassID = fnv1a32(tiny_class.ClassName);
    tiny_property.ShortIdentifier = fnv1a32(tiny_property.Name);
    tiny_property.FullIdentifier = fnv1a32("TinyPropertyOwner.TinyValue");
    OBJ_RegisterClass(&tiny_class);
    mem_init();
    const struct { const char *name; void (*run)(void); } tests[] = {
        TEST(destruction_delivers_component_cleanup_message), TEST(mutable_stack_is_copied), TEST(heap_source_can_be_freed), TEST(properties_own_independent_copies),
        TEST(equal_value_and_exact_alias_are_safe), TEST(interior_alias_string_setter),
        TEST(interior_alias_generic_setter), TEST(alias_at_terminator_becomes_empty),
        TEST(long_and_unicode_values_are_owned), TEST(replacement_clear_and_reuse_stress),
        TEST(lua_replacements_release_conversion_storage), TEST(lua_snapshots_outlive_native_property),
        TEST(lua_nil_empty_and_reuse_release_owned_strings),
        TEST(lua_large_source_and_native_copy_have_separate_lifetimes), TEST(specialization_preserves_owned_base), TEST(specialization_of_unset_string),
        TEST(inherited_string_override_owns_copy), TEST(inherited_string_reparent_and_parent_release),
        TEST(binding_expression_is_copied), TEST(binding_expression_can_replace_itself),
        TEST(public_attach_expression_alias_is_owned), TEST(public_attach_rejects_missing_target_without_leaking),
        TEST(rejected_binding_preserves_existing_ownership), TEST(binding_clear_and_recompute), TEST(replacement_of_binding_and_disabled_binding),
        TEST(nested_conditional_binding_stress), TEST(binding_chain_preserves_snapshots),
        TEST(binding_fanout_and_source_destruction), TEST(specialization_preserves_binding_ownership),
        TEST(bound_result_survives_vm_temporary_reuse), TEST(long_nested_binding_keeps_live_operands),
        TEST(nested_program_overflow_keeps_outer_operands), TEST(failed_overflow_evaluation_releases_scratch),
        TEST(binding_failure_releases_expression_and_token),
        TEST(writable_binding_rejects_readonly_and_mismatched_sources),
        TEST(writable_binding_rejects_incompatible_compound_types),
        TEST(small_component_property_cache_is_aligned), TEST(destruction_order_stress),
    };
    for (size_t i = 0; i < ARRAY_SIZE(tests); i++) {
        if (argc > 1 && !strstr(tests[i].name, argv[1])) continue;
        s_current_test = tests[i].name;
        printf("Running %s...\n", s_current_test);
        fflush(stdout);
        long baseline = MEM_SNAPSHOT();
        tests[i].run();
        MEM_CHECK_LEAK(baseline, s_current_test);
        assert(MEM_SNAPSHOT() == baseline);
        s_tests_run++;
    }
    assert(s_tests_run > 0);
    printf("PASS: %d string ownership cases, %d allocation failures\n", s_tests_run, s_tests_failed);
    return s_tests_failed ? 1 : 0;
}
