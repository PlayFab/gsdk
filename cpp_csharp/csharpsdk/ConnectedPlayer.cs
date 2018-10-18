using System;

namespace Microsoft.Playfab.Gaming.GSDK.CSharp
{
    public class ConnectedPlayer
    {
        public string PlayerId { get; set; }

        public ConnectedPlayer(string playerid)
        {
            this.PlayerId = playerid;
        }
    }
}
