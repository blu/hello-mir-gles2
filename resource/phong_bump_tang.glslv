////////////////////////////////////////////////////////////////////////////////////////////////////////////
// tangential space bump mapping, vertex shader
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if GL_ES == 1

#define in_qualifier attribute
#define out_qualifier varying

#else

#define in_qualifier in
#define out_qualifier out

#endif

in_qualifier vec3 at_Vertex;
in_qualifier vec3 at_Normal;
in_qualifier vec2 at_MultiTexCoord0;

out_qualifier vec2 tcoord_i;
out_qualifier vec3 p_obj_i;		// vertex position in object space
out_qualifier vec3 n_obj_i;		// vertex normal in object space
out_qualifier vec3 l_obj_i;		// light_source vector in object space
out_qualifier vec3 h_obj_i;		// half-direction vector in object space

uniform mat4 mvp;
uniform vec4 lp_obj;	// light-source position in object space
uniform vec4 vp_obj;	// viewer position in object space

void main()
{
	gl_Position = mvp * vec4(at_Vertex, 1.0);

	tcoord_i = at_MultiTexCoord0.xy;
	p_obj_i = at_Vertex;
	n_obj_i = at_Normal;

	vec3 l_obj = normalize(lp_obj.xyz - at_Vertex * lp_obj.w);
	vec3 v_obj = normalize(vp_obj.xyz - at_Vertex * vp_obj.w);

	l_obj_i = l_obj;
	h_obj_i = l_obj + v_obj;
}
