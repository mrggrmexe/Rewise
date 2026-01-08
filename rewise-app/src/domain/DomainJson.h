#ifndef REWISE_DOMAIN_DOMAINJSON_H
#define REWISE_DOMAIN_DOMAINJSON_H

namespace rewise::domain::json_keys {

// Common
inline constexpr const char* kId = "id";
inline constexpr const char* kName = "name";

// Folder
inline constexpr const char* kFolders = "folders";

// Card
inline constexpr const char* kCards = "cards";
inline constexpr const char* kFolderId = "folderId";
inline constexpr const char* kQuestion = "question";
inline constexpr const char* kAnswer = "answer";
inline constexpr const char* kCreatedAtMs = "createdAtMs";
inline constexpr const char* kUpdatedAtMs = "updatedAtMs";

} // namespace rewise::domain::json_keys

#endif // REWISE_DOMAIN_DOMAINJSON_H
