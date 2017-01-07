#version 100

precision mediump float;

uniform float blend_factor;
uniform sampler2D textures[2];

varying vec2 texcoord;

void main()
{
	const vec4 a = vec4(1.0, 0.0, 0.0, 1.0);
	const vec4 b = vec4(0.0, 0.0, 1.0, 1.0);

	gl_FragColor = mix(a, b, blend_factor);
}
