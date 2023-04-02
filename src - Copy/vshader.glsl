#version 410

in vec4 vPosition;
in vec3 vNormal;
in vec4 vColor;
out vec4 color;
out vec3 fN;
out vec3 fE;

uniform mat4 ModelView;
uniform mat4 Projection;

void main()
{
    gl_Position = Projection * ModelView * vPosition;
    color = vColor;
    fN = vNormal;
    fE = vPosition.xyz;
}

