cbuffer ConstantBuffer
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4 lightAmbient;
    float4 lightDiffuse;
    float4 lightSpecular;
    float4 lightPosition;
    float4 materialAmbient;
    float4 materialDiffuse;
    float4 materialSpecular;
    float materialShininess;
    uint keyPressed;
}
struct vertex_output
{
    float4 position : SV_POSITION;
    float3 transformedNormals : NORMAL0;
    float3 lightDirection : NORMAL1;
    float3 viewerVector : NORMAL2;
};
float4 main(vertex_output input) : SV_TARGET
{
    float3 phongADSLight;
    if (keyPressed == 1)
    {
        float3 normalisedTransformedNormal = normalize(input.transformedNormals);
        float3 normalisedLightDirection = normalize(input.lightDirection);
        float3 normalisedViewerVector = normalize(input.viewerVector);
        float3 reflectionVector = reflect(-normalisedLightDirection, normalisedTransformedNormal);
        float3 ambientLight = lightAmbient * materialAmbient;
        float3 diffuseLight = lightDiffuse * materialDiffuse * max(dot(normalisedLightDirection, normalisedTransformedNormal), 0.0);
        float3 specularLight = lightSpecular * materialSpecular * pow(max(dot(reflectionVector, normalisedViewerVector), 0.0), materialShininess);
        phongADSLight = ambientLight + diffuseLight + specularLight;
    }
    else
    {
        phongADSLight = float3(1.0, 1.0, 1.0);
    }
    float4 color = float4(phongADSLight, 1.0f);
    return color;
}
