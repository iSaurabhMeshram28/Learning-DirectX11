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
vertex_output main(float4 pos : POSITION, float3 norm : NORMAL)
{
    vertex_output output;
    if (keyPressed == 1)
    {
        float4 iCoordinates = mul(viewMatrix, mul(worldMatrix, pos));
        output.transformedNormals = mul((float3x3)worldMatrix, norm);
        output.lightDirection = (float3)(lightPosition - iCoordinates);
        output.viewerVector = -iCoordinates.xyz;
    }
    else
    {
        output.transformedNormals = float3(0.0, 0.0, 0.0);
        output.lightDirection = float3(0.0, 0.0, 0.0);
        output.viewerVector = float3(0.0, 0.0, 0.0);
    }
    float4 position = mul(projectionMatrix, mul(viewMatrix, mul(worldMatrix, pos)));
    output.position = position;
    return output;
}
