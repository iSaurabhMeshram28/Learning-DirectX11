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
    float3 phongADSLight : COLOR;
};
vertex_output main(float4 pos : POSITION, float3 norm : NORMAL)
{
    vertex_output output;
    if (keyPressed == 1)
    {
        float4 iCoordinates = mul(viewMatrix, mul(worldMatrix, pos));
        float3 transformedNormals = normalize(mul((float3x3)worldMatrix, norm));
        float3 lightDirection = normalize((float3)(lightPosition - iCoordinates));
        float3 reflectionVector = reflect(-lightDirection, transformedNormals);
        float3 viewerVector = normalize(-iCoordinates.xyz);
        float3 ambientLight = lightAmbient * materialAmbient;
        float3 diffuseLight = lightDiffuse * materialDiffuse * max(dot(lightDirection, transformedNormals), 0.0);
        float3 specularLight = lightSpecular * materialSpecular * pow(max(dot(reflectionVector, viewerVector), 0.0), materialShininess);
        output.phongADSLight = ambientLight + diffuseLight + specularLight;
    }
    else
    {
        output.phongADSLight = float3(1.0, 1.0, 1.0);
    }
    float4 position = mul(projectionMatrix, mul(viewMatrix, mul(worldMatrix, pos)));
    output.position = position;
    return output;
}