/** The error types raised by the package. */

import { PPPP_ALLOCATION, PPPP_OK } from "./constants.js";
import { fn } from "./library.js";

/** An error that reports the native message of a `pppp_result`. */
function nativeError(status) {
  if (status === PPPP_ALLOCATION) {
    return new RangeError("cannot allocate");
  }

  const error = new Error(fn.pppp_result_message(status));
  error.code = "ERR_PPPP";
  return error;
}

/** Throw unless `status` is `PPPP_OK`. */
function check(status) {
  if (status !== PPPP_OK) {
    throw nativeError(status);
  }
}

export { check };
