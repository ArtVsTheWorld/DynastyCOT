using System.Collections.Generic;
namespace DynastyPlayClock
{
    public sealed partial class DynastyPlayClockAction
    {
        private static Layout[] GetLayouts()
        {
            return new Layout[] {
                new Layout("September 2026", Regions, RuntimeOriginal, new Dictionary<long, long>()),
                new Layout("November 19, 2026 update", new Region[] {
                    new Region("Clock list", 0x79976d0, 0x15c0, "8df427414dbc6de1b99dda566d00de2e9a20b8f2e68ed7f494646aacd47c4123"),
                    new Region("Dynasty writer", 0x79a5cc0, 0x2d4d, "c4d14b4bd11d2f9998115a3c2cf78f41f76bc281c53845bd5d3ddb82dedb8473"),
                    new Region("Choice serializer", 0x799faa0, 0x2a2, "5d30cb50c5cd3afa720e764b01b8e11c3c1bb32f70077b64f3df75899d8735e0"),
                    new Region("Profile accessor", 0x27a8710, 0x42, "e73de0614979d9e7796531aa360e8f945803baff2e0082c1bca433be75f507eb"),
                    new Region("Main-menu enum getter", 0x7f338d0, 0x4f, "28c890f4aff84d5b83ac35df6f0f7ade4f9ad6feb573d47c8cd6177bd5974ecf"),
                    new Region("Main-menu enum setter", 0x7f440a0, 0x54, "c37b3a5bfedda67aa19081b6a79de5a700c0adfa40208d2c3217e97feb91ae37"),
                    new Region("Profile getter", 0x27aa910, 0x36, "e95b362535e731df8bb563040224cdffb487cce91e4a97dd9f574bdd6a4abdeb"),
                    new Region("Profile setter", 0x27b34f0, 0x49, "b95b74998860536576cc496ba479242024307f9d70d62b23f1e891bbf4bbe083"),
                    new Region("Enum metadata", 0x5cf9470, 0x1d, "d6e2c6afe13f1c52203ded2ada6ece95ed230517ba369959a920479c2cf8e7b4"),
                    new Region("Native Play Clock decision", 0x35e5b3e, 0x12f, "e61ac46195de811da145babf70bb102822845e782b32ccf443cf05fa0b95aec6"),
                    new Region("Remaining-clock setter", 0x5d061f0, 0x99, "6d7f0835b2c07ddcb11c76a224e74f46ffecb942a824f5833581eebb209c140e"),
                    new Region("Clock transition dispatcher", 0x64598d0, 0x3c, "3d63926963ea64ee1ff059bb65db9a77aa434a8789a78f8862c75af028d34876"),
                    new Region("Clock transition reset", 0x646d670, 0x81, "a6531242282583ece301d958c10fcf864b680109bb274b23db0ffee3328640e5"),
                    new Region("CPU snap cadence", 0x360b670, 0xcc, "45cc3b1e997667d483d3dee476f67c04fd70b67c274b8a0b72117b95c40b9787"),
                    new Region("Play-clock upper limit", 0x5d879da, 0x31, "490b8e23f4f5145dca0576c360ea63fa9a027e22b690521e6e4b348f3cdf3972"),
                    new Region("Clock duration reset", 0x5d05ea0, 0x9e, "4b64a3260df8d79da3373b293f74daa4729ecabf0f65998ed21c1ea290e5d0b3"),
                    new Region("Match enum getter", 0x5cf98c0, 0x10, "4a94c23be822b90127b5ce6c14f9a13ddcda763bd3d4442ae3382f410e3d6792"),
                    new Region("Match type predicate", 0x3002a30, 0x10, "18297b2b90f0fadca9b12d6188e65cb8f3dba8b07f351f097e0f292a57f437cd"),
                    new Region("Legacy league mode predicate", 0x7ee5be0, 0x45, "9337f5162f1d4eae3c66bddf942cece2303e4a4371342537b15f87540eb5b140"),
                }, Hex("488b0d6798ce0aba01000000e82d3d7102"), new Dictionary<long, long> {
                    { 0x79a76d0, 0x79976d0 },
                    { 0x79a79ce, 0x79979ce },
                    { 0x79a7a9c, 0x7997a9c },
                    { 0x79b5d53, 0x79a5d53 },
                    { 0x79b850d, 0x79a850d },
                    { 0x79afaa0, 0x799faa0 },
                    { 0x27ac630, 0x27a8710 },
                    { 0xac9aed0, 0xad960f0 },
                    { 0x35e7592, 0x35e5b82 },
                    { 0xe1963e0, 0xe2cf3f0 },
                    { 0x5d0e630, 0x5d061f0 },
                    { 0x6461440, 0x64598f0 },
                    { 0x360d0b0, 0x360b6b0 },
                    { 0x5d8fc30, 0x5d879da },
                    { 0x5d0db10, 0x5d056d0 },
                    { 0x5d0e2e0, 0x5d05ea0 },
                    { 0xf7261f8, 0xf85f308 },
                    { 0x5d01d50, 0x5cf98c0 },
                    { 0xd8b4808, 0xd9eb808 },
                    { 0xf2c4d48, 0xf3fddbc },
                    { 0xe1963d0, 0xe2cf3e0 },
                    { 0xe1963d8, 0xe2cf3e8 },
                    { 0xb67e30c, 0xb77929c },
                })
            };
        }
        private sealed class Layout
        {
            public readonly string Name;
            public readonly Region[] Regions;
            public readonly byte[] RuntimeOriginal;
            private readonly Dictionary<long, long> addresses;
            public Layout(string name, Region[] regions, byte[] runtimeOriginal, Dictionary<long, long> addresses)
            { Name = name; Regions = regions; RuntimeOriginal = runtimeOriginal; this.addresses = addresses; }
            public long Map(long original) { long mapped; return addresses.TryGetValue(original, out mapped) ? mapped : original; }
        }
    }
}
