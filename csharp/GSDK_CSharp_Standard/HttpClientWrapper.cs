namespace Microsoft.Playfab.Gaming.GSDK.CSharp
{
    using System;
    using System.Net.Http;
    using System.Text;
    using System.Threading.Tasks;
    using Model;
    using Newtonsoft.Json;

    internal interface IHttpClient
    {
        Task<HeartbeatResponse> SendHeartbeatAsync(HeartbeatRequest request);
        Task SendInfoAsync(string url);
    }

    internal class HttpClientWrapper : IHttpClient
    {
        private readonly string _baseUrl;
        private readonly HttpClient _client;

        public HttpClientWrapper(string baseUrl)
        {
            _baseUrl = baseUrl;
            _client = new HttpClient();

            _client.DefaultRequestHeaders.Clear();
            _client.DefaultRequestHeaders.Add("Accept", "application/json");
            _client.DefaultRequestHeaders.Add("User-Agent", "Microsoft PlayFab Game SDK");
        }

        public async Task<HeartbeatResponse> SendHeartbeatAsync(HeartbeatRequest request)
        {
            string formattedText = JsonConvert.SerializeObject(request, Formatting.Indented);
            HttpRequestMessage requestMessage = new HttpRequestMessage
            {
                Method = new HttpMethod("PATCH"),
                RequestUri = new Uri(_baseUrl),
                Content = new StringContent(formattedText, Encoding.UTF8, "application/json")
            };

            HttpResponseMessage responseMessage = await _client.SendAsync(requestMessage);

            responseMessage.EnsureSuccessStatusCode();

            HeartbeatResponse response = JsonConvert.DeserializeObject<HeartbeatResponse>(
                await responseMessage.Content.ReadAsStringAsync());

            return response;
        }

        public async Task SendInfoAsync(string url)
        {
            string formattedText = JsonConvert.SerializeObject(new GSDKInfo(), Formatting.Indented);
            HttpRequestMessage requestMessage = new HttpRequestMessage
            {
                Method = new HttpMethod("POST"),
                RequestUri = new Uri(url),
                Content = new StringContent(formattedText, Encoding.UTF8, "application/json")
            };

            HttpResponseMessage responseMessage = await _client.SendAsync(requestMessage);

            responseMessage.EnsureSuccessStatusCode();
        }
    }
}