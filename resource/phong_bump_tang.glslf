///essl #version 100
///glsl #version 150

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// tangential space bump mapping, fragment shader
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if GL_ES == 1
#extension GL_OES_standard_derivatives : require

#ifdef GL_FRAGMENT_PRECISION_HIGH
	precision highp float;
#else
	precision mediump float;
#endif

#define in_qualifier varying
#define xx_FragColor gl_FragColor

#else

#define in_qualifier in
out vec4 xx_FragColor;

#endif

const vec4 scene_ambient	= vec4(0.2, 0.2, 0.2, 1.0);
const vec3 lprod_diffuse	= vec3(0.5, 0.5, 0.5);
const vec3 lprod_specular	= vec3(0.7, 0.7, 0.5);
const float shininess		= 64.0;

in_qualifier vec3 p_obj_i;	// vertex position in object space
in_qualifier vec3 n_obj_i;	// vertex normal in object space
in_qualifier vec3 l_obj_i;	// light_source vector in object space
in_qualifier vec3 h_obj_i;	// half-direction vector in object space
in_qualifier vec2 tcoord_i;

uniform sampler2D normal_map;
uniform sampler2D albedo_map;

void main()
{
	vec3 p_dx = dFdx(p_obj_i);
	vec3 p_dy = dFdy(p_obj_i);

#if 1 // compute tangent and bi-tangent from texcoords
	vec2 tc_dx = dFdx(tcoord_i);
	vec2 tc_dy = dFdy(tcoord_i);

	vec3 t = normalize( tc_dy.y * p_dx - tc_dx.y * p_dy );
	vec3 b = normalize( tc_dx.x * p_dy - tc_dy.x * p_dx );

#else
	vec3 t = normalize(p_dx);
	vec3 b = normalize(p_dy);

#endif

#if 1 // build TBN taking into account the mesh-supplied normals
	vec3 n = normalize(n_obj_i);

	b = cross(n, t);
	b = normalize(b);
	t = cross(b, n);

#else
	vec3 n = cross(t, b);

#endif

	mat3 tbn = mat3(t, b, n);

	vec3 l_tan = normalize(l_obj_i) * tbn;
	vec3 h_tan = normalize(h_obj_i) * tbn;

	vec3 bump = normalize(texture2D(normal_map, tcoord_i).xyz * 2.0 - 1.0);

	vec3 d = lprod_diffuse * max(dot(l_tan, bump), 0.0);
	vec3 s = lprod_specular * pow(max(dot(h_tan, bump), 0.0), shininess);

#if 1 // apply albedo map with alpha
	vec4 tcolor = texture2D(albedo_map, tcoord_i);

#else
	const vec4 tcolor = vec4(1.0);

#endif

	// be considerate of the older SIMD architectures and compilers
	xx_FragColor =
		vec4(s, 0.0) +
		vec4(d, 0.0) * tcolor +
		scene_ambient * tcolor;
}
