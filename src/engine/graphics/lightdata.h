#ifndef ACAENGINE_LIGHTDATA_H
#define ACAENGINE_LIGHTDATA_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

namespace graphics {
    enum LightType : int {
        directional = 0,
        spot = 1,
        point = 2
    };

    struct LightData {
        /**
         * Directional Light:
         *   position     -
         *   direction    direction the light is emitted in
         *   range        -
         *   spot_angle   -
         *   color        color of the emitted light
         *   intensity    simple multiplier for emitted light
         * Spot-Light:
         *   position     position of the light source in world space
         *   direction    direction the spot-light is facing
         *   range        maximum range of emitted light
         *   spot_angle   max angle the light is emitted at in degrees (e.g. 5° means light is emitted in a cone of 2.5° from the spot-light's direction)
         *   color        color of the emitted light
         *   intensity    resulting light-intensity at a given point is `intensity * ( 1 - ([point_to_light_distance] / [range]) )²`
         * Point-Light:
         *   position     position of the light source in world space
         *   direction    -
         *   range        maximum range of emitted light
         *   spot_angle   -
         *   color        color of the emitted light
         *   intensity    resulting light-intensity at a given point is `intensity * ( 1 - ([point_to_light_distance] / [range]) )²`
         */

        LightType light_type;
        glm::vec3 light_position;
        glm::vec3 light_direction;
        float light_range;
        float light_spot_angle;
        glm::vec3 light_color;
        float light_intensity;

        static constexpr LightData directional(glm::vec3 direction, glm::vec3 lightColor, float lightIntensity) {
            return LightData{
                    LightType::directional,
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    direction,
                    0.0f,
                    0.0f,
                    lightColor,
                    lightIntensity
            };
        }

        static constexpr LightData spot(glm::vec3 position, glm::vec3 direction, float range, float spotAngle, glm::vec3 lightColor, float lightIntensity) {
            return LightData{
                    LightType::directional,
                    position,
                    direction,
                    range,
                    spotAngle,
                    lightColor,
                    lightIntensity
            };
        }

        static constexpr LightData point(glm::vec3 position, float range, glm::vec3 lightColor, float lightIntensity) {
            return LightData{
                    LightType::directional,
                    position,
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    range,
                    0.0f,
                    lightColor,
                    lightIntensity
            };
        }

        void bindData(const unsigned int &programID);

    private:
        constexpr LightData(LightType lightType, glm::vec3 lightPosition, glm::vec3 lightDirection, float lightRange, float lightSpotAngle,
                            glm::vec3 lightColor, float lightIntensity) :
                light_type(lightType),
                light_position(lightPosition),
                light_direction(lightDirection),
                light_range(lightRange),
                light_spot_angle(lightSpotAngle),
                light_color(lightColor),
                light_intensity(lightIntensity) {}

        void prepareBinding(const unsigned int &programID);

        unsigned int currentProgramID = -1;
        GLint glsl_light_type = 0;
        GLint glsl_light_position = 0;
        GLint glsl_light_direction = 0;
        GLint glsl_light_range = 0;
        GLint glsl_light_spot_angle = 0;
        GLint glsl_light_color = 0;
        GLint glsl_light_intensity = 0;
    };
}


#endif //ACAENGINE_LIGHTDATA_H
