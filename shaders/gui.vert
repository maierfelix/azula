#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex {
	vec4 gl_Position;
};

layout (location = 0) out vec2 uv;

void main() {
  vec2 pos = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(pos * 2.0 - 1.0, 0.0, 1.0);
  uv = pos;
}
