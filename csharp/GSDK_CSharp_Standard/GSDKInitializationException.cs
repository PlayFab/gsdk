using System;

namespace Microsoft.Playfab.Gaming.GSDK.CSharp
{
    public class GSDKInitializationException : Exception
    {
        public GSDKInitializationException(string message)
            : base(message) { }

        public GSDKInitializationException(string message, Exception innerException)
            : base(message, innerException) { }
    }
}