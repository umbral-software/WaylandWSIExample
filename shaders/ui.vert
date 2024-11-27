#version 450 core

layout(location = 0)
in vec2 in_pos;
layout(location = 1)
in vec2 in_uv;
layout(location = 2)
in vec4 in_color;

layout(push_constant) uniform PUSH_CONSTANTS {
    vec2 u_scale;
    vec2 u_translate;
};

layout(location = 0)
out vec4 out_color;

layout(location = 1)
out vec2 out_uv;

void main()
{
    gl_Position = vec4((in_pos * u_scale + u_translate) * vec2(1, -1), 0, 1);

    out_color = in_color;
    out_uv = in_uv;
}
