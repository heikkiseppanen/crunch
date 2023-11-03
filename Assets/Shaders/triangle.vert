#version 460

layout(location = 0) out vec3 frag_color;

vec2 positions[3] = vec2[](
	vec2(-0.5f,  -0.5f ),
	vec2( 0.5f,  -0.5f ),
	vec2( 0.0f,   0.5f )
);

vec3 colors[3] = vec3[](
	vec3( 1.0f, 0.0f, 0.0f ),
	vec3( 0.0f, 1.0f, 0.0f ),
	vec3( 0.0f, 0.0f, 1.0f )
);

void main()
{
	gl_Position = vec4(positions[gl_VertexIndex], 0.0f, 1.0f);
	frag_color = colors[gl_VertexIndex];
}
