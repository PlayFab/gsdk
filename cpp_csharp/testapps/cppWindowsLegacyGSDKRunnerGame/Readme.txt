Package Steps:
    a. The output folder is XcloudDevEx\GSDK\cpp_csharp\x64\(Debug|Release)\GSDKCppWindowsLegacyGSDKRunnerGame
    b. Copy the content out to a separate target folder. We will zip that folder later for asset package
    c. Copy ServiceDefinition.json (from the project folder) to the target folder
    d. If you are building release package, you can directly go to step (e).
       For debug package:
        Copy msvcp140d.dll, concrt140d.dll, vccorlib140d.dll, vcruntime140d.dll (from C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\debug_nonredist\x64\Microsoft.VC140.DebugCRT) into the target folder
        Copy ucrtbased.dll (from C:\Windows\System32) into the target folder
    e. zip the folder and it can be uploaded through GameManager portal. For functional tests, the zip file is required to upload to https://xcloudtestrunner.blob.core.windows.net/winrunnerassets/legacy_assets_v(version).zip
