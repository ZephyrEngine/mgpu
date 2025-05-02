#version 450

layout(binding = 0) uniform Transform {
  mat4 u_projection_matrix;
  mat4 u_modelview_matrix;
  mat4 u_view_matrix;
};

layout(location = 0) in vec3 v_view_normal;
layout(location = 1) in vec3 v_view_position;

layout(location = 0) out vec4 frag_color;

#define PI (3.141592653)

void main() {
  const vec3 light_direction = vec3(1.0, 0.0, 1.0);

  vec3 view_light_direction = normalize((u_view_matrix * vec4(light_direction, 0.0)).xyz);
  vec3 view_direction = -normalize(v_view_position);
  vec3 view_normal = normalize(v_view_normal);
  vec3 halfway_vector = normalize(view_light_direction + view_direction);
  float n_dot_l = max(dot(view_normal, view_light_direction), 0.0);
  float n_dot_h = max(dot(view_normal, halfway_vector), 0.0);
  float n_dot_v = max(dot(view_normal, view_direction), 0.0);
  float fresnel = 0.04 + 0.96 * pow(1.0 - n_dot_v, 5.0);

  vec3 diffuse = vec3(0.5, 0.0, 0.0) * n_dot_l;
  vec3 specular = vec3(pow(n_dot_h, 1024.0)) * PI;// * n_dot_l;
  vec3 lighting = mix(diffuse, specular, fresnel);

  frag_color = vec4(pow(lighting, vec3(1.0/2.2)), 1.0);
}