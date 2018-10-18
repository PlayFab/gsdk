namespace Microsoft.Playfab.Gaming.GSDK.CSharp
{
    class ShutdownCallback : interop_ShutdownCallback
    {
        Action callback;

        public ShutdownCallback(Action newCallback) : base()
        {
            this.callback = newCallback;
        }

        public override void OnShutdown()
        {
            callback();
        }
    }
}
