using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using System;
using System.Collections.Generic;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Playfab.Gaming.GSDK.CSharp
{
    class HttpClientProxy : HttpClient, IHttpClient
    {
        private string _baseUrl;
        private HttpClient _client;

        public HttpClientProxy(string baseUrl)
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
            var requestMessage = new HttpRequestMessage
            {
                Method = new HttpMethod("PATCH"),
                RequestUri = new Uri(_baseUrl),
                Content = new StringContent(formattedText, Encoding.UTF8, "application/json"),
            };

            HttpResponseMessage responseMessage = await _client.SendAsync(requestMessage);

            responseMessage.EnsureSuccessStatusCode();

            HeartbeatResponse response = JsonConvert.DeserializeObject<HeartbeatResponse>(
                await responseMessage.Content.ReadAsStringAsync());

            return response;
        }
    }
}
