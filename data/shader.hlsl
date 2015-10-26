//-----------------------------------------------------------------------------
cbuffer Transforms
{
	matrix WorldViewProjMatrix;	
	float4 Distance;
};

//-----------------------------------------------------------------------------
struct VS_INPUT
{
	float4 position : POSITION;
	float4 color : COLOR;
};

struct VS_OUTPUT
{
	float4 position : SV_Position;
	float4 color : COLOR;
};

struct GS_INPUTOUTPUT
{
	float4 position			: SV_Position;
	float4 color			: COLOR;
};

//-----------------------------------------------------------------------------
VS_OUTPUT VSMain( in VS_INPUT v )
{
	VS_OUTPUT o = (VS_OUTPUT)0;

	o.position = mul(v.position, WorldViewProjMatrix);
	o.color = v.color;

	return o;
}

[maxvertexcount(6)]
void GSMain( triangle GS_INPUTOUTPUT input[3], inout TriangleStream<GS_INPUTOUTPUT> TriangleOutputStream, uint instance : SV_PrimitiveID )
{
	GS_INPUTOUTPUT output;

	float2 p0 = input[0].position.xy / input[0].position.w;
	float2 p1 = input[1].position.xy / input[1].position.w;
	float2 p2 = input[2].position.xy / input[2].position.w;

	p0 = (p0 + float2(1, 1)) * float2(400, 300);
	p1 = (p1 + float2(1, 1)) * float2(400, 300);
	p2 = (p2 + float2(1, 1)) * float2(400, 300);

	float a = distance(p1, p2);
	float b = distance(p0, p2);
	float c = distance(p0, p1);
	float alpha = acos( (b*b + c*c - a*a) / (2*b*c) );
	float beta  = acos( (a*a + c*c - b*b) / (2*a*c) );

	float ha = abs(c * sin(beta));
	float hb = abs(c * sin(alpha));
	float hc = abs(b * sin(alpha));

	float3 d = float3(ha, hb, hc);

	// Output vertices
	for (int i = 0; i < 3; i++) {
		output.position = input[i].position;

		output.color = 0;
		output.color[i] = d[i];

		TriangleOutputStream.Append(output);
	}

	TriangleOutputStream.RestartStrip();

}


//-----------------------------------------------------------------------------
float4 PSMain( in GS_INPUTOUTPUT input ) : SV_Target
{
	float threshold = 0.5f;
	float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	if(input.color.x < threshold 
		|| input.color.y < threshold
		|| input.color.z < threshold)
		color = 0;

	return( color );
}
//-----------------------------------------------------------------------------