#version 330

uniform sampler2D in_Src;

uniform float in_Step;
uniform bool in_Horizontal;
	
const float kernel[5] = float[]
	(0.054, 0.242, 0.399, 0.242, 0.054);

smooth in vec2 texCoord;

out vec4 gl_FragColor;

void main() {
	vec4 accum = vec4(0.0);
	
	if(in_Horizontal) {
		for(int k = 0; k < 5; k++)
			accum += kernel[k] * texture(in_Src, texCoord + vec2(in_Step * (k - 2), 0.0));		
	} else {
		for(int k = 0; k < 5; k++)
			accum += kernel[k] * texture(in_Src, texCoord + vec2(0.0, in_Step * (k - 2)));	
	}
	
	
	gl_FragColor = accum;

}