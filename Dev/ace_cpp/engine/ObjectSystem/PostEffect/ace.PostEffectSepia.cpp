﻿#include "ace.PostEffectSepia.h"

#include <vector>
namespace ace{

	// 現在のアルゴリズム:グレースケール化→セピア色(107, 74, 43)をグレースケール化した場合の
	// 明度とRGB各値の比を取り、グレースケール化した各画素の成分ごとに乗ずる
	PostEffectSepia::PostEffectSepia(Graphics *g)
	{

		static const char* shader2d_dx_ps = R"(

Texture2D		g_texture		: register( t0 );
SamplerState	g_sampler		: register( s0 );

float4 main( const PS_Input Input ) : SV_Target
{
	float4 output = g_texture.Sample(g_sampler, Input.UV);
	if(output.a == 0.0f) discard;
	float y = output.r * 0.298912f + output.g * 0.586611f + output.b * 0.114478f;
	output.rgb = y;
	output.r *= 1.332249;
	output.r = output.r > 1.0f? 1.0f: output.r;
	output.g *= 0.921369f;
	output.b *= 0.535390f;
	return output;
}

)";

		static const char* shader2d_gl_ps = R"(

uniform sampler2D g_texture;

void main()
{
	float4 output = texture2D(g_texture, inUV.xy);
	float y = output.r * 0.298912f + output.g * 0.586611f + output.b * 0.114478f;
	output.rgb = y;
	output.r *= 1.332249;
	output.r = output.r > 1.0f? 1.0f: output.r;
	output.g *= 0.921369f;
	output.b *= 0.535390f;
	gl_FragColor = output; 
}

)";


		std::vector<ace::ShaderVariableProperty> props;
		auto prop_tex = ace::ShaderVariableProperty();
		prop_tex.Name = ace::ToAString("g_texture").c_str();
		prop_tex.Offset = 0;
		prop_tex.Type = ace::SHADER_VARIABLE_TYPE_TEXTURE2D;
		props.push_back(prop_tex);

		if (g->GetGraphicsType() == ace::GRAPHICS_TYPE_DX11)
		{
			m_shader = g->CreateShader2D(
				ace::ToAString(shader2d_dx_ps).c_str(),
				props
				);
		}
		else if (g->GetGraphicsType() == ace::GRAPHICS_TYPE_GL)
		{
			std::vector<ace::ShaderVariableProperty> prop;
			m_shader = g->CreateShader2D(
				ace::ToAString(shader2d_gl_ps).c_str(),
				props
				);
		}
		else
		{
			assert(0);
		}

		m_material2d = g->CreateMaterial2D(m_shader);
	}

	void PostEffectSepia::OnDraw(std::shared_ptr<RenderTexture2D> dst, std::shared_ptr<RenderTexture2D> src)
	{
		m_material2d->SetTexture2D(ace::ToAString("g_texture").c_str(), src);

		DrawOnTexture2DWithMaterial(dst, m_material2d);
	}

}