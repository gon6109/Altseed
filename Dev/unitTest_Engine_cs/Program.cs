﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using asd;

namespace unitTest_Engine_cs
{
	class TestResult
	{
		public TestResult()
		{
			Exceptions = new Dictionary<GraphicsDeviceType, Exception>();
		}

		public string Title { get; set; }
		public Dictionary<GraphicsDeviceType, Exception> Exceptions { get; private set; }
	}

	class Program
	{
		[STAThread]
		static void Main(string[] args)
		{
			/*
			TestOnAllDevice(typeof(FamilySystem.ChildManagement));
			Console.ReadKey();
			return;
			//*/

			TestAll();

			Console.ReadKey();
		}

		/// <summary>
		/// このプロジェクト内に定義されたテストクラスを収集し、すべて実行する。
		/// </summary>
		private static void TestAll()
		{
			var errors = Assembly.GetAssembly(typeof(Program))
				.GetTypes()
				.Where(_ => !_.IsAbstract)
				.Where(_ => _.IsSubclassOf(typeof(TestFramework)))
				.Where(x => x.GetConstructor(new Type[0]) != null)
				.Where(x => x.Namespace == "unitTest_Engine_cs.ObjectSystem2D.LifeCycle")
				.Where(x => !x.Namespace.Contains("BugFix"))
				.Select(TestOnAllDevice)
				.Where(x => x.Exceptions.Any(y => y.Value != null))
				.ToArray();

			if (errors.Any())
			{
				Console.ForegroundColor = ConsoleColor.Red;
				Console.WriteLine("{0} tests failed.", errors.Length);
				Console.ForegroundColor = ConsoleColor.Gray;
				foreach(var testResult in errors)
				{
					if (testResult.Exceptions[GraphicsDeviceType.OpenGL] != null)
					{
						Console.ForegroundColor = ConsoleColor.Red;
						Console.WriteLine("[{0}(OpenGL)]", testResult.Title);
						Console.ForegroundColor = ConsoleColor.Gray;
						Console.WriteLine(testResult.Exceptions[GraphicsDeviceType.OpenGL]);
					}
					if(testResult.Exceptions[GraphicsDeviceType.DirectX11] != null)
					{
						Console.ForegroundColor = ConsoleColor.Red;
						Console.WriteLine("[{0}(DirectX)]", testResult.Title);
						Console.ForegroundColor = ConsoleColor.Gray;
						Console.WriteLine(testResult.Exceptions[GraphicsDeviceType.DirectX11]);
					}
				}
			}
		}

		/// <summary>DirectXが有効なシステムか。
		/// </summary>
		/// <returns>DirectXが有効なシステムなら<c>true</c>、そうでなければ<c>false</c>を返す。</returns>
		private static bool IsDirectXAvailable(){
			var os = System.Environment.OSVersion;
			var isUnix = os.Platform == PlatformID.Unix || os.Platform == PlatformID.MacOSX;
			return !isUnix;
		}

		/// <summary>
		/// 指定したテストクラスに対して、OpenGL と DirectX 向けのテストを行う。
		/// </summary>
		/// <remarks>個別にテストしたい場合に利用してください。</remarks>
		/// <param name="testType">対象のテストクラス。</param>
		private static TestResult TestOnAllDevice(Type testType)
		{
			TestResult result = new TestResult()
			{
				Title = testType.FullName
			};

			try
			{
                var target = Activator.CreateInstance(testType) as TestFramework;
                if(target != null)
                {
                    target.Test(asd.GraphicsDeviceType.OpenGL);
                }
				result.Exceptions[GraphicsDeviceType.OpenGL] = null;
			}
			catch (Exception e)
			{
				Console.WriteLine(e.ToString());
				result.Exceptions[GraphicsDeviceType.OpenGL] = e;
			}

			Task.Delay(50).Wait();

			if ( IsDirectXAvailable() ) {
				try
                {
                    var target = Activator.CreateInstance(testType) as TestFramework;
                    if(target != null)
                    {
                        target.Test(asd.GraphicsDeviceType.DirectX11);
					}
					result.Exceptions[GraphicsDeviceType.DirectX11] = null;
				}
				catch (Exception e)
				{
					Console.WriteLine(e.ToString());
					result.Exceptions[GraphicsDeviceType.DirectX11] = e;
				}

				Task.Delay(50).Wait();
			}

			return result;
		}
	}

	public abstract class TestFramework
	{
		public abstract void Test(asd.GraphicsDeviceType graphicsType);
	}
}
