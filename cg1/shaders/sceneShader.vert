#version 330


/////////////////////////////////////////////////////////////////////////////
// Attributes
/////////////////////////////////////////////////////////////////////////////
layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

/////////////////////////////////////////////////////////////////////////////
// Uniforms
/////////////////////////////////////////////////////////////////////////////
uniform mat4 matModel;
uniform mat4 matNormal;
uniform mat4 matVP;

uniform float time;
uniform int shaderMode;

/////////////////////////////////////////////////////////////////////////////
// Varyings
/////////////////////////////////////////////////////////////////////////////
out vec3 fragNormal;
out vec2 fragTexCoord;
out vec3 fragVertWorld;

/////////////////////////////////////////////////////////////////////////////
// ShadowMapping
/////////////////////////////////////////////////////////////////////////////
uniform int enableShadowMapping;	// 0: disabled, 1:enabled
#define MAX_LIGHTS 10
uniform mat4 matDepthVP[MAX_LIGHTS];
out vec3 fragVertShadowClip[MAX_LIGHTS];
uniform int numLights;


struct waveData{
	float amplitude;
	vec2 dir;
	float frequency;
	float phase;
	float k;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Empty shader
////////////////////////////////////////////////////////////////////////////////////////////////////
void emptyShader(){

    fragTexCoord = texCoord;
    fragNormal = mat3(matNormal) * normal;
    fragVertWorld = vec3(matModel*position);
    gl_Position = matVP * matModel* position;
    
    if(enableShadowMapping == 1){
	    for(int i = 0; i < numLights; i++){
		    vec4 shCoord = (matDepthVP[i] * matModel * position);
		    fragVertShadowClip[i] = vec3(shCoord);
	    }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Water shader
////////////////////////////////////////////////////////////////////////////////////////////////////


float evalWaveFkt(waveData data, vec2 pos){
	// Wolfram alpha: f = d/dy a*2*((sin((d*x+e*y)*w + t*p)+1)/2)^k, d= dir.x, e = dir.y
	
	float a = data.amplitude;
	float w = data.frequency;
	float p = data.phase;
	float k = data.k;
	
	float res = 2*a* pow((sin(dot(data.dir,pos)*w + time*p) + 1)/2,k);
	return res;
}

vec2 evalWaveFktDxz(waveData data, vec2 pos){

	// Wolfram alpha: f' = {d,e}* a×2^(1-k) k w cos(w (d x+e y)+p t) (sin(w (d x+e y)+p t)+1)^(k-1)

	float a = data.amplitude;
	float w = data.frequency;
	float p = data.phase;
	float k = data.k;

	vec2 d;
	float constDeriv = a*k*pow(2,1-k)*w*cos(dot(data.dir,pos)*w + time*p)*pow(sin(dot(data.dir,pos)*w + time*p) + 1,k-1);
	d.x = constDeriv*data.dir.x;
	d.y = constDeriv*data.dir.y;
	
	return d;
}

void waterShader(){
    fragTexCoord = texCoord;
    fragNormal = mat3(matNormal) * normal;
    fragVertWorld = vec3(matModel*position);

	vec4 pos = position;
	
    // http://www.cc.gatech.edu/~ybai30/cs_7490_final_website/cg_water.html
	waveData data[4];
	
	data[0].amplitude = .2;
	data[0].dir = normalize(vec2(-1,-3));
	data[0].frequency = .4;
	data[0].phase = 2;
	data[0].k = 4;
	
	
	data[1].amplitude = .3;
	data[1].dir = normalize(vec2(4,3));
	data[1].frequency = .5;
	data[1].phase = 1;
	data[1].k = 2;
	
	data[2].amplitude = .1;
	data[2].dir = normalize(vec2(4,-8));
	data[2].frequency = .2;
	data[2].phase = 3;
	data[2].k = 5;
	
	data[3].amplitude = .2;
	data[3].dir = normalize(vec2(-5,10));
	data[3].frequency = 1;
	data[3].phase = 3;
	data[3].k = 5;
	
	
	float hRes = 0;
	vec2 dxz = vec2(0,0);
	
	for(int i = 0; i < 4; i++){
		hRes += evalWaveFkt(data[i],pos.xz);
		dxz += evalWaveFktDxz(data[i],pos.xz);
	}
	
	vec3 n = vec3(-dxz.x,1,-dxz.y);	
	pos.y = hRes;
	
    fragNormal = normalize(mat3(matNormal)*n);
    gl_Position = matVP * matModel* pos;
    
   
    if(enableShadowMapping == 1){
	    for(int i = 0; i < numLights; i++){
		    vec4 shCoord = (matDepthVP[i] * matModel * pos);
		    fragVertShadowClip[i] = vec3(shCoord);
	    }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////////////////////////////////
void main()
{
	switch(shaderMode){
		case 1:
		case 5:
			waterShader();
			break;
		case 0:
		case 4:
			emptyShader();
			break;
		default:
			emptyShader();
	}
    
}
