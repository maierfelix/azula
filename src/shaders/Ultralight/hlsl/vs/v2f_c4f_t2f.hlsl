cbuffer Uniforms : register(b0)
{
  float4 State;
  matrix Transform;
  float4 Scalar4[2];
  float4 Vector[8];
  uint ClipSize;
  matrix Clip[8];
};

float Time() { return State[0]; }
float ScreenWidth() { return State[1]; }
float ScreenHeight() { return State[2]; }
float Scalar(int i) { if (i < 4) return Scalar4[0][i]; else return Scalar4[1][i - 4]; }
float4 sRGBToLinear(float4 val) { return float4(val.xyz * (val.xyz * (val.xyz * 0.305306011 + 0.682171111) + 0.012522878), val.w); }

struct VS_OUTPUT
{
  float4 Position    : SV_POSITION;
  float4 Color       : COLOR0;
  float2 ObjectCoord : TEXCOORD0;
};

VS_OUTPUT VS(float2 Position : POSITION,
             uint4  Color    : COLOR0,
             float2 ObjCoord : TEXCOORD0)
{
  VS_OUTPUT output;
  output.Position = mul(Transform, float4(Position, 0.0, 1.0));
  output.Color = sRGBToLinear(float4(Color) / 255.0);
  output.ObjectCoord = ObjCoord;
  return output;
}