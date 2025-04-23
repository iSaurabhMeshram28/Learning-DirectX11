cbuffer ConstantBuffer
{
    float4x4 worldViewProjectionMatrix;
}
struct vertex_output
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};
vertex_output main(float4 pos : POSITION, float2 tex : TEXCOORD)
{
    vertex_output output;
    output.position = mul(worldViewProjectionMatrix, pos);
    output.texcoord = tex;
    return output;
}