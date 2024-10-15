// Generates a fullscreen triangle. See https://www.saschawillems.de/blog/2016/08/13/vulkan-tutorial-on-rendering-a-fullscreen-quad-without-buffers/
out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	vec2 uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(uv * 2.0f - 1.0f, 0.0f, 1.0f);

}