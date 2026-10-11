cbuffer Rect : register(b0) { float4 region; float4 edge; }
Texture2D<float4> sourceImage : register(t0);
SamplerState sourceSampler : register(s0);
struct VOut { float4 position:SV_Position; float2 uv:TEXCOORD0; };
VOut VS(uint id:SV_VertexID) { VOut o; o.uv=float2((id<<1)&2,id&2);o.position=float4(o.uv*float2(2,-2)+float2(-1,1),0,1);return o; }
float4 PS(VOut i):SV_Target { return sourceImage.SampleLevel(sourceSampler,clamp(region.xy+i.uv*region.zw,edge.xy,edge.zw),0); }
