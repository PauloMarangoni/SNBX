struct VSInput
{
    float3 position  : POSITION0;  
};

struct VSOutput
{
	float4 position : SV_POSITION;
    float3 fragPos  : POSITION0;
};

struct SceneBuffer
{
	float4x4 viewProjection;
};

struct PushConstants
{
    float4x4 model;
}

ConstantBuffer<SceneBuffer> scene : register(b0)


[[vk::push_constant]] PushConstants constants;


VSOutput MainVS(VSInput input)
{
	VSOutput output = (VSOutput)0;
	output.fragPos  = (float3)mul(constants.model, float4(input.position, 1.0));
	output.position = mul(scene.viewProjection, float4(output.fragPos, 1.0));	
	return output;
}

float4 MainPS() : SV_TARGET
{
	return 1.0;
}
