#version 130

uniform vec3 fragNormal;
uniform vec3 fragPosition;

uniform vec3 lightPosition1;
uniform vec3 lightPosition2;
uniform vec3 cameraPosition;
uniform bool enableLight1;
uniform bool enableLight2;

out vec4 fragColor;

void main() 
{
    vec3 normal = normalize(fragNormal);
    vec3 viewDirection = normalize(cameraPosition - fragPosition);

    vec3 result = vec3(0.0);

    if (enableLight1) 
    {
        vec3 lightDirection1 = normalize(lightPosition1 - fragPosition);
        float diff1 = max(dot(normal, lightDirection1), 0.0);
        vec3 diffuse1 = vec3(0.8, 0.8, 0.8) * diff1;
        result += diffuse1;
    }

    if (enableLight2) 
    {
        vec3 lightDirection2 = normalize(lightPosition2 - fragPosition);
        float diff2 = max(dot(normal, lightDirection2), 0.0);
        vec3 diffuse2 = vec3(0.8, 0.8, 0.8) * diff2;
        result += diffuse2;
    }

    vec3 ambient = vec3(0.2, 0.2, 0.2);

    fragColor = vec4(result + ambient, 1.0);
}