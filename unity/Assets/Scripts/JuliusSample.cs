using UnityEngine;

class JuliusSample : MonoBehaviour
{
    void Awake()
    {
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
