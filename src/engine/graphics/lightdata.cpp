#include "lightdata.h"

namespace graphics {

    void LightData::prepareBinding(const unsigned int &programID) {
        currentProgramID = programID;
        glsl_light_type = glGetUniformLocation(programID, "light_type");
        glsl_light_position = glGetUniformLocation(programID, "light_position");
        glsl_light_direction = glGetUniformLocation(programID, "light_direction");
        glsl_light_range = glGetUniformLocation(programID, "light_range");
        glsl_light_spot_angle = glGetUniformLocation(programID, "light_spot_angle");
        glsl_light_color = glGetUniformLocation(programID, "light_color");
        glsl_light_intensity = glGetUniformLocation(programID, "light_intensity");
    }

    void LightData::bindData(const unsigned int &programID) {
        if (currentProgramID != programID) {
            prepareBinding(programID);
        }

        glUniform1i(glsl_light_type, light_type);
        glUniform3fv(glsl_light_position, 1, glm::value_ptr(light_position));
        glUniform3fv(glsl_light_direction, 1, glm::value_ptr(light_direction));
        glUniform1f(glsl_light_range, light_range);
        glUniform1f(glsl_light_spot_angle, light_spot_angle);
        glUniform3fv(glsl_light_color, 1, glm::value_ptr(light_color));
        glUniform1f(glsl_light_intensity, light_intensity);
    }
}