#version 450

layout(location=0)
in vec3 in_position;

layout(location=1)
in vec4 in_color;

layout(binding=0)
uniform MATRIX_UNIFORMS {
    mat4 u_modelview;
    mat4 u_projection;
};

layout(location=0)
out vec4 out_color;

void main() {
    gl_Position = u_projection * u_modelview * vec4(in_position, 1.0);
    out_color = in_color;
}