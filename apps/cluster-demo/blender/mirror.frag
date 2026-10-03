// Polished surface: screen-space reflection of the animated scene captured
// before this model, falling back to the studio cubemap where the ray leaves
// the screen or finds nothing. Works on flat decks and curved chrome alike:
// the ray starts from the interpolated per-pixel normal.
in vec3 v_worldPosition;
in vec3 v_worldNormal;
in vec3 v_eyePosition;
uniform mat4 u_viewTransform;
uniform mat4 u_projectionTransform;
uniform sampler2D u_sceneColor;
uniform sampler2D u_sceneDepth;
uniform vec2 u_sceneTextureSize;

float viewDepth(vec2 uv) {
    float z = texture(u_sceneDepth, uv).r * 2.0 - 1.0;
    return u_projectionTransform[3][2] / (z + u_projectionTransform[2][2]);
}

bool projectRay(vec3 point, out vec2 uv) {
    vec4 clip = u_projectionTransform * vec4(point, 1.0);
    if (clip.w <= 0.001) return false;
    uv = clip.xy / clip.w * 0.5 + 0.5;
    return all(greaterThan(uv, vec2(0.001))) && all(lessThan(uv, vec2(0.999)));
}

// March the reflected ray in view space with growing steps, then bisect the
// first crossing behind the depth buffer. Returns colour and confidence.
vec4 traceReflection(vec3 origin, vec3 direction) {
    if (u_sceneTextureSize.x < 1.0) return vec4(0.0);
    float previous = 0.01;
    float previousGap = -1000.0;
    for (int i = 1; i <= 64; i++) {
        float t = 0.01 + pow(float(i) / 64.0, 1.65) * 9.0;
        vec3 point = origin + direction * t;
        vec2 uv;
        if (!projectRay(point, uv)) break;
        float gap = -point.z - viewDepth(uv);
        if (gap >= 0.0 && previousGap < 0.0) {
            float lo = previous, hi = t;
            for (int refine = 0; refine < 5; refine++) {
                float mid = (lo + hi) * 0.5;
                vec2 sampleUV;
                if (!projectRay(origin + direction * mid, sampleUV)) break;
                if (-(origin + direction * mid).z - viewDepth(sampleUV) > 0.0) hi = mid;
                else lo = mid;
            }
            vec3 hit = origin + direction * hi;
            if (!projectRay(hit, uv)) break;
            // Rays that pass far behind a surface are occluded, not hits.
            if (abs(-hit.z - viewDepth(uv)) < 0.06 + 0.01 * hi) {
                vec2 blur = (0.7 + clamp(Roughness, 0.0, 1.0) * 6.0 + hi * 0.6) / u_sceneTextureSize;
                vec3 color = texture(u_sceneColor, uv).rgb * 0.36;
                color += texture(u_sceneColor, uv + vec2(blur.x, 0.0)).rgb * 0.16;
                color += texture(u_sceneColor, uv - vec2(blur.x, 0.0)).rgb * 0.16;
                color += texture(u_sceneColor, uv + vec2(0.0, blur.y)).rgb * 0.16;
                color += texture(u_sceneColor, uv - vec2(0.0, blur.y)).rgb * 0.16;
                vec2 edge = min(uv, vec2(1.0) - uv);
                float confidence = smoothstep(0.0, 0.06, min(edge.x, edge.y));
                confidence *= 1.0 - smoothstep(5.0, 9.0, hi);
                return vec4(color, confidence);
            }
        }
        previous = t;
        previousGap = gap;
    }
    return vec4(0.0);
}

vec3 studio(vec3 R) {
    float spread = .004 + Roughness * Roughness * .12;
    vec3 T = normalize(cross(R, abs(R.y) < .9 ? vec3(0, 1, 0) : vec3(1, 0, 0)));
    vec3 B = cross(R, T);
    vec3 env = texture(EnvironmentMap, R).rgb * .5;
    env += (texture(EnvironmentMap, normalize(R + T * spread)).rgb +
            texture(EnvironmentMap, normalize(R - T * spread)).rgb +
            texture(EnvironmentMap, normalize(R + B * spread)).rgb +
            texture(EnvironmentMap, normalize(R - B * spread)).rgb) * .125;
    return env;
}

void main() {
    vec3 N = normalize(v_worldNormal);
    vec3 V = normalize(v_eyePosition - v_worldPosition);
    if (dot(N, V) < 0.0) N = -N;
    float ndv = max(dot(N, V), 0.0);
    // Main pass (no capture bound): the surface itself. Reflection pass: only
    // the reflected light, which the engine adds on top.
    if (u_sceneTextureSize.x < 1.0) {
        vec3 base = Tint.rgb * (.6 + .4 * ndv);
        vec3 spill = Accent.rgb * Level * Spill * pow(1.0 - ndv, 2.0);
        FragColor = vec4(base + spill, 1.0);
        return;
    }
    vec3 R = reflect(-V, N);
    vec3 origin = (u_viewTransform * vec4(v_worldPosition + N * 0.01, 1.0)).xyz;
    vec4 traced = traceReflection(origin, normalize(mat3(u_viewTransform) * R));
    vec3 radiance = mix(studio(R) * Studio, traced.rgb, traced.a);
    float fresnel = pow(1.0 - ndv, 5.0);
    float reflection = mix(Reflectance, 1.0, fresnel) * (1.0 - clamp(Roughness, 0.0, 1.0) * 0.4);
    FragColor = vec4(radiance * reflection, 1.0);
}
