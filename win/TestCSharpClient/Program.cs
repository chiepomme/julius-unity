using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

namespace TestCSharpClient
{
    class Program
    {
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void DebugLogDelegate(string message);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void OutputResultDelegate(IntPtr resultPtr, int length);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate IntPtr AudioReadCallback(int requestedBytes, IntPtr len);

#if UNITY_IPHONE || UNITY_XBOX360
        [DllImport ("__Internal")]
#else
        [DllImport("julius-unity-win.dll")]
#endif
        public static extern void start(string filename);

#if UNITY_IPHONE || UNITY_XBOX360
        [DllImport ("__Internal")]
#else
        [DllImport("julius-unity-win.dll")]
#endif
        static extern void set_debug_log_func(DebugLogDelegate debugLog);

#if UNITY_IPHONE || UNITY_XBOX360
        [DllImport ("__Internal")]
#else
        [DllImport("julius-unity-win.dll")]
#endif
        static extern void set_audio_callback(AudioReadCallback callback);

#if UNITY_IPHONE || UNITY_XBOX360
        [DllImport ("__Internal")]
#else
        [DllImport("julius-unity-win.dll")]
#endif
        static extern void set_result_func(OutputResultDelegate outputResult);

#if UNITY_IPHONE || UNITY_XBOX360
        [DllImport ("__Internal")]
#else
        [DllImport("julius-unity-win.dll")]
#endif
        static extern void set_log_to_file(string path);

#if UNITY_IPHONE || UNITY_XBOX360
        [DllImport ("__Internal")]
#else
        [DllImport("julius-unity-win.dll")]
#endif
        static extern void set_log_to_stdout(bool useStderrInstead);

#if UNITY_IPHONE || UNITY_XBOX360
        [DllImport ("__Internal")]
#else
        [DllImport("julius-unity-win.dll")]
#endif
        static extern void finish();

        static AudioReadCallback audioCallback;
        static DebugLogDelegate debugLogCallback;
        static OutputResultDelegate outputResultCallback;
        static IntPtr sharedBuffer;

        static void Main(string[] args)
        {
            sharedBuffer = Marshal.AllocCoTaskMem(0);

            debugLogCallback = DebugLog;
            audioCallback = ReadAudio;
            outputResultCallback = OutputResult;

            set_debug_log_func(debugLogCallback);
            // set_audio_callback(audioCallback);
            set_result_func(outputResultCallback);
            set_log_to_file("julius_log.txt");

            var thread = new Thread(new ThreadStart(() =>
            {
                start(@"../../../../../unity/Assets/StreamingAssets/grammar-kit/grammar/mic.jconf");
                DebugLog("FINISHED");
            }));


            thread.Start();
            Thread.Sleep(100000);
            finish();
            Thread.Sleep(3000);
        }

        public static void Stop()
        {
            finish();
            Marshal.FreeCoTaskMem(sharedBuffer);
        }

        static void OutputResult(IntPtr resultStringPointer, int length)
        {
            var buffer = new byte[length];
            Marshal.Copy(resultStringPointer, buffer, 0, length);
            var result = Encoding.GetEncoding(932).GetString(buffer);
            Console.WriteLine(result);
        }

        static void DebugLog(string msg)
        {
            Console.WriteLine(msg);
        }

        static IntPtr ReadAudio(int sampleNums, IntPtr numSamplesPointer)
        {
            Marshal.WriteInt32(numSamplesPointer, 0);
            return sharedBuffer;
        }
    }
}
