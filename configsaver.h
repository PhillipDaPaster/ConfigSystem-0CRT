#pragma once
#define CFG_USE_IMCOLOR // define this if you want to use imcolor 

#ifdef CFG_USE_IMCOLOR
#include "imgui_internal.h" // put your projects file path here 
#endif

extern "C" {
    void* __stdcall CreateFileA(
        const char* lpFileName,
        unsigned long dwDesiredAccess,
        unsigned long dwShareMode,
        void* lpSecurityAttributes,
        unsigned long dwCreationDisposition,
        unsigned long dwFlagsAndAttributes,
        void* hTemplateFile
    );
    int __stdcall ReadFile(
        void* hFile,
        void* lpBuffer,
        unsigned long nNumberOfBytesToRead,
        unsigned long* lpNumberOfBytesRead,
        void* lpOverlapped
    );
    int __stdcall WriteFile(
        void* hFile,
        const void* lpBuffer,
        unsigned long nNumberOfBytesToWrite,
        unsigned long* lpNumberOfBytesWritten,
        void* lpOverlapped
    );
    int __stdcall CloseHandle(void* hObject);
}

#ifndef CFG_INVALID_HANDLE_VALUE
#define CFG_INVALID_HANDLE_VALUE ((void*)-1)
#endif
#ifndef CFG_GENERIC_READ
#define CFG_GENERIC_READ 0x80000000ul
#endif
#ifndef CFG_GENERIC_WRITE
#define CFG_GENERIC_WRITE 0x40000000ul
#endif
#ifndef CFG_FILE_SHARE_READ
#define CFG_FILE_SHARE_READ 0x00000001ul
#endif
#ifndef CFG_OPEN_EXISTING
#define CFG_OPEN_EXISTING 3ul
#endif
#ifndef CFG_CREATE_ALWAYS
#define CFG_CREATE_ALWAYS 2ul
#endif
#ifndef CFG_FILE_ATTRIBUTE_NORMAL
#define CFG_FILE_ATTRIBUTE_NORMAL 0x00000080ul
#endif

namespace CFG {

    struct ConfigVarInitializerLite {
        const char* m_cpp_ns_path{ nullptr };
        const char* m_name{ nullptr };
        unsigned int m_key_hash{ 0 };
#ifndef CFG_KEY_MAX
#define CFG_KEY_MAX 192
#endif
        char m_key[CFG_KEY_MAX]{ 0 };
        void* m_ptr{ nullptr };
        unsigned int m_type_id{ 0 };
        bool m_no_save{ false };
    };

#ifndef CONFIGSAVER_LITE_MAX_VARS
#define CONFIGSAVER_LITE_MAX_VARS 512
#endif

    typedef unsigned long long size_t_like;

    inline ConfigVarInitializerLite vars[CONFIGSAVER_LITE_MAX_VARS]{};
    inline size_t_like vars_count = 0;

    enum TypeId : unsigned int {
        TYPE_BOOL = 1,
        TYPE_INT = 2,
        TYPE_FLOAT = 3,
        TYPE_IMCOLOR = 4
    };

    template <typename T>
    inline constexpr unsigned int TypeIdOf() { return 0; }

    template <>
    inline constexpr unsigned int TypeIdOf<bool>() { return TYPE_BOOL; }

    template <>
    inline constexpr unsigned int TypeIdOf<int>() { return TYPE_INT; }

    template <>
    inline constexpr unsigned int TypeIdOf<float>() { return TYPE_FLOAT; }
#ifdef CFG_USE_IMCOLOR
    template <>
    inline constexpr unsigned int TypeIdOf<ImColor>() { return TYPE_IMCOLOR; }
#endif

    static inline bool IsWs(char c) {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    }

    static inline int StrEq(const char* a, const char* b) {
        if (a == b) return 1;
        if (!a || !b) return 0;
        while (*a && *b) {
            if (*a != *b) return 0;
            ++a; ++b;
        }
        return (*a == '\0' && *b == '\0') ? 1 : 0;
    }

    static inline unsigned int Fnv1a32_Init() { return 2166136261u; }
    static inline unsigned int Fnv1a32_Add(unsigned int h, char c) { return (h ^ (unsigned char)c) * 16777619u; }
    static inline unsigned int HashKeyString(const char* s) {
        unsigned int h = Fnv1a32_Init();
        if (!s) return h;
        for (const char* p = s; *p; ++p)
            h = Fnv1a32_Add(h, *p);
        return h;
    }

    static inline void CopyChar(char* dst, size_t_like cap, size_t_like& n, char c) {
        if (n + 1 < cap) dst[n++] = c;
    }

    inline bool NextNsSegment(const char*& p, char* out, size_t_like out_cap) {
        if (!p || !out || out_cap == 0)
            return false;

        for (;;) {
            while (IsWs(*p)) ++p;
            if (p[0] == ':' && p[1] == ':') { p += 2; continue; }
            if (p[0] == '.') { ++p; continue; }
            break;
        }

        if (*p == '\0')
            return false;

        size_t_like n = 0;
        while (*p != '\0') {
            if (p[0] == ':' && p[1] == ':')
                break;
            if (p[0] == '.')
                break;
            if (IsWs(*p)) {
                ++p;
                continue;
            }
            CopyChar(out, out_cap, n, *p);
            ++p;
        }
        out[n] = '\0';
        return n != 0;
    }

    inline void BuildKey(const char* cpp_ns_path, const char* name, char* out, size_t_like out_cap) {
        if (!out || out_cap == 0) return;
        size_t_like n = 0;
        const char* p = cpp_ns_path ? cpp_ns_path : "";
        char seg[128];
        bool first = true;
        while (NextNsSegment(p, seg, (size_t_like)sizeof(seg))) {
            if (!first) CopyChar(out, out_cap, n, '.');
            first = false;
            for (const char* s = seg; *s; ++s) CopyChar(out, out_cap, n, *s);
        }
        if (name && *name) {
            if (!first) CopyChar(out, out_cap, n, '.');
            for (const char* s = name; *s; ++s) CopyChar(out, out_cap, n, *s);
        }
        out[n] = '\0';
    }

    inline unsigned int BuildKeyHash(const char* cpp_ns_path, const char* name) {
        unsigned int h = Fnv1a32_Init();
        const char* p = cpp_ns_path ? cpp_ns_path : "";
        char seg[128];
        bool first = true;
        while (NextNsSegment(p, seg, (size_t_like)sizeof(seg))) {
            if (!first) h = Fnv1a32_Add(h, '.');
            first = false;
            for (const char* s = seg; *s; ++s) h = Fnv1a32_Add(h, *s);
        }
        if (name && *name) {
            if (!first) h = Fnv1a32_Add(h, '.');
            for (const char* s = name; *s; ++s) h = Fnv1a32_Add(h, *s);
        }
        return h;
    }

    inline void RegisterVar(const char* cpp_ns_path, const char* name, void* ptr, unsigned int type_id, bool no_save) {
        if (vars_count >= CONFIGSAVER_LITE_MAX_VARS)
            return;
        ConfigVarInitializerLite v{};
        v.m_cpp_ns_path = cpp_ns_path;
        v.m_name = name;
        v.m_ptr = ptr;
        v.m_type_id = type_id;
        v.m_no_save = no_save;
        v.m_key_hash = BuildKeyHash(cpp_ns_path, name);
        BuildKey(cpp_ns_path, name, v.m_key, (size_t_like)CFG_KEY_MAX);
        vars[vars_count++] = v;
    }

    static inline void TrimRight(char* s) {
        if (!s) return;
        char* end = s;
        while (*end) ++end;
        while (end > s && IsWs(end[-1])) --end;
        *end = '\0';
    }

    static inline char* SkipWs(char* p) {
        while (p && IsWs(*p)) ++p;
        return p;
    }

    static inline int ParseInt(const char* s, int& out) {
        if (!s) return 0;
        int sign = 1;
        if (*s == '-') { sign = -1; ++s; }
        int v = 0;
        int any = 0;
        while (*s >= '0' && *s <= '9') {
            any = 1;
            v = v * 10 + (*s - '0');
            ++s;
        }
        out = v * sign;
        return any;
    }

    static inline int ParseFloat(const char* s, float& out) {
        if (!s)
            return 0;

        int sign = 1;
        if (*s == '-') { sign = -1; ++s; }
        double v = 0.0;
        int any = 0;
        while (*s >= '0' && *s <= '9') {
            any = 1;
            v = v * 10.0 + double(*s - '0');
            ++s;
        }
        if (*s == '.') {
            ++s;
            double place = 0.1;
            while (*s >= '0' && *s <= '9') {
                any = 1;
                v += double(*s - '0') * place;
                place *= 0.1;
                ++s;
            }
        }
        if (!any)
            return 0;

        out = float(v * double(sign));
        return 1;
    }

#ifdef CFG_USE_IMCOLOR
    static inline int ParseImColor(const char* s, ImColor& out) {
        if (!s) return 0;
        float v[4]{};
        const char* p = s;
        for (int i = 0; i < 4; ++i) {
            while (IsWs(*p)) ++p;
            if (!ParseFloat(p, v[i])) return 0;
            while (*p && *p != ',') ++p;
            if (i != 3) {
                if (*p != ',')
                    return 0;
                ++p;
            }
        }
        out = ImColor(v[0], v[1], v[2], v[3]);
        return 1;
    }
#endif

    inline void Save(const char* filepath) {
        if (!filepath || !*filepath)
            return;

        void* h = CreateFileA(
            filepath,
            CFG_GENERIC_WRITE,
            CFG_FILE_SHARE_READ,
            0,
            CFG_CREATE_ALWAYS,
            CFG_FILE_ATTRIBUTE_NORMAL,
            0
        );
        if (!h || h == CFG_INVALID_HANDLE_VALUE) return;

        auto write_all = [&](const char* s) {
            if (!s)
                return;

            unsigned long long len64 = 0;
            const char* p = s;
            while (*p++) ++len64;
            unsigned long len = (len64 > 0xFFFFFFFFull) ? 0xFFFFFFFFul : (unsigned long)len64;
            unsigned long written = 0;
            if (len) WriteFile(h, s, len, &written, 0);
        };

        char val[160];

        for (size_t_like i = 0; i < vars_count; ++i) {
            const auto& var = vars[i];
            if (var.m_no_save) continue;

            write_all(var.m_key);
            write_all("=");

            if (var.m_type_id == TYPE_BOOL) {
                val[0] = (*static_cast<bool*>(var.m_ptr)) ? '1' : '0';
                val[1] = '\0';
                write_all(val);
                write_all("\n");
            }
            else if (var.m_type_id == TYPE_INT) {
                int v = *static_cast<int*>(var.m_ptr);
                char tmp[32];
                int n = 0;
                unsigned int uv = (v < 0) ? unsigned(-v) : unsigned(v);
                do { tmp[n++] = char('0' + (uv % 10)); uv /= 10; } while (uv && n < 30);
                int o = 0;
                if (v < 0) val[o++] = '-';
                while (n--) val[o++] = tmp[n];
                val[o] = '\0';
                write_all(val);
                write_all("\n");
            }
            else if (var.m_type_id == TYPE_FLOAT) {
                float fv = *static_cast<float*>(var.m_ptr);
                int sign = (fv < 0.0f) ? -1 : 1;
                if (fv < 0.0f) fv = -fv;
                int ip = (int)fv;
                int fp = (int)((fv - (float)ip) * 1000.0f + 0.5f);
                char* p = val;
                if (sign < 0) *p++ = '-';
                char tmp[32];
                int n = 0;
                unsigned int uv = (unsigned int)ip;
                do { tmp[n++] = char('0' + (uv % 10)); uv /= 10; } while (uv && n < 30);
                while (n--) *p++ = tmp[n];
                *p++ = '.';
                *p++ = char('0' + (fp / 100) % 10);
                *p++ = char('0' + (fp / 10) % 10);
                *p++ = char('0' + (fp / 1) % 10);
                *p = '\0';
                write_all(val);
                write_all("\n");
            }
#ifdef CFG_USE_IMCOLOR
            else if (var.m_type_id == TYPE_IMCOLOR) {
                ImColor c = *static_cast<ImColor*>(var.m_ptr);
                char* p = val;
                float comps[4] = { c.Value.x, c.Value.y, c.Value.z, c.Value.w };
                for (int k = 0; k < 4; ++k) {
                    if (k) *p++ = ',';
                    float fv = comps[k];
                    int sign = (fv < 0.0f) ? -1 : 1;
                    if (fv < 0.0f) fv = -fv;
                    int ip = (int)fv;
                    int fp = (int)((fv - (float)ip) * 1000.0f + 0.5f);
                    if (sign < 0) *p++ = '-';
                    char tmp[32];
                    int n = 0;
                    unsigned int uv = (unsigned int)ip;
                    do { tmp[n++] = char('0' + (uv % 10)); uv /= 10; } while (uv && n < 30);
                    while (n--) *p++ = tmp[n];
                    *p++ = '.';
                    *p++ = char('0' + (fp / 100) % 10);
                    *p++ = char('0' + (fp / 10) % 10);
                    *p++ = char('0' + (fp / 1) % 10);
                }

                *p = '\0';
                write_all(val);
                write_all("\n");
            }
#endif
        }

        CloseHandle(h);
    }

    inline void Load(const char* filepath) {
        if (!filepath || !*filepath)
            return;

        void* h = CreateFileA(
            filepath,
            CFG_GENERIC_READ,
            CFG_FILE_SHARE_READ,
            0,
            CFG_OPEN_EXISTING,
            CFG_FILE_ATTRIBUTE_NORMAL,
            0
        );
        if (!h || h == CFG_INVALID_HANDLE_VALUE)
            return;

        char line[512];
        unsigned long long line_len = 0;

        auto process_line = [&](char* ln) {
            TrimRight(ln);
            char* p = SkipWs(ln);
            if (!p || *p == '\0')
                return;
            if (*p == ';' || *p == '#')
                return;

            char* eq = p;
            while (*eq && *eq != '=') ++eq;
            if (*eq != '=')
                return;
            *eq = '\0';

            char* key = p;
            char* val = eq + 1;
            TrimRight(key);
            val = SkipWs(val);

            const unsigned int key_hash = HashKeyString(key);

            for (size_t_like i = 0; i < vars_count; ++i) {
                auto& var = vars[i];
                if (var.m_no_save)
                    continue;

                if (var.m_key_hash != key_hash)
                    continue;
                if (!StrEq(var.m_key, key))
                    continue;

                if (var.m_type_id == TYPE_BOOL) {
                    *static_cast<bool*>(var.m_ptr) = (val && (*val == '1' || *val == 't' || *val == 'T' || *val == 'y' || *val == 'Y')) ? true : false;
                }
                else if (var.m_type_id == TYPE_INT) {
                    int v = 0;
                    if (ParseInt(val, v)) *static_cast<int*>(var.m_ptr) = v;
                }
                else if (var.m_type_id == TYPE_FLOAT) {
                    float v = 0.0f;
                    if (ParseFloat(val, v)) *static_cast<float*>(var.m_ptr) = v;
                }
#ifdef CFG_USE_IMCOLOR
                else if (var.m_type_id == TYPE_IMCOLOR) {
                    ImColor c;
                    if (ParseImColor(val, c)) *static_cast<ImColor*>(var.m_ptr) = c;
                }
#endif
                break;
            }
        };

        char chunk[1024];
        for (;;) {
            unsigned long got = 0;
            if (!ReadFile(h, chunk, (unsigned long)sizeof(chunk), &got, 0) || got == 0)
                break;

            for (unsigned long i = 0; i < got; ++i) {
                char c = chunk[i];
                if (c == '\r')
                    continue;
                if (c != '\n') {
                    if (line_len + 1 < (unsigned long long)sizeof(line))
                        line[line_len++] = c;
                    continue;
                }
                line[line_len] = '\0';
                line_len = 0;
                process_line(line);
            }
        }

        if (line_len) {
            line[line_len] = '\0';
            process_line(line);
        }

        CloseHandle(h);
    }

} // namespace CFG

#define CFGVAR(cpp_namespacepath, configname, state) \
namespace cpp_namespacepath { \
    inline auto configname = state; \
    namespace configvar_initializers_lite { \
        inline auto configname##_initializer = []() { \
            CFG::RegisterVar(#cpp_namespacepath, #configname, &configname, CFG::TypeIdOf<decltype(configname)>(), false); \
            return true; \
        }(); \
    } \
}

#define CFGVAR_NOSAVE(cpp_namespacepath, configname, state) \
namespace cpp_namespacepath { \
    inline auto configname = state; \
    namespace configvar_initializers_lite { \
        inline auto configname##_initializer = []() { \
            CFG::RegisterVar(#cpp_namespacepath, #configname, &configname, CFG::TypeIdOf<decltype(configname)>(), true); \
            return true; \
        }(); \
    } \
}
