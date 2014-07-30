﻿
#pragma once

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#include <ace.common.Base.h>
#include "../../ace.ReferenceObject.h"

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace ace {
	//----------------------------------------------------------------------------------
	//
	//----------------------------------------------------------------------------------
	class MaterialCommand
	{
	protected:
		NativeShader_Imp*					shader;
		std::vector<ShaderConstantValue>	constantValues;
	public:
		MaterialCommand(NativeShader_Imp* shader, std::map<astring, ShaderConstantValue>& values);
		virtual ~MaterialCommand();

		NativeShader_Imp* GetShader() { return shader; }
		std::vector<ShaderConstantValue>& GetConstantValues() { return constantValues; }
	};

	class Material_Imp
		: public ReferenceObject
	{
	protected:
		std::map<astring, ShaderConstantValue>		m_values;

		Material_Imp(){}
		virtual ~Material_Imp();
	public:

		float GetFloat_Imp(const achar* name);
		void SetFloat_Imp(const achar* name, float value);

		Vector2DF GetVector2DF_Imp(const achar* name);
		void SetVector2DF_Imp(const achar* name, Vector2DF value);

		Vector3DF GetVector3DF_Imp(const achar* name);
		void SetVector3DF_Imp(const achar* name, Vector3DF value);

		Vector4DF GetVector4DF_Imp(const achar* name);
		void SetVector4DF_Imp(const achar* name, Vector4DF value);

		/**
			@brief	テクスチャを取得する。
			@name	名称
			@note
			テクスチャを取得した際にテクスチャの参照カウンタが1加算される。
		*/
		Texture2D* GetTexture2D_(const achar* name);
		void SetTexture2D_(const achar* name, Texture2D* value);

		TextureFilterType GetTextureFilterType(const achar* name);
		void SetTextureFilterType(const achar* name, TextureFilterType filter);

		TextureWrapType GetTextureWrapType(const achar* name);
		void SetTextureWrapType(const achar* name, TextureWrapType wrap);

	};

	//----------------------------------------------------------------------------------
	//
	//----------------------------------------------------------------------------------

}