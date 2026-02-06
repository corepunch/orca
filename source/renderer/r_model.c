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

#ifdef ORCA_FEATURE_3D
#include "../SceneKit/model_stab.h"
#else
HANDLER(Mesh, Start) {
  fprintf(stderr, "Error: Shader support is part of SceneKit extension.\n");
  abort();
}
HANDLER(Mesh, Destroy) { return 0; }
#endif
