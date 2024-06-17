#ifdef __cplusplus
extern "C" {
#endif

int controller_init(long frequency);
void controller_accept_packet(uint8_t* panic, uint16_t* joyX1, uint16_t* joyY1, uint16_t* joyX2, uint16_t* joyY2);

#ifdef __cplusplus
}
#endif
