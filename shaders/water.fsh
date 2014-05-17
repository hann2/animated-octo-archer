#version 330 core

in vec2 UV;
in vec3 pos;
in vec3 normal;
in vec3 du;
in vec3 dv;

out vec4 fragColor;

uniform vec3 eye;

uniform sampler2D bump_map;

const float del = 0.001;
const float h_max = 0.04;
const float s = 0.002;
const vec3 light_pos = vec3(0, 0, -4);
const vec3 water_color = vec3(102, 205, 170) / 255.0;

mat3 texture_transform = mat3(du, dv, normal);
vec3 dir = normalize(eye - pos);
vec3 d2 = texture_transform * dir;

float bias(float b, float t) {
    return b * t / (2 * b * t - b - t + 1);
}

float gain(float g, float t) {
    if (t < 0.5) {
        return bias(g, t * 2) / 2;
    } else {
        return bias(1 - g, t * 2 - 1) / 2 + 0.5;
    }
}

float h(vec2 v) {
    return h_max * texture(bump_map, v).r;
}

vec3 u_good() {
    vec3 u = vec3(UV, h_max);
    for (int i = 0; i < 4096; i++) {
        if (u.z < h(u.xy)) {
            break;
        }
        u = u - s * d2;
    }
    return u;
}

void main() {
    vec3 u_ = u_good();
    // vec3 pos2 = inverse(texture_transform) * u_;
    vec2 u = vec2(u_);
    float dhu = (h(u + vec2(-del, 0)) - h(u + vec2(del, 0))) / (2 * del);
    float dhv = (h(u + vec2(0, -del)) - h(u + vec2(0, del))) / (2 * del);
    vec3 light_dir = normalize(pos - light_pos);

    vec3 n = normalize(normal - du * dhu - dv * dhv);
    // float light = bias(0.25, 1.0 * dot(n, light_dir));
    // float light = 1.0 * pow(max(0, dot(reflect(-light_dir, n), dir)), 1);
    float light = 0.1 + 0.7 * bias(0.25, 1.0 * dot(n, light_dir)) + 0.2 * pow(max(0, dot(reflect(-light_dir, n), -dir)), 8);
    // light = 1;
    fragColor = vec4(water_color * light, 0.5);
    // fragColor = vec4((pos + vec3(4,4,-4)) / 8, 1.0);
    // fragColor = vec4(light_dir, 1.0);
}
