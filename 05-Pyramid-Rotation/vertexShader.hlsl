cbuffer ConstantBuffer
{
    float4x4 worldViewProjectionMatrix;
}
struct vertex_output
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};
vertex_output main(float4 pos : POSITION, float4 col : COLOR)
{
    vertex_output output;
    output.position = mul(worldViewProjectionMatrix, pos);
    output.color = col;
    return output;
}