// Calculated slider paths. Expected values match osu!'s SliderPath and its
// legacy decoder (upstream tests/test_slider_paths.cc).
#include "test_support.h"

#include <fosu/enums.h>
#include <fosu/slider_path.h>

#include <cmath>
#include <vector>

using namespace fosu;

namespace {
    std::string beatmap(const std::string& hit_objects, int version = 14) {
        return "osu file format v" + to_str(version) +
               "\n[Difficulty]\nSliderMultiplier:1\n[TimingPoints]\n0,500\n"
               "[HitObjects]\n" +
               hit_objects;
    }

    ParseOptions path_options(Client::Value client) {
        ParseOptions options = lazer_options();
        options.calculate_slider_paths = true;
        options.client = client;
        return options;
    }

    // A copy that outlives the parser's next call.
    struct Path {
        std::vector<PathPoint> points;
        double distance;

        bool operator==(const Path& other) const {
            return points == other.points && distance == other.distance;
        }
    };

    Path path(const std::string& input, size_t object = 0, Client::Value client = Client::Stable) {
        const Beatmap map = parse_str(input, path_options(client));
        const SliderPath& slider_path = map.slider_paths[map.hit_objects[object].slider];
        Path result;
        result.points.assign(slider_path.points.begin(), slider_path.points.end());
        result.distance = slider_path.distance();
        return result;
    }

    // The same slider with another curve description. Only lazer has v128 rules.
    Path path_as(const std::string& slider, int version = 14) {
        return path(beatmap("0,0,1000,2,0," + slider + "\n", version), 0,
                    version >= 128 ? Client::Lazer : Client::Stable);
    }

    Path lazer_path_as(const std::string& slider) {
        return path(beatmap("0,0,1000,2,0," + slider + "\n"), 0, Client::Lazer);
    }

    std::vector<PathPoint> points4(float x0, float y0, float x1, float y1, float x2, float y2, float x3,
                                   float y3) {
        std::vector<PathPoint> v;
        v.push_back(PathPoint::make(x0, y0));
        v.push_back(PathPoint::make(x1, y1));
        v.push_back(PathPoint::make(x2, y2));
        v.push_back(PathPoint::make(x3, y3));
        return v;
    }
} // namespace

TEST_CASE("the declared length trims or extends the path") {
    const Path trimmed = path(beatmap("10,20,1000,2,0,L|110:20,2,50\n"));
    CHECK(trimmed.distance == 50);
    CHECK(trimmed.points.back() == PathPoint::make(50, 0));
    const Path extended = path(beatmap("10,20,1000,2,0,L|110:20,2,150\n"));
    CHECK(extended.distance == 150);
    CHECK(extended.points.back() == PathPoint::make(150, 0));
}

TEST_CASE("each curve type honours the declared length") {
    static const char* const curves[] = {"B|110:120|210:20", "P|110:120|210:20", "C|110:120|210:20"};
    for (size_t i = 0; i < 3; i++) {
        const std::string input = beatmap(std::string("10,20,1000,2,0,") + curves[i] + ",2,200\n");
        CHECK(path(input).distance == 200);
    }
}

// A path only extends along a final segment with a direction.
TEST_CASE("a repeated end point prevents extension") {
    CHECK(path(beatmap("10,20,1000,2,0,L|110:20|110:20,2,150\n")).distance == 100);
    // lazer's duplicate-last-position-slider.osu.
    CHECK(path(beatmap("261,171,1000,2,0,B|262:171|262:171|262:171,1,2\n")).distance == 1);
}

// A repeated last point ends a Bezier segment instead, so the path still
// extends along the curve's final direction.
TEST_CASE("a repeated bezier end point still extends") {
    CHECK(path_as("B|100:100|200:0|200:0,1,500").distance == 500);
}

TEST_CASE("a point at the head makes a zero length path") {
    CHECK_UNARY(path(beatmap("10,20,1000,2,0,B|10:20,2,100\n")).distance == 0);
}

// A repeated point ends one Bezier segment and starts the next; repeating it
// again changes nothing.
TEST_CASE("a repeated point splits bezier segments") {
    CHECK(path_as("B|100:0|100:0|100:100,1,0") == path_as("L|100:0|100:100,1,0"));
    CHECK(path_as("B|1:1|2:2|3:3|3:3|3:3|3:3|4:4,2,200") == path_as("B|1:1|2:2|3:3|3:3|4:4,2,200"));
}

// A perfect curve needs exactly three points that are not collinear.
TEST_CASE("the perfect curve falls back for unusable points") {
    CHECK(path_as("P|100:100,1,0") == path_as("L|100:100,1,0"));
    CHECK(path_as("P|100:0|200:0,1,0") == path_as("L|100:0|200:0,1,0"));
    CHECK(path_as("P|100:100|200:0|300:100,1,0") == path_as("B|100:100|200:0|300:100,1,0"));
    // Lazer-format maps keep the perfect curve type, but a triangle with
    // |cross product| <= 1e-3 has no usable circle: osu! draws a Bezier.
    CHECK(path_as("P|100:0|200:0,1,0", 128) == path_as("B|100:0|200:0,1,0", 128));
    CHECK(path_as("P|200:0|100:0,1,0", 128) == path_as("B|200:0|100:0,1,0", 128));
    const Path nearly = path_as("P|100:0|200:0.00001,1,200", 128);
    CHECK(nearly.points.size() == 3u);
    CHECK(nearly.distance == 200);
}

TEST_CASE("the perfect curve follows a circular arc") {
    const Path arc = path_as("P|100:100|200:0,1,0");
    // A semicircle of radius 100 approximated by chords.
    CHECK(std::fabs(arc.distance - 314.05381774902344) <= 1e-3);
    CHECK(std::fabs(arc.points.back().x - 200) <= 1e-3);
    CHECK(std::fabs(arc.points.back().y - 0) <= 1e-3);
}

// lazer's catmull-duplicate-initial-controlpoint.osu.
TEST_CASE("a catmull point at the head is merged") {
    CHECK(path(beatmap("200,304,1000,2,0,C|200:304|288:304|288:208|352:208,1,260\n")) ==
          path(beatmap("200,304,1000,2,0,C|288:304|288:208|352:208,1,260\n")));
}

// lazer's adjacent-catmull-segments.osu. Stable-format maps keep one Catmull
// curve through the repeated points. Lazer-format maps split there, and a
// two-point Catmull segment is straight.
TEST_CASE("repeated catmull points split only in lazer format") {
    const std::string slider = "200,304,1000,2,0,C|288:304|288:304|288:208|288:208|352:208,1,0\n";
    const Path lazer = path(beatmap(slider, 128), 0, Client::Lazer);
    CHECK(std::fabs(lazer.distance - (88 + 96 + 64)) <= 1e-3);
    CHECK(std::fabs(lazer.points.back().x - 152) <= 1e-3);
    CHECK(std::fabs(lazer.points.back().y - (-96)) <= 1e-3);
    CHECK(path(beatmap(slider)).distance > lazer.distance);
}

// lazer's multi-segment-slider.osu: a letter starts a segment at the
// preceding point.
TEST_CASE("lazer format letters start new segments") {
    const std::string input =
        beatmap("63,301,1000,2,0,P|224:57|B|439:298|131:316|322:169|155:194,1,1040\n", 128);
    const Beatmap map = parse_str(input, path_options(Client::Lazer));
    const nonstd::span<const CurveSegment> segments = map.slider_segments;
    CHECK(segments.size() == 2u);
    CHECK(segments[0].type == CurveType::PerfectCurve);
    CHECK(segments[1].type == CurveType::Bezier);
    // The arc ends where the Bezier starts.
    bool joined = false;
    for (size_t i = 0; i < map.slider_paths[0].points.size(); i++) {
        joined = joined || map.slider_paths[0].points[i] == PathPoint::make(376, -3);
    }
    CHECK(joined);
    CHECK(map.slider_paths[0].distance() == 1040);
}

// Before v128, lazer also starts a segment at each letter, then applies the
// legacy rules to each segment, ending at the next one's first point. stable
// gives the whole slider the last type instead; see test_clients.
TEST_CASE("lazer letters start legacy segments") {
    CHECK(lazer_path_as("L|100:0|B|100:100|0:100,1,0").points == points4(0, 0, 100, 0, 100, 100, 0, 100));
    // A perfect curve needs exactly three points, counting that end point,
    // and draws a line through collinear ones.
    const Path arc = lazer_path_as("P|100:100|L|200:0|300:0,1,0");
    CHECK(std::fabs(arc.distance - (314.05381774902344 + 100)) <= 1e-3);
    CHECK(lazer_path_as("P|100:0|L|200:0|200:100,1,0").points == points4(0, 0, 100, 0, 200, 0, 200, 100));
    // A last segment holding only its first point adds nothing.
    std::vector<PathPoint> three;
    three.push_back(PathPoint::make(0, 0));
    three.push_back(PathPoint::make(100, 0));
    three.push_back(PathPoint::make(100, 100));
    CHECK(lazer_path_as("L|100:0|B|100:100,1,0").points == three);
    // A repeated point splits a segment, unless it is the segment's last
    // before the next one's first.
    CHECK(std::fabs(lazer_path_as("B|100:100|100:100|200:0|L|300:100,1,0").distance - 370.9743161201477) <=
          1e-3);
    CHECK(std::fabs(lazer_path_as("B|100:100|200:0|200:0|L|300:100,1,0").distance - 338.47502517700195) <=
          1e-3);
}

TEST_CASE("the bspline degree shapes the curve") {
    // Degree 1 joins the control points with straight lines.
    const Path linear = path_as("B1|100:0|200:50|300:0,1,0", 128);
    CHECK(linear.points == points4(0, 0, 100, 0, 200, 50, 300, 0));
    // Higher degrees curve between them, even with distant points.
    const Path distant = path_as("B8|10000:0|-10000:10000|10000:-10000|-10000:0|10000:10000|"
                                 "-10000:-10000|10000:0|0:10000,1,500",
                                 128);
    CHECK(distant.distance == 500);
    CHECK(std::fabs(distant.points.back().x - 499.805969) <= 1e-3);
    CHECK(std::fabs(distant.points.back().y - 11.9145603) <= 1e-3);
}

TEST_CASE("segments of different types join") {
    CHECK(path_as("L|100:0|B3|100:100|0:100,1,300", 128).points == points4(0, 0, 100, 0, 100, 100, 0, 100));
}

TEST_CASE("the position query clamps and interpolates") {
    const Beatmap map = parse_str(beatmap("10,20,1000,2,0,L|110:20,1,100\n"), path_options(Client::Stable));
    const SliderPath& slider_path = map.slider_paths[0];
    CHECK(slider_position_at(slider_path, 0.5) == PathPoint::make(50, 0));
    CHECK(slider_position_at(slider_path, -1) == PathPoint::make(0, 0));
    CHECK(slider_position_at(slider_path, 2) == PathPoint::make(100, 0));
}
