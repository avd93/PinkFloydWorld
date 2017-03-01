#version 330

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

in vec3 position;
in vec3 color;
in vec3 normal;
in vec2 uv;

out vec3 fposition;
out vec3 fcolor;
out vec3 fnormal;
out vec2 fuv;

void main() {
  gl_Position = projection * view * model * vec4(position, 1);
  fcolor = color;
  fposition = (model*vec4(position,1)).xyz;
  fnormal = normalize((transpose(inverse(model))*vec4(normal,0)).xyz);
  fuv = uv;
}
