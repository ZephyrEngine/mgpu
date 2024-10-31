#version 450

layout(location = 0) out vec3 v_color;

void main() {
  switch(gl_VertexIndex) {
    case 0: {
      gl_Position = vec4(-0.5,  0.5, 0.0, 1.0);
      v_color = vec3(1.0, 0.0, 0.0);
      break;
    }
    case 1: {
      gl_Position = vec4( 0.5,  0.5, 0.0, 1.0);
      v_color = vec3(0.0, 1.0, 0.0);
      break;
    }
    case 2: {
      gl_Position = vec4( 0.0, -0.5, 0.0, 1.0);
      v_color = vec3(0.0, 0.0, 1.0);
      break;
    }
  }
}