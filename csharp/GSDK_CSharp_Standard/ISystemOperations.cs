namespace Microsoft.Playfab.Gaming.GSDK.CSharp
{
    using System;
    using System.IO;

    public interface ISystemOperations
    {
        string FileReadAllText(string filename);

        bool FileExists(string filename);

        string GetEnvironmentVariableValue(string variable);
    }

    public class SystemOperations : ISystemOperations
    {
        public static SystemOperations Instance { get; } = new SystemOperations();

        private SystemOperations()
        {
        }

        public string FileReadAllText(string filename)
        {
            return File.ReadAllText(filename);
        }

        public bool FileExists(string filename)
        {
            return File.Exists(filename);
        }

        public string GetEnvironmentVariableValue(string variable)
        {
            return Environment.GetEnvironmentVariable(variable);
        }
    }
}