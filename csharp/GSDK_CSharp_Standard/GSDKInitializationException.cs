namespace Microsoft.Playfab.Gaming.GSDK.CSharp
{
    using System;

    public class GSDKInitializationException : Exception
    {
        public GSDKInitializationException(string message)
            : base(message)
        {
        }

        public GSDKInitializationException(string message, Exception innerException)
            : base(message, innerException)
        {
        }
    }
}