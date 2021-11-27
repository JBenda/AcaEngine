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
    vec3 ambientColor = max(phongData.r * ambient_light, 0.0f);
    vec3 diffuseColor;
    vec3 specularColor;

    // TODO light bounces? occlusion?
    if (light_type == LIGHT_TYPE_DIRECTIONAL){
        // direction, color, intensity

        float diffuseDot = max(dot(-light_direction, normal_vec_frag), 0.0f);
        float specularDot = max(dot(reflect(light_direction, normal_vec_frag), normalize(camera_position - render_position_frag)), 0.0f);
        diffuseColor = phongData.g * diffuseDot * light_color * light_intensity;
        specularColor = phongData.b * 8 * pow(specularDot, phongData.a * 255) * light_color * light_intensity;

    } else if (light_type == LIGHT_TYPE_SPOT){
        // position, direction, range, spot_angle, color, intensity
        vec3 spotLightDirection = normalize(render_position_frag - light_position);
        // here we're (ab-)using the fact that the dot product of two normalized vectors is the cosine of their angle
        float spotLightAngle = dot(light_direction, spotLightDirection); 
        float spotLightDistance = length(render_position_frag - light_position);
        if (spotLightAngle > light_spot_angle && spotLightDistance <= light_range){
            float distanceFactor = 0.01f / pow(spotLightDistance / light_range, 2);
            float diffuseDot = max(dot(-spotLightDirection, normal_vec_frag), 0.0f);
            float specularDot = max(dot(reflect(spotLightDirection, normal_vec_frag), normalize(camera_position - render_position_frag)), 0.0f);
            diffuseColor = phongData.g * diffuseDot * light_color * light_intensity * distanceFactor;
            specularColor = phongData.b * 8 * pow(specularDot, phongData.a * 255) * light_color * light_intensity * distanceFactor;
        }

    } else if (light_type == LIGHT_TYPE_POINT){
        // position, range, color, intensity
        // light intensity should diminish proportionally to `1 / distance_squared`
        vec3 pointLightDirection = normalize(render_position_frag - light_position);
        float pointLightDistance = length(render_position_frag - light_position);
        if (pointLightDistance <= light_range){
            float distanceFactor = 0.01f / pow(pointLightDistance / light_range, 2);
            float diffuseDot = max(dot(-pointLightDirection, normal_vec_frag), 0.0f);
            float specularDot = max(dot(reflect(pointLightDirection, normal_vec_frag), normalize(camera_position - render_position_frag)), 0.0f);
            diffuseColor = phongData.g * diffuseDot * light_color * light_intensity * distanceFactor;
            specularColor = phongData.b * 8 * pow(specularDot, phongData.a * 255) * light_color * light_intensity * distanceFactor;
        }
    }

    out_color = texColor * vec4(ambientColor + diffuseColor + specularColor, 1.0f);
}