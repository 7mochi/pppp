// Parser storage, arenas, copies and the derived-pass options (upstream tests/test_storage.cc,
// without the engine-injection and arena-pool cases).
#include "test_support.h"
#include <cstdio>
#include <cstring>
#include <limits>

using namespace fosu;

static bool padded(const char* text, FileBuffer& buffer) { return make_padded(StringView(text), buffer); }
static bool padded(const std::string& text, FileBuffer& buffer) {
    return make_padded(StringView(text.data(), text.size()), buffer);
}

PPPP_TEST(mods_and_derived_options) {
    {
        Parser parser;
        FileBuffer input;
        CHECK(padded("[General]\nMode:0\n[Difficulty]\nHPDrainRate:4\nCircleSize:4\n"
                     "OverallDifficulty:4\nApproachRate:4\n[TimingPoints]\n0,500\n"
                     "[HitObjects]\n100,100,1500,1,0\n",
                     input));
        ParseOptions options;
        options.mods = Mods::HardRock | Mods::DoubleTime;
        const Beatmap& map = require_parse(parser.parse(input, options));
        CHECK(map.hit_objects[0].time == 1000);
        CHECK(map.hit_objects[0].y == 284);
        CHECK(map.hp == 4 * 1.4);
        CHECK(map.cs == 4 * 1.3);
        CHECK(map.timing_points[0].beat_length == 500.0 / 1.5);
        options.mods = Mods::Easy | Mods::HardRock;
        CHECK(parser.parse(input, options).failed());
    }
    {
        Parser parser;
        FileBuffer input;
        CHECK(padded("osu file format v14\n[HitObjects]\n100,100,0,1,0\n100,100,10,1,0\n", input));
        ParseOptions options;
        options.apply_stacking = true;
        const Beatmap& map = require_parse(parser.parse(input, options));
        CHECK(map.stacking[0].stack_height == 1);
        CHECK(map.hit_objects[0].x < 100);
        CHECK(map.hit_objects[0].raw_position(map.stacking[0].stack_offset).first == 100);
        CHECK(map.raw_positions.size() == 2u && map.raw_positions[0].x == 100);
        Arena* destination = arena_alloc();
        Result<Beatmap> copy = map.copy(*destination);
        CHECK(copy.ok());
        CHECK(copy.value().stacking.data() != map.stacking.data());
        FileBuffer empty;
        CHECK(padded("", empty));
        require_parse(parser.parse(empty));
        CHECK(copy.value().stacking[0].stack_height == 1);
        CHECK(copy.value().hit_objects[0].raw_position(copy.value().stacking[0].stack_offset).first == 100);
        arena_release(destination);
    }
    {
        Parser parser;
        FileBuffer input;
        CHECK(padded("[Difficulty]\nSliderMultiplier:1\n[TimingPoints]\n0,500\n"
                     "[HitObjects]\n0,0,0,2,0,L|400:0,2,400\n",
                     input));
        ParseOptions options;
        options.calculate_slider_events = true;
        const Beatmap& map = require_parse(parser.parse(input, options));
        CHECK(map.slider_events[0].size() == 10u);
        CHECK(map.slider_events[0].front().type == SliderEventType::Head);
        CHECK(map.slider_events[0].back().type == SliderEventType::Tail);
        CHECK(map.slider_events[0][8].type == SliderEventType::LegacyLastTick);
        CHECK(map.slider_events[0][8].time == 3964);
        CHECK(map.slider_events[0].back().time == map.hit_objects[0].end_time);
        const internal::ParserStorage storage = internal::parser_storage(parser);
        CHECK(arena_pos(storage.scratch_arena) == kArenaHeaderSize);
        CHECK(arena_pos(storage.result_arena) > kArenaHeaderSize);
        Arena* destination = arena_alloc();
        Result<Beatmap> copy = map.copy(*destination);
        CHECK(copy.ok());
        CHECK(copy.value().slider_events[0].data() != map.slider_events[0].data());
        FileBuffer empty;
        CHECK(padded("", empty));
        require_parse(parser.parse(empty));
        CHECK(copy.value().slider_events[0].back().time == 4000);
        arena_release(destination);
    }
    {
        Parser parser;
        FileBuffer input;
        CHECK(padded("[HitObjects]\n10,20,1000,2,0,L|110:20,1,150\n", input));
        ParseOptions options;
        options.calculate_slider_paths = true;
        const Beatmap& map = require_parse(parser.parse(input, options));
        CHECK(map.hit_objects[0].end_time == 0);
        CHECK(map.slider_paths.size() == 1u);
        CHECK(map.slider_paths[0].distance() == 150);
        CHECK(slider_position_at(map.slider_paths[0], 0.5).x == 75);
        Arena* arena = arena_alloc();
        CHECK(arena);
        Result<Beatmap> copy = map.copy(*arena);
        CHECK(copy.ok());
        CHECK(copy.value().slider_paths[0].points.data() != map.slider_paths[0].points.data());
        FileBuffer empty;
        CHECK(padded("", empty));
        require_parse(parser.parse(empty));
        CHECK(slider_position_at(copy.value().slider_paths[0], 1).x == 150);
        arena_release(arena);
    }
    {
        Parser parser;
        const std::string input = "[HitObjects]\n0,0,1000,2,0,L|100:0,1,140\n";
        ParseOptions options;
        options.calculate_slider_end_times = false;
        const Beatmap& skipped = require_parse(parser.parse(input.data(), input.size(), options));
        CHECK(skipped.hit_objects[0].end_time == 0);
        Arena* destination = arena_alloc();
        CHECK(destination);
        Result<Beatmap> copy = skipped.copy(*destination);
        CHECK(copy.ok());
        CHECK(require_parse(parser.parse(input.data(), input.size())).hit_objects[0].end_time == 0);
        options.calculate_slider_end_times = true;
        const Beatmap& calculated = require_parse(parser.parse(input.data(), input.size(), options));
        CHECK(calculated.hit_objects[0].end_time > calculated.hit_objects[0].time);
        CHECK(copy.value().hit_objects[0].end_time == 0);
        arena_release(destination);
    }
}

PPPP_TEST(reparse_reuses_arena_memory) {
    FileBuffer input;
    CHECK(padded("[HitObjects]\n16,32,100,1,0\n32,64,200,1,0\n", input));
    Parser parser;
    const Beatmap& first = require_parse(parser.parse(input));
    const HitObject* allocation = first.hit_objects.data();

    const Beatmap& second = require_parse(parser.parse(input));
    CHECK(second.hit_objects.size() == 2u);
    CHECK(second.hit_objects.data() == allocation);
    Parser expected;
    CHECK(canonical(second) == canonical(require_parse(expected.parse(input))));
}

PPPP_TEST(reparse_accepts_larger_arrays) {
    FileBuffer initial;
    CHECK(padded("[HitObjects]\n24,48,500,1,0\n", initial));
    std::string text = "[HitObjects]\n";
    for (size_t i = 0; i < 5000; ++i) {
        text += "72,144,600,1,4\n";
    }
    FileBuffer replacement;
    CHECK(padded(text, replacement));

    Parser parser;
    require_parse(parser.parse(initial));
    const Beatmap& beatmap = require_parse(parser.parse(replacement));
    CHECK(beatmap.hit_objects.size() == 5000u);
    Parser expected;
    CHECK(canonical(beatmap) == canonical(require_parse(expected.parse(replacement))));
}

PPPP_TEST(reparse_clears_omitted_sections) {
    FileBuffer initial;
    CHECK(padded("[General]\nAudioFilename:previous.mp3\nSampleSet:Soft\n"
                 "[Editor]\nGridSize:16\n"
                 "[Metadata]\nTitle:Previous title\nBeatmapID:123\n"
                 "[Difficulty]\nOverallDifficulty:9\nApproachRate:10\n"
                 "[Events]\n0,0,\"previous.jpg\",0,0\n2,50,100\n"
                 "[TimingPoints]\n0,500\n"
                 "[Colours]\nCombo1:0,128,255\n"
                 "[HitObjects]\n64,96,700,2,0,B|128:192|192:96,1,200\n",
                 initial));
    FileBuffer replacement;
    CHECK(padded("[Metadata]\nTitle:Replacement title\n", replacement));
    Parser parser;
    const Beatmap& initial_map = require_parse(parser.parse(initial));
    CHECK(!initial_map.sliders.empty() && !initial_map.timing_points.empty());

    const Beatmap& beatmap = require_parse(parser.parse(replacement));
    CHECK(beatmap.title == StringView("Replacement title"));
    CHECK(beatmap.beatmap_id == -1);
    CHECK(beatmap.audio_filename.empty() && beatmap.background.empty());
    CHECK(beatmap.sample_set == SampleSet::Normal);
    CHECK(beatmap.grid_size == 0);
    CHECK(beatmap.od == 5);
    CHECK(beatmap.ar == 5);
    CHECK(beatmap.breaks.empty() && beatmap.combo_colours.empty());
    CHECK(beatmap.timing_points.empty() && beatmap.hit_objects.empty());
    CHECK(beatmap.sliders.empty() && beatmap.slider_points.empty());
    Parser expected;
    CHECK(canonical(beatmap) == canonical(require_parse(expected.parse(replacement))));
}

PPPP_TEST(empty_reparse_resets_defaults) {
    FileBuffer initial;
    CHECK(padded("[General]\nStackLeniency:0.2\nSampleSet:Drum\n"
                 "[Metadata]\nTitle:Before empty input\n"
                 "[HitObjects]\ninvalid\n96,192,800,1,0\n",
                 initial));
    FileBuffer empty;
    CHECK(padded("", empty));
    Parser parser;
    const Beatmap& initial_map = require_parse(parser.parse(initial));
    CHECK(initial_map.stats.malformed_lines == 1u);

    const Beatmap& beatmap = require_parse(parser.parse(empty));
    CHECK(beatmap.title.empty() && beatmap.hit_objects.empty());
    CHECK(beatmap.stats.malformed_lines == 0u);
    CHECK(beatmap.stats.fast_path_lines == 0u);
    CHECK(beatmap.stats.slow_path_lines == 0u);
    CHECK(beatmap.stack_leniency == double(0.7f));
    CHECK(beatmap.sample_set == SampleSet::Normal);
    Parser expected;
    CHECK(canonical(beatmap) == canonical(require_parse(expected.parse(empty))));
}

PPPP_TEST(large_arena_arrays) {
    std::string circles = "[HitObjects]\n";
    std::string sliders = "[HitObjects]\n";
    std::string timing = "[TimingPoints]\n";
    for (int i = 0; i < 3000; ++i) {
        if (i == 1500) {
            circles += "[HitObjects]\n";
            sliders += "[HitObjects]\n";
            timing += "[TimingPoints]\n";
        }
        circles += "1,2,3,1,0\n";
        if (i == 0 || i == 1500) {
            sliders += "1,2,3,2,0,B|1:2|bad,1,10\n";
            sliders += "1,2,3,2,0,B|7:8|9:10,9001,10\n";
        }
        sliders += "1,2,3,2,0,B|1:2|3:4|5:6,1,10\n";
        timing += "0,500\n";
    }
    FileBuffer circle_input, slider_input, timing_input;
    CHECK(padded(circles, circle_input));
    CHECK(padded(sliders, slider_input));
    CHECK(padded(timing, timing_input));

    Parser parser;
    const Beatmap& circles_map = require_parse(parser.parse(circle_input));
    CHECK(circles_map.hit_objects.size() == 3000u);
    CHECK(circles_map.sliders.empty() && circles_map.slider_points.empty());
    CHECK(circles_map.hit_objects.back().slider == HitObject::kNoSlider);
    CHECK(circles_map.hit_objects.back().hit_sample.empty());

    const Beatmap& sliders_map = require_parse(parser.parse(slider_input));
    CHECK(sliders_map.hit_objects.size() == 3000u);
    CHECK(sliders_map.sliders.size() == 3000u);
    CHECK(sliders_map.slider_points.size() == 9000u);
    CHECK(sliders_map.stats.malformed_lines == 4u);
    bool sliders_ok = true;
    for (size_t i = 0; i < 3000; ++i) {
        const Slider& slider = sliders_map.sliders[i];
        sliders_ok = sliders_ok && sliders_map.hit_objects[i].slider == i && slider.point_begin == 3 * i &&
                     slider.point_count == 3u && sliders_map.slider_points[slider.point_begin].x == 1 &&
                     sliders_map.slider_points[slider.point_begin + 2].y == 6;
    }
    CHECK(sliders_ok);

    const Beatmap& timing_map = require_parse(parser.parse(timing_input));
    CHECK(timing_map.timing_points.size() == 3000u);
    CHECK(timing_map.timing_points.back().beat_length == 500);
    CHECK(timing_map.hit_objects.empty());
}

PPPP_TEST(rejected_slider_points) {
    static const char* const cases[] = {
        "B|7:8|bad,1,10",      "B|7:8|9:10",           "B|7:8|9:10,bad,10",
        "B|7:8|9:10,1,131073", "B|7:8|9:10,1,10,,/:0", "B|7:8|9:10,1,10,,,/:0",
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        FileBuffer input;
        CHECK(
            padded(std::string("[HitObjects]\n1,2,3,2,0,") + cases[i] + "\n1,2,4,2,0,L|11:12,1,10\n", input));
        Parser parser;
        const Beatmap& beatmap = require_parse(parser.parse(input));
        CHECK_MSG(beatmap.stats.malformed_lines == 1u, cases[i]);
        CHECK_MSG(beatmap.hit_objects.size() == 1u, cases[i]);
        CHECK_MSG(beatmap.sliders.size() == 1u, cases[i]);
        CHECK_MSG(beatmap.slider_points.size() == 1u, cases[i]);
        CHECK_MSG(beatmap.sliders[0].point_begin == 0u, cases[i]);
        CHECK_MSG(beatmap.slider_points[0].x == 11, cases[i]);
    }
    const Beatmap map = parse_str("osu file format v128\n[HitObjects]\n"
                                  "1,2,3,2,0,B|7:8|L|9:10,1,bad\n"
                                  "1,2,4,2,0,L|11:12,1,10\n");
    CHECK(map.stats.malformed_lines == 1u);
    CHECK(map.hit_objects.size() == 1u);
    CHECK(map.sliders.size() == 1u);
    CHECK(map.slider_points.size() == 1u);
    CHECK(map.slider_segments.empty());
}

template <typename T>
static bool canonical_detects(Beatmap& map, T& field, T replacement, const std::string& original) {
    const T saved = field;
    field = replacement;
    const bool detected = canonical(map) != original;
    field = saved;
    return detected;
}

PPPP_TEST(canonical_dump_covers_parsed_values) {
    Parser parser;
    ParseOptions options;
    options.calculate_slider_events = true;
    options.apply_stacking = true;
    const std::string text = "osu file format v128\n[Editor]\nVelocityPresets:1,2\n"
                             "[TimingPoints]\n0,500\n[HitObjects]\n"
                             "10.25,20.25,1000,2,0,B2|100.25:0|100:100|0:100,1,300\n";
    Beatmap& map = require_parse(parser.parse(text.data(), text.size(), options));
    CHECK(map.slider_segments.size() == 1u);
    CHECK(map.slider_paths.size() == 1u);
    CHECK(map.slider_events.size() == 1u);
    CHECK(map.stacking.size() == 1u);
    const std::string original = canonical(map);
    CHECK(canonical_detects(map, map.hit_objects[0].x, 10.75f, original));
    CHECK(canonical_detects(map, map.slider_points[0].x, 100.75f, original));
    CHECK(canonical_detects(map, map.slider_segments[0].degree, 3u, original));
    CHECK(canonical_detects(map, map.velocity_presets[0], 4.0, original));
    CHECK(canonical_detects(map, map.slider_paths[0].points[map.slider_paths[0].points.size() - 1].y, 42.0f,
                            original));
    CHECK(canonical_detects(map, map.slider_events[0][0].time, 999.0, original));
    CHECK(canonical_detects(map, map.stacking[0].stack_height, 42, original));
}

PPPP_TEST(option_combinations_on_reused_parser) {
    const std::string input = "osu file format v14\n[General]\nMode:0\n[Difficulty]\n"
                              "SliderMultiplier:1\nSliderTickRate:1\n[TimingPoints]\n0,500\n"
                              "[HitObjects]\n100,100,1000,1,0\n100,100,1100,2,0,"
                              "L|200:100,2,100\n";
    const fosu_uint32 mods = Mods::HardRock | Mods::DoubleTime;
    Parser parser;
    Parser fresh;

    ParseOptions full;
    full.calculate_slider_events = true;
    full.apply_stacking = true;
    full.mods = mods;
    const Beatmap& with_events = require_parse(parser.parse(input.data(), input.size(), full));
    CHECK(with_events.slider_paths.size() == 1u);
    CHECK(with_events.slider_events.size() == 1u);
    CHECK(with_events.stacking.size() == 2u);
    CHECK(canonical(with_events) == canonical(require_parse(fresh.parse(input.data(), input.size(), full))));
    const double end_time = with_events.hit_objects[1].end_time;

    ParseOptions end_times;
    end_times.calculate_slider_end_times = true;
    end_times.mods = mods;
    const Beatmap& duration = require_parse(parser.parse(input.data(), input.size(), end_times));
    CHECK(duration.hit_objects[1].end_time == end_time);
    CHECK(duration.slider_paths.empty() && duration.slider_events.empty());
    CHECK(duration.stacking.empty());
    CHECK(canonical(duration) ==
          canonical(require_parse(fresh.parse(input.data(), input.size(), end_times))));

    ParseOptions partial_options;
    partial_options.sections = kSectionHitObjects;
    partial_options.calculate_slider_events = true;
    partial_options.apply_stacking = true;
    const Beatmap& partial = require_parse(parser.parse(input.data(), input.size(), partial_options));
    CHECK(partial.timing_points.empty());
    CHECK(partial.slider_paths.size() == 1u);
    CHECK(partial.slider_events.size() == 1u);
    CHECK(partial.stacking.size() == 2u);
    CHECK(canonical(partial) ==
          canonical(require_parse(fresh.parse(input.data(), input.size(), partial_options))));

    const Beatmap& raw = require_parse(parser.parse(input.data(), input.size()));
    CHECK(raw.slider_paths.empty() && raw.slider_events.empty());
    CHECK(raw.stacking.empty());
    CHECK(raw.hit_objects[1].end_time == 0);
    CHECK(canonical(raw) == canonical(require_parse(fresh.parse(input.data(), input.size()))));
}

PPPP_TEST(event_budget_failure_and_reuse) {
    const std::string oversized = "[Difficulty]\nSliderMultiplier:1\nSliderTickRate:8\n"
                                  "[TimingPoints]\n0,500\n[HitObjects]\n"
                                  "0,0,1000,2,0,L|100000:0,9000,100000\n";
    const std::string small = "[TimingPoints]\n0,500\n[HitObjects]\n"
                              "0,0,1000,2,0,L|100:0,1,100\n";
    ParseOptions options;
    options.calculate_slider_events = true;
    Parser parser;
    const Result<Beatmap*> failed = parser.parse(oversized.data(), oversized.size(), options);
    CHECK(failed.failed());
    CHECK(failed.error().code == ErrorCode::AllocationFailure);
    const Beatmap& recovered = require_parse(parser.parse(small.data(), small.size(), options));
    CHECK(recovered.slider_events.size() == 1u);
    CHECK(recovered.hit_objects.size() == 1u);
}

PPPP_TEST(tick_endpoint_exclusion) {
    struct Case {
        const char* length;
        size_t ticks;
    };
    static const Case cases[] = {{"14.49", 0u}, {"14.5", 0u}, {"14.51", 1u}};
    ParseOptions options;
    options.calculate_slider_events = true;
    Parser parser;
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        const std::string input = std::string("[Difficulty]\nSliderMultiplier:1\nSliderTickRate:8\n"
                                              "[TimingPoints]\n0,500\n[HitObjects]\n"
                                              "0,0,1000,2,0,L|100:0,1,") +
                                  cases[i].length + "\n";
        const Beatmap& map = require_parse(parser.parse(input.data(), input.size(), options));
        CHECK_MSG(map.slider_events.size() == 1u, cases[i].length);
        size_t ticks = 0;
        for (size_t e = 0; e < map.slider_events[0].size(); e++) {
            ticks += map.slider_events[0][e].type == SliderEventType::Tick;
        }
        CHECK_MSG(ticks == cases[i].ticks, cases[i].length);
    }
}

PPPP_TEST(multisegment_path_trim) {
    const std::string input = "osu file format v128\n[HitObjects]\n"
                              "0,0,1000,2,0,L|100:0|L|100:100,1,50\n";
    ParseOptions options;
    options.calculate_slider_paths = true;
    Parser parser;
    const Beatmap& map = require_parse(parser.parse(input.data(), input.size(), options));
    CHECK(map.slider_segments.size() == 2u);
    CHECK(map.slider_paths.size() == 1u);
    CHECK(map.slider_paths[0].distance() == 50);
    const PathPoint end = slider_position_at(map.slider_paths[0], 1);
    CHECK(end.x == 50);
    CHECK(end.y == 0);
}

PPPP_TEST(copy_owns_all_data) {
    Arena* program_arena = arena_alloc();
    CHECK(program_arena != 0);
    Beatmap owned;
    std::string expected;
    {
        FileBuffer input;
        CHECK(padded("[General]\nAudioFilename:song.mp3\n"
                     "[Metadata]\nTitle:Owned title\nArtist:Owned artist\n"
                     "[Events]\n0,0,\"background.jpg\",0,0\n2,10,20\n"
                     "[TimingPoints]\n0,500\n"
                     "[Colours]\nCombo1:1,2,3\n"
                     "[HitObjects]\n"
                     "1,2,3,1,0,1:2:3:4:sample.wav\n"
                     "1,2,4,2,0,B|7:8|9:10,1,20,2|0,1:2|3:4,"
                     "1:2:3:4:slider.wav\n",
                     input));
        Parser parser;
        const Beatmap& parsed = require_parse(parser.parse(input));
        expected = canonical(parsed);
        Result<Beatmap> copied = parsed.copy(*program_arena);
        CHECK(copied.ok());
        if (copied.ok()) {
            owned = copied.value();
        }
        owned.hit_objects[0].x = 42;
        CHECK(parsed.hit_objects[0].x == 1);
        owned.hit_objects[0].x = 1;
        std::memset(input.data, 'x', input.size);
        CHECK(canonical(parsed) == expected);
        FileBuffer replacement;
        CHECK(padded("[Metadata]\nTitle:Replacement\n[HitObjects]\n1,2,5,1,0\n", replacement));
        require_parse(parser.parse(replacement));
    }
    CHECK(canonical(owned) == expected);
    arena_release(program_arena);
}

PPPP_TEST(failed_parse_resets_and_parser_remains_reusable) {
    Parser parser;
    FileBuffer input;
    CHECK(padded("[Metadata]\nTitle:Before failure\n", input));
    CHECK(parser.parse(input).ok());

    const Result<Beatmap*> failed = parser.parse(0, 1);
    CHECK(failed.failed());
    CHECK(failed.error().code == ErrorCode::InvalidInput);
    const internal::ParserStorage storage = internal::parser_storage(parser);
    CHECK(storage.input == 0);
    CHECK(storage.result_arena != storage.scratch_arena);
    CHECK(arena_pos(storage.result_arena) == kArenaHeaderSize);
    CHECK(arena_pos(storage.scratch_arena) == kArenaHeaderSize);

    const Beatmap& recovered = require_parse(parser.parse(input));
    CHECK(recovered.title == StringView("Before failure"));
}

PPPP_TEST(failed_copy_rewinds_destination) {
    std::string text = "[HitObjects]\n";
    for (size_t i = 0; i < 2000; ++i) {
        text += "1,2,3,1,0,1:2:3:4:sample.wav\n";
    }
    FileBuffer input;
    CHECK(padded(text, input));
    Parser parser;
    const Beatmap& beatmap = require_parse(parser.parse(input));

    // A single 4 KiB block that cannot chain.
    ArenaParams params;
    params.block_size = 4096;
    params.flags = 0;
    Arena* destination = arena_alloc(params);
    CHECK(destination != 0);
    fosu_uint32* existing = arena_push_array<fosu_uint32>(destination, 1);
    CHECK(existing != 0);
    *existing = 0x12345678;
    const size_t checkpoint = arena_pos(destination);

    const Result<Beatmap> copied = beatmap.copy(*destination);
    CHECK(copied.failed());
    CHECK(copied.error().code == ErrorCode::AllocationFailure);
    CHECK(arena_pos(destination) == checkpoint);
    CHECK(*existing == 0x12345678u);
    arena_release(destination);
}

PPPP_TEST(arena_interface) {
    ArenaParams params;
    params.block_size = 4u << 10;
    params.flags = ArenaFlagChain;
    Arena* arena = arena_alloc(params);
    CHECK(arena != 0);
    CHECK(arena_push(arena, 1, kMaxArenaAlignment * 2) == 0);
    const size_t initial = arena_pos(arena);
    fosu_uint32* first = arena_push_array<fosu_uint32>(arena, 16);
    CHECK(first != 0);
    size_t checkpoint;
    {
        const TempArena temp(arena);
        checkpoint = temp.position();
        CHECK(arena_push(arena, 96u << 10, AlignmentOf<fosu_uint64>::value) != 0);
        CHECK(arena->current != arena);
        // positions stay monotonic across chained blocks
        CHECK(arena_pos(arena) > checkpoint + (96u << 10));
    }
    CHECK(arena->current == arena);
    CHECK(arena_pos(arena) == checkpoint);
    arena_clear(arena);
    CHECK(arena_pos(arena) == initial);
    arena_release(arena);

    // AlignmentOf agrees with the natural alignment of the fundamental types.
    CHECK(AlignmentOf<char>::value == 1);
    CHECK(AlignmentOf<fosu_uint16>::value == 2);
    CHECK(AlignmentOf<fosu_uint32>::value == 4);
    CHECK(AlignmentOf<double>::value == sizeof(double) || AlignmentOf<double>::value == 4);
}

PPPP_TEST(read_into_reuse) {
    const char* path = "fosu_read_into_test.osu";
    const std::string big(10000, 'A');
    const std::string little(100, 'B');
    std::FILE* file = std::fopen(path, "wb");
    CHECK(file != 0);
    if (!file) {
        return;
    }
    std::fwrite(big.data(), 1, big.size(), file);
    std::fclose(file);

    FileBuffer buffer;
    CHECK(read_into(path, buffer));
    const char* allocation = buffer.data;
    const size_t capacity = buffer.capacity;

    file = std::fopen(path, "wb");
    CHECK(file != 0);
    if (!file) {
        return;
    }
    std::fwrite(little.data(), 1, little.size(), file);
    std::fclose(file);
    CHECK(read_into(path, buffer));
    CHECK(buffer.data == allocation);
    CHECK(buffer.capacity == capacity);
    CHECK(std::memcmp(buffer.data, little.data(), little.size()) == 0);
    std::remove(path);

    Parser parser;
    CHECK(parser.parse_file("fosu_missing_file.osu").failed());
    CHECK(parser.parse_file("fosu_missing_file.osu").error().code == ErrorCode::IoFailure);
}

PPPP_TEST(input_size_overflow) {
    char byte = 0;
    Parser parser;
    const Result<Beatmap*> memory_result = parser.parse(&byte, std::numeric_limits<size_t>::max());
    CHECK(memory_result.failed());
    CHECK(memory_result.error().code == ErrorCode::InputTooLarge);
    ParseOptions options;
    options.sections = 1;
    CHECK(parser.parse(&byte, 0, options).failed());
}

PPPP_TEST_MAIN()
