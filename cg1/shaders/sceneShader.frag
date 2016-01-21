#version 330
#extension GL_EXT_texture_array : enable

#define MAX_LIGHTS 10
/////////////////////////////////////////////////////////////////////////////
// Uniforms
/////////////////////////////////////////////////////////////////////////////
uniform sampler2D tex;	// Diffuse texture

/////////////////////////////////////////////////////////////////////////////
// Light
// Source: http://www.tomdalling.com/blog/modern-opengl/08-even-more-lighting-directional-lights-spotlights-multiple-lights/
/////////////////////////////////////////////////////////////////////////////
// array of lights
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
uniform vec3 camPos;

/////////////////////////////////////////////////////////////////////////////
// Shadow Mapping
/////////////////////////////////////////////////////////////////////////////
uniform int enableShadowMapping;	// 0: disabled, 1:enabled
uniform sampler2DArray shadowTexArray;
in vec3 fragVertShadowClip[MAX_LIGHTS];

/////////////////////////////////////////////////////////////////////////////
// Varyings
/////////////////////////////////////////////////////////////////////////////
in vec3 fragNormal;
in vec2 fragTexCoord;
in vec3 fragVertWorld;
uniform int shaderMode;

/////////////////////////////////////////////////////////////////////////////
// Output
/////////////////////////////////////////////////////////////////////////////
out vec4 outputColor;

bool isShadowed(int lightNr, vec3 surfaceToLight)
{ 
	float cosTheta = clamp(dot(fragNormal, surfaceToLight),0,1);
	float bias = 0.005*cosTheta;
	bias = clamp(bias, 0.0,0.01);
	return texture2DArray( shadowTexArray, vec3(fragVertShadowClip[lightNr].xy, lightNr) ).x  <  fragVertShadowClip[lightNr].z - bias;
}

// Applies the specified light
vec3 ApplyLight(int lightNr, vec3 surfaceColor, vec3 normal, vec3 surfacePos, vec3 surfaceToCamera) {
	Light light = allLights[lightNr];
	
    vec3 surfaceToLight;
    float attenuation = 1.0;
    if(light.position.w == 0.0) {
        //directional light
        surfaceToLight = normalize(light.position.xyz);
        attenuation = 1.0; //no attenuation for directional lights
    } else {
        //point light
        surfaceToLight = normalize(light.position.xyz - surfacePos);
        float distanceToLight = length(light.position.xyz - surfacePos);
        attenuation = min(1,1/(light.att_c1+distanceToLight*light.att_c2+distanceToLight*distanceToLight*light.att_c3));

        //cone restrictions (affects attenuation)
        float lightToSurfaceAngle = degrees(acos(dot(-surfaceToLight, normalize(light.coneDirection))));
        if(lightToSurfaceAngle > light.coneAngle){
            attenuation = 0.0;
        }
    }

    //ambient
    vec3 ambient = light.ambientCoefficient * surfaceColor.rgb * light.intensities;

    //diffuse
    float diffuseCoefficient = max(0.0, dot(normal, surfaceToLight));
    vec3 diffuse = diffuseCoefficient * surfaceColor.rgb * light.intensities;
    
    //specular
    float specularCoefficient = 1;
    if(material.shininess > 0)
    	specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-surfaceToLight, normal))), material.shininess);
    vec3 specular = specularCoefficient * surfaceColor * light.intensities;

	// Shadow Mapping
	float visibility = 1.0;
	
	if (enableShadowMapping == 1 && isShadowed(lightNr, surfaceToLight)){
		visibility = 0.1;
		
		// Error output for unshadowed regions
 		if(fragVertShadowClip[lightNr].y < 0 ||fragVertShadowClip[lightNr].y > 1 || 
 		   fragVertShadowClip[lightNr].x < 0 ||fragVertShadowClip[lightNr].x > 1 || 
 		   fragVertShadowClip[lightNr].z < 0 ||fragVertShadowClip[lightNr].z > 1)
			return vec3(1,0,0);
 	}
		
    // inear color (color before gamma correction)
    return ambient + visibility*attenuation*(diffuse + specular);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Phong lighting model + shadow mapping
////////////////////////////////////////////////////////////////////////////////////////////////////
void LightShader(){
	vec3 surfaceColor = vec3(texture(tex,fragTexCoord.xy));
	vec3 surfaceToCamera = normalize(vec3(camPos - fragVertWorld));
	
	vec3 linearColor = vec3(0);
	for(int i = 0; i < numLights; ++i){
	    linearColor += ApplyLight(i, surfaceColor, fragNormal, vec3(fragVertWorld), surfaceToCamera);
	}
	outputColor = vec4(linearColor,1.0f);
}


void emptyShader(){
	outputColor = texture(tex,fragTexCoord.xy);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////////////////////////////////
void main()
{
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
