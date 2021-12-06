# Connect to MPS hosted build

This example sequence covered adding the Unreal GSDK to a standard ThirdPersonMP tutorial project. In doing so, we set up the server connection side, but we ignored a proper client connection sequence. PlayFab requires servers to be provisioned and activated before clients can connect to them. We will not cover that process here, but more details are available in the [MPS Documentation](https://docs.microsoft.com/en-us/gaming/playfab/features/multiplayer/servers/connecting-clients-to-game-servers). Instead, for this example, we will bypass those complicated steps and instead use a manual utility to test connections between the client and server.

## Prerequisites

* Clone or download the [MpsSamples repo](https://github.com/PlayFab/MpsSamples)
* A fully deployed ThirdPersonMP server in "Deployed" status, with at least 1 server fully propped and in "Standby" mode
    * ThirdPersonMP is the sample project [built](ThirdPersonMPBuild.md) and [deployed](ThirdPersonMPCloudDeploy.md) in earlier pages of this guide.
    * A build's status becomes "Deployed" when it contains servers are ready for connections. These servers are considered in "Standby"
* A [built game client](ThirdPersonMPBuild.md), ready to connect
* Visual Studio with .Net Core 3.1 (available as an optional checkbox in the Visual Studio Installer)

## How to connect a client automatically to the server

Your built game client will likely be __ThirdPersonMP.exe__ and be located in or near your earlier project. If you did not use ThirdPersonMP as a starting template, then your built game client will be <YourProjectName>.exe, similarly located in or near your Unreal project. Find this file.

* Find ```ThirdPersonMP.exe```
* Create a new adjacent file ```RunCloud.bat```
* Open this file in a text editor of your choice, and set the following contents:

```Batch
ThirdPersonMP <IP-ADDRESS>:<PORT> -log
```

* For now, we do not yet know the IP-ADDRESS or PORT, so just save, and leave the editor open.
* Find the file ```{MpsSamples Repo}/MpsAllocatorSample/MpsAllocatorSample.csproj```
* Open this file in Visual Studio
* Run this program
* Follow the on-screen instructions, and input the following values when prompted:
    * <Your PlayFab TitleId>
	* <Any valid and active secret key for your PlayFab Title>
	* Choose __1__ to "RequestMultiplayerServer"
	* Enter the <BuildId> for the __Deployed__-Status server build you wish to test
	    * This can be found next to the name in the list of all builds, in the __Details__ tab of the specific build, or in the URL for any of the build specific pages
    * If you have multiple regions defined, you'll be prompted to select one. Otherwise, it will auto-select if there is only 1 available region.
	* For Player IDs, you can type real, or fake hex-number value
	    * For users who followed the ThirdPersonMP setup, you can pick any hex-number, such as 1337, abc123, or etc.
		* Input at least 1 number, and then hit enter an extra time to finish
* You should receive a response that looks like this:

```json
{
  "ConnectedPlayers": [],
  "FQDN": "...",
  "IPV4Address": "52.180.68.101",
  "LastStateTransitionTime": "2021-12-06T22:12:46.212Z",
  "Ports": [
    {
      "Name": "UnrealServerGsdkHostPort",
      "Num": 30000,
      "Protocol": "UDP"
    }
  ],
  "Region": "WestUs",
  "ServerId": "...",
  "SessionId": "...",
  "State": "Active",
  "VmId": "..."
}
```

* Values unimportant to this example have been replaced with ```...```
* Look for __IPV4Address__ and __Ports.Num__ in your response
	* In the example above, these values are: 52.180.68.101 and 30000 respectively
	* Your values can/will be different
* Return to your ```RunCloud.bat``` file, and fill in these values for IP-ADDRESS and PORT. Example:

```Batch
ThirdPersonMP 52.180.68.101:30000 -log
```

* From windows explorer, double click this .bat file, to run the game client, wait a moment, and then double click it again
* This should start two instances of your game client, both connected to your cloud hosted game server
* From here, you can run around in both games, and you should see that both users movements and actions are reflected in both windows, for both players
	* SUCCESS! If you see that, you've completed the full Setup/Build/Deploy/Connect process

# Navigation

For some users, this guide sequence is now finished. You can return to the main [Unreal GSDK Plugin](README.md) guide.

If your test was run on Windows servers, then you can try the more advanced [Linux Server](LinuxBuildsUE5.md) deployment guide.
