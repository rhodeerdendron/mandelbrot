#version 330 core

layout(location = 0) in vec2 v_position;

out vec2 f_st;

void main()
{
    f_st = v_position;
    gl_Position = vec4(v_position, 0.0, 1.0);
}

