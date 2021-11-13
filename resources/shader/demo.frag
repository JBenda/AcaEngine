#version 450

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_SPOT 1
#define LIGHT_TYPE_POINT 2

layout(location = 0) in vec2 uv_coord_frag;
layout(location = 1) in vec3 normal_vec_frag;
layout(location = 2) in vec3 render_position_frag;

layout(binding = 0) uniform sampler2D tx_color;
layout(binding = 1) uniform sampler2D tx_phong;// stores ambient/diffuse/specular/shinyness weights in r/g/b/a respectively

layout(location = 0) out vec4 out_color;

uniform vec3 camera_position;
uniform vec3 ambient_light;

// TODO figure out what to do about multiple light sources (ideally with variable length - "Shader Storage Buffer Objects" ? )
uniform int light_type;
uniform vec3 light_position;
uniform vec3 light_direction;
uniform float light_range;
uniform float light_spot_angle;
uniform vec3 light_color;
uniform float light_intensity;

void main()
{
    vec4 texColor = texture(tx_color, uv_coord_frag);
    vec4 phongData = texture(tx_phong, uv_coord_frag);
    vec3 ambientColor = max(phongData.r * ambient_light, 0f);
    vec3 diffuseColor;
    vec3 specularColor;

    // TODO light bounces? occlusion?
    if (light_type == LIGHT_TYPE_DIRECTIONAL){
        // direction, color, intensity
        diffuseColor = max(phongData.g * dot(-light_direction, normal_vec_frag) * light_color, 0f);
        specularColor = max(phongData.b * pow(dot(reflect(light_direction, normal_vec_frag), normalize(camera_position - render_position_frag)), phongData.a * 255) * light_color, 0f);
    } else if (light_type == LIGHT_TYPE_SPOT){
        // TODO implement spot lights
    } else if (light_type == LIGHT_TYPE_POINT){
        // TODO implement point lights
    }
    
    out_color = texColor * vec4(ambientColor + diffuseColor + specularColor, 1f);
    //out_color = vec4(normal_vec_frag, 1f);
}