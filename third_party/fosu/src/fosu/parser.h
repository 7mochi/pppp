#ifndef FOSU_PARSER_H
#define FOSU_PARSER_H

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>

#include <fosu/beatmap.h>
#include <fosu/engine/parse_document.h>
#include <fosu/engine/parsing/key_value.h>
#include <fosu/engine/runtime/loader.h>
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
        inline size_t velocity_preset_capacity(nonstd::string_view document) {
            const nonstd::string_view field("VelocityPresets");
            size_t capacity = 3;
            size_t search_from = 0;
            for (;;) {
                const size_t field_begin = document.find(field, search_from);
                if (field_begin == nonstd::string_view::npos) {
                    break;
                }
                const size_t key_end = field_begin + field.size();
                const size_t line_end = document.find('\n', key_end);
                const size_t value_end = line_end == nonstd::string_view::npos ? document.size() : line_end;
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
        inline bool allocate_span(Arena* arena, nonstd::span<T>& span, size_t capacity) {
            T* values = arena_push_array<T>(arena, capacity);
            if (!values) {
                return false;
            }
            span = nonstd::span<T>(values, capacity);
            return true;
        }

        inline bool allocate_beatmap_arrays(Arena* arena, Beatmap& beatmap, nonstd::string_view input,
                                            fosu::u32 selected_sections, bool lazer_format,
                                            bool segments) {
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
                if (segments) {
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
              beatmap_(),
              error_() {}

        ~Parser() {
            arena_release(result_arena_);
            arena_release(scratch_arena_);
        }

        // Returns null on failure; error() then says why.
        Beatmap* parse(const char* data, size_t size, const ParseOptions& opts = ParseOptions()) {
            reset_working_result();
            if ((!data && size) || invalid_options(opts)) {
                return fail(ParseErrorCode::InvalidOptions);
            }
            if (!prepare_input(size, data)) {
                return fail(ParseErrorCode::OutOfMemory);
            }
            return finish_parse(opts);
        }

        Beatmap* parse(nonstd::string_view input, const ParseOptions& opts = ParseOptions()) {
            return parse(input.data(), input.size(), opts);
        }

        Beatmap* parse(const FileBuffer& input, const ParseOptions& opts = ParseOptions()) {
            return parse(input.data, input.size, opts);
        }

        Beatmap* parse_file(const char* path, const ParseOptions& opts = ParseOptions()) {
            reset_working_result();
            if (!path || invalid_options(opts)) {
                return fail(ParseErrorCode::InvalidOptions);
            }

            const internal::InputFile file = internal::open_input_file(path);
            if (file == internal::kInvalidInputFile) {
                return fail(ParseErrorCode::ReadFailed);
            }
            u64 size64 = 0;
            if (!internal::input_file_size(file, size64) ||
                size64 > static_cast<u64>(std::numeric_limits<size_t>::max())) {
                const int error = errno;
                internal::close_input_file(file);
                errno = error;
                return fail(ParseErrorCode::ReadFailed);
            }
            const size_t size = static_cast<size_t>(size64);
            if (!can_pad_input(size)) {
                internal::close_input_file(file);
                return fail(ParseErrorCode::OutOfMemory);
            }

            if (!prepare_input(size, 0)) {
                internal::close_input_file(file);
                return fail(ParseErrorCode::OutOfMemory);
            }
            size_t done = 0;
            while (done < size) {
                const ptrdiff_t count = internal::read_input_file(file, input_ + done, size - done);
                if (count < 0 && errno == EINTR) {
                    continue;
                }
                if (count <= 0) {
                    const int error = errno;
                    internal::close_input_file(file);
                    errno = error ? error : EIO;
                    return fail(ParseErrorCode::ReadFailed);
                }
                done += static_cast<size_t>(count);
            }
            const int error = errno;
            internal::close_input_file(file);
            errno = error;
            return finish_parse(opts);
        }

        // Why the last parse failed; code is None after a success.
        ParseError error() const { return error_; }

    private:
        friend internal::ParserStorage internal::parser_storage(Parser& parser);

        Parser(const Parser&);
        Parser& operator=(const Parser&);

        Beatmap* fail(ParseErrorCode::Value code, fosu::u32 line = 0) {
            reset_working_result();
            error_ = ParseError(code, line);
            return 0;
        }

        static bool invalid_sections(fosu::u32 sections) {
            return (sections & ~static_cast<fosu::u32>(kAllSections)) != 0;
        }

        static bool invalid_options(const ParseOptions& opts) {
            if (invalid_sections(opts.sections) || internal::invalid_mods(opts.mods)) {
                return true;
            }
            return internal::has_difficulty_mod(opts.mods) &&
                   (opts.sections & (kSectionGeneral | kSectionDifficulty)) !=
                       (kSectionGeneral | kSectionDifficulty);
        }

        bool prepare_input(size_t size, const char* data) {
            if (!can_pad_input(size)) {
                return false;
            }
            if (!result_arena_) {
                result_arena_ = arena_alloc();
            }
            if (!scratch_arena_) {
                scratch_arena_ = arena_alloc();
            }
            if (!result_arena_ || !scratch_arena_) {
                return false;
            }
            input_ = static_cast<char*>(arena_push(result_arena_, size + kBufferPadding, 1));
            if (!input_) {
                return false;
            }
            if (data && size) {
                std::memmove(input_, data, size);
            }
            std::memset(input_ + size, 0, kBufferPadding);
            input_size_ = size;
            return true;
        }

        Beatmap* finish_parse(const ParseOptions& opts) {
            const nonstd::string_view input(input_, input_size_);
            const internal::Preamble preamble =
                internal::parse_preamble(beatmap_, input.data(), input.data() + input.size(), opts.client);
            if (preamble.error.code != ParseErrorCode::None) {
                return fail(preamble.error.code, preamble.error.line);
            }
            if (input_size_ != 0 &&
                !internal::allocate_beatmap_arrays(result_arena_, beatmap_, input, opts.sections,
                                                   reads_lazer_format(beatmap_.format_version, opts.client),
                                                   opts.client == Client::Lazer)) {
                return fail(ParseErrorCode::OutOfMemory);
            }
            const ParseError parse_error =
                internal::selected_engine()->parse_document(input.data(), input.size(), beatmap_, opts);
            if (parse_error.code != ParseErrorCode::None) {
                return fail(parse_error.code, parse_error.line);
            }
            if (!internal::apply_legacy_rules(beatmap_, scratch_arena_)) {
                return fail(ParseErrorCode::OutOfMemory);
            }
            if (!internal::apply_mods_before_calculations(beatmap_, opts.mods)) {
                return fail(ParseErrorCode::InvalidOptions);
            }
            const bool stacking = opts.apply_stacking && beatmap_.mode == 0;
            size_t stacking_scratch_pos = 0;
            nonstd::span<double> stacking_end_times;
            if (stacking && !beatmap_.sliders.empty()) {
                stacking_scratch_pos = arena_pos(scratch_arena_);
                double* end_times = arena_push_array<double>(scratch_arena_, beatmap_.sliders.size());
                if (!end_times) {
                    return fail(ParseErrorCode::OutOfMemory);
                }
                stacking_end_times = nonstd::span<double>(end_times, beatmap_.sliders.size());
            }
            if ((opts.calculate_slider_paths || opts.calculate_slider_events || stacking) &&
                !internal::set_slider_paths(beatmap_, opts.client, result_arena_, scratch_arena_)) {
                return fail(ParseErrorCode::OutOfMemory);
            }
            if (opts.calculate_slider_events &&
                !internal::set_slider_events(beatmap_, opts.client, result_arena_, scratch_arena_,
                                             stacking_end_times)) {
                return fail(ParseErrorCode::OutOfMemory);
            }
            if ((opts.calculate_slider_end_times || stacking) && !opts.calculate_slider_events &&
                !internal::set_slider_end_times(beatmap_, opts.client, scratch_arena_,
                                                nonstd::span<internal::SliderTiming>(), stacking_end_times)) {
                return fail(ParseErrorCode::OutOfMemory);
            }
            if (stacking && !internal::apply_stacking(beatmap_, result_arena_, stacking_end_times)) {
                return fail(ParseErrorCode::OutOfMemory);
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
            error_ = ParseError();
        }

        Arena* result_arena_;
        Arena* scratch_arena_;
        char* input_;
        size_t input_size_;
        Beatmap beatmap_;
        ParseError error_;
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
