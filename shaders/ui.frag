#version 450 core

layout(location = 0)
in vec4 in_color;

layout(location = 1)
in vec2 in_uv;

layout(set=0, binding=0)
uniform sampler2D u_texture;

layout(location = 0)
out vec4 out_color;

void main()
{
    out_color = in_color * vec4(1, 1, 1, texture(u_texture, in_uv));
}
