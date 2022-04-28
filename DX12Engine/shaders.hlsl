//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************




cbuffer SceneConstantBuffer : register(b0)
{
    //float4x4 world;
    float4x4 wvp;
    float4x4 finalBonesMatrices[100];
};

struct VS_INPUT
{
    float3 Position	: POSITION;		// vertex position 
    float2 UV		: TEXCOORD0;	// vertex texture coords 
    float3 Normal	: NORMAL;
    float4 BoneIds	: BONEIDS;
    float4 BoneWeights	: BONEWEIGHTS;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 normal : NORMAL;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

PSInput VSMain(VS_INPUT VertexShaderInput)
{
    PSInput result;

    float4x4 BoneTransform = finalBonesMatrices[VertexShaderInput.BoneIds[0]] * VertexShaderInput.BoneWeights[0];
    BoneTransform += finalBonesMatrices[VertexShaderInput.BoneIds[1]] * VertexShaderInput.BoneWeights[1];
    BoneTransform += finalBonesMatrices[VertexShaderInput.BoneIds[2]] * VertexShaderInput.BoneWeights[2];
    BoneTransform += finalBonesMatrices[VertexShaderInput.BoneIds[3]] * VertexShaderInput.BoneWeights[3];

    float4 posL = mul(float4(VertexShaderInput.Position.xyz, 1.0), BoneTransform);
    result.position = mul(float4(posL.xyz, 1.0), wvp);
    result.uv = VertexShaderInput.UV.xy;
    result.normal = float4(VertexShaderInput.Normal.xyz, 1.0);
    return result;

/*/
    float4 totalPosition = float4(0.0, 0.0, 0.0, 0.0);
    for (int i = 0; i < 4; i++)
    {
        if (VertexShaderInput.BoneIds[i] == -1)
            continue;
        if (VertexShaderInput.BoneIds[i] >= 100)
        {
            totalPosition = float4(VertexShaderInput.Position, 1.0f);
            break;
        }
        float4 localPosition = mul(float4(VertexShaderInput.Position.xyz, 1.0), finalBonesMatrices[VertexShaderInput.BoneIds[i]]);
        totalPosition += localPosition * VertexShaderInput.BoneWeights[i];
        //float3 localNormal = float3x3(finalBonesMatrices[VertexShaderInput.BoneIds[i]]) * VertexShaderInput.Normal;
    }
    result.position = mul(float4(totalPosition.xyz, 1.0), wvp);
    result.uv = VertexShaderInput.UV.xy;
    result.normal = float4(VertexShaderInput.Normal.xyz, 1.0);
    return result;*/
    /*
    result.position = mul(float4(VertexShaderInput.Position.xyz, 1.0), wvp);
    result.uv = VertexShaderInput.UV.xy;
    result.normal = float4(VertexShaderInput.Normal.xyz, 1.0);
    return result;*/
}

/*
PSInput VSMain(VS_INPUT VertexShaderInput)
{
    PSInput result;

    float4x4 skinTransform = (float4x4)0;
    skinTransform += finalBonesMatrices[VertexShaderInput.BoneIds.x] * VertexShaderInput.BoneWeights.x;
    skinTransform += finalBonesMatrices[VertexShaderInput.BoneIds.y] * VertexShaderInput.BoneWeights.y;
    skinTransform += finalBonesMatrices[VertexShaderInput.BoneIds.z] * VertexShaderInput.BoneWeights.z;
    skinTransform += finalBonesMatrices[VertexShaderInput.BoneIds.w] * VertexShaderInput.BoneWeights.w;

    float4 position = mul(VertexShaderInput.Position, skinTransform);
    result.position = mul(position, wvp);
    result.uv = VertexShaderInput.UV;

    float4 normal = mul(float4(VertexShaderInput.Normal, 0), skinTransform);
   // result.normal = normalize(mul(normal, world).xyz);

    return result;
}*/
/*
PSInput VSMain(VS_INPUT VertexShaderInput)
{
    PSInput result;

    result.position = mul(float4(VertexShaderInput.Position.xyz, 1.0), wvp);
    result.uv = VertexShaderInput.UV.xy;
    result.normal = float4(VertexShaderInput.Normal.xyz, 1.0);
    return result;
}*/

float4 PSMain(PSInput input) : SV_TARGET
{
	float4 AmbientColor = float4(1, 1, 1, 1);
	float AmbientIntensity = 0.1;
    return AmbientColor * AmbientIntensity;// *g_texture.Sample(g_sampler, input.uv);
}
