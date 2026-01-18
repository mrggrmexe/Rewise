#ifndef REWISE_STORAGE_STORAGEJSON_H
#define REWISE_STORAGE_STORAGEJSON_H

namespace rewise::storage::json_keys {

inline constexpr int kSchemaVersion = 1;

// Root keys
inline constexpr const char* kVersion = "version";
inline constexpr const char* kFolders = "folders";
inline constexpr const char* kCards = "cards";

} // namespace rewise::storage::json_keys

#endif // REWISE_STORAGE_STORAGEJSON_H
