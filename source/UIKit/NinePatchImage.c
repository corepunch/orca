#include <include/api.h>
#include <include/orca.h>
#include <include/plugapi.h>
#include <include/renderer.h>

#include <source/UIKit/UIKit.h>


enum
{
  kImageTopLeft,
  kImageTop,
  kImageTopRight,
  kImageLeft,
  kImageCenter,
  kImageRight,
  kImageBottomLeft,
  kImageBottom,
  kImageBottomRight,
  kImageCount,
};

HANDLER(NinePatchImage, UpdateLayout)
{
  //    struct image_format format;
  //    if (ImageView_compile(object->subclass->data, &format)) {
  //        int const width = format.image->width;
  //        int const height = format.image->height;
  //        int const scale = 100;//format.image->scale;
  //        struct edges const *e = &format.insets;
  //        switch (axis) {
  //            case kDirectionHorizontal:
  //                Node2D_SetFrame(object, kBox3FieldWidth, (width - e->left -
  //                e->right) * 100 / scale); break;
  //            case kDirectionVertical:
  //                Node2D_SetFrame(object, kBox3FieldHeight, (height - e->top
  //                - e->bottom) * 100 / scale); break;
  //            default:
  //                break;
  //        }
  //        return TRUE;
  //    } else {
  return TRUE;
  //    }
}

static float
grid_value(int index, int node_width, int const* image_widths)
{
  switch (index) {
    case 0:
      return 0;
    case 1:
      return image_widths[0];
    case 2:
      return node_width - image_widths[2];
    case 3:
      return node_width;
    default:
      return 0;
  }
}

static uint32_t
GetImageWidth(handle_t _in)
{
  struct image_info img;
  Image_GetInfo(_in, &img);
  return img.bmWidth;
}

static uint32_t
GetImageHeight(handle_t _in)
{
  struct image_info img;
  Image_GetInfo(_in, &img);
  return img.bmHeight;
}

HANDLER(NinePatchImage, ForegroundContent)
{
  return TRUE;
}

HANDLER(NinePatchImage, DrawBrush)
{
	if (!memcmp(pDrawBrush->brush,
							&(struct BrushShorthand){0},
							sizeof(struct BrushShorthand)))
    return FALSE;

  Node2DPtr pNode2D = GetNode2D(hObject);
  int const node_width = Node2D_GetFrame(pNode2D, kBox3FieldWidth);
  int const node_height = Node2D_GetFrame(pNode2D, kBox3FieldHeight);

  struct Texture** images = &pNinePatchImage->ImageTopLeft;

  FOR_LOOP(index, kImageCount)
  {
    int const row = index % 3;
    int const column = index / 3;
    int const widths[] = {
      GetImageWidth(images[row * 3]),
      GetImageWidth(images[row * 3 + 1]),
      GetImageWidth(images[row * 3 + 2]),
    };
    int const heights[] = {
      GetImageHeight(images[column]),
      GetImageHeight(images[column + 3]),
      GetImageHeight(images[column + 6]),
    };
    struct vec2 origin = {
      .x = grid_value(row, node_width, widths),
      .y = grid_value(column, node_height, heights),
    };
    struct rect const frame = {
      .x = origin.x,
      .y = origin.y,
      .width = grid_value(row + 1, node_width, widths) - origin.x,
      .height = (grid_value(column + 1, node_height, heights) - origin.y),
    };
    struct ViewEntity entity;

    Node2D_GetViewEntity(hObject, &entity, images[index], pDrawBrush->brush);

    entity.bbox = BOX3_FromRect(frame);

    R_DrawEntity(pDrawBrush->viewdef, &entity);
  }
  return TRUE;
}
