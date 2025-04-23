struct vertex_output
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};
Texture2D myTexture2D;
SamplerState mySamplerState;
float4 main(vertex_output input) : SV_TARGET
{
    float4 color = myTexture2D.Sample(mySamplerState, input.texcoord);
    return color;
}