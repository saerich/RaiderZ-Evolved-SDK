
// transformations
float4x4 WorldViewProj : 	WORLDVIEWPROJ;

struct VS_OUTPUT
{
    float4 Pos  : POSITION;
    float4 col : COLOR0;
       
};

VS_OUTPUT VS(
    float3 Pos  : POSITION 
)
{
    VS_OUTPUT Out = (VS_OUTPUT)0;


    Out.Pos  = mul(float4(Pos,1),WorldViewProj);    // position (projected)
    Out.col = float4(1.0f,0.0f,0.0f,1.0f);		//bright red	

    return Out;
}


technique Error
{
    pass P0
    {
        // shaders
        CullMode = CW;
        FillMode = WIREFRAME;
        VertexShader = compile vs_1_1 VS();
        
    }  
}