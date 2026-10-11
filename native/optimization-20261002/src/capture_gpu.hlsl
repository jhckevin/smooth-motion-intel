cbuffer Sizes : register(b0) { uint sourceWidth, sourceHeight, outputWidth, outputHeight; };
Texture2D<float4> source : register(t0);
RWTexture2D<float4> destination : register(u0);
[numthreads(16,16,1)]
void main(uint3 id : SV_DispatchThreadID) {
 if(id.x>=outputWidth || id.y>=outputHeight) return;
 uint2 sampleAt=uint2(id.x*sourceWidth/outputWidth,id.y*sourceHeight/outputHeight);
 destination[id.xy]=float4(source.Load(int3(sampleAt,0)).rgb,1.0);
}
