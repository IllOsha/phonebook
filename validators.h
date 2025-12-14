#pragma once
#include <string>
#include <regex>
#include <QDate>
#include <algorithm>
#include <cctype>
#include <QString>
#include <QChar>
#include <QRegularExpression>
#include <set>

namespace Validators {

inline std::string trim(const std::string& s) {
    size_t b = 0;
    while (b < s.size() && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    size_t e = s.size();
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

inline std::string normalize_name(const std::string& s) {
    QString qs = QString::fromStdString(s).trimmed();

    // Убираем пробелы вокруг дефиса
    qs.replace(QRegularExpression("\\s*-\\s*"), "-");

    // Удаляем ВСЕ пробелы/табы/переносы
    qs.remove(QRegularExpression("\\s+"));

    return qs.toStdString();
}

inline std::string normalize_phone(const std::string& s) {
    std::string t;
    for (char ch : s) {
        if (ch == ' ' || ch == '(' || ch == ')' || ch == '-' || ch == '\t') continue;
        t.push_back(ch);
    }
    return t;
}

inline std::string transliterate_for_email(const std::string& s) {
    static const std::vector<std::pair<QString, QString>> map = {
        { "а","a" },{ "б","b" },{ "в","v" },{ "г","g" },{ "д","d" },{ "е","e" },{ "ё","e" },
        { "ж","zh" },{ "з","z" },{ "и","i" },{ "й","y" },{ "к","k" },{ "л","l" },{ "м","m" },
        { "н","n" },{ "о","o" },{ "п","p" },{ "р","r" },{ "с","s" },{ "т","t" },{ "у","u" },
        { "ф","f" },{ "х","kh" },{ "ц","ts" },{ "ч","ch" },{ "ш","sh" },{ "щ","shch" },
        { "ъ","" },{ "ы","y" },{ "ь","" },{ "э","e" },{ "ю","yu" },{ "я","ya" }
    };
    QString qs = QString::fromStdString(s);
    qs = qs.toLower();
    QString out;
    for (int i = 0; i < qs.size(); ++i) {
        QString ch = qs.mid(i,1);
        bool replaced = false;
        for (auto &p : map) {
            if (p.first == ch) { out += p.second; replaced = true; break; }
        }
        if (!replaced) {
            QChar qc = ch[0];
            if (qc.isLetterOrNumber() || qc == '_' || qc == '.' || qc == '-') out += ch;
        }
    }
    return out.toStdString();
}

inline bool validname(const std::string& s) {
    std::string norm = normalize_name(s);
    if (norm.empty()) return false;

    QString t = QString::fromStdString(norm);

    if (t.startsWith('-') || t.endsWith('-')) return false;
    if (!t[0].isLetter()) return false;

    for (int i = 0; i < t.size(); ++i) {
        QChar ch = t[i];
        if (ch.isLetter() || ch.isDigit() || ch == '-') {
            continue;
        }
        return false;
    }
    return true;
}

inline bool validphone(const std::string& s) {
    std::string t = normalize_phone(s);
    static const std::regex re(R"(^(?:\+7|8)\d{10}$)");
    return std::regex_match(t, re);
}

inline std::string canonical_phone(const std::string& s) {
    return normalize_phone(s);
}

inline bool valiemail(const std::string& s) {
    std::string t = trim(s);
    if (t.empty()) return false;

    QString qmail = QString::fromStdString(t);
    qmail.replace(QRegularExpression("\\s*@\\s*"), "@");
    t = qmail.toStdString();
    static const std::regex re(
        R"(^[A-Za-z0-9]+([._%+\-][A-Za-z0-9]+)*@[A-Za-z0-9]+([\-\.][A-Za-z0-9]+)*\.[A-Za-z]{2,}$)"
    );
    if (!std::regex_match(t, re))
        return false;

    for (unsigned char ch : t) {
        if (ch >= 0xC0 && ch <= 0xFF)
            return false;
    }

    return true;
}

inline bool isPopularEmailDomain(const std::string& s) {
    auto at = s.find('@');
    if (at == std::string::npos) return false;
    std::string domain = s.substr(at + 1);
    std::transform(domain.begin(), domain.end(), domain.begin(), [](unsigned char c){ return std::tolower(c); });

    static const std::set<std::string> popular = {
        "gmail.com","yandex.ru","mail.ru","hotmail.com","outlook.com","icloud.com",
        "yandex.com","gmail.ru","inbox.ru","list.ru","bk.ru","rambler.ru","ya.ru"
    };
    return popular.find(domain) != popular.end();
}

inline bool validbirthday(const std::string& s) {
    auto t = trim(s);
    if (t.empty()) return false;
    QDate date = QDate::fromString(QString::fromStdString(t), "dd.MM.yyyy");
    if (!date.isValid()) return false;
    if (date >= QDate::currentDate()) return false;
    return true;
}

inline bool validaddress(const std::string& s) {
    return !trim(s).empty();
}

}
