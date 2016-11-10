using UnityEngine;
using System.Runtime.InteropServices;
using System;
using System.Threading;

public static class Julius
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
    [DllImport("julius-unity-win")]
#endif
    public static extern void start(string filename);

#if UNITY_IPHONE || UNITY_XBOX360
        [DllImport ("__Internal")]
#else
    [DllImport("julius-unity-win")]
#endif
    static extern void set_debug_log_func(DebugLogDelegate debugLog);

#if UNITY_IPHONE || UNITY_XBOX360
        [DllImport ("__Internal")]
#else
    [DllImport("julius-unity-win")]
#endif
    static extern void set_audio_callback(AudioReadCallback callback);

#if UNITY_IPHONE || UNITY_XBOX360
        [DllImport ("__Internal")]
#else
    [DllImport("julius-unity-win")]
#endif
    static extern void set_result_func(OutputResultDelegate outputResult);

#if UNITY_IPHONE || UNITY_XBOX360
        [DllImport ("__Internal")]
#else
    [DllImport("julius-unity-win")]
#endif
    static extern void set_log_to_file(string path);

#if UNITY_IPHONE || UNITY_XBOX360
        [DllImport ("__Internal")]
#else
    [DllImport("julius-unity-win")]
#endif
    static extern void set_log_to_stdout(bool useStderrInstead);

#if UNITY_IPHONE || UNITY_XBOX360
        [DllImport ("__Internal")]
#else
    [DllImport("julius-unity-win")]
#endif
    static extern void finish();

    const int ClipLengthSeconds = 5;
    const int SamplingRate = 16000;

    static UnitySynchronizationContext context;
    static Thread thread;
    static AudioClip clip;

    // コールバックのインスタンスを保持しておかないと GC されてしまう
    static AudioReadCallback audioCallback;
    static DebugLogDelegate debugLogCallback;
    static OutputResultDelegate outputResultCallback;

    // static にしないとうまくデータを共有できない
    static float[] samplesFromMicrophone;
    static Int16[] samplesInInt16;
    static int previousReadPos;
    static int numSamplesToCopy;
    static IntPtr sharedBuffer;

    public static void Begin(string path)
    {
        if (thread != null && thread.IsAlive)
        {
            // TODO: 前回のが生きている場合にはなんとかする
            finish();
        }

        context = UnitySynchronizationContext.Create();
        clip = Microphone.Start("", true, ClipLengthSeconds, SamplingRate);

        samplesFromMicrophone = new float[clip.samples * clip.channels];
        samplesInInt16 = new Int16[clip.samples * clip.channels];
        sharedBuffer = Marshal.AllocCoTaskMem(clip.samples * clip.channels * 2);

        debugLogCallback = DebugLog;
        audioCallback = ReadAudio;
        outputResultCallback = OutputResult;

        set_debug_log_func(debugLogCallback);
        set_audio_callback(audioCallback);
        set_result_func(outputResultCallback);
        set_log_to_file("julius_log.txt");

        thread = new Thread(new ThreadStart(() =>
        {
            start(path);
            DebugLog("Thread Finished");
        }));
        thread.Start();
    }

    public static void Finish()
    {
        finish();
        context.Update();

        if (sharedBuffer != IntPtr.Zero)
        {
            Marshal.FreeCoTaskMem(sharedBuffer);
            sharedBuffer = IntPtr.Zero;
        }
    }

    [AOT.MonoPInvokeCallback(typeof(OutputResultDelegate))]
    static void OutputResult(IntPtr resultStringPointer, int length)
    {
        var buffer = new byte[length];
        Marshal.Copy(resultStringPointer, buffer, 0, length);
        var result = USEncoder.ToEncoding.ToUnicode(buffer);
        DebugLog(result);
    }

    [AOT.MonoPInvokeCallback(typeof(DebugLogDelegate))]
    static void DebugLog(string msg)
    {
        Debug.Log(msg);
    }

    [AOT.MonoPInvokeCallback(typeof(AudioReadCallback))]
    static IntPtr ReadAudio(int sampleNums, IntPtr numSamplesPointer)
    {
        context.Send((_) =>
        {
            if (sampleNums <= 0)
            {
                numSamplesToCopy = 0;
                return;
            }

            // 前回の録音位置から今回の録音位置までを返す
            var currentReadPos = Microphone.GetPosition("");
            if (previousReadPos < currentReadPos) // 巻き戻りが起きていない
            {
                var sampleRected = (currentReadPos - previousReadPos);
                numSamplesToCopy = Math.Min(sampleRected, sampleNums);

                CopySamplesFromClipToSharedBuffer(numSamplesToCopy);
            }
            else if (currentReadPos < previousReadPos) // 巻き戻りが起きている
            {
                var sampleRected = ((clip.samples - previousReadPos) + currentReadPos);
                numSamplesToCopy = Math.Min(sampleRected, sampleNums);

                CopySamplesFromClipToSharedBuffer(numSamplesToCopy);
            }
            else
            {
                numSamplesToCopy = 0;
            }

            previousReadPos = currentReadPos;

        }, null);

        Marshal.WriteInt32(numSamplesPointer, numSamplesToCopy);
        return sharedBuffer;
    }

    static void CopySamplesFromClipToSharedBuffer(int samplesToRead)
    {
        clip.GetData(samplesFromMicrophone, previousReadPos);

        samplesInInt16 = Array.ConvertAll(samplesFromMicrophone, (v) => (Int16)(Int16.MaxValue * (v * 0.5)));
        Marshal.Copy(samplesInInt16, 0, sharedBuffer, samplesToRead);
    }
}
