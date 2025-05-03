#version 450

layout(binding = 0) uniform Transform {
  mat4 u_projection_matrix;
  mat4 u_modelview_matrix;
  mat4 u_view_matrix;
};

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

layout(location = 0) out vec3 v_view_normal;
layout(location = 1) out vec3 v_view_position;
layout(location = 2) out vec2 v_uv;

void main() {
  vec4 view_position = u_modelview_matrix * vec4(a_position, 1.0);
  v_view_normal = normalize((u_modelview_matrix * vec4(a_normal, 0.0)).xyz);
  v_view_position = view_position.xyz;
  v_uv = a_uv * 3.0;
  gl_Position = u_projection_matrix * view_position;
}