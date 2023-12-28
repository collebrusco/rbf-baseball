#version 410 core

in vec2 iUV;
in vec3 iPos;
out vec4 outColor;

uniform sampler2D utex;

void main() {
	// vec2 samp = vec2(ivec2(iUV * 10.)) / 10.;
	// outColor = vec4(samp, 0, 1);
	float moddedX = (iUV.x + 0.5) * 3;
	while (moddedX > 1) moddedX -= 1;
	vec2 moddediUV = vec2(moddedX, iUV.y);
	outColor = texture(utex, moddediUV);
}
