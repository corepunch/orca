#ifndef ORCA_CG_MODEL_H
#define ORCA_CG_MODEL_H

#include "cg_api.h"

int cg_model_load(char const *xml_path, cg_model *out);
void cg_model_free(cg_model *model);

#endif
