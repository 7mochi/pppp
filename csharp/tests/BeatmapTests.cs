using System;
using System.IO;
using Pppp;
using Xunit;

namespace Pppp.Tests {
    public sealed class BeatmapTests {
        [Fact]
        public void VersionIsAVersion() {
            string version = VersionInfo.Current;
            Assert.NotNull(version);
            Assert.Equal(3, version.Split('.').Length);
        }

        [Fact]
        public void BackendReportsTheEngine() {
            string name = Backend.Name;
            Assert.Contains(name, new[] { "scalar", "avx2", "neon" });
        }

        [Fact]
        public void FromFileReadsMetadata() {
            using (Beatmap beatmap = Beatmap.FromFile(TestResources.MapPath("osu/2785319.osu"))) {
                Assert.Equal("re[in]flaw", beatmap.Title);
                Assert.Equal("MYUKKE.", beatmap.Artist);
                Assert.Equal("toybot's Expert", beatmap.Version);
                Assert.Equal(2785319, beatmap.BeatmapId);
                Assert.Equal(14, beatmap.FormatVersion);
                Assert.Equal(0, beatmap.Mode);
                Assert.Equal(0.5, beatmap.StackLeniency);
                Assert.Equal(4.5, beatmap.Difficulty.CircleSize);
                Assert.Equal(5.0, beatmap.Difficulty.DrainRate);
                Assert.Equal(8.8, beatmap.Difficulty.OverallDifficulty, 1e-6);
                Assert.Equal(9.3, beatmap.Difficulty.ApproachRate, 1e-6);
                Assert.Equal(1.7, beatmap.Difficulty.SliderMultiplier);
                Assert.Equal(1.0, beatmap.Difficulty.SliderTickRate);
            }
        }

        [Fact]
        public void BytesAndTextMatchTheFile() {
            string path = TestResources.MapPath("osu/2785319.osu");
            using (Beatmap fromFile = Beatmap.FromFile(path))
            using (Beatmap fromBytes = Beatmap.FromBytes(File.ReadAllBytes(path)))
            using (Beatmap fromText = Beatmap.FromText(File.ReadAllText(path))) {
                Assert.Equal(fromFile.Title, fromBytes.Title);
                Assert.Equal(fromFile.BeatmapId, fromBytes.BeatmapId);
                Assert.Equal(fromFile.Title, fromText.Title);
                Assert.Equal(fromFile.BeatmapId, fromText.BeatmapId);
            }
        }

        [Fact]
        public void FromTextDecodesUtf8() {
            string text = "osu file format v14\n[General]\nMode: 0\n[Metadata]\n" +
                          "Title:Canción\nArtist:Artísta\nVersion:Versión\n";
            using (Beatmap beatmap = Beatmap.FromText(text)) {
                Assert.Equal("Canción", beatmap.Title);
                Assert.Equal("Artísta", beatmap.Artist);
                Assert.Equal("Versión", beatmap.Version);
            }
        }

        [Fact]
        public void EmptyBytesAreAccepted() {
            using (Beatmap beatmap = Beatmap.FromBytes(new byte[0])) {
                Assert.Equal("", beatmap.Title);
                Assert.Equal(-1, beatmap.BeatmapId);
                Assert.Equal(14, beatmap.FormatVersion);
            }
        }

        [Fact]
        public void MissingFileIsTheFrameworkException() {
            string missing = Path.Combine(Path.GetTempPath(),
                                          "pppp-missing-" + Guid.NewGuid().ToString("N") + ".osu");
            Assert.Throws<FileNotFoundException>(delegate { Beatmap.FromFile(missing); });
        }

        [Fact]
        public void NullInputsAreRejected() {
            Assert.Throws<ArgumentNullException>(delegate { Beatmap.FromBytes(null); });
            Assert.Throws<ArgumentNullException>(delegate { Beatmap.FromText(null); });
        }

        [Fact]
        public void ClosedBeatmapCannotBeRead() {
            Beatmap beatmap = Beatmap.FromFile(TestResources.MapPath("osu/2785319.osu"));
            beatmap.Dispose();
            Assert.Throws<ObjectDisposedException>(delegate { _ = beatmap.Title; });
        }

        [Fact]
        public void RulesetAndHitResultValues() {
            Assert.Equal(0, (int)Ruleset.Osu);
            Assert.Equal(3, (int)Ruleset.Mania);
            Assert.Equal(5, (int)HitResult.Great);
            Assert.Equal(16, (int)HitResult.SliderTailHit);
            Assert.Equal(17, (int)HitResult.LegacyComboIncrease);
        }
    }
}
