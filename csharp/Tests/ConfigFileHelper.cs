using Newtonsoft.Json;
using System;
using System.IO;
using System.Threading.Tasks;

namespace Microsoft.Playfab.Gaming.GSDK.CSharp.Test
{
    class ConfigFileWrapper
    {
        private ConfigFileWrapper() { }

        public async static Task WrapAsync<T>(T objRef, Func<string,Task> doWorkAsync)
        {
            // TODO: Is this necessary?
            string testConfigFileName = Path.ChangeExtension(Path.GetTempFileName(), ".json");

            using (var writer = new StreamWriter(testConfigFileName))
            {
                writer.WriteLine(JsonConvert.SerializeObject(objRef, Formatting.Indented));
                writer.Flush();
            }

            try
            {
                await doWorkAsync(testConfigFileName);
            }
            catch (Exception)
            {
                using (var reader = new StreamReader(testConfigFileName))
                {
                    Console.WriteLine();
                    Console.WriteLine("*******************************");
                    Console.WriteLine("  CONFIGURATION FILE CONTENTS  ");
                    Console.WriteLine("*******************************");
                    Console.WriteLine(reader.ReadToEnd());
                    Console.WriteLine();
                    Console.WriteLine();
                }

                throw;
            }
            finally
            {
                try
                {
                    File.Delete(testConfigFileName);
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Could not delete the temp file {testConfigFileName}: \r\n\r\n{ex}");
                }
            }
        }
    }
}