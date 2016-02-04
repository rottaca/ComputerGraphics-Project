#version 330


/////////////////////////////////////////////////////////////////////////////
// Attributes
/////////////////////////////////////////////////////////////////////////////
layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 tangent;

/////////////////////////////////////////////////////////////////////////////
// Uniforms
/////////////////////////////////////////////////////////////////////////////
uniform mat4 matModel;
uniform mat4 matNormal;
uniform mat4 matVP;
uniform mat4 matV;

uniform float time;
uniform int shaderMode;
uniform	int waterMode;
uniform int enablePostProc;

/////////////////////////////////////////////////////////////////////////////
// Varyings
/////////////////////////////////////////////////////////////////////////////
out vec3 fragVaryingNormalViewSpace;
out vec2 fragTexCoord;
out vec3 fragVertViewSpace;

/////////////////////////////////////////////////////////////////////////////
// ShadowMapping
/////////////////////////////////////////////////////////////////////////////
uniform int enableShadowMapping;	// 0: disabled, 1:enabled
#define MAX_LIGHTS 10
uniform mat4 matDepthVP[MAX_LIGHTS];
out vec4 fragVertShadowClip[MAX_LIGHTS];
uniform int numLights;

/////////////////////////////////////////////////////////////////////////////
// NormalMapping
/////////////////////////////////////////////////////////////////////////////
uniform int enableNormalMapping; // 0: disabled, 1:enabled
uniform int hasNormalMap; // 0: disabled, 1:enabled
out vec3 fragTangentViewSpace;

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
    fragVaryingNormalViewSpace = normalize(mat3(matV)*mat3(matNormal)*normal);
    fragVertViewSpace = vec3(matV*matModel*position);
    gl_Position = matVP * matModel* position;
    
    if(enableShadowMapping == 1){
	    for(int i = 0; i < numLights; i++){
		    fragVertShadowClip[i] = matDepthVP[i] * matModel * position;
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

	vec4 pos = position;
	
    // http://www.cc.gatech.edu/~ybai30/cs_7490_final_website/cg_water.html
	waveData data[4];
	if(waterMode == 0){
		data[0].amplitude = .1;
		data[0].dir = normalize(vec2(-3,-3));
		data[0].frequency = .4;
		data[0].phase = 0;
		data[0].k = 2;
		
		data[1].amplitude = .1;
		data[1].dir = normalize(vec2(4,6));
		data[1].frequency = .5;
		data[1].phase = 1;
		data[1].k = 2;
		
		data[2].amplitude = .1;
		data[2].dir = normalize(vec2(9,-8));
		data[2].frequency = .2;
		data[2].phase = 3;
		data[2].k = 2;
		
		data[3].amplitude = .1;
		data[3].dir = normalize(vec2(-4,10));
		data[3].frequency = 1;
		data[3].phase = 3;
		data[3].k = 2;
	}else if(waterMode == 1){
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
	}else if(waterMode == 2){
		data[0].amplitude = 0.5;
		data[0].dir = normalize(vec2(1,3));
		data[0].frequency = 0.5;
		data[0].phase = 3;
		data[0].k = 7;
		
		data[1].amplitude = .2;
		data[1].dir = normalize(vec2(4,3));
		data[1].frequency = .3;
		data[1].phase = 3;
		data[1].k = 2;
		
		data[2].amplitude = .3;
		data[2].dir = normalize(vec2(4,-3));
		data[2].frequency = .2;
		data[2].phase = 3;
		data[2].k = 1;
		
		data[3].amplitude = .5;
		data[3].dir = normalize(vec2(-1,3));
		data[3].frequency = .4;
		data[3].phase = 3 ;
		data[3].k = 3;
	}
	
	float hRes = 0;
	vec2 dxz = vec2(0,0);
	vec2 moveDir = vec2(0,0);
	
	for(int i = 0; i < 4; i++){
		hRes += evalWaveFkt(data[i],pos.xz);
		dxz += evalWaveFktDxz(data[i],pos.xz);
		moveDir += -data[i].dir*data[i].amplitude*data[i].frequency;
	}
	
	vec3 n = vec3(-dxz.x,1,-dxz.y);	
	pos.y = hRes;
	
    fragVertViewSpace = vec3(matV*matModel*position);
    fragVaryingNormalViewSpace = normalize(mat3(matV)*mat3(matNormal)*n);
    gl_Position = matVP * matModel* pos;
    
    fragTexCoord = texCoord+moveDir*time;
   
    if(enableShadowMapping == 1){
	    for(int i = 0; i < numLights; i++){
		    fragVertShadowClip[i] = matDepthVP[i] * matModel * pos;
	    }
    }
}

void postProcShader(){
	fragTexCoord  = texCoord;
    gl_Position = matVP * position;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////////////////////////////////
void main()
{
	if(enablePostProc == 1)
	{
		postProcShader();
		return;
	}
	
	if(enableNormalMapping == 1 && hasNormalMap == 1)
		fragTangentViewSpace = normalize(mat3(matV)*mat3(matNormal)*tangent);

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
