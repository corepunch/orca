extern "C" {
#include <include/shared.h>
}

#if defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdocumentation"
#pragma GCC diagnostic ignored "-Wdocumentation-unknown-command"
#pragma GCC diagnostic ignored "-Wdocumentation-pedantic"
#endif

#include <fbxsdk.h>

#if defined(__clang__)
#pragma GCC diagnostic pop
#endif

#include <stdio.h>

#include <vector>

#define MAKE_FOURCC(a, b, c, d) ((int)(d) << 24 | (int)(c) << 16 | (b) << 8 | (a))

static int const IDMESHHEADER = MAKE_FOURCC('M', 'E', 'S', 'H');

typedef uint32_t DRAWINDEX, *PDRAWINDEX;
typedef uint32_t DRAWSURFATTR, *PSURFATTR;

typedef struct _MORPHTRG {
  struct vec3 position;
  struct vec3 normal;
  struct vec3 tangent;
} MORPHTRG, *PMORPHTRG;

typedef struct _DRAWVERT {
  struct vec3  xyz;
  struct vec3  normal;
  struct vec3  tangent;
  struct vec3  binormal;
  struct vec2  texcoord[5];
  struct vec4  weight;
  struct vec4  matrix_indices;
  MORPHTRG morph_target[4];
  struct color32  color;
  struct color32  userdata;
} DRAWVERT, *PDRAWVERT;

typedef struct _DRAWSUBMESH {
  uint32_t start;
  uint32_t numIndices;
} DRAWSUBMESH, *PDRAWSUBMESH;

typedef struct _DRAWSURF {
  PDRAWVERT  vertices;
  PDRAWINDEX   indices;
  PDRAWSUBMESH submeshes;
  PDRAWINDEX   neighbors;
  uint32_t     numVertices;
  uint32_t     numIndices;
  uint32_t     numSubmeshes;
} DRAWSURF, *PDRAWSURF;

uint32_t const kNumSubmaterials = 256;

struct vertexLink_t {
  vertexLink_t() : next(nullptr), index(-1) {}
  ~vertexLink_t() {
    if (next) delete next;
  }
  vertexLink_t *next;
  int       index;
};

static void API_Copy(const FbxVector4 &a, vec3 &b) {
  b.x = static_cast<float>(a[0]);
  b.y = static_cast<float>(a[1]);
  b.z = static_cast<float>(a[2]);
}

// static void API_Copy(const FbxDouble4& a, vec4& b) {
//   b[0] = static_cast<float>(a[0]);
//   b[1] = static_cast<float>(a[1]);
//   b[2] = static_cast<float>(a[2]);
//   b[3] = static_cast<float>(a[3]);
// }

static void API_Copy(const FbxDouble2 &a, vec2 &b) {
  b.x = static_cast<float>(a[0]);
  b.y = static_cast<float>(a[1]);
}

static void API_Copy(const FbxDouble4 &a, struct color32 &b) {
  int8_t color[4];
  color[0] = static_cast<int8_t>(a[0] * 127);
  color[1] = static_cast<int8_t>(a[1] * 127);
  color[2] = static_cast<int8_t>(a[2] * 127);
  color[3] = static_cast<int8_t>(a[3] * 127);
  memcpy(&b, color, 4);
}

static void API_Copy2(const FbxDouble3 &a, struct color32 &b) {
  int8_t color[4];
  color[0] = static_cast<int8_t>(a[0] * 127);
  color[1] = static_cast<int8_t>(a[1] * 127);
  color[2] = static_cast<int8_t>(a[2] * 127);
  color[3] = 127;
  memcpy(&b, color, 4);
}

// static void API_Copy(const FbxDouble3& a, vec4& b) {
//   float const sRGB = 1.0f / 2.2f;
//   b[0] = static_cast<float>(pow(a[0], sRGB));
//   b[1] = static_cast<float>(pow(a[1], sRGB));
//   b[2] = static_cast<float>(pow(a[2], sRGB));
//   b[3] = 1.0f;
// }

static void API_Copy(const FbxColor &a, struct color32 &b) {
  uint8_t color[4];
  color[0] = static_cast<uint8_t>(a[0] * 255);
  color[1] = static_cast<uint8_t>(a[1] * 255);
  color[2] = static_cast<uint8_t>(a[2] * 255);
  color[3] = 255;
  memcpy(&b, color, 4);
}

template <typename T1, typename T2>
bool API_Copy(const FbxLayerElementTemplate<T1> *fbx,
        int                vertex,
        int                ctrlPoint,
        T2                &output) {
  if (fbx == nullptr) return FALSE;
  int index  = 0;
  int FBXIndex = fbx->GetMappingMode() == FbxGeometryElement::eByPolygonVertex
             ? vertex
             : ctrlPoint;
  switch (fbx->GetReferenceMode()) {
    case FbxGeometryElement::eDirect:
      index = FBXIndex;
      break;
    case FbxGeometryElement::eIndexToDirect:
      index = fbx->GetIndexArray().GetAt(FBXIndex);
      break;
    default:
      return FALSE;
  }
  API_Copy(fbx->GetDirectArray().GetAt(index), output);
  return TRUE;
}

template <typename T1, typename T2>
bool API_CopyTransformed(const FbxLayerElementTemplate<T1> *fbx,
                         int vertex,
                         int ctrlPoint,
                         T2 &output,
                         FbxAMatrix const &geomTrans) {
  if (fbx == nullptr) return FALSE;
  int index  = 0;
  int FBXIndex = fbx->GetMappingMode() == FbxGeometryElement::eByPolygonVertex
  ? vertex
  : ctrlPoint;
  switch (fbx->GetReferenceMode()) {
    case FbxGeometryElement::eDirect:
      index = FBXIndex;
      break;
    case FbxGeometryElement::eIndexToDirect:
      index = fbx->GetIndexArray().GetAt(FBXIndex);
      break;
    default:
      return FALSE;
  }
  API_Copy(geomTrans.MultT(fbx->GetDirectArray().GetAt(index)), output);
  return TRUE;
}


//static FbxAMatrix API_GetGeomTransform(const FbxNode *object) {
//  FbxAMatrix mat;
//  mat.SetT(object->GetGeometricTranslation(FbxNode::eSourcePivot));
//  mat.SetR(object->GetGeometricRotation(FbxNode::eSourcePivot));
//  mat.SetS(object->GetGeometricScaling(FbxNode::eSourcePivot));
//  return mat;
//}

static FbxAMatrix API_GetGeomTransform(FbxNode* object) {
  if (!object) return FbxAMatrix();
  
  // Global transform includes everything: local T/R/S + parent hierarchy
//  FbxAMatrix globalTransform = object->EvaluateLocalTransform();
  
  // Geometric transform (offsets inside 3D authoring tools)
  FbxAMatrix geomTransform;
  geomTransform.SetT(object->GetGeometricTranslation(FbxNode::eSourcePivot));
  geomTransform.SetR(object->GetGeometricRotation(FbxNode::eSourcePivot));
  geomTransform.SetS(object->GetGeometricScaling(FbxNode::eSourcePivot));
  
  // Final transform = Global * Geometric
//  return globalTransform * geomTransform;
  return geomTransform;
}

enum vertex_semantic
{
  VERTEX_SEMANTIC_POSITION,        // 0
  VERTEX_SEMANTIC_NORMAL,          // 1
  VERTEX_SEMANTIC_TANGENT,         // 2
  VERTEX_SEMANTIC_BINORMAL,        // 3
  VERTEX_SEMANTIC_COLOR,           // 4
  VERTEX_SEMANTIC_TEXCOORD0,       // 5
  VERTEX_SEMANTIC_TEXCOORD1,       // 6
  VERTEX_SEMANTIC_TEXCOORD2,       // 7
  VERTEX_SEMANTIC_TEXCOORD3,       // 8
  VERTEX_SEMANTIC_TEXCOORD4,       // 9
  VERTEX_SEMANTIC_WEIGHT,          // 10
  VERTEX_SEMANTIC_MATRIX_INDICES,  // 11
  VERTEX_SEMANTIC_MORPH0_POSITION, // 12
  VERTEX_SEMANTIC_MORPH1_POSITION, // 13
  VERTEX_SEMANTIC_MORPH2_POSITION, // 14
  VERTEX_SEMANTIC_MORPH3_POSITION, // 15
  VERTEX_SEMANTIC_MORPH0_NORMAL,   // 16
  VERTEX_SEMANTIC_MORPH1_NORMAL,   // 17
  VERTEX_SEMANTIC_MORPH2_NORMAL,   // 18
  VERTEX_SEMANTIC_MORPH3_NORMAL,   // 19
  VERTEX_SEMANTIC_MORPH0_TANGENT,  // 20
  VERTEX_SEMANTIC_MORPH1_TANGENT,  // 21
  VERTEX_SEMANTIC_MORPH2_TANGENT,  // 22
  VERTEX_SEMANTIC_MORPH3_TANGENT,  // 23
  VERTEX_SEMANTIC_USERDATA,        // 24
  VERTEX_SEMANTIC_COUNT,
};

enum va_datatype
{
  VERTEX_ATTR_DATATYPE_UNDEFINED,
  VERTEX_ATTR_DATATYPE_FLOAT16,
  VERTEX_ATTR_DATATYPE_FLOAT32,
  VERTEX_ATTR_DATATYPE_UINT8,
  VERTEX_ATTR_DATATYPE_INT8,
  VERTEX_ATTR_DATATYPE_NORMALIZED = 64,
};

void writeint(int value, FILE *fp) {
  fwrite(&value, sizeof(value), 1, fp);
}

void writevec3(vec3 const *vec, FILE *fp, float scale, FbxAxisSystem const& axis) {
  int upSign, frontSign;
  FbxAxisSystem::EUpVector up = axis.GetUpVector(upSign);
  FbxAxisSystem::EFrontVector front = axis.GetFrontVector(frontSign);
  FbxAxisSystem::ECoordSystem coordSys = axis.GetCoorSystem();
  
  vec3 tmp;
  
  switch (up) {
    case FbxAxisSystem::eYAxis:
      tmp.y = vec->y * scale * upSign;
      if (front == fbxsdk::FbxAxisSystem::eParityOdd) {
//        tmp.z = vec->z * scale * -frontSign;
//        tmp.x = vec->x * scale * (coordSys ? frontSign : -frontSign);
        tmp.z = vec->z * scale * frontSign;
        tmp.x = vec->x * scale * (coordSys ? -frontSign : frontSign);
      } else {
        tmp.z = vec->x * scale * -frontSign;
        tmp.x = vec->z * scale * (coordSys ? frontSign : -frontSign);
      }
      break;
    case FbxAxisSystem::eZAxis:
    default:
      tmp.y = vec->z * scale * upSign;
      if (front == fbxsdk::FbxAxisSystem::eParityOdd) {
        tmp.z = vec->y * scale * -frontSign;
        tmp.x = vec->x * scale * (coordSys ? frontSign : -frontSign);
      } else {
        tmp.z = vec->x * scale * frontSign;
        tmp.x = vec->y * scale * (coordSys ? -frontSign : frontSign);
      }
      break;
  }
  fwrite(&tmp, sizeof(float), 3, fp);
}

//void writevec3(vec3 const *vec, FILE *fp, float scale, FbxAxisSystem const& axis) {
//  int upSign, frontSign;
//  FbxAxisSystem::EUpVector up = axis.GetUpVector(upSign);
//  FbxAxisSystem::EFrontVector front = axis.GetFrontVector(frontSign);
//  FbxAxisSystem::ECoordSystem coordSys = axis.GetCoorSystem();
//  
//  // We'll map FBX coordinates to our target coordinate system: X, Y, Z
//  // First, determine which FBX axis corresponds to our Up, Front, and Right
//  
//  // Index 0=X, 1=Y, 2=Z
//  int upAxis = 0;
//  switch (up) {
//    case FbxAxisSystem::eXAxis: upAxis = 0; break;
//    case FbxAxisSystem::eYAxis: upAxis = 1; break;
//    case FbxAxisSystem::eZAxis: upAxis = 2; break;
//  }
//  
//  // Determine Front Axis (perpendicular to Up, depends on parity)
//  // If up is Z, then front is either X or Y (depending on parity)
//  int frontAxis = -1;
//  int rightAxis = -1;
//  
//  // Construct orthonormal basis
//  // Right = Up x Front
//  // Axis indices: 0=X, 1=Y, 2=Z
//  // ParityEven → right-handed (default)
//  // ParityOdd  → flip front axis for left-handed
//  
//  // We'll just use the standard bases
//  // Mapping from FBX axes to output (X=0, Y=1, Z=2)
//  int fbxToTarget[3];  // index: FBX axis, value: which target axis (0=X,1=Y,2=Z)
//  int fbxSign[3];      // sign for each axis
//  
//  // Default init
//  fbxToTarget[upAxis] = 1; // Up maps to Y
//  fbxSign[upAxis] = upSign;
//  
//  // Use FBX SDK's internal assumption:
//  // If Up is Y, and front is eParityOdd, then Front is -Z
//  // If Up is Y, and front is eParityEven, then Front is +Z
//  // We'll use a lookup based on up axis and parity
//  
//  // Determine frontAxis
//  switch (upAxis) {
//    case 0: frontAxis = 2; rightAxis = 1; break; // Up = X → Front = Z, Right = Y
//    case 1: frontAxis = 2; rightAxis = 0; break; // Up = Y → Front = Z, Right = X
//    case 2: frontAxis = 0; rightAxis = 1; break; // Up = Z → Front = X, Right = Y
//  }
//  
//  fbxToTarget[frontAxis] = 2; // Front maps to Z
//  fbxToTarget[rightAxis] = 0; // Right maps to X
//  
//  // Parity affects handedness → flip front sign if odd
//  fbxSign[frontAxis] = (front == FbxAxisSystem::eParityOdd) ? -1 : +1;
//  fbxSign[rightAxis] = +1;
//  
//  // Left-handed coordinate system → flip one axis to make it right-handed
//  if (coordSys == FbxAxisSystem::eLeftHanded) {
//    fbxSign[frontAxis] *= -1;  // flip Z
//  }
//  
//  // Prepare output vector
//  float output[3];
//  float input[3] = { vec->x, vec->y, vec->z };
//  
//  for (int i = 0; i < 3; ++i) {
//    int srcAxis = -1;
//    for (int j = 0; j < 3; ++j) {
//      if (fbxToTarget[j] == i) {
//        srcAxis = j;
//        break;
//      }
//    }
//    output[i] = input[srcAxis] * fbxSign[srcAxis] * scale;
//  }
//  
//  fwrite(output, sizeof(float), 3, fp);
//}

static void API_SaveModel(FbxMesh *mesh, FbxScene *scene, PDRAWSURF model, lpcString_t  filename) {
  FILE *fp = fopen(filename, "wb");
  if (fp) {
    fwrite(&IDMESHHEADER, sizeof(IDMESHHEADER), 1, fp);
    
    int num_uv = std::min(3, mesh->GetElementUVCount());
    
    uint32_t numAttributes = 4 + num_uv;

    fwrite(&model->numVertices, sizeof(model->numVertices), 1, fp);
    fwrite(&model->numIndices, sizeof(model->numIndices), 1, fp);
    fwrite(&model->numSubmeshes, sizeof(model->numSubmeshes), 1, fp);
    fwrite(&numAttributes, sizeof(numAttributes), 1, fp);

    fwrite(model->indices, sizeof(DRAWINDEX), model->numIndices, fp);
    fwrite(model->submeshes, sizeof(DRAWSUBMESH), model->numSubmeshes, fp);
    
    FbxSystemUnit systemUnit = scene->GetGlobalSettings().GetSystemUnit();
    double scaleFactor = systemUnit.GetScaleFactor() / 100.f;
    
    FbxAxisSystem axis = scene->GetGlobalSettings().GetAxisSystem();
//    upVector = axisSystem.GetUpVector(sign);
    
    writeint(VERTEX_SEMANTIC_POSITION, fp);
    writeint(VERTEX_ATTR_DATATYPE_FLOAT32, fp);
    for (int i = 0; i < model->numVertices; writevec3(&model->vertices[i++].xyz, fp, scaleFactor, axis));

    writeint(VERTEX_SEMANTIC_NORMAL, fp);
    writeint(VERTEX_ATTR_DATATYPE_FLOAT32, fp);
    for (int i = 0; i < model->numVertices; writevec3(&model->vertices[i++].normal, fp, 1, axis));

    writeint(VERTEX_SEMANTIC_BINORMAL, fp);
    writeint(VERTEX_ATTR_DATATYPE_FLOAT32, fp);
    for (int i = 0; i < model->numVertices; writevec3(&model->vertices[i++].binormal, fp, 1, axis));

    writeint(VERTEX_SEMANTIC_TANGENT, fp);
    writeint(VERTEX_ATTR_DATATYPE_FLOAT32, fp);
    for (int i = 0; i < model->numVertices; writevec3(&model->vertices[i++].tangent, fp, 1, axis));

    for (int j = 0; j < num_uv; j++) {
      writeint(VERTEX_SEMANTIC_TEXCOORD0+j, fp);
      writeint(VERTEX_ATTR_DATATYPE_FLOAT32, fp);
      for (int i = 0; i < model->numVertices; fwrite(&model->vertices[i++].texcoord[j], sizeof(float), 2, fp));
    }

    fclose(fp);
  } else {
    Con_Error("Can not open file for writing %s", filename);
  }
}

static void ExportMesh(FbxMesh *mesh, lpcString_t  directory) {
  std::vector<DRAWVERT>  outVertices;
  std::vector<DRAWINDEX>   outIndices;
  std::vector<DRAWSUBMESH> outSubmeshes;
  std::vector<uint32_t>  tmpSubmeshes[kNumSubmaterials];
  
  auto name = mesh->GetNode()->GetName();
  auto parent = mesh->GetNode()->GetParent();
  auto name2 = parent->GetName();
  auto normals = mesh->GetElementNormal();
  auto uv0 = mesh->GetElementUV();
  auto uv1 = mesh->GetElementUVCount() > 1 ? mesh->GetElementUV(1) : nullptr;
  auto uv2 = mesh->GetElementUVCount() > 2 ? mesh->GetElementUV(2) : nullptr;
  auto tangents     = mesh->GetElementTangent();
  auto binormals    = mesh->GetElementBinormal();
  auto colors       = mesh->GetElementVertexColor();
  auto geomTrans    = API_GetGeomTransform(mesh->GetNode());
  auto materials    = mesh->GetElementMaterial();
  auto numPolygons    = mesh->GetPolygonCount();
  auto numControlPoints = mesh->GetControlPointsCount();

  outVertices.reserve(numControlPoints);

  std::vector<vertexLink_t> _vlinks(numControlPoints);

  int debugNumVertices      = 0;
  int debugNumVerticesOptimized = 0;
  
  FbxAMatrix rotationMatrix;
  rotationMatrix.SetR(geomTrans.GetR()); // Extract just rotation

  for (int i = 0, v = 0; i < numPolygons; i++, v += mesh->GetPolygonSize(i)) {
    std::vector<DRAWINDEX> *ptrIndices = &outIndices;
    if (materials) {
      auto k = materials->GetIndexArray().GetAt(i);
      if (kNumSubmaterials <= k) {
        Con_Error("Can not add submesh %d. Max submeshes %d", k,
              kNumSubmaterials);
        continue;
      }
      ptrIndices = &tmpSubmeshes[k];
    }
    if (mesh->GetPolygonSize(i) != 3) {
      Con_Error("Unsupported polygon size %d at index %d",
            mesh->GetPolygonSize(i), i);
      continue;
    }

    for (int k = 0; k < 3; k++) {
      DRAWVERT vtx;
      uint32_t index;
      auto   index0 = v + k;
      auto   index1 = mesh->GetPolygonVertex(i, k);

      *((int *)&vtx.color) = -1;

      auto a = geomTrans.MultT(mesh->GetControlPointAt(index1));
      auto b = mesh->GetControlPointAt(index1);

      API_Copy(geomTrans.MultT(mesh->GetControlPointAt(index1)), vtx.xyz);
      API_CopyTransformed(normals, index0, index1, vtx.normal, rotationMatrix);
      API_Copy(uv0, index0, index1, vtx.texcoord[0]);
      if (uv1) API_Copy(uv1, index0, index1, vtx.texcoord[1]);
      if (uv2) API_Copy(uv2, index0, index1, vtx.texcoord[2]);
      
      API_CopyTransformed(tangents, index0, index1, vtx.tangent, rotationMatrix);
      API_CopyTransformed(binormals, index0, index1, vtx.binormal, rotationMatrix);
      API_Copy(colors, index0, index1, vtx.color);

      debugNumVertices++;

      auto vlink = &_vlinks[index1];
      if (vlink->index == -1) {
        index    = static_cast<uint32_t>(outVertices.size());
        vlink->index = index;
        outVertices.push_back(vtx);
        debugNumVerticesOptimized++;
      } else {
        for (auto link = vlink; link;
             vlink = link->next ? link->next : vlink, link = link->next)
        {
          if (memcmp(&outVertices[link->index], &vtx,
                 sizeof(DRAWVERT)) == 0) {
            index = link->index;
            goto doneAddingVertex;
          }
        }
        auto link   = new vertexLink_t;
        link->next  = nullptr;
        index     = static_cast<uint32_t>(outVertices.size());
        link->index = index;
        vlink->next = link;
        outVertices.push_back(vtx);
        debugNumVerticesOptimized++;
      }
    doneAddingVertex:
      ptrIndices->push_back(index);
    }
  }

  //  Con_Error("Mesh %s imported, %d vertices reduced to %d",
  //  mesh->GetNode()->GetName(), debugNumVertices,
  //  debugNumVerticesOptimized);

  uint32_t numSubmeshes = 0;

  if (materials) {
    for (int i = 0; i < kNumSubmaterials; i++) {
      if (tmpSubmeshes[i].size() > 0) {
        numSubmeshes = i + 1;
      }
      DRAWSUBMESH submesh = DRAWSUBMESH{
        static_cast<uint32_t>(outIndices.size()),
        static_cast<uint32_t>(tmpSubmeshes[i].size()),
      };
      outSubmeshes.push_back(submesh);
      std::copy(tmpSubmeshes[i].begin(), tmpSubmeshes[i].end(),
            std::back_inserter(outIndices));
    }
  }

  DRAWSURF dsurf = {outVertices.data(),
            outIndices.data(),
            outSubmeshes.data(),
            NULL,
            (uint32_t)outVertices.size(),
            (uint32_t)outIndices.size(),
            numSubmeshes};

  char filename[2048];
  snprintf(filename, sizeof(filename), "%s/%s.mesh", directory,
       mesh->GetNode()->GetName());
  API_SaveModel(mesh, mesh->GetScene(), &dsurf, filename);
//  API_SaveModel(&dsurf, directory);
}

void ConvertCollada(lpcString_t  input, lpcString_t  output) {}

int main(int argc, lpcString_t  argv[]) {
  if (argc < 3) {
    Con_Error("fbxtool usage:");
    Con_Error("fbxtool [-options] [input] [output]");
    return 0;
  }
  lpcString_t input = argv[1];
  lpcString_t output = argv[2];
  
//  input = "/Users/igor/Developer/titanium-ui/Titanium/MB/03_Contexts/01_Classic/00_Resources/Views/ArtRing/3D/230616_MB_CC_ART_Ring_+_Marker v2.fbx";
//  output = "/Users/igor/Developer/titanium-ui-orca/Titanium/MB/03_Contexts/01_Classic/00_Resources/Views/ArtRing/3D/230616_MB_CC_ART_Ring_+_Marker v2";

//  input = "/Users/igor/Developer/titanium-ui/Titanium/MB/03_Contexts/01_Classic/00_Resources/Views/Tube/3D/MB_CC_Tube_Shine.fbx";
//  output = "/Users/igor/Developer/titanium-ui-orca/Titanium/MB/03_Contexts/01_Classic/00_Resources/Views/Tube/3D/MB_CC_Tube_Shine";

//  input = "/Users/igor/Developer/titanium-ui/Titanium/MB/02_GlobalElements/MB_Startup/3D/MB_Star.fbx";
//  output = "/Users/igor/Developer/titanium-ui-orca/Titanium/MB/02_GlobalElements/MB_Startup/3D/MB_Star";

  input = "/Users/igor/Developer/titanium-ui/Titanium/MB/02_GlobalElements/MB_Startup/3D/light_shadow.fbx";
  output = "/Users/igor/Developer/titanium-ui-orca/Titanium/MB/02_GlobalElements/MB_Startup/3D/light_shadow";
  
  FbxManager    *manager    = FbxManager::Create();
  FbxImporter   *importer   = FbxImporter::Create(manager, "");
  FbxIOSettings *iosettings = FbxIOSettings::Create(manager, IOSROOT);

  if (importer->Initialize(input, -1, iosettings)) {
    FbxScene *scene = FbxScene::Create(manager, "FBX Scene");
    FbxAxisSystem unityAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);
    if (importer->Import(scene)) {
      FbxAxisSystem axisSystem = scene->GetGlobalSettings().GetAxisSystem();
      FbxGeometryConverter fbxGeometryConverter(manager);
      fbxGeometryConverter.Triangulate(scene, TRUE);
//      FbxSystemUnit::m.ConvertScene(scene);
//      unityAxisSystem.ConvertScene(scene); // Convert to Unity coordinate system
      for (int i = 0; i < scene->GetGeometryCount(); i++) {
        FbxGeometry *geom = scene->GetGeometry(i);
        if (geom->GetAttributeType() == FbxNodeAttribute::eMesh) {
          ExportMesh((FbxMesh *)geom, output);
        }
      }
    } else {
      FbxString error = importer->GetStatus().GetErrorString();
      FBXSDK_printf("FbxImporter::Import(): %s\n\n", error.Buffer());
    }
    scene->Destroy();
  } else {
    FbxString error = importer->GetStatus().GetErrorString();
    FBXSDK_printf("FbxImporter::Initialize(): %s\n\n", error.Buffer());
  }

  iosettings->Destroy();
  importer->Destroy();
  manager->Destroy();

  return 0;
}
