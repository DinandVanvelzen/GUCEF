#version 330 core

// Fragment program of the GUI material of guidriverOgre (COgreGuiMaterial). OpenGL 3.3 core.

uniform sampler2D guiTexture;

in vec4 guiColour;
in vec2 guiUv;

out vec4 fragColour;

void main()
{
    fragColour = guiColour * texture( guiTexture, guiUv );
}
