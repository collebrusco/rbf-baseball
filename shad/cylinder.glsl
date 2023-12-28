#version 410 core

in vec3 iNorm;
out vec4 outColor;

uniform vec3 uLightDirection; // Direction of the light source

void main() {
    // // Compute diffuse lighting
    // float diff = max(dot(iNorm, -uLightDirection), 0.0);
    // vec3 diffuseColor = diff * vec3(1.0, 0.5, 0.31); // Change this color as needed

    // outColor = vec4(diffuseColor * 10, 1.0);
    outColor = vec4(normalize(iNorm) * 0.5 + 0.5, 1.0);

}
