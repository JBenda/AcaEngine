#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 uv_coord;
layout(location = 0) out vec2 tex_coord;

void main()
{
	gl_Position = vec4(in_position,1.f);
	tex_coord = uv_coord;
}