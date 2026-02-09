#include <stdio.h>

#include "r_local.h"
#include "r_tangents.h"

HRESULT
Model_Release(struct model* model)
{
  if (*model->vertexBuffers) {
    R_Call(glDeleteBuffers, VERTEX_SEMANTIC_COUNT, model->vertexBuffers);
  }
  if (model->indexBuffer) {
    R_Call(glDeleteBuffers, 1, &model->indexBuffer);
  }
  if (model->vao) {
    R_Call(glDeleteVertexArrays, 1, &model->vao);
  }
  SafeFree(model->submeshes);
  free(model);
  return S_OK;
}

void
Model_Init(struct model* model, PSURFATTR attr, PDRAWSURF dsurf)
{
  model->submeshes = ZeroAlloc(sizeof(DRAWSUBMESH) * dsurf->numSubmeshes);
  model->numVertices = dsurf->numVertices;
  model->numIndices = dsurf->numIndices;
  model->numSubmeshes = dsurf->numSubmeshes;

  memcpy(model->submeshes,
         dsurf->submeshes,
         sizeof(DRAWSUBMESH) * dsurf->numSubmeshes);

  R_Call(glGenVertexArrays, 1, &model->vao);
  R_Call(glBindVertexArray, model->vao);
  R_Call(glGenBuffers, VERTEX_SEMANTIC_COUNT, model->vertexBuffers);
  R_Call(glGenBuffers, 1, &model->indexBuffer);

  for (; *attr != VERTEX_SEMANTIC_COUNT; attr++) {
    enum vertex_semantic semantic = *attr;
    struct shader_attr const* attrib = vertex_semantic_attributes(semantic);
    int size = (attrib->size * (attrib->type == GL_FLOAT ? 4 : 1));
    void* mem = ZeroAlloc(size * dsurf->numVertices);
    void* write = mem;
    FOR_LOOP(vert, dsurf->numVertices)
    {
      void const* ptr = &dsurf->vertices[vert];
      size_t off = (size_t)attrib->ptr;
      memcpy(write, ptr + off, size);
      write += size;
    }
    R_Call(glBindBuffer, GL_ARRAY_BUFFER, model->vertexBuffers[*attr]);
    R_Call(glBufferData,
           GL_ARRAY_BUFFER,
           size * dsurf->numVertices,
           mem,
           GL_STATIC_DRAW);
    model->attr[semantic] = *(++attr);
    free(mem);
  }

  R_Call(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, model->indexBuffer);
  R_Call(glBufferData,
         GL_ELEMENT_ARRAY_BUFFER,
         sizeof(DRAWINDEX) * dsurf->numIndices,
         dsurf->indices,
         GL_STATIC_DRAW);
}

HRESULT
Model_Create(PSURFATTR attr, PDRAWSURF dsurf, struct model** ppModel)
{
  *ppModel = ZeroAlloc(sizeof(struct model));
  if (*ppModel) {
    Model_Init(*ppModel, attr, dsurf);
    return S_OK;
  } else {
    return E_OUTOFMEMORY;
  }
}

struct dvertattr_header
{
  enum vertex_semantic semantic;
  enum va_datatype datatype;
};

static struct model*
Model_Load(lpcString_t filename)
{
  dmeshheader header;
  
  if (*filename == '#') {
    if (!strcmp(filename, "#Quad")) {
      struct model* model;
      Model_CreatePlane(1, 1, &model);
      return model;
    }
    if (!strcmp(filename, "#Plane")) {
      struct model* model;
      HRESULT
      Model_CreatePlaneXZ(float, float, struct model**);
      Model_CreatePlaneXZ(10, 10, &model);
      return model;
    }
  }
  
  struct file* file = FS_LoadFile(filename);
  
  if (!file) {
    Con_Error("%s: model not found", filename);
    return Model_CreateError();
  }
  
  if (file->size < sizeof(dmeshheader)) {
    Con_Error("%s: file incorrect", filename);
    FS_FreeFile(file);
    return Model_CreateError();
  }
  
  memcpy(&header, file->data, sizeof(dmeshheader));
  
  if (header.identifier != IDMESHHEADER) {
    Con_Error("%s is not a MESH file", filename);
    FS_FreeFile(file);
    return Model_CreateError();
  }
  
  int ofs_tris = sizeof(dmeshheader);
  int ofs_subm = ofs_tris + sizeof(DRAWINDEX) * header.numIndices;
  int ofs_verts = ofs_subm + sizeof(DRAWSUBMESH) * header.numSubmeshes;
  
  void const* mem = file->data + ofs_verts;
  struct model* model = ZeroAlloc(sizeof(struct model));
  
  R_Call(glGenVertexArrays, 1, &model->vao);
  R_Call(glBindVertexArray, model->vao);
  R_Call(glGenBuffers, VERTEX_SEMANTIC_COUNT, model->vertexBuffers);
  R_Call(glGenBuffers, 1, &model->indexBuffer);
  R_Call(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, model->indexBuffer);
  R_Call(glBufferData,
         GL_ELEMENT_ARRAY_BUFFER,
         sizeof(DRAWINDEX) * header.numIndices,
         file->data + ofs_tris,
         GL_STATIC_DRAW);
  
  FOR_LOOP(attr, header.numAttributes)
  {
    struct dvertattr_header const* dvattr = mem;
    struct va_datatype_desc const desc = va_datatype_info(dvattr->datatype);
    int const attrsize = desc.size * vertex_semantic_attributes(dvattr->semantic)->size;
    
    R_Call(glBindBuffer, GL_ARRAY_BUFFER, model->vertexBuffers[dvattr->semantic]);
    R_Call(glBufferData,
           GL_ARRAY_BUFFER,
           attrsize * header.numVertices,
           mem + sizeof(struct dvertattr_header),
           GL_STATIC_DRAW);
    mem += attrsize * header.numVertices + sizeof(struct dvertattr_header);
    model->attr[dvattr->semantic] = dvattr->datatype;
  }
  
  model->numVertices = header.numVertices;
  model->numIndices = header.numIndices;
  model->numSubmeshes = header.numSubmeshes;
  
  FS_FreeFile(file);
  
  return model;
}

HANDLER(Mesh, Start) {
  pMesh->model = Model_Load(pMesh->Source);
  return TRUE;
}

HANDLER(Mesh, Destroy) {
  Model_Release(pMesh->model);
  return TRUE;
}
