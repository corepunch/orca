#include <include/orca.h>

#include "animation.h"
#include "core_local.h"

size_t
animation_size(uint32_t num_keyframes) {
  return sizeof(struct curve) + sizeof(struct keyframe) * num_keyframes;
}

static uint32_t num_keyframes(xmlNodePtr xml) {
  uint32_t numkeyframes = 0;
  xmlForEach(child, xml) {
    if (!xmlStrcmp(child->name, XMLSTR("Keyframe"))) {
      numkeyframes++;
    }
  }
  return numkeyframes;
}

static struct curve*
animation_load_xml(xmlNodePtr xml) {
  size_t size = animation_size(num_keyframes(xml));
  struct curve* anim = ZeroAlloc(size);
  xmlWith(xmlChar, Path, xmlGetProp(xml, XMLSTR("Path")), xmlFree) {
    strncpy(anim->path, (LPSTR)Path, sizeof(anim->path));
  }
  xmlWith(xmlChar, Property, xmlGetProp(xml, XMLSTR("Property")), xmlFree) {
    strncpy(anim->property, (LPSTR)Property, sizeof(anim->property));
  }
  xmlForEach(child, xml) {
    if (!xmlStrcmp(child->name, XMLSTR("Keyframe"))) {
      xmlWith(xmlChar, time, xmlGetProp(child, XMLSTR("time")), xmlFree) {
        anim->keyframes[anim->num_keyframes].time = atof((LPSTR)time);
      }
      xmlWith(xmlChar, tangentMode, xmlGetProp(child, XMLSTR("tangentMode")), xmlFree) {
        anim->keyframes[anim->num_keyframes].tangentMode = atoi((LPSTR)tangentMode);
      }
      xmlWith(xmlChar, weightedMode, xmlGetProp(child, XMLSTR("weightedMode")), xmlFree) {
        anim->keyframes[anim->num_keyframes].weightedMode = atoi((LPSTR)weightedMode);
      }
      xmlWith(xmlChar, inSlope, xmlGetProp(child, XMLSTR("inSlope")), xmlFree) {
        float *out = anim->keyframes[anim->num_keyframes].inSlope;
        sscanf((LPSTR)inSlope, "%f %f %f %f", &out[0], &out[1], &out[2], &out[3]);
      }
      xmlWith(xmlChar, outSlope, xmlGetProp(child, XMLSTR("outSlope")), xmlFree) {
        float *out = anim->keyframes[anim->num_keyframes].outSlope;
        sscanf((LPSTR)outSlope, "%f %f %f %f", &out[0], &out[1], &out[2], &out[3]);
      }
      xmlWith(xmlChar, inWeight, xmlGetProp(child, XMLSTR("inWeight")), xmlFree) {
        float *out = anim->keyframes[anim->num_keyframes].inWeight;
        sscanf((LPSTR)inWeight, "%f %f %f %f", &out[0], &out[1], &out[2], &out[3]);
      }
      xmlWith(xmlChar, outWeight, xmlGetProp(child, XMLSTR("outWeight")), xmlFree) {
        float *out = anim->keyframes[anim->num_keyframes].outWeight;
        sscanf((LPSTR)outWeight, "%f %f %f %f", &out[0], &out[1], &out[2], &out[3]);
      }
      xmlWith(xmlChar, value, xmlGetProp(child, XMLSTR("value")), xmlFree) {
        float *out = anim->keyframes[anim->num_keyframes].value;
        sscanf((LPSTR)value, "%f %f %f %f", &out[0], &out[1], &out[2], &out[3]);
//        anim->keyframes[anim->num_keyframes].value[0] = atof((LPSTR)value);
      }
      anim->num_keyframes++;
    }
  }
  return anim;
}

#define USE_CUBIC_BEZIER
//#define USE_HERMITE

#ifdef USE_CUBIC_BEZIER
float cubic_bezier(float t, float p0, float h0, float h1, float p1) {
  float u = 1.0f - t;
  float tt = t * t;
  float uu = u * u;
  float uuu = uu * u;
  float ttt = tt * t;
  return uuu * p0 + 3 * uu * t * h0 + 3 * u * tt * h1 + ttt * p1;
}
float animation_evaluate(struct curve* animation, float time, int xyzw) {
  if (!animation || animation->num_keyframes == 0)
    return 0;
  
  struct keyframe const* first = &animation->keyframes[0];
  struct keyframe const* last = &animation->keyframes[animation->num_keyframes - 1];
  
  if (time <= first->time) return first->value[xyzw];
  if (time >= last->time) return last->value[xyzw];
  
  for (int i = 0, j = 1; j < animation->num_keyframes; i++, j++) {
    struct keyframe const* a = &animation->keyframes[i];
    struct keyframe const* b = &animation->keyframes[j];
    if (b->time >= time) {
      float dt = b->time - a->time;
      float t = (time - a->time) / dt;
      
      float v0 = a->value[xyzw];
      float v1 = b->value[xyzw];
      
      float m0 = a->outSlope[xyzw];
      float m1 = b->inSlope[xyzw];
      
      float w0 = a->outWeight[xyzw];
      float w1 = b->inWeight[xyzw];
      
      float h0 = v0 + m0 * w0 * dt;
      float h1 = v1 - m1 * w1 * dt;
      
      if (a->tangentMode == (8 << 4 | 8) || a->tangentMode == 0) {
        return cubic_bezier(t, v0, h0, h1, v1);
      } else {
        return LERP(a->value[xyzw], b->value[xyzw], t);
      }
    }
  }
  
  return first->value[xyzw];
}
#elif defined(USE_HERMITE)
float hermite(float t, float p0, float m0, float p1, float m1) {
  float t2 = t * t;
  float t3 = t2 * t;
  float h00 = 2*t3 - 3*t2 + 1;
  float h10 = t3 - 2*t2 + t;
  float h01 = -2*t3 + 3*t2;
  float h11 = t3 - t2;
  return h00*p0 + h10*m0 + h01*p1 + h11*m1;
}
float animation_evaluate(struct curve* animation, float time, int xyzw) {
  if (!animation || animation->num_keyframes == 0)
    return 0;
  
  struct keyframe const* first = &animation->keyframes[0];
  struct keyframe const* last = &animation->keyframes[animation->num_keyframes - 1];
  
  if (time <= first->time) return first->value[xyzw];
  if (time >= last->time) return last->value[xyzw];
  
  for (int i = 0, j = 1; j < animation->num_keyframes; i++, j++) {
    struct keyframe const* a = &animation->keyframes[i];
    struct keyframe const* b = &animation->keyframes[j];
    if (b->time >= time) {
      float dt = b->time - a->time;
      float t = (time - a->time) / dt;
      
      // Tangents (assuming outSlope is m0 and inSlope is m1)
      float m0 = a->outSlope[xyzw] * dt;
      float m1 = b->inSlope[xyzw] * dt;
      
      return hermite(t, a->value[xyzw], m0, b->value[xyzw], m1);
    }
  }
  
  return first->value[xyzw];
}

#else
float
animation_evaluate(struct curve* animation, float time, int xyzw)
{
  if (!animation || animation->num_keyframes == 0)
    return 0;
  struct keyframe const* first_keyframe = &animation->keyframes[0];
  struct keyframe const* last_keyframe =
    &animation->keyframes[animation->num_keyframes - 1];
  if (time <= first_keyframe->time)
    return first_keyframe->value[xyzw];
  if (time >= last_keyframe->time)
    return last_keyframe->value[xyzw];
  for (int i = 0, j = 1; j < animation->num_keyframes; i++, j++) {
    struct keyframe const* a = &animation->keyframes[i];
    struct keyframe const* b = &animation->keyframes[j];
    if (b->time >= time) {
      float const t = (time - a->time) / MAX(0.0001f, b->time - a->time);
      return LERP(a->value[xyzw], b->value[xyzw], t);
    }
  }
  return first_keyframe->value[xyzw];
}
#endif

handle_t
Animation_Register(xmlDocPtr doc)
{
  xmlNodePtr xml = xmlDocGetRootElement(doc);
  if (xmlStrcmp(xml->name, XMLSTR("Animation")) &&
      xmlStrcmp(xml->name, XMLSTR("Curve")))
  {
    Con_Error("<Animation> or <Curve> root tag expected in %s", doc->URL);
    return ZeroAlloc(animation_size(0));
  }
  return animation_load_xml(xml);
}

lpKeyframeAnim_t
ANIM_Load(xmlDocPtr doc)
{
  lpKeyframeAnim_t anim = NULL;
  xmlNodePtr xml = xmlDocGetRootElement(doc);
  if (xmlStrcmp(xml->name, XMLSTR("AnimationClip"))) {
    Con_Error("<Animation> root tag expected in %s", doc->URL);
    return NULL;
  }
  anim = ZeroAlloc(sizeof(KeyframeAnim_t));
  anim->filename = strdup((lpcString_t)doc->URL);
  xmlWith(xmlChar, Mode, xmlGetProp(xml, XMLSTR("Mode")), xmlFree) {
    if (!xmlStrcmp(Mode, XMLSTR("PlayOnce"))) {
      anim->mode = kAnimationPlayOnce;
    } else if (!xmlStrcmp(Mode, XMLSTR("Loop"))) {
      anim->mode = kAnimationLoop;
    } else if (!xmlStrcmp(Mode, XMLSTR("PingPong"))) {
      anim->mode = kAnimationPingPong;
    }
  }
  xmlWith(xmlChar, StartTime, xmlGetProp(xml, XMLSTR("StartTime")), xmlFree) {
    anim->start_time = atof((LPSTR)StartTime);
  }
  xmlWith(xmlChar, StopTime, xmlGetProp(xml, XMLSTR("StopTime")), xmlFree) {
    anim->stop_time = atof((LPSTR)StopTime);
  }
  xmlForEach(child, xml)
  {
    if (!xmlStrcmp(child->name, XMLSTR("Curve"))) {
      struct curve *curve = animation_load_xml(child);
      ADD_TO_LIST(curve, anim->curves);
    }
  }
  return anim;
}

void
ANIM_Release(lpKeyframeAnim_t anim) {
  SafeDelete(anim->next, ANIM_Release);
  FOR_EACH_LIST(struct curve, curve, anim->curves) {
    free(curve);
  }
  free(anim->filename);
  free(anim->name);
  free(anim);
}

lpKeyframeAnim_t
ANIM_FindByName(lpKeyframeAnim_t anims, lpcString_t name)
{
  if (!name)
    return NULL;
  FOR_EACH_LIST(KeyframeAnim_t, it, anims) {
    if (!strcmp(it->name, name)) {
      return it;
    }
  }
  return NULL;
}

void luaX_pushKeyframeAnim(lua_State* L, lpcKeyframeAnim_t anim) {
  lua_pushlightuserdata(L, (void*)anim);
}

lpKeyframeAnim_t luaX_checkKeyframeAnim(lua_State* L , int idx) {
  return lua_touserdata(L, idx);
}
