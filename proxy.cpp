#include <windows.h>

// مؤشر للملف الأصلي بعد تغيير اسمه
HMODULE hOriginal = NULL;

// تعريف أنواع الدوال الأصلية التي قد يستدعيها البرنامج
typedef int (*pfn_hasp_logout)(void* handle);
typedef int (*pfn_hasp_encrypt)(void* handle, void* buffer, unsigned int length);
typedef int (*pfn_hasp_decrypt)(void* handle, void* buffer, unsigned int length);

pfn_hasp_logout orig_hasp_logout = NULL;
pfn_hasp_encrypt orig_hasp_encrypt = NULL;
pfn_hasp_decrypt orig_hasp_decrypt = NULL;

// دالة لتحميل المكتبة الأصلية تلقائياً عند تشغيل البرنامج
void InitializeOriginal() {
    if (!hOriginal) {
        // سيتم تحميل الملف الأصلي الذي قمت بتغيير اسمه إلى hasp_original.dll
        hOriginal = LoadLibraryA("hasp_original.dll");
        if (hOriginal) {
            orig_hasp_logout = (pfn_hasp_logout)GetProcAddress(hOriginal, "hasp_logout");
            orig_hasp_encrypt = (pfn_hasp_encrypt)GetProcAddress(hOriginal, "hasp_encrypt");
            orig_hasp_decrypt = (pfn_hasp_decrypt)GetProcAddress(hOriginal, "hasp_decrypt");
        }
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        InitializeOriginal();
        break;
    case DLL_PROCESS_DETACH:
        if (hOriginal) FreeLibrary(hOriginal);
        break;
    }
    return TRUE;
}

// ==========================================
// الدالة الأهم: hasp_login (المسؤولة عن خطأ H0007)
// ==========================================
extern "C" __declspec(dllexport) int hasp_login(unsigned int feature_id, const char* scope, const void* vendor_code, void* handle) {
    // نقوم بخداع البرنامج وإخباره أن المفتاح متصل بنجاح
    if (handle) {
        // إعطاء مؤشر جلسة وهمي صالح (Session Handle)
        *(unsigned long long*)handle = 0x12345678;
    }
    
    // إرجاع الصفر (0) يعني HASP_STATUS_OK (تم التحقق بنجاح وتجاوز الخطأ نهائياً)
    return 0; 
}

// تمرير دالة الخروج من الجلسة للملف الأصلي
extern "C" __declspec(dllexport) int hasp_logout(void* handle) {
    InitializeOriginal();
    if (orig_hasp_logout) {
        return orig_hasp_logout(handle);
    }
    return 0;
}

// تمرير دالة التشفير للملف الأصلي
extern "C" __declspec(dllexport) int hasp_encrypt(void* handle, void* buffer, unsigned int length) {
    InitializeOriginal();
    if (orig_hasp_encrypt) {
        return orig_hasp_encrypt(handle, buffer, length);
    }
    return 0;
}

// تمرير دالة فك التشفير للملف الأصلي
extern "C" __declspec(dllexport) int hasp_decrypt(void* handle, void* buffer, unsigned int length) {
    InitializeOriginal();
    if (orig_hasp_decrypt) {
        return orig_hasp_decrypt(handle, buffer, length);
    }
    return 0;
}
