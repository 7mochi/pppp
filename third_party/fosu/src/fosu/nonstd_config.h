#ifndef FOSU_NONSTD_CONFIG_H
#define FOSU_NONSTD_CONFIG_H

#ifndef optional_CONFIG_SELECT_OPTIONAL
#define optional_CONFIG_SELECT_OPTIONAL optional_OPTIONAL_NONSTD
#endif
#ifndef optional_CONFIG_NO_EXCEPTIONS
#define optional_CONFIG_NO_EXCEPTIONS 1
#endif

#ifndef span_CONFIG_SELECT_SPAN
#define span_CONFIG_SELECT_SPAN span_SPAN_NONSTD
#endif
#ifndef span_CONFIG_NO_EXCEPTIONS
#define span_CONFIG_NO_EXCEPTIONS 1
#endif

#ifndef nssv_CONFIG_SELECT_STRING_VIEW
#define nssv_CONFIG_SELECT_STRING_VIEW nssv_STRING_VIEW_NONSTD
#endif
#ifndef nssv_CONFIG_NO_EXCEPTIONS
#define nssv_CONFIG_NO_EXCEPTIONS 1
#endif

#include <algorithm>
#include <exception>

#include <nonstd/atomic.hpp>
#include <nonstd/optional.hpp>
#include <nonstd/span.hpp>
#include <nonstd/string_view.hpp>

#endif
