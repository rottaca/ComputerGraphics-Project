#version 330

/////////////////////////////////////////////////////////////////////////////
// Uniforms
/////////////////////////////////////////////////////////////////////////////
uniform sampler2D tex;
uniform sampler2D depthTex;

/////////////////////////////////////////////////////////////////////////////
// Light
// Source: http://www.tomdalling.com/blog/modern-opengl/08-even-more-lighting-directional-lights-spotlights-multiple-lights/
/////////////////////////////////////////////////////////////////////////////
// array of lights
#define MAX_LIGHTS 10
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
// Varyings
/////////////////////////////////////////////////////////////////////////////
in vec3 fragNormal;
in vec2 fragTexCoord;
in vec3 fragVertWorld;
in vec3 fragVertShadowClip;
flat in int shaderMode_;

/////////////////////////////////////////////////////////////////////////////
// Output
/////////////////////////////////////////////////////////////////////////////
out vec4 outputColor;

// Applies the specified light
vec3 ApplyLight(Light light, vec3 surfaceColor, vec3 normal, vec3 surfacePos, vec3 surfaceToCamera) {
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

    //linear color (color before gamma correction)
    return ambient + attenuation*(diffuse + specular);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Phong lighting model
////////////////////////////////////////////////////////////////////////////////////////////////////
void LightShader(){
	vec3 surfaceColor = vec3(texture(tex,fragTexCoord.xy));
	vec3 surfaceToCamera = normalize(vec3(camPos - fragVertWorld));
	
	vec3 linearColor = vec3(0);
	for(int i = 0; i < numLights; ++i){
	    linearColor += ApplyLight(allLights[i], surfaceColor, fragNormal, vec3(fragVertWorld), surfaceToCamera);
	}
	outputColor = vec4(linearColor,1.0f);
}

bool isShadowed(vec3 surfaceToLight)
{ 
	float cosTheta = clamp(dot(fragNormal, surfaceToLight),0,1);
	float bias = 0.005*cosTheta;
	bias = clamp(bias, 0,0.01);
	return texture( depthTex, fragVertShadowClip.xy ).x  <  fragVertShadowClip.z -bias;
}
void emptyShader(){

	float visibility = 1.0;
	
	if ( isShadowed(normalize(allLights[0].position.xyz))){
		visibility = 0.5;
 	}
 	
	outputColor = visibility*texture(tex,fragTexCoord.xy);
	
	// Invalid coordinates, no shadow data here
	if(fragVertShadowClip.y < 0 ||fragVertShadowClip.y > 1 || fragVertShadowClip.x < 0 ||fragVertShadowClip.x > 1)
		outputColor = vec4(1,0,0,1);

}

void depthDisplayShader(){
	float z = texture(depthTex,fragTexCoord.xy).x;
		
	float n = 1.0;                                // the near plane
	float f = 100.0;                               // the far plane
	float c = (2.0 * n) / (f + n - z * (f - n));  // convert to linear values
	
	outputColor = vec4(c,c,c,1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////////////////////////////////
void main()
{
	switch(shaderMode_){
		case 0:
		case 1:
			emptyShader();
			break;
		case 4:
		case 5:
			emptyShader();
			break;
	
		default:
			emptyShader();
	}
}
