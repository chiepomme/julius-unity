using System.Collections;
using UnityEngine;

class JuliusSample : MonoBehaviour
{
    IEnumerator Start()
    {
        yield return Application.RequestUserAuthorization(UserAuthorization.Microphone);

        var path = Application.dataPath;
        switch (Application.platform)
        {
            case RuntimePlatform.WindowsEditor:
                path += "/StreamingAssets/";
                break;
            case RuntimePlatform.IPhonePlayer:
                path += "/Raw/";
                break;
        }
        path += "grammar-kit/grammar/mic.jconf";

        Julius.ResultReceived += OnResultReceived;
        Julius.Begin(path);
    }

    string lastResult = "";

    void OnResultReceived(string result)
    {
        if (result.Contains("confidence_score"))
        {
            lastResult = "<FinalResult>\n";
        }
        else
        {
            lastResult = "<First Pass Progress>\n";
        }

        lastResult += result;
    }

    void OnGUI()
    {
        GUI.Label(new Rect(0, 0, Screen.width, Screen.height), lastResult);
    }

    void OnDestroy()
    {
        Julius.Finish();
    }
}
