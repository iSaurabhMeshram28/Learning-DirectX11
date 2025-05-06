cbuffer ConstantBuffer
{
    float4x4 worldViewProjectionMatrix;
}
float4 main(float4 pos : POSITION) : SV_POSITION
{
    float4 position = mul(worldViewProjectionMatrix, pos);
    return position;
}