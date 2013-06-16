#version 130

uniform sampler2D edgeTexture;
uniform sampler2D idTexture;

//texture addressing based on http://www.phasersonkill.com/?p=495
uniform float width;
uniform float height;

varying vec4 idColour;


void main()
{ 
	vec2 texCoord = vec2(gl_FragCoord.x/width,gl_FragCoord.y/height);
	//flip y coordinate as gl_FragCoord's origin is the lower left
	texCoord.y = texCoord.y*(-1.0);
	vec4 currentId = texture2D(idTexture,texCoord);
	if(idColour.r == currentId.r && idColour.g == currentId.g && idColour.b == currentId.b)
	{
		gl_FragColor = texture2D(edgeTexture,gl_TexCoord[0].st);
	}
	else
	{
		gl_FragColor = vec4(1,1,1,0);
	}
		
}