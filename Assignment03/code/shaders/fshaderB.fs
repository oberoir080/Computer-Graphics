#version 330 core

in vec3 n;
in vec3 e;
in vec3 l;
in vec3 fragCol;  // calling fragCol from vshader
in float spotFactor;  // receiving the spotlight factor from vshader
out vec4 outColor;

vec3 Ls = vec3(1.0, 1.0, 1.0);
vec3 Ld = vec3(0.7, 0.7, 0.7);
vec3 La = vec3(0.6, 0.3, 0.4);

vec3 ks = vec3(1.0, 1.0, 1.0);
vec3 kd = vec3(0.5, 0.6, 0.4);
vec3 ka = vec3(1.0, 1.0, 1.0);

float spec_exp = 32.0;

void main(void) {
    vec3 Ia = ka * La;
    vec3 N = normalize(n);
    vec3 L = normalize(l);

    // Apply spotlight factor
    float diffuse = max(dot(N, L), 0.0) * spotFactor;

    vec3 Id = kd * Ld * diffuse;

    vec3 r = reflect(-L, N);
    vec3 v = normalize(e);

    float specular = pow(max(dot(r, v), 0.0), spec_exp);
    vec3 Is = ks * Ls * specular;

    vec3 lightCol = Ia + Id + Is;

    outColor = vec4(lightCol * fragCol, 1.0);
}
