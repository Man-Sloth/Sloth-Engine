Shader "Custom/Edge Tiles - Diffuse"
{
	Properties
	{
		_MainTex ("Sprite Texture", 2D) = "white" {}
		_Color ("Tint", Color) = (1,1,1,1)
		[MaterialToggle] PixelSnap ("Pixel snap", Float) = 0

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
			"Queue"="Transparent" 
			"IgnoreProjector"="True" 
			"RenderType"="Transparent" 
			"PreviewType"="Plane"
			"CanUseSpriteAtlas"="True"
		}

		Cull Off
		Lighting Off
		ZWrite Off
		Blend One OneMinusSrcAlpha

		CGPROGRAM
		#pragma surface surf Lambert vertex:vert nofog keepalpha
		#pragma multi_compile _ PIXELSNAP_ON
		#pragma multi_compile _ ETC1_EXTERNAL_ALPHA

		sampler2D _MainTex;
		fixed4 _Color;
		sampler2D _AlphaTex;
		float _AlphaSplitEnabled;

		sampler2D _BlendTex;
		float _SplatAmount;
		float _BottomRight;
		float _TopRight;
		float _BottomLeft;
		float _TopLeft;
		float _BlendAmount;

		struct Input
		{
			float2 uv_MainTex;
			fixed4 color;
		};
		
		void vert (inout appdata_full v, out Input o)
		{
			#if defined(PIXELSNAP_ON)
			v.vertex = UnityPixelSnap (v.vertex);
			#endif
			
			UNITY_INITIALIZE_OUTPUT(Input, o);
			o.color = v.color * _Color;
		}

		fixed4 SampleSpriteTexture (float2 uv)
		{
			fixed4 color = tex2D (_MainTex, uv);

			fixed4 color2 = tex2D(_BlendTex, uv);
			float alphaMain = (color.r + color.g + color.b) / 3;
			float blendOffset = 0;
			fixed4 blend = color;

			float a = (uv.x + uv.y) > (_BottomLeft + 0.5) ? 1 : 0;
			float b = (uv.x + uv.y) > ((1 - _TopRight) + 0.5) ? 0 : 1;
			float c = (uv.y - uv.x) > (_BottomRight - 0.5) ? 1 : 0;
			float d = (uv.y - uv.x) > (-(_TopLeft - 0.5)) ? 0 : 1;
			float e = alphaMain > _SplatAmount ? 1 : 0;

			blendOffset = (_BottomLeft + 0.5) - (uv.x + uv.y);
			blend = (a*b*c*d*e) > 0 ? color : lerp(color, color2, clamp(((1 - _BlendAmount) + blendOffset), 0.0, 1.0));

			blendOffset = -(((1 - _TopRight) + 0.5) - (uv.x + uv.y));
			blend = (a*b*c*d*e) > 0 ? blend : lerp(blend, color2, clamp(((1 - _BlendAmount) + blendOffset), 0.0, 1.0));

			blendOffset = (_BottomRight - 0.5) - (uv.y - uv.x);
			blend = (a*b*c*d*e) > 0 ? blend : lerp(blend, color2, clamp(((1 - _BlendAmount) + blendOffset), 0.0, 1.0));

			blendOffset = 1 - ((1 - (_TopLeft - 0.5) - (uv.y - uv.x)));
			blend = (a*b*c*d*e) > 0 ? blend : lerp(blend, color2, clamp(((1 - _BlendAmount) + blendOffset), 0.0, 1.0));

#if ETC1_EXTERNAL_ALPHA
			//color.a = tex2D (_AlphaTex, uv).r;
#endif //ETC1_EXTERNAL_ALPHA

			return blend;
		}

		void surf (Input IN, inout SurfaceOutput o)
		{
			fixed4 c = SampleSpriteTexture (IN.uv_MainTex) * IN.color;
			o.Albedo = c.rgb * c.a;
			o.Alpha = c.a;
		}
		ENDCG
	}

Fallback "Transparent/VertexLit"
}
