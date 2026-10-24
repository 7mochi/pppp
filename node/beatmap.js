/** A parsed `.osu` file and its lazily read contents. */

import { promises, readFileSync } from "node:fs";
import { decode } from "koffi";

import { PPPP_OK } from "./constants.js";
import { check } from "./errors.js";
import { fn } from "./library.js";
import { camelCase } from "./names.js";
import { types } from "./native.g.js";

/** The public form of a beatmap struct, with its fields in camel case. */
function plain(native) {
  const out = {};
  for (const [key, value] of Object.entries(native)) {
    out[camelCase(key)] = value;
  }
  return out;
}

/** Decode the `count` values of a pointer the beatmap owns, flattened into an array. */
function array(pointer, type, count) {
  const total = Number(count);
  if (pointer === null || total === 0) {
    return [];
  }
  return decode(pointer, type, total);
}

/** The public form of a `pppp_slider_event_descriptor`. */
function sliderEvent(native) {
  return {
    type: native.type,
    time: native.time,
    spanIndex: native.span_index,
    spanStartTime: native.span_start_time,
    pathProgress: native.path_progress,
    position: { x: native.position.x, y: native.position.y },
  };
}

/**
 * The public form of a `pppp_slider`. Its arrays are pointers into the beatmap, so each is decoded
 * here with the count that belongs to it.
 */
function slider(native) {
  const eventType = types.pppp_slider_event_descriptor;
  return {
    slides: native.slides,
    expectedLength: native.expected_length,
    nodeSounds: array(native.node_sounds, "uint32", native.node_sound_count),
    controlPoints: array(native.control_points, types.pppp_vector2, native.control_point_count),
    path: array(native.path, types.pppp_vector2, native.path_count),
    cumulativeLengths: array(native.cumulative_lengths, "double", native.cumulative_length_count),
    undecimatedPath: array(
      native.undecimated_path,
      types.pppp_vector2,
      native.undecimated_path_count,
    ),
    undecimatedCumulativeLengths: array(
      native.undecimated_cumulative_lengths,
      "double",
      native.undecimated_cumulative_length_count,
    ),
    events: array(native.events, eventType, native.event_count).map(sliderEvent),
    catchEvents: array(native.catch_events, eventType, native.catch_event_count).map(sliderEvent),
  };
}

/** A loaded beatmap. */
class Beatmap {
  constructor(handle) {
    if (handle === undefined) {
      const error = new TypeError("use Beatmap.fromFile, Beatmap.fromFileSync or Beatmap.fromBytes");
      error.code = "ERR_INVALID_ARG_TYPE";
      throw error;
    }
    this._handle = handle;
    this._prepared = false;
    this._cache = null;
  }

  /** Parse a `Beatmap` by providing the content of a `.osu` file as a slice of bytes. */
  static fromBytes(data) {
    if (!(data instanceof Uint8Array)) {
      const error = new TypeError("expected a Uint8Array");
      error.code = "ERR_INVALID_ARG_TYPE";
      throw error;
    }
    const slot = [null];
    const status = fn.pppp_beatmap_from_bytes(data, data.length, slot);
    if (status !== PPPP_OK) {
      const error = new Error("cannot parse the beatmap");
      error.code = "ERR_PPPP_PARSE";
      throw error;
    }
    return new Beatmap(slot[0]);
  }

  /** Parse a `Beatmap` by providing a path to a `.osu` file, reading it synchronously. */
  static fromFileSync(file) {
    return Beatmap.fromBytes(readFileSync(file));
  }

  /** Parse a `Beatmap` by providing a path to a `.osu` file, resolved in a promise. */
  static async fromFile(file) {
    return Beatmap.fromBytes(await promises.readFile(file));
  }

  _ensureOpen() {
    if (this._handle === null) {
      const error = new Error("beatmap is not loaded");
      error.code = "ERR_PPPP_CLOSED";
      throw error;
    }
    return this._handle;
  }

  /** Reject a value that is not a loaded beatmap. */
  static require(value) {
    if (!(value instanceof Beatmap)) {
      const error = new TypeError("expected a Beatmap");
      error.code = "ERR_INVALID_ARG_TYPE";
      throw error;
    }
    return value._ensureOpen();
  }

  /** Free the native handle. Called by the collector; safe to call more than once. */
  close() {
    if (this._handle !== null) {
      fn.pppp_beatmap_free(this._handle);
      this._handle = null;
      this._prepared = false;
      this._cache = null;
    }
  }

  _prepare() {
    if (!this._prepared) {
      check(fn.pppp_beatmap_prepare_data(this._ensureOpen()));
      this._prepared = true;
    }
  }

  _data() {
    if (this._cache !== null) {
      return this._cache;
    }
    this._prepare();
    const handle = this._ensureOpen();

    const difficulty = [null];
    check(fn.pppp_beatmap_get_difficulty(handle, difficulty));

    this._cache = {
      difficulty: plain(difficulty[0]),
      hitObjects: this._array(handle, "pppp_beatmap_hit_objects", types.pppp_hit_object).map(plain),
      sliders: this._array(handle, "pppp_beatmap_sliders", types.pppp_slider).map(slider),
      timingPoints: this._array(handle, "pppp_beatmap_timing_points", types.pppp_timing_point).map(plain),
      breaks: this._array(handle, "pppp_beatmap_breaks", types.pppp_break_period).map(plain),
    };
    return this._cache;
  }

  /** Read one of the arrays the beatmap lends out. */
  _array(handle, name, type) {
    const values = [null];
    const count = [0];
    check(fn[name](handle, values, count));
    if (values[0] === null || count[0] === 0) {
      return [];
    }
    return decode(values[0], type, Number(count[0]));
  }

  get formatVersion() {
    return fn.pppp_beatmap_format_version(this._ensureOpen());
  }

  get mode() {
    return fn.pppp_beatmap_mode(this._ensureOpen());
  }

  get stackLeniency() {
    return fn.pppp_beatmap_stack_leniency(this._ensureOpen());
  }

  get difficulty() {
    return this._data().difficulty;
  }

  get hitObjects() {
    return this._data().hitObjects;
  }

  get sliders() {
    return this._data().sliders;
  }

  get timingPoints() {
    return this._data().timingPoints;
  }

  get breaks() {
    return this._data().breaks;
  }
}

export { Beatmap };
