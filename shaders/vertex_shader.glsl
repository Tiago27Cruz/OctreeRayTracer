#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 FragCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 iResolution;   

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragCoord = (gl_Position.xy / gl_Position.w + 1.0) / 2.0 * iResolution.xy;
}