using System;
using System.Collections.Generic;
using System.Text;

namespace Microsoft.Playfab.Gaming.GSDK.CSharp
{
    class MaintenanceCallback : interop_MaintenanceCallback
    {
        static readonly DateTime UnixStartTimeUtc = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);

        Action<DateTimeOffset> callback;

        public MaintenanceCallback(Action<DateTimeOffset> newCallback) : base()
        {
            this.callback = newCallback;
        }

        public override void OnMaintenanceScheduled(long newTimeUtc)
        {
            // Convert UTC time_t to UTC DateTime
            this.callback(new DateTimeOffset(UnixStartTimeUtc.AddSeconds(newTimeUtc)));
        }
    }
}
