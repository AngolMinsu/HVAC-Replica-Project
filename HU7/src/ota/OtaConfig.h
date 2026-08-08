#pragma once

#ifndef OTA_SERVER_BASE_URL
#define OTA_SERVER_BASE_URL "http://172.30.1.12:8080"
#endif

namespace hu7::ota {

inline constexpr char kServerBaseUrl[] = OTA_SERVER_BASE_URL;
inline constexpr unsigned long kHttpTimeoutMs = 5000;

}  // namespace hu7::ota
