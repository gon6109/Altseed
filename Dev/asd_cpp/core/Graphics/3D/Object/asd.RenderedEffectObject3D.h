﻿
#pragma once

#include <Math/asd.Vector2DI.h>
#include <Math/asd.Vector3DF.h>

#include "asd.RenderedObject3D.h"

namespace asd
{
	class RenderedEffectObject3DProxy
		: public RenderedObject3DProxy
	{
	public:
		RenderedObject3DType GetObjectType() const override { return RenderedObject3DType::Effect; }
	};

	class RenderedEffectObject3D
		: public RenderedObject3D
	{
	private:

		enum class CommandType : uint8_t
		{
			Play,
			Stop,
			StopRoot,
			Show,
			Hide,
		};

		struct Command
		{
			CommandType Type;
			int32_t ID;
		};

		
		struct InternalState
		{
			Effekseer::Handle effekseerHandle = -1;

			//! 再生の遅延処理のため
			bool isCommandExecuted = false;
		};

		static const int32_t GCThreshold = 32;

		std::map<int32_t, InternalState> internalHandleToState;
		int32_t							nextInternalHandle = 0;

		std::vector<Effekseer::Handle>	m_handles;
		std::vector<Command>			commands;

		Effect*							m_effect = nullptr;
		Renderer3D*						m_renderer = nullptr;
		bool							m_syncEffects = false;

		RenderedEffectObject3DProxy*				proxy = nullptr;

		int32_t PlayInternal(int32_t id);

	public:
		RenderedEffectObject3D(Graphics* graphics);
		virtual ~RenderedEffectObject3D();

		void Flip(float deltaTime) override;

		RenderedObject3DProxy* GetProxy() const override { return proxy; }

		void SetEffect(Effect* effect);

		/**
			@brief	設定されている全てのエフェクトを再生する。
		*/
		int32_t Play();

		void Show();

		void Hide();

		/**
			@brief	このオブジェクトから再生されたエフェクトを全て停止する。
		*/
		void Stop();

		/**
		@brief	このオブジェクトから再生されたエフェクトのルートを全て停止する。
		*/
		void StopRoot();

		/**
		@brief	このオブジェクトから再生されたエフェクトが再生中か取得する。
		*/
		bool GetIsPlaying();

		/**
		@brief	このオブジェクトから再生されたエフェクトをオブジェクトに合わせて移動させるか取得する。
		@return	フラグ
		*/
		bool GetSyncEffects() { return m_syncEffects; }

		/**
		@brief	このオブジェクトから再生されたエフェクトをオブジェクトに合わせて移動させるか設定する。
		@param	value	フラグ
		*/
		void SetSyncEffects(bool value) { m_syncEffects = value; }

		void OnAdded(Renderer3D* renderer) override;

		void OnRemoving(Renderer3D* renderer) override;

		RenderedObject3DType GetObjectType() const override { return RenderedObject3DType::Effect; }
	};
}
