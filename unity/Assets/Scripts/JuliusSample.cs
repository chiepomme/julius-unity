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

        Julius.Begin(path);
    }

    void OnDestroy()
    {
        Julius.Finish();
    }
}
