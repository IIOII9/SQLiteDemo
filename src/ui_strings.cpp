#include "ui_strings.h"
#include <fstream>
#include <codecvt>
#include <locale>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::unordered_map<UIKey, std::wstring> UIStrings::translations;
// 映射枚举到 JSON 字符串键
static const std::unordered_map<UIKey, std::string> keyMap = {
    {UIKey::ChooseFile, "ChooseFile"},
    {UIKey::SupportedFiles, "SupportedFiles"},
    {UIKey::SaveSuccess, "SaveSuccess"},
    {UIKey::NoAttachment, "NoAttachment"},
    {UIKey::ExportAttachment, "ExportAttachment"},
    {UIKey::AttachmentSaved, "AttachmentSaved"},
    {UIKey::SaveFailed, "SaveFailed"},
    {UIKey::NoAttachmentInRecord, "NoAttachmentInRecord"},
    {UIKey::SelectRecordHint, "SelectRecordHint"},
    {UIKey::ReplaceAttachment, "ReplaceAttachment"},
    {UIKey::FileReadFailed, "FileReadFailed"},
    {UIKey::OnlyAllow, "OnlyAllow"},
    {UIKey::InvalidType, "InvalidType"},
};

// 加载 JSON 并填入 translations
bool UIStrings::LoadLanguage(const std::string& langJsonPath) {
    std::ifstream file(langJsonPath);
    if (!file.is_open()) return false;

    json j;
    try {
        file >> j;
    } catch (...) {
        return false;
    }

    // 重置已有内容
    translations.clear();

    for (const auto& [key, jsonKey] : keyMap) {
        if (j.contains(jsonKey)) {
            std::string utf8 = j[jsonKey];
            // UTF-8 -> wstring
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
            translations[key] = conv.from_bytes(utf8);
        }
    }

    return true;
}

const std::wstring& UIStrings::Get(UIKey key) {
    static const std::wstring fallback = L"";
    auto it = translations.find(key);
    return it != translations.end() ? it->second : fallback;
}
