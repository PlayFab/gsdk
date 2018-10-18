namespace Microsoft.Playfab.Gaming.GSDK.CSharp
{
    class HealthCallback : interop_HealthCallback
    {
        Func<bool> callback;

        public HealthCallback(Func<bool> newCallback) : base()
        {
            this.callback = newCallback;
        }

        public override bool OnHealthCheck()
        {
            return callback();
        }
    }
}
