#ifdef __cplusplus
extern "C"
{
#endif

void status_init();
void status_set(int state);
void status_blink(float speed, int blinks);
int fatal_error(char* message);

#define ARRLEN(arr) (sizeof(arr) / sizeof((arr)[0]))

#ifdef __cplusplus
}
#endif
