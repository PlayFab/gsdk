namespace PlayFab.MultiplayerAgent.Model
{
    using System;
    using System.Collections.Generic;
    using Helpers;

    /// <summary>
    /// A class that captures details on how a game server operates.
    /// </summary>
    public class GameServerConnectionInfo
    {
        public GameServerConnectionInfo()
        {
        }

        /// <summary>
        /// The IPv4 address of the game server.
        /// </summary>
        [JsonProperty(PropertyName = "publicIpV4Address")]
        public string PublicIPv4Address { get; set; }


        [Obsolete("Please use PublicIPv4Address instead.")]
        [JsonProperty(PropertyName = "publicIpV4Adress")]
        public string PublicIpV4Adress { get => PublicIPv4Address; set { if (!string.IsNullOrWhiteSpace(value) && PublicIPv4Address != value) { PublicIPv4Address = value; } } }

        /// <summary>
        /// The ports configured for the game server.
        /// </summary>
        [JsonProperty(PropertyName = "gamePortsConfiguration")]
        public IEnumerable<GamePort> GamePortsConfiguration { get; set; }
    }

    /// <summary>
    /// A class that captures details about a game server port.
    /// </summary>
    public class GamePort
    {
        public GamePort()
        {
        }

        /// <summary>
        /// The friendly name / identifier for the port, specified by the game developer in the Build configuration.
        /// </summary>
        [JsonProperty(PropertyName = "name")]
        public string Name { get; set; }

        /// <summary>
        /// The port at which the game server should listen on (maps externally to <see cref="ClientConnectionPort" />).
        /// For process based servers, this is determined by Control Plane, based on the ports available on the VM.
        /// For containers, this is specified by the game developer in the Build configuration.
        /// </summary>
        [JsonProperty(PropertyName = "serverListeningPort")]
        public int ServerListeningPort { get; set; }

        /// <summary>
        /// The public port to which clients should connect (maps internally to <see cref="ServerListeningPort" />).
        /// </summary>
        [JsonProperty(PropertyName = "clientConnectionPort")]
        public int ClientConnectionPort { get; set; }
    }
}