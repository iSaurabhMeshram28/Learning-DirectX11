struct vertex_output
{
    float4 position : SV_POSITION;
    float3 diffuseLight : COLOR;
};
float4 main(vertex_output input) : SV_TARGET
{
    float4 color = float4(input.diffuseLight, 1.0f);
    return color;
}