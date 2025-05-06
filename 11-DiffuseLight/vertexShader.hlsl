cbuffer ConstantBuffer
{
    float4x4 worldViewMatrix;
    float4x4 projectionMatrix;
    float3 ld;
    float3 kd;
    float4 lightPosition;
    uint keyPressed;
}
struct vertex_output
{
    float4 position : SV_POSITION;
    float3 diffuseLight : COLOR;
};
vertex_output main(float4 pos : POSITION, float3 norm : NORMAL)
{
    vertex_output output;
    if (keyPressed == 1)
    {
        float4 iPosition = mul(worldViewMatrix, pos);
        float3x3 normalMatrix = (float3x3)(worldViewMatrix);
        float3 n = normalize(mul(normalMatrix, norm));
        float3 s = normalize((float3)(lightPosition - iPosition));
        output.diffuseLight = ld * kd * max(dot(s, n), 0.0);
    }
    else
    {
        output.diffuseLight = float3(1.0f, 1.0f, 1.0f);
    }
    float4 position = mul(projectionMatrix, mul(worldViewMatrix, pos));
    output.position = position;
    return output;
}