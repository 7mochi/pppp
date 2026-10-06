using System;

namespace Pppp {
    [Flags]
    public enum LegacyMods : uint {
        None = 0,
        NoFail = 1,
        Easy = 1 << 1,
        TouchDevice = 1 << 2,
        Hidden = 1 << 3,
        HardRock = 1 << 4,
        SuddenDeath = 1 << 5,
        DoubleTime = 1 << 6,
        Relax = 1 << 7,
        HalfTime = 1 << 8,
        Nightcore = 1 << 9,
        Flashlight = 1 << 10,
        Autoplay = 1 << 11,
        SpunOut = 1 << 12,
        Autopilot = 1 << 13,
        Perfect = 1 << 14,
        Key4 = 1 << 15,
        Key5 = 1 << 16,
        Key6 = 1 << 17,
        Key7 = 1 << 18,
        Key8 = 1 << 19,
        FadeIn = 1 << 20,
        Random = 1 << 21,
        Cinema = 1 << 22,
        Target = 1 << 23,
        Key9 = 1 << 24,
        KeyCoop = 1 << 25,
        Key1 = 1 << 26,
        Key3 = 1 << 27,
        Key2 = 1 << 28,
        ScoreV2 = 1 << 29,
        Mirror = 1 << 30,
    }

    public struct Mods {
        private readonly string specification;
        private readonly LegacyMods legacy;
        private readonly bool isLegacyBitmask;
        private readonly bool classic;

        private Mods(string specification, LegacyMods legacy, bool isLegacyBitmask, bool classic) {
            this.specification = specification;
            this.legacy = legacy;
            this.isLegacyBitmask = isLegacyBitmask;
            this.classic = classic;
        }

        public static implicit operator Mods(string specification) {
            return new Mods(specification == null ? "" : specification, LegacyMods.None, false, false);
        }

        public static implicit operator Mods(LegacyMods legacy) { return new Mods(null, legacy, true, false); }

        public Mods WithClassic() { return new Mods(specification, legacy, isLegacyBitmask, true); }

        internal string Specification {
            get { return specification == null ? "" : specification; }
        }

        internal LegacyMods Legacy {
            get { return legacy; }
        }

        internal bool IsLegacyBitmask {
            get { return isLegacyBitmask; }
        }

        internal bool Classic {
            get { return classic; }
        }

        public override string ToString() { return isLegacyBitmask ? legacy.ToString() : Specification; }
    }
}
