using System.IO;
using UnityEditor;
using UnityEditor.Callbacks;
using UnityEditor.iOS.Xcode;

static class IOSPostBuildProcessor
{
    [PostProcessBuild]
    public static void OnPostprocessBuild(BuildTarget platform, string projectPath)
    {
        if (platform != BuildTarget.iOS) return;

        EditProjectFile(projectPath);
        EditPlist(projectPath);
    }

    static void EditProjectFile(string projectPath)
    {
        var pbxFile = PBXProject.GetPBXProjectPath(projectPath);
        var pbxProject = new PBXProject();
        pbxProject.ReadFromFile(pbxFile);

        var target = pbxProject.TargetGuidByName(PBXProject.GetUnityTargetName());
        // ZLib を使用しているので -lz を追加する
        pbxProject.AddBuildProperty(target, "OTHER_LDFLAGS", "-ObjC -lz");
        pbxProject.WriteToFile(pbxFile);
    }

    static void EditPlist(string projectPath)
    {
        var plistPath = Path.Combine(projectPath, "Info.plist");
        var plist = new PlistDocument();
        plist.ReadFromFile(plistPath);

        // マイクへのアクセスが必要な場合には plist に用途を書かないといけない
        // see https://developer.apple.com/library/content/documentation/General/Reference/InfoPlistKeyReference/Articles/CocoaKeys.html#//apple_ref/doc/uid/TP40009251-SW25
        plist.root.SetString("NSMicrophoneUsageDescription", "音声認識に使用します");
        plist.WriteToFile(plistPath);
    }
}
