// Copyright (C) Microsoft Corporation. All rights reserved.

using System.Threading.Tasks;
using PlayFab;
using PlayFab.AuthenticationModels;
using PlayFab.MultiplayerModels;
using PlayFab.ServerModels;
using EntityKey = PlayFab.AuthenticationModels.EntityKey;

namespace WinTestRunnerGame
{
    using Microsoft.Playfab.Gaming.GSDK.CSharp;
    using Newtonsoft.Json;
    using System;
    using System.Collections.Generic;
    using System.Configuration;
    using System.IO;
    using System.Linq;
    using System.Net;
    using System.Security.Cryptography.X509Certificates;
    using System.Threading;

    public class SessionCookie
    {
        /// <summary>
        /// For test purposes only, do not actually send your secret key in the session cookie
        /// </summary>
        public string SecretKey { get; set; }
        
        /// <summary>
        /// The time before automatically shutting down
        /// </summary>
        public long TimeoutSecs { get; set; }
    }

    class Program
    {
        const int ListeningPort = 3600;
        const string AssetFilePath = @"C:\Assets\testassetfile.txt";
        const string SessionTimeoutInSecondsName = "SessionTimeoutInSeconds";

        private static readonly HttpListener _listener = new HttpListener();
        private static bool _isActivated = false;
        private static bool _isShutdown = false;
        private static bool _delayShutdown = false;
        private static string _assetFileText = String.Empty;
        private static string _installedCertThumbprint = String.Empty;
        private static string _cmdArgs = String.Empty;
        private static List<ConnectedPlayer> players = new List<ConnectedPlayer>();
        private static int requestCount = 0;
        private static DateTimeOffset _nextMaintenance = DateTimeOffset.MinValue;
        static DateTimeOffset _sessionTimeoutTimestamp = DateTimeOffset.MaxValue;
        private const string TimeoutSessionCookiePrefix = "timeoutsecs=";

        private static void LogMessage(string message)
        {
            GameserverSDK.LogMessage(message);
            Console.WriteLine(message);
        }
        
        static void OnShutdown()
        {
            LogMessage("Shutting down...");
            _isShutdown = true;

            if (!_delayShutdown)
            {
                _listener.Stop();
                _listener.Close();
            }
        }

        static bool GetGameHealth()
        {
            // Flip so we can test both
            return requestCount % 2 == 0;
        }

        static void OnMaintenanceScheduled(DateTimeOffset time)
        {
            _nextMaintenance = time;
        }
        
        

        static void Main(string[] args)
        {
            if (args != null && args.Length > 0)
            {
                _cmdArgs = string.Join(" ", args);
            }

            string address = $"http://*:{ListeningPort}/";
            _listener.Prefixes.Add(address);
            _listener.Start();

            GameserverSDK.Start(true);
            GameserverSDK.RegisterShutdownCallback(OnShutdown);
            GameserverSDK.RegisterHealthCallback(GetGameHealth);
            GameserverSDK.RegisterMaintenanceCallback(OnMaintenanceScheduled);
            

            if (File.Exists(AssetFilePath))
            {
                _assetFileText = File.ReadAllText(AssetFilePath);
            }

            // See if we have a configured timeout in the App.config, this was
            // used in our initial round of stress tests to make sure sessions ended
            string timeoutstr = ConfigurationManager.AppSettings.Get(SessionTimeoutInSecondsName);
            if (!string.IsNullOrEmpty(timeoutstr))
            {
                // failure to convert is intentionally unhandled
                long timeoutint = Convert.ToInt64(timeoutstr.Trim());

                _sessionTimeoutTimestamp = DateTimeOffset.Now.AddSeconds(timeoutint);
            }

            IDictionary<string, string> initialConfig = GameserverSDK.getConfigSettings();
            if (initialConfig?.ContainsKey("winRunnerTestCert") == true)
            {
                string expectedThumbprint = initialConfig["winRunnerTestCert"];
                X509Store store = new X509Store(StoreName.My, StoreLocation.LocalMachine);
                store.Open(OpenFlags.ReadOnly);
                X509Certificate2Collection certificateCollection = store.Certificates.Find(X509FindType.FindByThumbprint, expectedThumbprint, false);

                if (certificateCollection.Count > 0)
                {
                    _installedCertThumbprint = certificateCollection[0].Thumbprint;
                }
                else
                {
                    LogMessage("Could not find installed game cert in LocalMachine\\My. Expected thumbprint is: " + expectedThumbprint);
                }
            }
            else
            {
                LogMessage("Config did not contain cert! Config is: " + string.Join(";", initialConfig.Select(x => x.Key + "=" + x.Value)));
            }

            Thread t = new Thread(ProcessRequests);
            t.Start();


            string titleId = initialConfig[GameserverSDK.TitleIdKey];
            string buildId = initialConfig[GameserverSDK.BuildIdKey];
            string region = initialConfig[GameserverSDK.RegionKey];
            
            LogMessage($"Processing requests for title:{titleId} build:{buildId} in region {region}");

            GameserverSDK.ReadyForPlayers();
            _isActivated = true;

            initialConfig = GameserverSDK.getConfigSettings();

            LogMessage("Config Settings");
            foreach (KeyValuePair<string,string> configTuple in initialConfig)
            {
                LogMessage($"\t{configTuple.Key}={configTuple.Value}");
            }
            
            SessionCookie sessionCookie = new SessionCookie();

            if (initialConfig.TryGetValue(GameserverSDK.SessionCookieKey, out string sessionCookieStr))
            {
                if (sessionCookieStr.StartsWith(TimeoutSessionCookiePrefix, StringComparison.InvariantCultureIgnoreCase))
                {
                    if (long.TryParse(sessionCookieStr.Substring(TimeoutSessionCookiePrefix.Length),
                        out long timeoutSecs))
                    {

                        sessionCookie.TimeoutSecs = timeoutSecs;
                    }
                }
                else
                {
                    sessionCookie = JsonConvert.DeserializeObject<SessionCookie>(sessionCookieStr);                    
                }
            }

            // If the session cookie contained a secret key
            // try to get the title data for this title
            if (!string.IsNullOrWhiteSpace(sessionCookie.SecretKey))
            {
                GetTitleData(titleId, sessionCookie).Wait();
            }

            // If the session cookie contained a timeout. Shutdown at this time.
            // this overrides whatever may have been set in the App config above
            // We use it for stress testing to make sessions end at random times.
            EnforceTimeout(sessionCookie);
        }

        private static async Task GetTitleData(string titleId, SessionCookie sessionCookie)
        {
            try
            {
                PlayFabSettings.TitleId = titleId;
                PlayFabSettings.DeveloperSecretKey = sessionCookie.SecretKey;

                var tokenRequest = new GetEntityTokenRequest() {Entity = new EntityKey() {Type = "Title"}};
                PlayFabResult<GetEntityTokenResponse> tokenResponse =
                    await PlayFabAuthenticationAPI.GetEntityTokenAsync(tokenRequest);
                
                if (tokenResponse.Error != null)
                {
                    LogMessage($"Error retrieving entity token: {tokenResponse.Error.Error.ToString()}: {tokenResponse.Error.ErrorMessage}");
                }
                
                PlayFabResult<GetTitleDataResult> titleData =
                    await PlayFabServerAPI.GetTitleDataAsync(new GetTitleDataRequest());

                if (titleData.Error != null)
                {
                    LogMessage($"Error retrieving title data: {titleData.Error.Error.ToString()}: {titleData.Error.ErrorMessage}");
                }
                else
                {
                    LogMessage("Title Data:");
                    foreach (KeyValuePair<string, string> titleDataTuple in titleData.Result.Data)
                    {
                        LogMessage($"\t{titleDataTuple.Key}={titleDataTuple.Value}");
                    }                    
                }

                

            }
            catch (Exception e)
            {
                LogMessage($"Exception calling for titleData {e}");
            }
        }

        private static void EnforceTimeout(SessionCookie sessionCookie)
        {
            if (sessionCookie.TimeoutSecs != 0)
            {
                _sessionTimeoutTimestamp = DateTimeOffset.Now.AddSeconds(sessionCookie.TimeoutSecs);
            }

            if (_sessionTimeoutTimestamp != DateTimeOffset.MaxValue)
            {
                while (!_isShutdown && _sessionTimeoutTimestamp > DateTimeOffset.Now)
                    Thread.Sleep(1000);

                // If we didn't shutdown yet, that means we timed out
                if (!_isShutdown)
                {
                    LogMessage($"Session Timeout exceeded at {DateTimeOffset.Now.ToString()}...");
                    OnShutdown();
                }
            }
        }

        /// <summary>
        /// Listens for any requests and responds with the game host's config values
        /// </summary>
        /// <param name="config">The config values we're returning</param>
        private static void ProcessRequests()
        {
            while (_listener.IsListening)
            {
                try
                {
                    HttpListenerContext context = _listener.GetContext();
                    HttpListenerRequest request = context.Request;
                    HttpListenerResponse response = context.Response;

                    LogMessage(string.Format("HTTP:Received {0}", request.Headers.ToString()));

                    IDictionary<string, string> config = null;

                    // For each request, "add" a connected player, but limit player count to 20.
                    const int maxPlayers = 20;
                    if (players.Count < maxPlayers)
                    {
                        players.Add(new ConnectedPlayer("gamer" + requestCount));
                    }
                    else
                    {
                        LogMessage($"Player not added since max of {maxPlayers} is reached. Current request count: {requestCount}.");
                    }

                    requestCount++;

                    GameserverSDK.UpdateConnectedPlayers(players);

                    config = GameserverSDK.getConfigSettings() ?? new Dictionary<string, string>();

                    // First, check if we need to delay shutdown for testing
                    if (config.TryGetValue(GameserverSDK.SessionCookieKey, out string sessionCookie) && sessionCookie.Equals("delayshutdown", StringComparison.OrdinalIgnoreCase))
                    {
                        _delayShutdown = true;
                    }

                    config.Add("isActivated", _isActivated.ToString());
                    config.Add("isShutdown", _isShutdown.ToString());
                    config.Add("assetFileText", _assetFileText);
                    config.Add("logsDirectory", GameserverSDK.GetLogsDirectory());
                    config.Add("sharedContentDirectory", GameserverSDK.GetSharedContentDirectory());
                    config.Add("installedCertThumbprint", _installedCertThumbprint);
                    config.Add("cmdArgs", _cmdArgs);

                    if (_nextMaintenance != DateTimeOffset.MinValue)
                    {
                        config.Add("nextMaintenance", _nextMaintenance.ToLocalTime().ToString());
                    }

                    string content = JsonConvert.SerializeObject(config, Formatting.Indented);

                    response.AddHeader("Content-Type", "application/json");
                    byte[] buffer = System.Text.Encoding.UTF8.GetBytes(content);
                    response.ContentLength64 = buffer.Length;
                    using (System.IO.Stream output = response.OutputStream)
                    {
                        output.Write(buffer, 0, buffer.Length);
                    }

                    // Once we're shut down, return a response one more time (so the tests can 
                    // verify the _isShutdown field) and then actually terminate
                    if (_isShutdown)
                    {
                        _listener.Stop();
                        _listener.Close();
                    }
                }
                catch (HttpListenerException httpEx)
                {
                    // This one is expected if we stopped the listener because we were asked to shutdown
                    LogMessage($"Got HttpListenerException: {httpEx.ToString()}, shutdown value is: {_isShutdown} ");
                }
                catch (Exception ex)
                {
                    LogMessage($"Got Exception: {ex.ToString()}");
                }
            }
        }
    }
}
