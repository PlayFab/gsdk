
using System;
using System.IO;

namespace Microsoft.Playfab.Gaming.GSDK.CSharp
{
    using System.Reflection;

    public class FilesystemLogger : ILogger
    {
        private StreamWriter _logWriter;
        private string _logFolder;

        public FilesystemLogger(string logFolder)
        {
            _logFolder = logFolder;
        }

        public void Log(string message)
        {
            if (_logWriter == null)
            {
                return; // Logging is disabled
            }

            _logWriter.WriteLine($"{DateTime.UtcNow.ToString("o")}\t{message}");
            _logWriter.Flush();
        }

        public void Start()
        {
            if (_logWriter != null)
            {
                return;
            }

            // Workaround to enable .Net 4.5 and netstandard1.6 (instead of using Environment.CurrentDirectory).
            string currentDirectory = typeof(FilesystemLogger).GetTypeInfo().Assembly.Location;
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
            _logWriter = new StreamWriter(File.OpenWrite(fileName));
        }

        public void Stop()
        {
            throw new System.NotImplementedException();
        }
    }
}