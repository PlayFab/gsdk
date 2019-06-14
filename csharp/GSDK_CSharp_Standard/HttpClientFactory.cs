namespace Microsoft.Playfab.Gaming.GSDK.CSharp
{
    internal interface IHttpClientFactory
    {
        IHttpClient CreateInstance(string baseUrl);
    }

    internal class HttpClientFactory : IHttpClientFactory
    {
        public static HttpClientFactory Instance { get; } = new HttpClientFactory();

        private HttpClientFactory()
        {
        }
            
        public IHttpClient CreateInstance(string baseUrl)
        {
            return new HttpClientWrapper(baseUrl);
        }
    }
}