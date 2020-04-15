namespace PlayFab.MultiplayerAgent.Helpers
{
    using System;
    using System.Globalization;
    using System.Reflection;

    public class SimpleJsonInstance
    {
        /// <summary>
        /// Most users shouldn't access this
        /// JsonWrapper.Serialize, and JsonWrapper.Deserialize will always use it automatically (Unless you deliberately mess with them)
        /// Any Serialization of an object in the PlayFab namespace should just use JsonWrapper
        /// </summary>
        public static PlayFabSimpleJsonCustomization ApiSerializerStrategy = new PlayFabSimpleJsonCustomization();
        public class PlayFabSimpleJsonCustomization : PocoJsonSerializerStrategy
        {
            /// <summary>
            /// Convert the json value into the destination field/property
            /// </summary>
            public override object DeserializeObject(object value, Type type)
            {
                var valueStr = value as string;
                if (valueStr == null) // For all of our custom conversions, value is a string
                    return base.DeserializeObject(value, type);

                var underType = Nullable.GetUnderlyingType(type);
                if (underType != null)
                    return DeserializeObject(value, underType);
                else if (type.GetTypeInfo().IsEnum)
                    return Enum.Parse(type, (string)value, true);
                else if (type == typeof(DateTime))
                {
                    DateTime output;
                    var result = DateTime.TryParseExact(valueStr, DefaultDateTimeFormats, CultureInfo.InvariantCulture, _dateTimeStyles, out output);
                    if (result)
                        return output;
                }
                else if (type == typeof(DateTimeOffset))
                {
                    DateTimeOffset output;
                    var result = DateTimeOffset.TryParseExact(valueStr, DefaultDateTimeFormats, CultureInfo.InvariantCulture, _dateTimeStyles, out output);
                    if (result)
                        return output;
                }
                else if (type == typeof(TimeSpan))
                {
                    double seconds;
                    if (double.TryParse(valueStr, out seconds))
                        return TimeSpan.FromSeconds(seconds);
                }
                return base.DeserializeObject(value, type);
            }

            /// <summary>
            /// Set output to a string that represents the input object
            /// </summary>
            protected override bool TrySerializeKnownTypes(object input, out object output)
            {
                if (input.GetType().GetTypeInfo().IsEnum)
                {
                    output = input.ToString();
                    return true;
                }
                else if (input is DateTime)
                {
                    output = ((DateTime)input).ToString(DefaultDateTimeFormats[DEFAULT_UTC_OUTPUT_INDEX], CultureInfo.InvariantCulture);
                    return true;
                }
                else if (input is DateTimeOffset)
                {
                    output = ((DateTimeOffset)input).ToString(DefaultDateTimeFormats[DEFAULT_UTC_OUTPUT_INDEX], CultureInfo.InvariantCulture);
                    return true;
                }
                else if (input is TimeSpan)
                {
                    output = ((TimeSpan)input).TotalSeconds;
                    return true;
                }
                return base.TrySerializeKnownTypes(input, out output);
            }
            
            public static readonly string[] DefaultDateTimeFormats = new string[]{ // All parseable ISO 8601 formats for DateTime.[Try]ParseExact - Lets us deserialize any legacy timestamps in one of these formats
                // These are the standard format with ISO 8601 UTC markers (T/Z)
                "yyyy-MM-ddTHH:mm:ss.FFFFFFZ",
                "yyyy-MM-ddTHH:mm:ss.FFFFZ",
                "yyyy-MM-ddTHH:mm:ss.FFFZ", // DEFAULT_UTC_OUTPUT_INDEX
                "yyyy-MM-ddTHH:mm:ss.FFZ",
                "yyyy-MM-ddTHH:mm:ssZ",

                // These are the standard format without ISO 8601 UTC markers (T/Z)
                "yyyy-MM-dd HH:mm:ss.FFFFFF",
                "yyyy-MM-dd HH:mm:ss.FFFF",
                "yyyy-MM-dd HH:mm:ss.FFF",
                "yyyy-MM-dd HH:mm:ss.FF", // DEFAULT_LOCAL_OUTPUT_INDEX
                "yyyy-MM-dd HH:mm:ss",

                // These are the result of an input bug, which we now have to support as long as the db has entries formatted like this
                "yyyy-MM-dd HH:mm.ss.FFFF",
                "yyyy-MM-dd HH:mm.ss.FFF",
                "yyyy-MM-dd HH:mm.ss.FF",
                "yyyy-MM-dd HH:mm.ss",
            };

            public const int DEFAULT_UTC_OUTPUT_INDEX = 2; // The default format everybody should use
            private static DateTimeStyles _dateTimeStyles = DateTimeStyles.RoundtripKind;
        }

        public T DeserializeObject<T>(string json)
        {
            return PlayFabSimpleJson.DeserializeObject<T>(json, ApiSerializerStrategy);
        }

        public T DeserializeObject<T>(string json, object jsonSerializerStrategy)
        {
            return PlayFabSimpleJson.DeserializeObject<T>(json, (IJsonSerializerStrategy)jsonSerializerStrategy);
        }

        public object DeserializeObject(string json)
        {
            return PlayFabSimpleJson.DeserializeObject(json, typeof(object), ApiSerializerStrategy);
        }

        public string SerializeObject(object json)
        {
            return PlayFabSimpleJson.SerializeObject(json, ApiSerializerStrategy);
        }

        public string SerializeObject(object json, object jsonSerializerStrategy)
        {
            return PlayFabSimpleJson.SerializeObject(json, (IJsonSerializerStrategy)jsonSerializerStrategy);
        }
    }
}