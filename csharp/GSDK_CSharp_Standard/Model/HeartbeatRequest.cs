namespace Microsoft.Playfab.Gaming.GSDK.CSharp.Model
{
    using Newtonsoft.Json;
    using Newtonsoft.Json.Converters;

    internal class HeartbeatRequest
    {
        [JsonConverter(typeof(StringEnumConverter))]
        public GameState CurrentGameState { get; set; }

        public string CurrentGameHealth { get; set; }

        public ConnectedPlayer[] CurrentPlayers { get; set; }
    }
}