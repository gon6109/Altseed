﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace asd
{
	/// <summary>
	/// 3Dモデルクラス
	/// </summary>
	public partial class Model : IDestroy
	{
		internal Model(swig.Model swig)
		{
#if DEBUG
			// 唯一の対応するクラスであることを保証
			if (GC.Models.GetObject(swig.GetPtr()) != null) Particular.Helper.ThrowException("");
#endif
			CoreInstance = swig;
		}

		~Model()
		{
			Destroy();
		}

		public bool IsDestroyed
		{
			get
			{
				return CoreInstance == null;
			}
		}

		/// <summary>
		/// 強制的に使用しているメモリを開放する。
		/// </summary>
		/// <remarks>
		/// 何らかの理由でメモリが不足した場合に実行する。
		/// 開放した後の動作の保証はしていないので、必ず参照が残っていないことを確認する必要がある。
		/// </remarks>
		public void Destroy()
		{
			lock (this)
			{
				if (CoreInstance == null) return;
				GC.Collector.AddObject(CoreInstance);
				CoreInstance = null;
			}
			Particular.GC.SuppressFinalize(this);
		}

		/// <summary>
		/// モデルが持つアニメーションクリップを取得する。
		/// </summary>
		/// <param name="index">アニメーションクリップのインデックス</param>
		/// <returns>アニメーションクリップ</returns>
		public AnimationClip GetAnimationClip(int index)
		{
			return GC.GenerateAnimationClip(swig.Accessor.Model_GetAnimationClip(CoreInstance, index), GC.GenerationType.Get);
		}

		/// <summary>
		/// メッシュを取得する。
		/// </summary>
		/// <param name="index">メッシュのインデックス</param>
		/// <returns></returns>
		public Mesh GetMesh(int index)
		{
			return GC.GenerateMesh(swig.Accessor.Model_GetMesh(CoreInstance, index), GC.GenerationType.Get);
		}
	}
}
