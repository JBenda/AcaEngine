#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 uv_coord;
layout(location = 2) in vec3 normal_vec;

layout(location = 0) out vec2 uv_coord_frag;
layout(location = 1) out vec3 normal_vec_frag;
layout(location = 2) out vec3 render_position_frag;

uniform mat4 world_to_camera_matrix;

void main()
{
    //gl_Position = vec4(in_position, 1f);
    gl_Position = world_to_camera_matrix * vec4(in_position, 1f);
    render_position_frag = in_position;
    
    uv_coord_frag = uv_coord;
    normal_vec_frag = normal_vec;
}