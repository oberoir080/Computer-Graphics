#version 330 core

in vec3 n;
in vec3 e;
in vec3 l;
in vec3 fragCol; //calling fragCol from vshader
out vec4 outColor;

vec3 Ls = vec3(1.0, 1.0, 1.0);
vec3 Ld = vec3(0.7, 0.7, 0.7);
vec3 La = vec3(0.6, 0.3, 0.4);

vec3 ks = vec3(1.0, 1.0, 1.0);
vec3 kd = vec3(0.5, 0.6, 0.4);
vec3 ka = vec3(1.0, 1.0, 1.0);

float spec_exp = 32.0;

//ambient
vec3 Ia = ka * La;

//diffuse
vec3 N = normalize(n);
vec3 L = normalize(l);
float diffuse = max(dot(N, L), 0.0);
vec3 Id = kd * Ld * diffuse;

//specular
vec3 r = reflect(-L, N);
vec3 v = normalize(e);
float specular = pow(max(dot(r, v), 0.0), spec_exp); 
vec3 Is = ks * Ls * specular;

//vec3 fColor = Ia + Id + Is; //color of the rabbit using the vectors

void main(void) {
    outColor = vec4(fragCol, 1.0);
}
