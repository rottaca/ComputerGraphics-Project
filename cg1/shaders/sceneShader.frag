#version 330
#extension GL_EXT_texture_array : enable

#define MAX_LIGHTS 10
/////////////////////////////////////////////////////////////////////////////
// Uniforms
/////////////////////////////////////////////////////////////////////////////
uniform sampler2D tex;	// Diffuse texture
uniform sampler2D normalTex;

/////////////////////////////////////////////////////////////////////////////
// Light
// Source: http://www.tomdalling.com/blog/modern-opengl/08-even-more-lighting-directional-lights-spotlights-multiple-lights/
/////////////////////////////////////////////////////////////////////////////
// array of lights
uniform int enableLighting; // 0: disabled, 1: enabled
uniform int numLights;
uniform struct Light {
   vec4 position;
   vec3 intensities; //a.k.a the color of the light
   float att_c1;
   float att_c2;
   float att_c3;
   float ambientCoefficient;
   float coneAngle;
   vec3 coneDirection;
} allLights[MAX_LIGHTS];

uniform struct Material{
	float shininess;
	vec3 specularColor;
} material;


/////////////////////////////////////////////////////////////////////////////
// Shadow Mapping
/////////////////////////////////////////////////////////////////////////////
uniform int enableShadowMapping;	// 0: disabled, 1:enabled
uniform sampler2DArray shadowTexArray;
in vec4 fragVertShadowClip[MAX_LIGHTS];

/////////////////////////////////////////////////////////////////////////////
// Bump Mapping
/////////////////////////////////////////////////////////////////////////////
uniform int enableBumpMapping; // 0: disabled, 1:enabled
uniform int hasBumpMap; // 0: disabled, 1:enabled
in vec3 fragTangentViewSpace;

/////////////////////////////////////////////////////////////////////////////
// Varyings
/////////////////////////////////////////////////////////////////////////////
in vec3 fragVaryingNormalViewSpace;
in vec2 fragTexCoord;
in vec3 fragVertViewSpace;
uniform int shaderMode;

/////////////////////////////////////////////////////////////////////////////
// Output
/////////////////////////////////////////////////////////////////////////////
out vec4 outputColor;

vec3 fragNormalViewSpace;

void emptyShader();

bool isShadowed(int lightNr, vec3 surfaceToLightViewSpace)
{ 
	float cosTheta = clamp(dot(fragNormalViewSpace, surfaceToLightViewSpace),0,1);
	float bias = 0.005;//*cosTheta;
	bias = clamp(bias, 0.0,0.01);
	
	if(allLights[lightNr].position.w == 0)
		return texture2DArray( shadowTexArray, vec3(fragVertShadowClip[lightNr].xy, lightNr) ).x  <  fragVertShadowClip[lightNr].z - bias;
	// Perspecive devision for spot light
	else if(allLights[lightNr].coneAngle <180)
		return texture2DArray( shadowTexArray, vec3(fragVertShadowClip[lightNr].xy/fragVertShadowClip[lightNr].w, lightNr) ).x  <  (fragVertShadowClip[lightNr].z - bias)/fragVertShadowClip[lightNr].w;
	
}

// Applies the specified light
vec3 ApplyLight(int lightNr, vec3 surfaceColor, vec3 normalViewSpace, vec3 surfacePosViewSpace, vec3 surfaceToCameraViewSpace) {
	Light light = allLights[lightNr];
	
    vec3 surfaceToLightViewSpace;
    float attenuation = 1.0;
    if(light.position.w == 0.0) {
        //directional light
        surfaceToLightViewSpace = normalize(light.position.xyz);
        attenuation = 1.0; //no attenuation for directional lights
    } else {
        //point light
        surfaceToLightViewSpace = normalize(light.position.xyz - surfacePosViewSpace);
        float distanceToLight = length(light.position.xyz - surfacePosViewSpace);
        attenuation = min(1,1/(light.att_c1+distanceToLight*light.att_c2+distanceToLight*distanceToLight*light.att_c3));

        //cone restrictions (affects attenuation)
        float lightToSurfaceAngle = degrees(acos(dot(-surfaceToLightViewSpace, normalize(light.coneDirection))));
        if(lightToSurfaceAngle > light.coneAngle){
            attenuation = 0.0;
        }
    }

    //ambient
    vec3 ambient = light.ambientCoefficient * surfaceColor.rgb * light.intensities;

    //diffuse
    float diffuseCoefficient = max(0.0, dot(surfaceToLightViewSpace, normalViewSpace));
    vec3 diffuse = diffuseCoefficient * surfaceColor.rgb * light.intensities;
    //specular
    float specularCoefficient = 1;
    if(material.shininess > 0){
    	specularCoefficient = pow(max(0.0, dot(-surfaceToCameraViewSpace, reflect(surfaceToLightViewSpace, normalViewSpace))), material.shininess);
	}
    vec3  specular = specularCoefficient * surfaceColor * light.intensities;


	// Shadow Mapping
	float visibility = 1.0;
	
	if (enableShadowMapping == 1 && isShadowed(lightNr, surfaceToLightViewSpace)){
		visibility = 0.1;
		
		// Error output for unshadowed regions
 		//if(fragVertShadowClip[lightNr].y < 0 ||fragVertShadowClip[lightNr].y > 1 || 
 		//   fragVertShadowClip[lightNr].x < 0 ||fragVertShadowClip[lightNr].x > 1 || 
 		//   fragVertShadowClip[lightNr].z < 0 ||fragVertShadowClip[lightNr].z > 1)
		//	return vec3(1,0,0);
 	}
	
    // linear color (color before gamma correction)
    return ambient + visibility*attenuation*(diffuse + specular);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Phong lighting model + shadow mapping
////////////////////////////////////////////////////////////////////////////////////////////////////
void LightShader(){
	if(enableLighting == 0)
	{
		emptyShader();
		return;
	}
	vec3 surfaceColor = vec3(texture(tex,fragTexCoord.xy));
	vec3 surfaceToCameraViewSpace = normalize(-fragVertViewSpace);
	
	vec3 linearColor = vec3(0);
	for(int i = 0; i < numLights; ++i){
	    linearColor += ApplyLight(i, surfaceColor, fragNormalViewSpace, fragVertViewSpace, surfaceToCameraViewSpace);
	}
	outputColor = vec4(linearColor,1.0f);
	//outputColor = vec4(fragTangentViewSpace,1.0f);
}


void emptyShader(){
	outputColor = texture(tex,fragTexCoord.xy);
	//outputColor = vec4(fragTangentViewSpace,1.0f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////////////////////////////////
void main()
{
	if(enableBumpMapping == 1 && hasBumpMap == 1){
		vec3 fragBitangentViewSpace = cross(fragTangentViewSpace, fragVaryingNormalViewSpace);

		mat3 TBN = mat3(fragTangentViewSpace.x, fragTangentViewSpace.y, fragTangentViewSpace.z,
					fragBitangentViewSpace.x, fragBitangentViewSpace.y, fragBitangentViewSpace.z,
					fragVaryingNormalViewSpace.x, fragVaryingNormalViewSpace.y, fragVaryingNormalViewSpace.z);

		fragNormalViewSpace = TBN * normalize(vec3(texture(normalTex,fragTexCoord.xy)) * 2 - 1);
	} else
		fragNormalViewSpace = fragVaryingNormalViewSpace;

	switch(shaderMode){
		case 0:
		case 1:
			LightShader();
			break;
		case 4:
		case 5:
			emptyShader();
			break;
	
		default:
			emptyShader();
	}
}
