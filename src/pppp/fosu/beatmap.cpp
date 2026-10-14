#include "pppp/beatmap.h"
#include "pppp/utils/precision.h"
#include <new>

#include <fosu/parser.h>

namespace pppp { namespace beatmaps {
    namespace {
        fosu::ParseOptions parse_options() {
            fosu::ParseOptions opts;

            opts.sections = fosu::kAllSections;
            opts.calculate_slider_end_times = true;
            opts.calculate_slider_paths = true;
            opts.calculate_slider_events = true;
            opts.apply_stacking = true;
            opts.mods = fosu::Mods::None;
            opts.client = fosu::Client::Lazer;

            return opts;
        }

        Status to_status(const fosu::Beatmap* map, const fosu::ParseError& error) {
            if (map) {
                return StatusCode::OK;
            }
            return error.code == fosu::ParseErrorCode::OutOfMemory ? StatusCode::ALLOCATION
                                                                   : StatusCode::PARSE;
        }

        SliderEventType to_slider_event_type(fosu::SliderEventType::Value type) {
            switch (type) {
            case fosu::SliderEventType::Head: return SLIDER_EVENT_HEAD;
            case fosu::SliderEventType::Tick: return SLIDER_EVENT_TICK;
            case fosu::SliderEventType::Repeat: return SLIDER_EVENT_REPEAT;
            case fosu::SliderEventType::LegacyLastTick: return SLIDER_EVENT_LEGACY_LAST_TICK;
            case fosu::SliderEventType::Tail: return SLIDER_EVENT_TAIL;
            }
            return SLIDER_EVENT_HEAD; // unreachable: every kind is mapped above
        }

        class SliderPathOpsContext {
        public:
            const fosu::Beatmap* map;
            fosu::Parser* parser;
            fosu::Arena* result_arena;
            fosu::Arena* scratch_arena;

            SliderPathOpsContext(const fosu::Beatmap* map_in, fosu::Parser* parser_in)
                : map(map_in),
                  parser(parser_in),
                  result_arena(0),
                  scratch_arena(0) {}
            ~SliderPathOpsContext() {
                fosu::arena_release(result_arena);
                fosu::arena_release(scratch_arena);
                delete parser;
            }

            bool ensure_arenas() {
                fosu::ArenaParams params;

                params.block_size = static_cast<size_t>(64) << 10;
                params.flags = fosu::ArenaFlagChain;

                if (!result_arena) {
                    result_arena = fosu::arena_alloc(params);
                }
                if (!scratch_arena) {
                    scratch_arena = fosu::arena_alloc(params);
                }

                return result_arena && scratch_arena;
            }

        private:
            SliderPathOpsContext(const SliderPathOpsContext&);
            SliderPathOpsContext& operator=(const SliderPathOpsContext&);
        };

        int recompute(void* ctx, unsigned hit_object_index, const utils::Vector2* relative_points,
                      size_t count, std::vector<utils::Vector2>* path,
                      std::vector<double>* cumulative_lengths) {
            SliderPathOpsContext* c = static_cast<SliderPathOpsContext*>(ctx);

            if (!c->ensure_arenas()) {
                return -1;
            }

            std::vector<fosu::PathPoint> pts(count);
            for (size_t i = 0; i < count; i++) {
                pts[i] = fosu::PathPoint::make(relative_points[i].x, relative_points[i].y);
            }

            const fosu::TempArena temp(c->result_arena);
            const fosu::Result<fosu::SliderPath> result =
                fosu::slider_path_from_relative_points(*c->map, hit_object_index, pts.empty() ? 0 : &pts[0],
                                                       count, c->result_arena, c->scratch_arena);

            if (result.failed()) {
                return -1;
            }

            const fosu::SliderPath& out = result.value();
            path->resize(out.points.size());
            cumulative_lengths->resize(out.points.size());

            for (size_t i = 0; i < out.points.size(); i++) {
                (*path)[i].x = out.points[i].x;
                (*path)[i].y = out.points[i].y;
                (*cumulative_lengths)[i] = out.cumulative_lengths[i];
            }

            return 0;
        }

        void release(void* ctx) { delete static_cast<SliderPathOpsContext*>(ctx); }

        void parse_node_sounds(std::vector<unsigned>& out, const nonstd::string_view& text) {
            out.clear();

            const char* p = text.data();
            const char* end = p + text.size();
            while (p < end) {
                unsigned value = 0;
                bool digits = false;
                while (p < end && *p >= '0' && *p <= '9') {
                    value = value * 10u + static_cast<unsigned>(*p - '0');
                    digits = true;
                    p++;
                }
                if (!digits) {
                    out.clear();
                    return; // malformed: treat the whole field as absent
                }
                out.push_back(value);

                if (p < end) {
                    if (*p != '|') {
                        out.clear();
                        return;
                    }
                    p++;
                }
            }
        }

        void copy_hit_objects(Beatmap& out, const fosu::Beatmap& map) {
            out.hit_objects.resize(map.hit_objects.size());

            for (size_t i = 0; i < map.hit_objects.size(); i++) {
                const fosu::HitObject& ho = map.hit_objects[i];
                HitObject& o = out.hit_objects[i];

                // fosu's x/y include its own stack offset, raw_positions keeps the parsed positions
                o.position.x = ho.x;
                o.position.y = ho.y;
                if (i < map.raw_positions.size()) {
                    o.position.x = map.raw_positions[i].x;
                    o.position.y = map.raw_positions[i].y;
                }

                o.type = ho.type;
                o.hitsound = ho.hitsound;
                o.start_time = ho.time;
                o.end_time = ho.end_time;
                o.duration = ho.duration;
                o.new_combo = ho.new_combo;
                o.combo_offset = ho.combo_skip;
                o.slider =
                    (ho.is_slider() && ho.slider < map.sliders.size()) ? static_cast<int>(ho.slider) : -1;
            }
        }

        void copy_events(std::vector<SliderEventDescriptor>& out,
                         const nonstd::span<fosu::SliderEvent>& group) {
            out.resize(group.size());
            for (size_t k = 0; k < group.size(); k++) {
                const fosu::SliderEvent& e = group[k];
                out[k].type = to_slider_event_type(e.type);
                out[k].time = e.time;
                out[k].span_index = e.span_index;
                out[k].span_start_time = e.span_start_time;
                out[k].path_progress = e.path_progress;
                out[k].position.x = e.position.x;
                out[k].position.y = e.position.y;
            }
        }

        void copy_sliders(Beatmap& out, const fosu::Beatmap& map) {
            out.sliders.resize(map.sliders.size());

            const nonstd::span<const fosu::SliderPoint> points =
                map.raw_slider_points.empty() ? nonstd::span<const fosu::SliderPoint>(map.slider_points)
                                              : nonstd::span<const fosu::SliderPoint>(map.raw_slider_points);

            for (size_t i = 0; i < map.hit_objects.size(); i++) {
                const fosu::HitObject& ho = map.hit_objects[i];
                if (out.hit_objects[i].slider < 0) {
                    continue;
                }

                const fosu::Slider& src = map.sliders[ho.slider];
                Slider& sl = out.sliders[ho.slider];

                sl.slides = src.slides;
                sl.expected_length = src.length;
                parse_node_sounds(sl.node_sounds, src.edge_sounds);

                sl.control_points.resize(src.point_count);
                for (size_t k = 0; k < src.point_count; k++) {
                    sl.control_points[k].x = points[src.point_begin + k].x;
                    sl.control_points[k].y = points[src.point_begin + k].y;
                }

                if (ho.slider < map.slider_paths.size()) {
                    const fosu::SliderPath& path = map.slider_paths[ho.slider];
                    sl.path.resize(path.points.size());
                    sl.cumulative_lengths.resize(path.points.size());
                    for (size_t k = 0; k < path.points.size(); k++) {
                        sl.path[k].x = path.points[k].x;
                        sl.path[k].y = path.points[k].y;
                        sl.cumulative_lengths[k] = path.cumulative_lengths[k];
                    }

                    sl.undecimated_path.resize(path.undecimated_points.size());
                    sl.undecimated_cumulative_lengths.resize(path.undecimated_points.size());
                    for (size_t k = 0; k < path.undecimated_points.size(); k++) {
                        sl.undecimated_path[k].x = path.undecimated_points[k].x;
                        sl.undecimated_path[k].y = path.undecimated_points[k].y;
                        sl.undecimated_cumulative_lengths[k] = path.undecimated_cumulative_lengths[k];
                    }
                }

                const std::vector<double>& decoder_lengths = sl.undecimated_cumulative_lengths.empty()
                                                                 ? sl.cumulative_lengths
                                                                 : sl.undecimated_cumulative_lengths;
                const double distance = decoder_lengths.empty() ? 0 : decoder_lengths.back();

                // there are known instances of beatmaps (https://osu.ppy.sh/beatmapsets/594828#osu/1258033)
                // which contain zero-length sliders with non-zero numbers of repeats. this was exploiting a
                // bug in stable in which the slider repeats would be generated as objects but never actually
                // judged as a hit *or* miss during gameplay, therefore increasing the theoretical possible
                // max combo to be gained from a slider while in practice never giving that extra combo. due
                // to lazer ensuring that an object has its nested part fully judged, this would result in
                // broken behaviours (either the zero-length slider giving hundreds of combo for nothing if
                // the repeats are judged as hit, or insta-failing the player due to HP if judged as miss). to
                // remedy this in a way that seems least damaging, detect this situation via a heuristic and
                // reset the number of repeats to zero. this technically *does not* match stable beatmap
                // parsing or conversion, *does not* match in-gameplay behaviour of such broken sliders, and
                // *will* fail conversion mapping tests, but again, this is supposed to be a least-worst
                // measure to prevent exploits. it is also applied centrally to all rulesets rather than in
                // specific ruleset converters because this failure scenario translates across rulesets (osu!
                // and catch are both affected).
                if (utils::almost_equals(distance, 0.0)) {
                    const int nodes = sl.slides + 1;
                    const unsigned first = sl.node_sounds.empty() ? ho.hitsound : sl.node_sounds[0];
                    const unsigned last = static_cast<size_t>(nodes - 1) < sl.node_sounds.size()
                                              ? sl.node_sounds[static_cast<size_t>(nodes - 1)]
                                              : ho.hitsound;
                    sl.slides = 1;
                    sl.node_sounds.clear();
                    sl.node_sounds.push_back(first);
                    sl.node_sounds.push_back(last);
                }

                if (ho.slider < map.slider_events.size()) {
                    copy_events(sl.events, map.slider_events[ho.slider]);
                }
                if (ho.slider < map.catch_slider_events.size()) {
                    copy_events(sl.catch_events, map.catch_slider_events[ho.slider]);
                }
            }
        }

        void copy_timing_points(Beatmap& out, const fosu::Beatmap& map) {
            out.timing_points.resize(map.timing_points.size());

            for (size_t i = 0; i < map.timing_points.size(); i++) {
                const fosu::TimingPoint& tp = map.timing_points[i];

                out.timing_points[i].time = tp.time;
                out.timing_points[i].beat_length = tp.beat_length;
                out.timing_points[i].meter = tp.meter;
                out.timing_points[i].uninherited = tp.uninherited;
                out.timing_points[i].effects = tp.effects;
            }
        }

        void copy_breaks(Beatmap& out, const fosu::Beatmap& map) {
            out.breaks.resize(map.breaks.size());

            for (size_t i = 0; i < map.breaks.size(); i++) {
                out.breaks[i].start_time = map.breaks[i].start;
                out.breaks[i].end_time = map.breaks[i].end;
            }
        }

        void copy_from_fosu(Beatmap& out, const fosu::Beatmap& map) {
            out.format_version = map.format_version;
            out.mode = map.mode;
            out.stack_leniency = static_cast<float>(map.stack_leniency);

            out.difficulty.drain_rate = static_cast<float>(map.hp);
            out.difficulty.circle_size = static_cast<float>(map.cs);
            out.difficulty.overall_difficulty = static_cast<float>(map.od);
            out.difficulty.approach_rate = static_cast<float>(map.ar);
            out.difficulty.slider_multiplier = map.slider_multiplier;
            out.difficulty.slider_tick_rate = map.slider_tick_rate;

            copy_hit_objects(out, map);
            copy_sliders(out, map);
            copy_timing_points(out, map);
            copy_breaks(out, map);
        }

        Status create_beatmap(Beatmap& out, const fosu::Beatmap* map, fosu::Parser* parser) {
            SliderPathOpsContext* ctx = new (std::nothrow) SliderPathOpsContext(map, parser);
            if (!ctx) {
                delete parser;
                return StatusCode::ALLOCATION;
            }

            out.clear();
            copy_from_fosu(out, *map);

            out.slider_path.recompute = recompute;
            out.slider_path.release = release;
            out.slider_path.ctx = ctx;

            return StatusCode::OK;
        }

        Status from_file(Beatmap& out, const char* path) {
            if (!path) {
                return StatusCode::INVALID_ARGUMENT;
            }

            fosu::Parser* parser = new (std::nothrow) fosu::Parser;
            if (!parser) {
                return StatusCode::ALLOCATION;
            }

            const fosu::Beatmap* result = parser->parse_file(path, parse_options());
            if (!result) {
                const Status status = to_status(result, parser->error());
                delete parser;
                return status;
            }
            return create_beatmap(out, result, parser);
        }

        Status from_bytes(Beatmap& out, const void* data, size_t size) {
            fosu::Parser* parser = new (std::nothrow) fosu::Parser;
            if (!parser) {
                return StatusCode::ALLOCATION;
            }

            const fosu::Beatmap* result =
                parser->parse(static_cast<const char*>(data), size, parse_options());
            if (!result) {
                const Status status = to_status(result, parser->error());
                delete parser;
                return status;
            }
            return create_beatmap(out, result, parser);
        }
    } // namespace

    Status from_parsed(Beatmap& out, const fosu::Beatmap* parsed) {
        if (!parsed) {
            return StatusCode::INVALID_ARGUMENT;
        }
        return create_beatmap(out, parsed, 0);
    }
}} // namespace pppp::beatmaps

namespace pppp {
    Status Beatmap::load_file(const char* path) { return beatmaps::from_file(*this, path); }

    Status Beatmap::load_buffer(const void* data, size_t size) {
        return beatmaps::from_bytes(*this, data, size);
    }
} // namespace pppp
