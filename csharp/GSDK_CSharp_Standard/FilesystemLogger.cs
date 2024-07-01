namespace Microsoft.Playfab.Gaming.GSDK.CSharp
{
    using System;
    using System.IO;

    public class FileSystemLogger : ILogger
    {
        private string _logFolder;
        private StreamWriter _logWriter;

        public FileSystemLogger(string logFolder)
        {
            _logFolder = logFolder;
        }

        public void Log(string message)
        {
            if (_logWriter == null)
            {
                return; // Logging is disabled
            }

            _logWriter.WriteLine($"{DateTime.UtcNow:o}\t{message}");
            _logWriter.Flush();
        }

        public void Start()
        {
            if (_logWriter != null)
            {
                return;
            }

            string currentDirectory = Directory.GetCurrentDirectory();
            if (string.IsNullOrWhiteSpace(_logFolder))
            {
                _logFolder = currentDirectory;
            }

            try
            {
                if (!Directory.Exists(_logFolder))
                {
                    Directory.CreateDirectory(_logFolder);
                }
            }
            catch (Exception ex)
            {
                _logFolder = currentDirectory;
                throw ex;
            }

            long datePart = DateTime.UtcNow.ToFileTime();
            string fileName = Path.Combine(_logFolder, $"GSDK_output_{datePart}.txt");
            FileStream fileStream = File.Open(fileName, FileMode.OpenOrCreate, FileAccess.Write, FileShare.Read);
            _logWriter = new StreamWriter(fileStream);
        }
    }
}