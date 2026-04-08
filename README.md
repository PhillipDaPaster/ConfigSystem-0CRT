[Original](https://github.com/PhillipDaPaster/Config-System) but zero crt and not using json

if you use this just know you can not use the crt less example on the json cfg system or vice versa

## Features

- Automatic save/load for:
  - `bool`
  - `int`
  - `float`
  - `ImColor` (optional)
- hierarchical namespace paths for organized configs (`Visual::Player` → `Visual.Player` in cfg)
- `CFGVAR` for regular config vars.
- `CFGVAR_NOSAVE` for vars that should never be saved.
- .h only, almost no overhead, no heavy stl usage per var.
- no CRT dependency works with k32 refrences of `CreateFileA`, `ReadFile`, `WriteFile`.
- simple hashing (`Fnv1a32`) for key lookup

---

## Setup

```cpp
#define _CRT_SECURE_NO_WARNINGS
#include "configsaver.h"

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

extern "C" {
    unsigned long __stdcall GetEnvironmentVariableA(const char* lpName, char* lpBuffer, unsigned long nSize);
    int __stdcall CreateDirectoryA(const char* lpPathName, void* lpSecurityAttributes);
}

static void MakeDir(const char* path) {
    CreateDirectoryA(path, 0);
}

static unsigned long long StrLenA(const char* s) {
    if (!s) return 0;
    const char* p = s;
    while (*p) ++p;
    return (unsigned long long)(p - s);
}

static void StrCpyA(char* dst, unsigned long long dst_cap, const char* src) {
    if (!dst || dst_cap == 0) return;
    unsigned long long i = 0;
    if (src) {
        while (src[i] && (i + 1) < dst_cap) {
            dst[i] = src[i];
            ++i;
        }
    }
    dst[i] = '\0';
}

static void StrCatA(char* dst, unsigned long long dst_cap, const char* src) {
    if (!dst || dst_cap == 0) return;
    unsigned long long n = StrLenA(dst);
    if (n >= dst_cap) { dst[dst_cap - 1] = '\0'; return; }
    unsigned long long i = 0;
    if (src) {
        while (src[i] && (n + i + 1) < dst_cap) {
            dst[n + i] = src[i];
            ++i;
        }
    }
    dst[n + i] = '\0';
}

static int SetupPath(char* out_base, unsigned long long out_cap) {
    if (!out_base || out_cap == 0) return 0;
    out_base[0] = '\0';

    char userprofile[MAX_PATH] = { 0 };
    unsigned long n = GetEnvironmentVariableA("USERPROFILE", userprofile, (unsigned long)sizeof(userprofile));
    if (n == 0 || n >= (unsigned long)sizeof(userprofile))
        return 0;

    char cfgsystem[MAX_PATH] = { 0 };
    char configs[MAX_PATH] = { 0 };

    StrCpyA(cfgsystem, (unsigned long long)sizeof(cfgsystem), userprofile);
    StrCatA(cfgsystem, (unsigned long long)sizeof(cfgsystem), "\\Documents\\ConfigSystem");

    StrCpyA(configs, (unsigned long long)sizeof(configs), userprofile);
    StrCatA(configs, (unsigned long long)sizeof(configs), "\\Documents\\ConfigSystem\\configs");

    StrCpyA(out_base, out_cap, userprofile);
    StrCatA(out_base, out_cap, "\\Documents\\ConfigSystem");

    MakeDir(cfgsystem);
    MakeDir(configs);

    return 1;
}

namespace Globals {
    CFGVAR(Main, ExampleBool, false);
    CFGVAR(Main::Element, ExampleInt, 24);
    CFGVAR_NOSAVE(Main::Element, RuntimeOnlyVar, 50);
}

int main() {
    char base[MAX_PATH] = { 0 };
    if (!SetupPath(base, (unsigned long long)sizeof(base))) {
        return 1;
    }

    char cfg[MAX_PATH] = { 0 };
    StrCpyA(cfg, (unsigned long long)sizeof(cfg), base);
    StrCatA(cfg, (unsigned long long)sizeof(cfg), "\\configs\\config_crtless.cfg");
    // Load existing config
    CFG::Load(cfg);

    // Toggle and save
    Globals::Main::ExampleBool = !Globals::Main::ExampleBool;
    CFG::Save(cfg);

    // Reset and reload
    Globals::Main::ExampleBool = false;
    CFG::Load(cfg);

    return 0;
}
```

## output from example
```cpp
Main.ExampleBool=1
Main.Element.ExampleInt=24
```
