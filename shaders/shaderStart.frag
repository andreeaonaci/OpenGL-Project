#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;
in vec3 fPos;

out vec4 fColor;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform	mat3 normalMatrix;
uniform mat3 lightDirMatrix;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

uniform int pointLight;
uniform vec3 pointLightPos;
uniform float pointLightStrength;

uniform vec3 lightPointPos;
uniform vec3 lightPointPos2;
uniform vec3 lightPos2;

uniform int rain;
uniform float iTime;

float constant = 1.0f;
float linear = 0.0045f;
float quadratic = 0.0075;

float ambientPoint = 0.5f;
float specularStrengthPoint = 0.5f;
float shininessPoint = 64.0f;

uniform mat4 view;

vec3 ambient;
float ambientStrength = 0.9f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.9f;
float shininess = 64.0f;

uniform int spotLight;
uniform int fragmentDiscarding;

float spotQuadratic = 0.05f;
float spotLinear = 0.15f;
float spotConstant = 5.0f;

vec3 spotLightAmbient = vec3(0.1f, 0.1f, 0.1f);
vec3 spotLightDiffuse = vec3(0.8f, 0.8f, 0.8f);
vec3 spotLightSpecular = vec3(1.0f, 1.0f, 1.0f);
vec3 spotLightColor = vec3(20,20,20);

uniform float spotLightCutoff;
uniform float spotLightInnerCutoff;

uniform vec3 spotLightDirection;
uniform vec3 spotLightPosition;

uniform int fogInitial;
uniform float fogDensity;

uniform int transparency;

vec3 computeLightComponents()
{ 
    vec3 cameraPosEye = vec3(0.0f);
	vec3 normalEye = normalize(fNormal);	
	vec3 lightDirN = normalize(lightDir);
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	ambient = ambientStrength * lightColor;
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
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
	return (ambient + diffuse + specular) * att;/// * vec3(2.0f,2.0f,2.0f);
}

float computeShadow()
{
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	normalizedCoords = normalizedCoords * 0.5 + 0.5;

	if (normalizedCoords.z > 1.0f)
		return 0.0f;
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
	float currentDepth = normalizedCoords.z;
    float bias2 = max(0.05f * (1.0f - dot(normalize(fNormal), lightDir)), 0.005f);
	float shadow2 = currentDepth - bias2 > closestDepth ? 1.0f : 0.0f;
	return shadow2;
}

vec4 spotLightComputation(vec4 spotLightPosEye) {
    vec3 cameraPosEye = vec3(0.0f);
    vec3 ambient = spotLightAmbient * lightColor;
    
    // Diffuse
    vec3 norm = normalize(fNormal);
    vec3 lightDirN = normalize(spotLightPosEye.xyz - fPosEye.xyz);
    float diff = max(dot(norm, lightDirN), 0.0);
    vec3 diffuse = spotLightDiffuse * diff * lightColor;
    
    // Specular
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
    vec3 reflectDir = reflect(-lightDirN, norm);
    float spec = pow(max(dot(reflectDir, viewDirN), 0.0), shininess);
    vec3 specular = spotLightSpecular * spec * lightColor;
    
    // Spotlight (soft edges)
    float theta = dot(lightDirN, normalize(-spotLightDirection));
    float epsilon = (spotLightCutoff - spotLightInnerCutoff);
    float intensity = clamp((theta - spotLightInnerCutoff) / epsilon, 0.0, 1.0);
    diffuse  *= intensity;
    specular *= intensity;
    
    // Attenuation
    float distance    = length(spotLightPosEye.xyz - fPosEye.xyz);
    float attenuation = 1.0f / (spotConstant + spotLinear * distance + spotQuadratic * (distance * distance));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    
    return vec4(ambient + diffuse + specular, 1.0f);
}

float computeFog()
{
    float fragmentDistance = length(fPosEye);
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

    return clamp(fogFactor, 0.0f, 1.0f);
}

void main() 
{
    vec3 light = computeLightComponents();

    vec3 baseColor = vec3(0.9f, 0.35f, 0.0f);
    vec4 diffuseColor = texture(diffuseTexture, fTexCoords);
    float shadow = computeShadow();

    ambient *= vec3(texture(diffuseTexture, fTexCoords)) * 1.2f;
    diffuse *= vec3(texture(diffuseTexture, fTexCoords));
    specular *= vec3(texture(specularTexture, fTexCoords));

    if (diffuseColor.x < 0.005 && fragmentDiscarding == 1)
    {
        discard;
    }
    else {
        if (pointLight == 1) {
            vec4 lightPosEye1 = view * vec4(lightPointPos, 1.0f);
            vec3 pointLightValue1 = computePointLight(lightPosEye1);
            vec3 ambient1 = pointLightValue1 * ambientStrength * diffuseColor.rgb;
            vec3 diffuse1 = pointLightValue1 * max(dot(normalize(normalMatrix * fNormal), normalize(lightPosEye1.xyz - fPosEye.xyz)), 0.0) * diffuseColor.rgb;
            vec3 specular1 = pointLightValue1 * specularStrength * pow(max(dot(normalize(normalMatrix * fNormal), reflect(-normalize(lightPosEye1.xyz - fPosEye.xyz), normalize(normalMatrix * fNormal))), 0.0), shininess) * texture(specularTexture, fTexCoords).rgb;

            vec4 lightPosEye2 = view * vec4(lightPointPos2, 1.0f);
            vec3 pointLightValue2 = computePointLight(lightPosEye2);
            vec3 ambient2 = pointLightValue2 * ambientStrength * diffuseColor.rgb;
            vec3 diffuse2 = pointLightValue2 * max(dot(normalize(normalMatrix * fNormal), normalize(lightPosEye2.xyz - fPosEye.xyz)), 0.0) * diffuseColor.rgb;
            vec3 specular2 = pointLightValue2 * specularStrength * pow(max(dot(normalize(normalMatrix * fNormal), reflect(-normalize(lightPosEye2.xyz - fPosEye.xyz), normalize(normalMatrix * fNormal))), 0.0), shininess) * texture(specularTexture, fTexCoords).rgb;

            ambient = ambient1 + ambient2;
            diffuse = diffuse1 + diffuse2;
            specular = specular1 + specular2;
        }


        vec3 color = min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular, 1.0f);
        vec3 colorF = min(color, (ambient + diffuse) + specular);

        float fogFactor = computeFog();
        vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);

        if (fogInitial == 0) {
            fColor = min(vec4(colorF, 1.0f) * vec4(light, 1.0f), 1.0f);
        } 
        else {
            fColor = mix(fogColor, min(vec4(colorF, 1.0f) * vec4(light, 1.0f), 1.0f), fogFactor);
        }

        if (spotLight == 1) {
            vec4 spotLightPosEye = view * vec4(spotLightPosition, 1.0f);
            vec4 spotlightEffect = 10 * spotLightComputation(spotLightPosEye);
            ambient *= spotlightEffect.xyz * ambientStrength * lightColor;
            vec3 lightDirN = normalize(spotLightPosEye.xyz - fPosEye.xyz);
            vec3 normalEyeN = normalize(normalMatrix * fNormal);
            diffuse *= spotlightEffect.xyz * max(dot(normalEyeN, lightDirN), 0.0) * lightColor;
            vec3 viewDirN = normalize(vec3(0.0f) - fPosEye.xyz);
            vec3 reflectDir = reflect(-lightDirN, normalize(fNormal));
            float spec = pow(max(dot(reflectDir, viewDirN), 0.0), shininess);
            specular *= spotlightEffect.xyz * specularStrength * spec * lightColor;

            fColor = min(vec4(min(color, (ambient + diffuse) + specular), 1.0f) * vec4(light, 1.0f), 1.0f) + spotlightEffect;
        }
        if (rain == 1) {
            vec4 rainColor = vec4(0.0, 0.0, 1.0, 0.0);
            vec2 uv = fColor.xy / fTexCoords.xy;
            vec2 u = fColor.xy / fTexCoords.xy, n = texture(specularTexture, u * 0.1).rg;
            
            rainColor = textureLod(diffuseTexture, u, 2.5);
            
            for (float r = 4.0; r > 0.0; r--) {
                vec2 x = fTexCoords.xy * r * 0.015, p = 6.28 * u * x + (n - 0.5) * 2.0, s = sin(p);
                vec4 d = texture(specularTexture, round(u * x - 0.25) / x);
                float t = (s.x + s.y) * max(0.0, 1.0 - fract(iTime * (d.b + 0.1) + d.g) * 2.0);
                
                if (d.r < (5.0 - r) * 0.08 && t > 0.5) {
                    rainColor = vec4(0.0, 0.0, 1.0, 1.0);
                    discard;
                }
            }
            fColor = fColor + rainColor;
        }
        else {
            vec4 rainColor = vec4(0.0, 0.0, 0.0, 0.0);
            fColor = fColor + rainColor;
        }
    }
}