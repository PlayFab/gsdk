using UnityEditor;
using UnityEditor.Build.Reporting;
using UnityEngine;

public class Builder
{
    [MenuItem("Test/Build Win64")]
    private static int BuildWin64()
    {
        // Setup build options (e.g. scenes, build output location)
        var options = new BuildPlayerOptions
        {
            // Change to scenes from your project
            scenes = new[]
            {
                "Assets/Scene.unity",
            },

            // Change to location the output should go
            locationPathName = "BuildOutputWindows/build.exe",
            options = BuildOptions.CleanBuildCache | BuildOptions.BuildScriptsOnly | BuildOptions.StrictMode,
            target = BuildTarget.StandaloneWindows64,
        };
        
        var report = BuildPipeline.BuildPlayer(options);

        int error = 0;
        if (report.summary.result == BuildResult.Succeeded)
        {
            Debug.Log($"Build successful - Build written to {options.locationPathName}");
        }
        else if (report.summary.result == BuildResult.Failed)
        {
            Debug.LogError($"Build failed");
            error = 1;
        }

        return error;
    }

    [MenuItem("Test/Build Linux64")]
    private static int BuildLinux64()
    {
        // Setup build options (e.g. scenes, build output location)
        var options = new BuildPlayerOptions
        {
            // Change to scenes from your project
            scenes = new[]
            {
                "Assets/Scene.unity",
            },

            // Change to location the output should go
            locationPathName = "BuildOutputLinux/build.exe",
            options = BuildOptions.CleanBuildCache | BuildOptions.BuildScriptsOnly | BuildOptions.StrictMode,
            target = BuildTarget.StandaloneLinux64,
        };

        var report = BuildPipeline.BuildPlayer(options);

        int error = 0;
        if (report.summary.result == BuildResult.Succeeded)
        {
            Debug.Log($"Build successful - Build written to {options.locationPathName}");
        }
        else if (report.summary.result == BuildResult.Failed)
        {
            Debug.LogError($"Build failed");
            error = 2;
        }

        return error;
    }

    // This function will be called from the build process
    public static void Build()
    {
        int error = 0;
        error += BuildWin64();
        error += BuildLinux64();

        if (error > 0)
        {
            EditorApplication.Exit(error);
        }
    }
}