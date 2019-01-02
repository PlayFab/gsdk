// Copyright (C) Microsoft Corporation. All rights reserved.

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

    class Program
    {
        const int ListeningPort = 3600;
        const string AssetFilePath = @"C:\Assets\testassetfile.txt";
        const string SessionTimeoutInSecondsName = "SessionTimeoutInSeconds";

        static private HttpListener _listener = new HttpListener();
        static private bool _isActivated = false;
        static private bool _isShutdown = false;
        static private bool _delayShutdown = false;
        static private string _assetFileText = String.Empty;
        static private string _installedCertThumbprint = String.Empty;
        static private string _cmdArgs = String.Empty;
        static private List<ConnectedPlayer> players = new List<ConnectedPlayer>();
        static private int requestCount = 0;
        static private DateTimeOffset _nextMaintenance = DateTimeOffset.MinValue;
        static DateTimeOffset _sessionTimeoutTimestamp = DateTimeOffset.MaxValue;
        private const string TimeoutSessionCookiePrefix = "timeoutsecs=";

        static void OnShutdown()
        {
            GameserverSDK.LogMessage("Shutting down...");
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

            GameserverSDK.Start();
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
                    GameserverSDK.LogMessage("Could not find installed game cert in LocalMachine\\My. Expected thumbprint is: " + expectedThumbprint);
                }
            }
            else
            {
                GameserverSDK.LogMessage("Config did not contain cert! Config is: " + string.Join(";", initialConfig.Select(x => x.Key + "=" + x.Value)));
            }

            Thread t = new Thread(ProcessRequests);
            t.Start();

            GameserverSDK.ReadyForPlayers();
            _isActivated = true;

            // After allocation, see if the session cookie contained a timeout. We use it for stress testing to make sessions end at random times.
            // If set, this overrides whatever may have been set in the App config above
            initialConfig = GameserverSDK.getConfigSettings();

            if (initialConfig.TryGetValue(GameserverSDK.SessionCookieKey, out string sessionCookie) && sessionCookie.StartsWith(TimeoutSessionCookiePrefix, StringComparison.InvariantCultureIgnoreCase))
            {
                if (Int64.TryParse(sessionCookie.Substring(TimeoutSessionCookiePrefix.Length), out long timeoutsecs))
                {
                    _sessionTimeoutTimestamp = DateTimeOffset.Now.AddSeconds(timeoutsecs);
                }
            }

            if (_sessionTimeoutTimestamp != DateTimeOffset.MaxValue)
            {
                while (!_isShutdown && _sessionTimeoutTimestamp > DateTimeOffset.Now)
                    Thread.Sleep(1000);

                // If we didn't shutdown yet, that means we timed out
                if (!_isShutdown)
                {
                    GameserverSDK.LogMessage($"Session Timeout exceeded at {DateTimeOffset.Now.ToString()}...");
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

                    GameserverSDK.LogMessage(string.Format("HTTP:Received {0}", request.Headers.ToString()));

                    IDictionary<string, string> config = null;

                    // For each request, "add" a connected player
                    players.Add(new ConnectedPlayer("gamer" + requestCount));
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
                    GameserverSDK.LogMessage($"Got HttpListenerException: {httpEx.ToString()}, shutdown value is: {_isShutdown} ");
                }
                catch (Exception ex)
                {
                    GameserverSDK.LogMessage($"Got Exception: {ex.ToString()}");
                }
            }
        }
    }
}
