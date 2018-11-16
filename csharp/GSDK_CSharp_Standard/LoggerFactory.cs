namespace Microsoft.Playfab.Gaming.GSDK.CSharp
{

    public interface ILogger
    {
        void Start();
        void Stop();
        void Log(string message);
    }

    public static class LoggerFactory
    {
        public static ILogger CreateInstance(string logFolder)
        {
            return new FilesystemLogger(logFolder);
        }
    }
}