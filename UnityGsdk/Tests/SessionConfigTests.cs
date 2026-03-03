namespace PlayFab.MultiplayerAgent.Tests
{
    using System.Collections.Generic;
    using Model;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class SessionConfigTests
    {
        [TestMethod]
        public void CopyNonNullFields_NullInput_NoChange()
        {
            var target = new SessionConfig
            {
                SessionId = "original",
                SessionCookie = "cookie"
            };

            target.CopyNonNullFields(null);

            Assert.AreEqual("original", target.SessionId);
            Assert.AreEqual("cookie", target.SessionCookie);
        }

        [TestMethod]
        public void CopyNonNullFields_AllFieldsSet_Copied()
        {
            var target = new SessionConfig();
            var source = new SessionConfig
            {
                SessionId = "newId",
                SessionCookie = "newCookie",
                InitialPlayers = new List<string> { "player1", "player2" },
                Metadata = new Dictionary<string, string> { { "key1", "value1" } }
            };

            target.CopyNonNullFields(source);

            Assert.AreEqual("newId", target.SessionId);
            Assert.AreEqual("newCookie", target.SessionCookie);
            Assert.AreEqual(2, target.InitialPlayers.Count);
            Assert.AreEqual("player1", target.InitialPlayers[0]);
            Assert.AreEqual("player2", target.InitialPlayers[1]);
            Assert.AreEqual("value1", target.Metadata["key1"]);
        }

        [TestMethod]
        public void CopyNonNullFields_PartialFields_OnlyNonNullCopied()
        {
            var target = new SessionConfig
            {
                SessionId = "original",
                SessionCookie = "originalCookie"
            };

            var source = new SessionConfig
            {
                SessionId = "newId"
            };

            target.CopyNonNullFields(source);

            Assert.AreEqual("newId", target.SessionId);
            Assert.AreEqual("originalCookie", target.SessionCookie);
        }

        [TestMethod]
        public void CopyNonNullFields_EmptyPlayers_NotCopied()
        {
            var target = new SessionConfig
            {
                InitialPlayers = new List<string> { "existingPlayer" }
            };

            var source = new SessionConfig
            {
                InitialPlayers = new List<string>()
            };

            target.CopyNonNullFields(source);

            Assert.AreEqual(1, target.InitialPlayers.Count);
            Assert.AreEqual("existingPlayer", target.InitialPlayers[0]);
        }

        [TestMethod]
        public void CopyNonNullFields_EmptyMetadata_NotCopied()
        {
            var target = new SessionConfig
            {
                Metadata = new Dictionary<string, string> { { "existingKey", "existingValue" } }
            };

            var source = new SessionConfig
            {
                Metadata = new Dictionary<string, string>()
            };

            target.CopyNonNullFields(source);

            Assert.AreEqual(1, target.Metadata.Count);
            Assert.AreEqual("existingValue", target.Metadata["existingKey"]);
        }

        [TestMethod]
        public void Equals_SameValues_ReturnsTrue()
        {
            var config1 = new SessionConfig
            {
                SessionId = "id1",
                SessionCookie = "cookie1"
            };

            var config2 = new SessionConfig
            {
                SessionId = "id1",
                SessionCookie = "cookie1"
            };

            Assert.IsTrue(config1.Equals(config2));
            Assert.IsTrue(config1 == config2);
        }

        [TestMethod]
        public void Equals_DifferentValues_ReturnsFalse()
        {
            var config1 = new SessionConfig
            {
                SessionId = "id1",
                SessionCookie = "cookie1"
            };

            var config2 = new SessionConfig
            {
                SessionId = "id2",
                SessionCookie = "cookie2"
            };

            Assert.IsFalse(config1.Equals(config2));
            Assert.IsTrue(config1 != config2);
        }

        [TestMethod]
        public void Equals_NullComparison_ReturnsFalse()
        {
            var config = new SessionConfig
            {
                SessionId = "id1"
            };

            Assert.IsFalse(config.Equals(null));
        }

        [TestMethod]
        public void GetHashCode_SameValues_SameHash()
        {
            var config1 = new SessionConfig
            {
                SessionId = "id1",
                SessionCookie = "cookie1"
            };

            var config2 = new SessionConfig
            {
                SessionId = "id1",
                SessionCookie = "cookie1"
            };

            Assert.AreEqual(config1.GetHashCode(), config2.GetHashCode());
        }
    }
}
