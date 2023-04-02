#version 410

in vec4 color;
out vec4 fragColor;
in vec3 fN;
in vec3 fE;

void main()
{
     fragColor = color;
     vec3 N = normalize(fN);
     vec3 E = normalize(fE);
}

