#version 450

layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_uv;

layout(location = 0) out vec3 v_color;

void main() {
//  switch(gl_VertexIndex) {
//    case 0: {
//      gl_Position = vec4(-0.5,  0.5, 0.0, 1.0);
//      v_color = vec3(1.0, 0.0, 0.0);
//      break;
//    }
//    case 1: {
//      gl_Position = vec4( 0.5,  0.5, 0.0, 1.0);
//      v_color = vec3(0.0, 1.0, 0.0);
//      break;
//    }
//    case 2: {
//      gl_Position = vec4( 0.0, -0.5, 0.0, 1.0);
//      v_color = vec3(0.0, 0.0, 1.0);
//      break;
//    }
//  }
  v_color = a_color.xyz;
  gl_Position = a_position;

}