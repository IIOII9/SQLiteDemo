// ===================== ui_strings.h =====================
#ifndef UI_STRINGS_H
#define UI_STRINGS_H
#include <string>
#include <unordered_map>
// #include <functional>
enum class UIKey {
    ChooseFile,
    SupportedFiles,
    SaveSuccess,
    NoAttachment,
    ExportAttachment,
    AttachmentSaved,
    SaveFailed,
    NoAttachmentInRecord,
    SelectRecordHint,
    ReplaceAttachment,
    FileReadFailed,
    OnlyAllow,
    InvalidType
    // ...
};
class UIStrings {
    public:
        static bool LoadLanguage(const std::string& lang);        // 加载 JSON 文件
        static const std::wstring& Get(UIKey key);                // 获取翻译文本
    private:
    static std::unordered_map<UIKey, std::wstring> translations;  
    };
#endif // UI_STRINGS_H