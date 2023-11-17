#version 330 core

in vec3 vVertex;
in vec3 vNormal;

uniform mat4 vModel;
uniform mat4 vView;
uniform mat4 vProjection;
uniform vec3 lpos_world;
uniform vec3 eye_normal;

// Add spotlight parameters
uniform vec3 lightPosition;  // Spotlight position
uniform vec3 lightDirection;  // Spotlight direction
uniform float lightCutoff;  // Spotlight cutoff angle

out vec3 fragCol;
out float spotFactor;  // Pass the spotlight factor to the fragment shader

void main() {
    vec3 Ls = vec3(1.0, 1.0, 1.0);  // Vectors taken from fshader
    vec3 Ld = vec3(0.7, 0.7, 0.7);
    vec3 La = vec3(0.6, 0.3, 0.4);
    
    vec3 ks = vec3(1.0, 1.0, 1.0);  // Vectors taken from fshader
    vec3 kd = vec3(0.5, 0.6, 0.4);
    vec3 ka = vec3(1.0, 1.0, 1.0);
    
    vec3 normal = normalize(mat3(transpose(inverse(vModel))) * vNormal);
    vec3 view = normalize(eye_normal - vVertex);
    vec3 light = normalize(lpos_world - vVertex);
    vec3 reflec = reflect(-light, normal);

    vec3 ambient = ka * La;

    // vector from the vertex to the light source
    vec3 lightToVertex = vVertex - lightPosition;

    // angle between the light direction and the vertex's normal vector
    float cosTheta = dot(-lightDirection, normalize(lightToVertex));

    // spotlight factor based on the cutoff angle
    float spotFactor = cosTheta > cos(lightCutoff) ? 1.0 : 0.0;

    float diffuse = max(dot(normal, light), 0.0);
    vec3 diffuseCol = kd * Ld;

    float spec_exp = 32.0;
    float spec = pow(max(dot(view, reflec), 0.0), spec_exp);
    vec3 specCol = ks * Ls;

    vec3 lightCol = ambient + (diffuse * spotFactor * diffuseCol) + (spec * specCol);

    fragCol = lightCol;
    spotFactor = spotFactor;  // Pass the spotlight factor to the fragment shader

    gl_Position = vProjection * vView * vModel * vec4(vVertex, 1.0);
}
