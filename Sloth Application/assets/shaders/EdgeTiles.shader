Shader "Custom/Edge Tiles"
{
	Properties
	{
		_MainTex("Sprite Texture", 2D) = "white" {}
		_Color("Tint", Color) = (1,1,1,1)
		[MaterialToggle] PixelSnap("Pixel snap", Float) = 0

		_BlendTex("Sprite to blend", 2D) = "white" {}
		_SplatAmount("Splat Amount", Range(0,1)) = 0
		_BottomRight("Bottom Right", Range(0,1)) = 0
		_TopRight("Top Right", Range(0,1)) = 0
		_BottomLeft("Bottom Left", Range(0,1)) = 0
		_TopLeft("Top Left", Range(0,1)) = 0
		_BlendAmount("Blend Amount", Range(0,1)) = 0
	}

		SubShader
	{
		Tags
	{
		"Queue" = "Transparent"
		"IgnoreProjector" = "True"
		"RenderType" = "Transparent"
		"PreviewType" = "Plane"
		"CanUseSpriteAtlas" = "True"
	}

		Cull Off
		Lighting Off
		ZWrite Off
		Blend One OneMinusSrcAlpha

		Pass
	{
		CGPROGRAM
#pragma vertex vert
#pragma fragment frag
#pragma target 2.0
#pragma multi_compile _ PIXELSNAP_ON
#pragma multi_compile _ ETC1_EXTERNAL_ALPHA
#include "UnityCG.cginc"

		struct appdata_t
	{
		float4 vertex   : POSITION;
		float4 color    : COLOR;
		float2 texcoord : TEXCOORD0;
		UNITY_VERTEX_INPUT_INSTANCE_ID
	};

	struct v2f
	{
		float4 vertex   : SV_POSITION;
		fixed4 color : COLOR;
		float2 texcoord  : TEXCOORD0;
		UNITY_VERTEX_OUTPUT_STEREO
	};

	fixed4 _Color;

	v2f vert(appdata_t IN)
	{
		v2f OUT;
		UNITY_SETUP_INSTANCE_ID(IN);
		UNITY_INITIALIZE_VERTEX_OUTPUT_STEREO(OUT);
		OUT.vertex = UnityObjectToClipPos(IN.vertex);
		OUT.texcoord = IN.texcoord;
		OUT.color = IN.color * _Color;
#ifdef PIXELSNAP_ON
		OUT.vertex = UnityPixelSnap(OUT.vertex);
#endif

		return OUT;
	}

	sampler2D _MainTex;
	sampler2D _AlphaTex;
	sampler2D _BlendTex;

	float _SplatAmount;
	float _BottomRight;
	float _TopRight;
	float _BottomLeft;
	float _TopLeft;
	float _BlendAmount;

	float4 _MainTex_TexelSize;

	fixed4 SampleSpriteTexture(float2 uv)
	{
		fixed4 color = tex2D(_MainTex, uv);
		fixed4 color2 = tex2D(_BlendTex, uv);
		float alphaMain = (color.r + color.g + color.b) / 3;
		float blendOffset = 0;
		fixed4 blend = color;

		float a = (uv.x + uv.y) > (_BottomLeft + 0.5) ? 1 : 0;
		float b = (uv.x + uv.y) > ((1-_TopRight ) +0.5) ? 0 : 1;
		float c = (uv.y - uv.x) > (_BottomRight - 0.5) ? 1 : 0;
		float d = (uv.y - uv.x) > (-(_TopLeft - 0.5)) ? 0 : 1;
		float e = alphaMain > _SplatAmount ? 1 : 0;

		blendOffset = (_BottomLeft + 0.5) - (uv.x + uv.y);  
		blend = (a*b*c*d*e) > 0 ? color : lerp(color, color2, clamp(((1 - _BlendAmount) + blendOffset), 0.0, 1.0));

		blendOffset = -(((1-_TopRight)+0.5) - (uv.x + uv.y));
		blend = (a*b*c*d*e) > 0 ? blend : lerp(blend, color2, clamp(((1 - _BlendAmount) + blendOffset), 0.0, 1.0));

		blendOffset = (_BottomRight - 0.5) - (uv.y - uv.x);
		blend = (a*b*c*d*e) > 0 ? blend : lerp(blend, color2, clamp(((1 - _BlendAmount) + blendOffset), 0.0, 1.0));

		blendOffset = 1-((1-(_TopLeft - 0.5) - (uv.y - uv.x)));
		blend = (a*b*c*d*e) > 0 ? blend : lerp(blend, color2, clamp(((1 - _BlendAmount) + blendOffset), 0.0, 1.0));

		//color2.a = _BlendAmount;
		//color2 = color2*color2.a;
		
		
		//blend = (a*b*c*d*e) > 0 ? color : lerp(color, color2, clamp(((1-_BlendAmount)+blendOffset), 0.0, 1.0));
		///fixed4 blend = (a*b*c*d*e) > 0 ? color : color2;
		//blend = ((_BottomLeft + 0.5)-(uv.x + uv.y))  * _BlendAmount;
		//fixed4 blend = (uv.x + uv.y) > (_BottomLeft * 2) ? color : color2;


		//float bA = _BlendAmount;

		//float alphaMain = (color.r + color.g + color.b) / 3;

		//fixed4 blend = alphaMain > bA ? color : color2 ;

#if ETC1_EXTERNAL_ALPHA
		// get the color from an external texture (usecase: Alpha support for ETC1 on android)
		//color.a = tex2D(_AlphaTex, uv).r;
#endif //ETC1_EXTERNAL_ALPHA

		return blend;
	}

	fixed4 frag(v2f IN) : SV_Target
	{
		fixed4 c = SampleSpriteTexture(IN.texcoord) * IN.color;
	c.rgb *= c.a;
		

	return c;
	}
		ENDCG
	}
	}
}
