#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;
in vec3 fPos;

out vec4 fColor;

// Lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform	mat3 normalMatrix;
uniform mat3 lightDirMatrix;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

//Point light

uniform int pointLight;
uniform vec3 pointLightPos;
uniform float pointLightStrength;

uniform vec3 lightPos1;
uniform vec3 lightPos2;

float constant = 1.0f;
float linear = 0.00225f;
float quadratic = 0.00375;

float ambientPoint = 0.5f;
float specularStrengthPoint = 0.5f;
float shininessPoint = 32.0f;

uniform mat4 view;

vec3 ambient;
float ambientStrength = 0.9f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.9f;
float shininess = 32.0f;

uniform int spotLight;

float spotQuadratic = 0.05f;
float spotLinear = 0.15f;
float spotConstant = 5.0f;

vec3 spotLightAmbient = vec3(0.0f, 0.0f, 0.0f);
vec3 spotLightSpecular = vec3(10.0f, 10.0f, 10.0f);
vec3 spotLightColor = vec3(50,50,50);

uniform float spotLightCutoff;
uniform float spotLightInnerCutoff;

uniform vec3 spotLightDirection;
uniform vec3 spotLightPosition;

uniform int fogInitial;
uniform float fogDensity;

vec3 computeLightComponents()
{       
    vec3 cameraPosEye = vec3(0.0f);
    vec3 normalEye = normalize(fNormal);      
    vec3 lightDirN = normalize(lightDir);
    vec3 pointLightDir = normalize(pointLightPos - fPosEye.xyz);
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
    ambient = ambientStrength * lightColor;
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
    vec3 reflection = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
    specular = specularStrength * specCoeff * lightColor;
    float pointLightDiffuse = max(dot(normalEye, pointLightDir), 0.0f);
    diffuse += pointLightStrength * pointLightDiffuse * lightColor;
    return (ambient + diffuse + specular);
}

vec3 computePointLight(vec4 lightPosEye)
{
	vec3 cameraPosEye = vec3(0.0f);
	vec3 normalEye = normalize(normalMatrix * fNormal);
	vec3 lightDirN = normalize(lightPosEye.xyz - fPosEye.xyz);
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	vec3 ambient = ambientPoint * lightColor;
	vec3 diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	vec3 halfVector = normalize(lightDirN + viewDirN);
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininessPoint);
	vec3 specular = specularStrengthPoint * specCoeff * lightColor;
	float distance = length(lightPosEye.xyz - fPosEye.xyz);
	float att = 1.0f / (constant + linear * distance + quadratic * distance * distance);
	return (ambient + diffuse + specular) * att * vec3(2.0f,2.0f,2.0f);
}

float computeShadow()
{
	// perform perspective divide
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	// Transform to [0,1] range
	normalizedCoords = normalizedCoords * 0.5 + 0.5;

	if (normalizedCoords.z > 1.0f)
		return 0.0f;


	// Get closest depth value from light's perspective
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
	
	// Get depth of current fragment from light's perspective
	float currentDepth = normalizedCoords.z;

	// Check whether current frag pos is in shadow
	//float shadow = currentDepth > closestDepth ? 1.0 : 0.0;

	// Check whether current frag pos is in shadow
	//float bias1 = 0.005f;
	//float shadow1 = currentDepth - bias1 > closestDepth ? 1.0f : 0.0f;

	// Check whether current frag pos is in shadow
	float bias2 = max(0.05f * (1.0f - dot(normalize(fNormal), lightDir)), 0.005f);
	float shadow2 = currentDepth - bias2 > closestDepth ? 1.0f : 0.0f;

	return shadow2;

}

vec3 spotLightComputation(vec3 ambient, vec3 diffuse, vec3 specular)
{
    vec3 cameraPosEye = vec3(0.0f);
    ambient = spotLightAmbient * vec3(texture(diffuseTexture, fTexCoords));

    vec3 norm = normalize(fNormal);
    vec3 lightDir = normalize(spotLightPosition - fPos);
    float diff = max(dot(fNormal, lightDir), 0.0);
    diffuse = spotLightColor * diff * vec3(texture(diffuseTexture, fTexCoords));
    
    vec3 viewDir = normalize(cameraPosEye - fPos.xyz);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    specular = spotLightSpecular * spec * vec3(texture(specularTexture, fTexCoords));
    
    float theta = dot(lightDir, normalize(-spotLightDirection));
    float epsilon = (spotLightCutoff - spotLightInnerCutoff);
    float intensity = clamp((theta - spotLightInnerCutoff) / epsilon, 0.0, 1.0);
    diffuse  *= intensity;
    specular *= intensity;
    
    // Attenuation
    float distance    = length(spotLightPosition - fPos);
    float attenuation = 1.0f / (spotConstant + spotLinear * distance + spotQuadratic * (distance * distance));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

float computeFog()
{
    float fragmentDistance = length(fPosEye);
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

    return clamp(fogFactor, 0.0f, 1.0f);
}

// spot light
// vec3 spotLightComputation() {
// 	vec3 cameraPosEye = vec3(0.0f);
// 	vec3 lightDir = normalize(spotLightPosition - fPos);
// 	vec3 normalEye = normalize(normalMatrix * fNormal);
// 	vec3 lightDirN = normalize(lightDirMatrix * lightDir);
// 	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
// 	vec3 halfVector = normalize(lightDirN + viewDirN);

// 	float diff = max(dot(fNormal, lightDir), 0.0f);
// 	float spec = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
// 	float distance = length(spotLightPosition - fPos);
// 	float attenuation = 1.0f / (spotConstant + spotLinear * distance + spotQuadratic * distance * distance);

// 	float theta = dot(lightDir, normalize(-spotLightDirection));
// 	float epsilon = spotLightCutoff - spotLightInnerCutoff;
// 	float intensity = clamp((theta - spotLightInnerCutoff)/epsilon, 0.0, 1.0);

// 	vec3 ambient = spotLightColor * spotLightAmbient * vec3(texture(diffuseTexture, fTexCoords));
// 	vec3 diffuse = spotLightColor * spotLightSpecular * diff * vec3(texture(diffuseTexture, fTexCoords));
// 	vec3 specular = spotLightColor * spotLightSpecular * spec * vec3(texture(specularTexture, fTexCoords));
// 	ambient *= attenuation * intensity;
// 	diffuse *= attenuation * intensity;
// 	specular *= attenuation * intensity;
	
// 	return ambient + diffuse + specular;
// }

void main() 
{
    vec3 light = computeLightComponents();

    vec3 baseColor = vec3(0.9f, 0.35f, 0.0f);
    vec4 diffuseColor = texture(diffuseTexture, fTexCoords);
	float shadow = computeShadow();
    // ambient = light * ambientStrength * diffuseColor.rgb;
    // diffuse = light * max(dot(normalize(normalMatrix * fNormal), normalize(lightDir)), 0.0) * diffuseColor.rgb;
    // specular = light * specularStrength * pow(max(dot(normalize(normalMatrix * fNormal), reflect(-normalize(lightDir), normalize(normalMatrix * fNormal))), 0.0), shininess) * texture(specularTexture, fTexCoords).rgb;
    // ambient *= light * vec3(texture(diffuseTexture, fTexCoords)) * 1.2f;
	// diffuse *= light * vec3(texture(diffuseTexture, fTexCoords)) ;//* 1.2f;
	// // modulate with specular map
	// specular *= light * vec3(texture(specularTexture, fTexCoords)) ;//* 1.2f;

    if (pointLight == 1) {
        vec4 lightPosEye1 = view * vec4(lightPos1, 1.0f);
        vec3 pointLightValue = computePointLight(lightPosEye1);

        ambient *= pointLightValue * ambientStrength * diffuseColor.rgb;
        diffuse *= pointLightValue * max(dot(normalize(normalMatrix * fNormal), normalize(lightPosEye1.xyz - fPosEye.xyz)), 0.0) * diffuseColor.rgb;
        specular *= pointLightValue * specularStrength * pow(max(dot(normalize(normalMatrix * fNormal), reflect(-normalize(lightPosEye1.xyz - fPosEye.xyz), normalize(normalMatrix * fNormal))), 0.0), shininess) * texture(specularTexture, fTexCoords).rgb;
    }

    vec3 spotlightaux = vec3(0.0);
    if (spotLight == 1) {
        vec4 lightPosEye1 = view * vec4(spotLightPosition, 1.0f);
	    spotlightaux += spotLightComputation(ambient, diffuse, specular);
        ambient *= spotlightaux * ambientStrength * diffuseColor.rgb;
        diffuse *= spotlightaux * max(dot(normalize(normalMatrix * fNormal), normalize(lightPosEye1.xyz - fPosEye.xyz)), 0.0) * diffuseColor.rgb;
        specular *= spotlightaux * specularStrength * pow(max(dot(normalize(normalMatrix * fNormal), reflect(-normalize(lightPosEye1.xyz - fPosEye.xyz), normalize(normalMatrix * fNormal))), 0.0), shininess) * texture(specularTexture, fTexCoords).rgb;
    }
    vec3 color = min((ambient + diffuse * (1 - shadow)) * texture(diffuseTexture, fTexCoords).rgb + specular * (1 - shadow) * texture(specularTexture, fTexCoords).rgb, 1.0f);

    vec3 colorF = min(color, (ambient + diffuse)+specular);
    float fogFactor = computeFog();
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    if ( fogInitial == 0)
	{

		fColor = min(vec4(colorF, 1.0f) * vec4(light, 1.0f), 1.0f);
	}
	else
	{
 
		fColor = mix(fogColor, min(vec4(colorF, 1.0f) * vec4(light, 1.0f), 1.0f), fogFactor);
	}
}