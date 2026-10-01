#ifndef FOSU_PARSER_H
#define FOSU_PARSER_H

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>

#include <fosu/beatmap.h>
#include <fosu/engine/parse_document.h>
#include <fosu/engine/parsing/key_value.h>
#include <fosu/io.h>
#include <fosu/legacy_rules.h>
#include <fosu/mods.h>
#include <fosu/parse_options.h>
#include <fosu/slider_events.h>
#include <fosu/slider_geometry.h>
#include <fosu/slider_timing.h>
#include <fosu/stacking.h>

namespace fosu {

    namespace internal {

        inline size_t velocity_preset_capacity(StringView document) {
            const StringView field("VelocityPresets");
            size_t capacity = 3;
            size_t search_from = 0;
            for (;;) {
                const size_t field_begin = document.find(field, search_from);
                if (field_begin == StringView::npos) {
                    break;
                }
                const size_t key_end = field_begin + field.size();
                const size_t line_end = document.find('\n', key_end);
                const size_t value_end = line_end == StringView::npos ? document.size() : line_end;
                const size_t colon = document.find(':', key_end);
                if (colon < value_end &&
                    trim_field(document.substr(field_begin, colon - field_begin)) == field) {
                    size_t values = 1;
                    for (size_t i = colon + 1; i < value_end; ++i) {
                        values += document[i] == ',';
                    }
                    capacity = std::max(capacity, values);
                }
                search_from = value_end;
            }
            return capacity;
        }

        template <typename T>
        inline bool allocate_span(Arena* arena, Span<T>& span, size_t capacity) {
            T* values = arena_push_array<T>(arena, capacity);
            if (!values) {
                return false;
            }
            span = Span<T>(values, capacity);
            return true;
        }

        inline bool allocate_beatmap_arrays(Arena* arena, Beatmap& beatmap, StringView input,
                                            fosu_uint32 selected_sections, bool lazer_format) {
            const size_t input_size = input.size();
            // Each capacity is a conservative bound derived from the shortest accepted
            // spelling. sizeof includes the string literal's trailing null byte.
            if (selected_sections & kSectionEvents) {
                if (!allocate_span(arena, beatmap.breaks, input_size / (sizeof("2,0,0") - 1) + 1)) {
                    return false;
                }
            }
            if (selected_sections & kSectionColours) {
                if (!allocate_span(arena, beatmap.combo_colours,
                                   input_size / (sizeof("Combo1:0,0,0") - 1) + 1)) {
                    return false;
                }
            }
            if (selected_sections & kSectionEditor) {
                const size_t capacity = lazer_format ? velocity_preset_capacity(input) : 3;
                if (!allocate_span(arena, beatmap.velocity_presets, capacity)) {
                    return false;
                }
            }
            if (selected_sections & kSectionTimingPoints) {
                if (!allocate_span(arena, beatmap.timing_points, input_size / (sizeof("0,0") - 1) + 1)) {
                    return false;
                }
            }
            if (selected_sections & kSectionHitObjects) {
                if (!allocate_span(arena, beatmap.hit_objects, input_size / (sizeof("0,0,0,1,0") - 1) + 1)) {
                    return false;
                }
                if (!allocate_span(arena, beatmap.sliders, input_size / (sizeof("0,0,0,2,0,L,0") - 1) + 1)) {
                    return false;
                }
                if (lazer_format) {
                    if (!allocate_span(arena, beatmap.slider_segments,
                                       input_size / (sizeof("|L|0:0") - 1) + 1)) {
                        return false;
                    }
                }
                if (!allocate_span(arena, beatmap.slider_points, input_size / (sizeof("|0:0") - 1) + 1)) {
                    return false;
                }
            }
            return true;
        }

    } // namespace internal

    class Parser;

    namespace internal {
        struct ParserStorage {
            Arena* result_arena;
            Arena* scratch_arena;
            const char* input;
            size_t input_size;
            size_t input_storage_size;
        };

        ParserStorage parser_storage(Parser& parser);
    } // namespace internal

    // Owns a result arena (the parsed beatmap and a padded copy of the input) and a scratch arena.
    // Each parse attempt resets both: a Beatmap stays valid until the next parse attempt or the
    // destruction of the parser (copy it out with Beatmap::copy to keep it).
    class Parser {
    public:
        Parser()
            : result_arena_(arena_alloc()),
              scratch_arena_(arena_alloc()),
              input_(0),
              input_size_(0),
              beatmap_() {}

        ~Parser() {
            arena_release(result_arena_);
            arena_release(scratch_arena_);
        }

        Result<Beatmap*> parse(const char* data, size_t size, const ParseOptions& opts = ParseOptions()) {
            reset_working_result();
            if ((!data && size) || invalid_options(opts)) {
                return Error(ErrorCode::InvalidInput);
            }
            const Result<char*> prepared = prepare_input(size, data);
            if (prepared.failed()) {
                const Error error = prepared.error();
                reset_working_result();
                return error;
            }
            return finish_parse(opts);
        }

        Result<Beatmap*> parse(StringView input, const ParseOptions& opts = ParseOptions()) {
            return parse(input.data(), input.size(), opts);
        }

        Result<Beatmap*> parse(const FileBuffer& input, const ParseOptions& opts = ParseOptions()) {
            return parse(input.data, input.size, opts);
        }

        Result<Beatmap*> parse_file(const char* path, const ParseOptions& opts = ParseOptions()) {
            reset_working_result();
            if (!path || invalid_options(opts)) {
                return Error(ErrorCode::InvalidInput);
            }

            size_t size;
            std::FILE* file = internal::open_input_file(path, size);
            if (!file) {
                return Error(ErrorCode::IoFailure, kNoErrorOffset, errno);
            }
            if (!can_pad_input(size)) {
                std::fclose(file);
                return Error(ErrorCode::InputTooLarge);
            }

            const Result<char*> prepared = prepare_input(size, 0);
            if (prepared.failed()) {
                const Error error = prepared.error();
                std::fclose(file);
                reset_working_result();
                return error;
            }
            if (!internal::read_input_file(file, input_, size)) {
                const int error = errno;
                std::fclose(file);
                reset_working_result();
                return Error(ErrorCode::IoFailure, kNoErrorOffset, error);
            }
            std::fclose(file);
            return finish_parse(opts);
        }

    private:
        friend internal::ParserStorage internal::parser_storage(Parser& parser);

        Parser(const Parser&);
        Parser& operator=(const Parser&);

        static bool invalid_sections(fosu_uint32 sections) {
            return (sections & ~static_cast<fosu_uint32>(kAllSections)) != 0;
        }

        static bool invalid_options(const ParseOptions& opts) {
            if (invalid_sections(opts.sections) || internal::invalid_mods(opts.mods)) {
                return true;
            }
            return internal::has_difficulty_mod(opts.mods) &&
                   (opts.sections & (kSectionGeneral | kSectionDifficulty)) !=
                       (kSectionGeneral | kSectionDifficulty);
        }

        Result<char*> prepare_input(size_t size, const char* data) {
            if (!can_pad_input(size)) {
                return Error(ErrorCode::InputTooLarge);
            }
            if (!result_arena_) {
                result_arena_ = arena_alloc();
            }
            if (!scratch_arena_) {
                scratch_arena_ = arena_alloc();
            }
            if (!result_arena_ || !scratch_arena_) {
                return Error(ErrorCode::AllocationFailure);
            }
            input_ = static_cast<char*>(arena_push(result_arena_, size + kBufferPadding, 1));
            if (!input_) {
                return Error(ErrorCode::AllocationFailure);
            }
            if (data && size) {
                std::memmove(input_, data, size);
            }
            std::memset(input_ + size, 0, kBufferPadding);
            input_size_ = size;
            return input_;
        }

        Result<Beatmap*> finish_parse(const ParseOptions& opts) {
            const StringView input(input_, input_size_);
            if (input_size_ != 0) {
                Beatmap preamble;
                internal::parse_preamble(preamble, input.data(), input.data() + input.size());
                if (!internal::allocate_beatmap_arrays(result_arena_, beatmap_, input, opts.sections,
                                                       preamble.format_version >= 128)) {
                    reset_working_result();
                    return Error(ErrorCode::AllocationFailure);
                }
            }
            internal::parse_document(input.data(), input.size(), beatmap_, opts);
            if (!internal::apply_legacy_rules(beatmap_, scratch_arena_)) {
                reset_working_result();
                return Error(ErrorCode::AllocationFailure);
            }
            if (!internal::apply_mods_before_calculations(beatmap_, opts.mods)) {
                reset_working_result();
                return Error(ErrorCode::InvalidInput);
            }
            const bool stacking = opts.apply_stacking && beatmap_.mode == 0;
            size_t stacking_scratch_pos = 0;
            Span<double> stacking_end_times;
            if (stacking && !beatmap_.sliders.empty()) {
                stacking_scratch_pos = arena_pos(scratch_arena_);
                double* end_times = arena_push_array<double>(scratch_arena_, beatmap_.sliders.size());
                if (!end_times) {
                    reset_working_result();
                    return Error(ErrorCode::AllocationFailure);
                }
                stacking_end_times = Span<double>(end_times, beatmap_.sliders.size());
            }
            if ((opts.calculate_slider_paths || opts.calculate_slider_events || stacking) &&
                !internal::set_slider_paths(beatmap_, result_arena_, scratch_arena_)) {
                reset_working_result();
                return Error(ErrorCode::AllocationFailure);
            }
            if (opts.calculate_slider_events &&
                !internal::set_slider_events(beatmap_, result_arena_, scratch_arena_, stacking_end_times)) {
                reset_working_result();
                return Error(ErrorCode::AllocationFailure);
            }
            if ((opts.calculate_slider_end_times || stacking) && !opts.calculate_slider_events &&
                !internal::set_slider_end_times(beatmap_, scratch_arena_, Span<internal::SliderTiming>(),
                                                stacking_end_times)) {
                reset_working_result();
                return Error(ErrorCode::AllocationFailure);
            }
            if (stacking && !internal::apply_stacking(beatmap_, result_arena_, stacking_end_times)) {
                reset_working_result();
                return Error(ErrorCode::AllocationFailure);
            }
            if (!stacking_end_times.empty()) {
                arena_pop_to(scratch_arena_, stacking_scratch_pos);
            }
            internal::apply_clock_rate(beatmap_, opts.mods);
            return &beatmap_;
        }

        void reset_working_result() {
            arena_clear(result_arena_);
            arena_clear(scratch_arena_);
            input_ = 0;
            input_size_ = 0;
            beatmap_ = Beatmap();
        }

        Arena* result_arena_;
        Arena* scratch_arena_;
        char* input_;
        size_t input_size_;
        Beatmap beatmap_;
    };

    namespace internal {
        inline ParserStorage parser_storage(Parser& parser) {
            ParserStorage storage;
            storage.result_arena = parser.result_arena_;
            storage.scratch_arena = parser.scratch_arena_;
            storage.input = parser.input_;
            storage.input_size = parser.input_size_;
            storage.input_storage_size = parser.input_size_ + kBufferPadding;
            return storage;
        }
    } // namespace internal

} // namespace fosu

#endif
