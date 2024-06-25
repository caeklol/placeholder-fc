// adapted from https://github.com/illusiaDecafish/bootselBoot/tree/main

#include <stdio.h>
#include <libusb.h>


int main() {
    int ret = libusb_init(NULL);
    if (ret != 0) {
        return -1;
    }
    
    uint16_t vendor_id = 0x2E8A;
    uint16_t product_id = 0x000A;

	libusb_device** devices;
	ssize_t found_devices = libusb_get_device_list(NULL, &devices);

	libusb_device* pico = NULL;

	for (ssize_t i = 0; i < found_devices; i++) {
		struct libusb_device_descriptor desc = { 0 };
		libusb_device* device = devices[i];
		libusb_get_device_descriptor(device, &desc);
		if (desc.idVendor == vendor_id && desc.idProduct == product_id) {
			pico = device;
			break;
		}
	}

	libusb_free_device_list(devices, 0);

	if (pico == NULL) {
		//printf("No Pico found\n");
        return(1);
	}

    libusb_device_handle *handle;
	ret = libusb_open(pico, &handle);
    if (ret != 0) {
		printf("Failed to open Pico: %s\n", libusb_strerror(ret));
        libusb_exit(NULL);
        return(1);
    }

    
    int interface_number = 2;
    ret = libusb_claim_interface(handle, interface_number);
    if (ret != 0) {
		printf("Found Pico in BOOTSEL mode!\n");
        libusb_exit(NULL);
        return(0);
    }

	printf("Pico not in BOOTSEL mode, sending reset request\n");
    
    uint8_t reqtype = (0 << 6)
                            | (1);
    uint16_t windex = interface_number;
    uint16_t wvalue = 0;
    ret = libusb_control_transfer(handle, reqtype, 0x01, wvalue, windex, NULL, 0, 10);
	libusb_close(handle);
    libusb_exit(NULL);
    
	printf("BOOTSEL signal sent!\n");
    
    return 0;
}
