#version 330 core

// Vertex program of the GUI material of guidriverOgre (COgreGuiMaterial): a vertex colour times a texture, for
// GUI geometry in pixels of the GUI. OpenGL 3.3 core.

uniform mat4 projection;
uniform mat4 transform;
uniform vec2 translation;

in vec4 vertex;
in vec4 colour;
in vec4 uv0;

out vec4 guiColour;
out vec2 guiUv;

void main()
{
    guiColour = colour;
    guiUv = uv0.xy;
    gl_Position = projection * ( transform * vec4( vertex.xy + translation, 0.0, 1.0 ) );
}
