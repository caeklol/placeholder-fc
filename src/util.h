#ifdef __cplusplus
extern "C" {
#endif

void status_init();
void status_set(int state);
void status_blink(float speed, int blinks);
int fatal_error(const char* message);

static inline float map(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

#define ARRLEN(arr) (sizeof(arr) / sizeof((arr)[0]))

#define max(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

#define min(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})

#ifdef __cplusplus
}
#endif
