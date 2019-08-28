namespace PlayFab.MultiplayerAgent.Model
{
    using System.Collections.Generic;
    using Json;

    public class GameServerConnectionInfo
    {
        public GameServerConnectionInfo()
        {
        }

        [JsonProperty(PropertyName = "publicIpV4Adress")]
        public string PublicIpV4Adress { get; set; }

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