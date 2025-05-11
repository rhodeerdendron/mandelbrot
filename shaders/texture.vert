#version 330 core

layout(location = 0) in vec2 v_texcoord;

uniform ivec2 tex_position;
uniform ivec2 tex_size;
uniform float tex_z;

uniform ivec2 screen_size;

out vec2 f_texcoord;

float remap_range_float(vec2 from_range, vec2 to_range, float value)
{
    float norm_value = (value - from_range.x) / (from_range.y - from_range.x);
    return norm_value * (to_range.y - to_range.x) + to_range.x;
}
vec2 remap_range_vec2(vec4 from_range, vec4 to_range, vec2 value)
{
    return vec2(
        remap_range_float(from_range.xy, to_range.xy, value.x),
        remap_range_float(from_range.zw, to_range.zw, value.y));
}


void main()
{
    // invert texcoord y
    f_texcoord = vec2(v_texcoord.x, 1.0 - v_texcoord.y);

    // remap screen-space to clip-space
    vec2 tex_clip_vertex = vec2(tex_position) + v_texcoord * vec2(tex_size);
    vec2 tex_clip_position = remap_range_vec2(
        vec4( vec2(0.0, screen_size.x), vec2(0.0, screen_size.y) ),
        vec4( vec2(-1.0, 1.0), vec2(1.0, -1.0) ),
        tex_clip_vertex);

    gl_Position = vec4(tex_clip_position, tex_z, 1.0);
}
