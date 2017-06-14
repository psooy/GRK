#version 430 core

uniform vec3 objectColor;
uniform vec3 lightDir;
uniform sampler2D activeTexture;

in vec3 interpNormal;
in vec2 TexCoord;

void main()
{
	vec3 normal = normalize(interpNormal);
	float diffuse = max(dot(normal, -lightDir), 0.0);
	vec4 textureColor = texture2D(activeTexture, TexCoord);
	vec3 newColor = textureColor.rgb;
	gl_FragColor = vec4(newColor * diffuse, 1.0);
}
