// SPDX-License-Identifier: MPL-2.0
// ble-wrapper (blew) — © 2019–2026 Sebyone Srl
#pragma once

#if defined(BLEW_STATIC) || !defined(BLEW_SHARED)
#define BLEW_EXPORT
#elif defined(_WIN32)
#ifdef BLEW_BUILDING
#define BLEW_EXPORT __declspec(dllexport)
#else
#define BLEW_EXPORT __declspec(dllimport)
#endif
#else
#define BLEW_EXPORT __attribute__((visibility("default")))
#endif
