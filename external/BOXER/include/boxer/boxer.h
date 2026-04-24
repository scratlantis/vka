#ifndef BOXER_H
#define BOXER_H

#if defined(BOXER_DLL) && defined(BOXER_BUILD_DLL)
#error "You may not have both BOXER_DLL and BOXER_BUILD_DLL defined"
#endif

#if defined(_WIN32) && defined(BOXER_BUILD_DLL)
#define BOXERAPI __declspec(dllexport)
#elif defined(_WIN32) && defined(BOXER_DLL)
#define BOXERAPI __declspec(dllimport)
#elif defined(__GNUC__) && defined(BOXER_BUILD_DLL)
#define BOXERAPI __attribute__((visibility("default")))
#else
#define BOXERAPI
#endif

namespace boxer {

enum class Style {
   Info,
   Warning,
   Error,
   Question
};

enum class Buttons {
   OK,
   OKCancel,
   YesNo,
   Quit
};

enum class Selection {
   OK,
   Cancel,
   Yes,
   No,
   Quit,
   None,
   Error
};

const Style kDefaultStyle = Style::Info;
const Buttons kDefaultButtons = Buttons::OK;

BOXERAPI Selection show(const char *message, const char *title, Style style, Buttons buttons);

inline Selection show(const char *message, const char *title, Style style) {
   return show(message, title, style, kDefaultButtons);
}

inline Selection show(const char *message, const char *title, Buttons buttons) {
   return show(message, title, kDefaultStyle, buttons);
}

inline Selection show(const char *message, const char *title) {
   return show(message, title, kDefaultStyle, kDefaultButtons);
}

} // namespace boxer

#endif
