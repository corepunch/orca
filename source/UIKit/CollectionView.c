#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <source/UIKit/UIKit.h>


// typedef void (*callback_t)(Node2DPtr, Node2DPtr, enum Direction, float const
// *, struct CollectionView const *);
//
// static bool_t
// CollectionView_do_layout(Node2DPtr node2d,
//						 CollectionViewPtr
// collectionView, 						 enum Direction
// axis, 						 callback_t callback)
//{
//     int const X = collectionView->Direction;
//     int const Y = !collectionView->Direction;
//
//     int const width  = kBox3FieldWidth + X;
//     int const height = kBox3FieldWidth + Y;
//
//     struct vec2 x_padding = { PADDING_TOP(node2d, X), PADDING_BOTTOM(node2d,
//     X)
//     }; struct vec2 y_padding = { PADDING_TOP(node2d, Y),
//     PADDING_BOTTOM(node2d, Y) };
//
//     float rowHeight = 0;
//     //    float avl_width2 = Node2D_GetFrame(OBJ_GetObject(object, kGetObjectPartParent),
//     //    width);
//     float avl_width = Node2D_GetFrame(node2d, width);
//     float origin[2] = { x_padding.x, y_padding.x };
//
//     // we need available space calculated
//     if (!Node2D_IsFrameSet(node2d, width))
//         return FALSE;
//
//	FOR_EACH_LAYOUTABLE(child, node2d->_object)
//     {
//		Node2DPtr subview = GetNode2D(child);
//         if (!Node2D_IsFrameSet(subview, width) ||
//         !Node2D_IsFrameSet(subview, height))
//             return FALSE;
//
//         float next = origin[0] + Node2D_GetFrame(subview, width)
//                      + collectionView->Spacing;
//
//         if (next > (avl_width - x_padding.y) && rowHeight > 0)
//         {
//             origin[0] = x_padding.x;
//             origin[1] = origin[1] + rowHeight + collectionView->Spacing;
//             next      = origin[0] + Node2D_GetFrame(subview, width) +
//             collectionView->Spacing; rowHeight = 0;
//         }
//
//         struct rect const frame = {
//             origin[X],
//             origin[Y],
//             Node2D_GetFrame(subview, kBox3FieldWidth),
//             Node2D_GetFrame(subview, kBox3FieldHeight),
//         };
//
//		callback(node2d, subview, axis, (float const *)&frame,
// collectionView);
//
//         rowHeight = MAX(rowHeight, Node2D_GetFrame(subview, height));
//         origin[0] = next;
//     }
//
//     return TRUE;
// }
//
// static void
// CollectionView_set_actual_height(
//	Node2DPtr                    object,
//	Node2DPtr                    subview,
//     enum Direction               axis,
//     float const                 *frame,
//     struct CollectionView const *collectionView
//)
//{
//     float const bottomPadding = PADDING_BOTTOM(subview, axis);
//     float const currentHeight = Node2D_GetFrame(subview, kBox3FieldWidth +
//     axis); float const height
//         = MAX(currentHeight, frame[0 + axis] + frame[2 + axis] +
//         bottomPadding);
//     Node2D_SetFrame(object, kBox3FieldWidth + axis, height);
// }
//
// static void
// CollectionView_set_position(
//     Node2DPtr                    object,
//     Node2DPtr                    subview,
//     enum Direction               axis,
//     float const                 *frame,
//     struct CollectionView const *collectionView
//)
//{
//     Node2D_SetFrame(subview, kBox3FieldX + axis, *(frame + axis));
// }
//
// HANDLER(CollectionView, Arrange)
//{
//	CollectionView_do_layout(GetNode2D(hObject),
//							 pCollectionView,
//							 pArrange->axis,
//							 CollectionView_set_position);
//     return TRUE;
// }
//
HANDLER(CollectionView, UpdateLayout)
{
  //    // fill whole available space in layout direction
  //    if (pContentSize->axis == pCollectionView->Direction)
  //    {
  //        return FALSE;
  //    }
  //    else if (CollectionView_do_layout(GetNode2D(hObject),
  //									  pCollectionView,
  //									  pContentSize->axis,
  //									  CollectionView_set_actual_height))
  //    {
  //        return TRUE;
  //    }
  //    else
  {
    return TRUE;
  }
}
