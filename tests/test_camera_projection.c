#include <include/orca.h>
#include <include/renderer.h>
#include <core/core.h>
#include <SceneKit/SceneKit.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

/* Private exported seam: capture the real render-pass camera matrices without
 * entering R_RenderViewport's GL pipeline-state code or adding public API. */
extern void _OBJ_Draws(struct Object *, struct Object *, struct ViewDef *);
extern struct ClassDesc _Node, _Node3D, _Scene, _Camera;
extern struct ClassDesc _RenderPass, _DrawObjectsRenderPass;

static struct ViewDef captured;
static unsigned captures;

static LRESULT CaptureProc(struct Object *object, void *component,
                          uint32_t message, wParam_t wp, lParam_t lp)
{
  (void)object; (void)component; (void)wp;
  if (message != ID_Node3D_Render) return FALSE;
  struct Node3D_RenderEventArgs const *args = (void const *)lp;
  captured = *args->ViewDef;
  captures++;
  return TRUE;
}

static struct ClassDesc capture_class = {
  .ClassName = "CameraProjectionCapture",
  .DefaultName = "CameraProjectionCapture",
  .ClassID = 0xaabb6721u,
  .ClassSize = 1,
  .ObjProc = CaptureProc,
  .ParentClasses = {ID_Node3D, 0},
};

static void expect_close(float actual, double expected)
{
  if (fabs(actual - expected) <= 0.00001) return;
  fprintf(stderr, "camera projection: %.9g != %.9g\n", actual, expected);
  assert(0);
}

static void check_projection(struct Object *pass, struct Object *scene,
                             struct Camera *camera, enum FovType type,
                             float fov, float width, float height)
{
  camera->Fov = fov;
  camera->FovType = type;
  struct ViewDef view = {.viewSize = {width, height}, .camera = fnv1a32("TestCamera")};
  captures = 0;
  _OBJ_Draws(pass, scene, &view);
  assert(captures == 1);

  /* Independent textbook perspective expectations, not another engine helper. */
  double aspect = (double)width / height;
  double tangent = tan(fov * acos(-1.0) / 360.0);
  double half_width = type == kFovTypeXfov ? tangent : tangent * aspect;
  double half_height = type == kFovTypeYfov ? tangent : tangent / aspect;
  struct mat4 const *p = &captured.projectionMatrix;
  expect_close(p->v[0], 1.0 / half_width);
  expect_close(p->v[5], 1.0 / half_height);
  expect_close(p->v[11], -1);
  expect_close(p->v[15], 0);
  expect_close(p->v[8], 0);
  expect_close(p->v[12], 0);

  /* At depth 2, authored frustum edges must land at exactly +/-1 in NDC. */
  expect_close((float)(p->v[0] * (2 * half_width) / 2), 1);
  expect_close((float)(p->v[0] * (-2 * half_width) / 2), -1);
  expect_close((float)(p->v[5] * (2 * half_height) / 2), 1);
  expect_close((float)(p->v[5] * (-2 * half_height) / 2), -1);
  expect_close((-p->v[10] * camera->ZNear + p->v[14]) / camera->ZNear, -1);
  expect_close((-p->v[10] * camera->ZFar + p->v[14]) / camera->ZFar, 1);

  /* Identity, Y-up, -Z-forward camera must preserve the world axes. */
  for (int i = 0; i < 16; i++)
    expect_close(captured.viewMatrix.v[i], i % 5 == 0 ? 1 : 0);
}

int main(void)
{
  OBJ_RegisterClass(&_Node);
  OBJ_RegisterClass(&_Node3D);
  OBJ_RegisterClass(&_Scene);
  OBJ_RegisterClass(&_Camera);
  OBJ_RegisterClass(&_RenderPass);
  OBJ_RegisterClass(&_DrawObjectsRenderPass);
  OBJ_RegisterClass(&capture_class);

  struct Object *scene = OBJ_Create(ID_Scene);
  struct Object *camera_object = OBJ_Create(ID_Camera);
  struct Object *capture = OBJ_Create(capture_class.ClassID);
  struct Object *pass = OBJ_Create(ID_DrawObjectsRenderPass);
  assert(scene && camera_object && capture && pass);
  OBJ_SetName(camera_object, "TestCamera");
  OBJ_AddChild(scene, camera_object);
  OBJ_AddChild(scene, capture);
  _SendMessage(scene, Node, UpdateMatrix, .parent = MAT4_Identity(), .opacity = 1, .force = TRUE);

  struct Camera *camera = GetCamera(camera_object);
  camera->ZNear = 0.1f;
  camera->ZFar = 100;
  camera->ZPositive = FALSE;
  float const sizes[][2] = {{1024, 1024}, {1536, 1024}, {1920, 1080}, {768, 1024}};
  for (unsigned i = 0; i < sizeof(sizes) / sizeof(*sizes); i++) {
    check_projection(pass, scene, camera, kFovTypeYfov, 60, sizes[i][0], sizes[i][1]);
    check_projection(pass, scene, camera, kFovTypeXfov, 90, sizes[i][0], sizes[i][1]);
  }
  OBJ_ReleaseRef(pass);
  OBJ_ReleaseRef(scene);
  puts("Camera projection: native render pass preserves authored FOV at four aspect ratios");
  return 0;
}
