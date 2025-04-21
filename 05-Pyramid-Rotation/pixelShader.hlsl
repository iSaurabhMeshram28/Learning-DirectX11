struct vertex_output
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};
float4 main(vertex_output input) : SV_TARGET
{
    float4 color = input.color;
    return color;
}