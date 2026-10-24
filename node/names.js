/** The field names the public API exposes, in camelCase instead of the ABI's snake_case. */

/** `snake_case` to `camelCase`. */
export function camelCase(name) {
  return name.replace(/_([a-z])/g, (_, letter) => letter.toUpperCase());
}
