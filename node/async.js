/** The promise form of a Koffi async call, shared by the calculators. */

import { check } from "./errors.js";
import { fn } from "./library.js";

/**
 * The mods handle has to outlive the queued call, so the callback frees it instead of the caller.
 */
export function asyncCall(call, handle, options, slot, mods, read, free) {
  return new Promise((resolve, reject) => {
    call.async(handle, options, slot, (error, status) => {
      try {
        if (error) {
          reject(error);
          return;
        }
        try {
          check(status);
          resolve(read(slot[0]));
        } finally {
          if (free !== undefined) {
            free(slot[0]);
          }
        }
      } catch (failure) {
        reject(failure);
      } finally {
        if (mods !== null) {
          fn.pppp_mods_free(mods);
        }
      }
    });
  });
}
