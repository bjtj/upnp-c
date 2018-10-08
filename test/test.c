#include <unistd.h>
#include "../src/listutil.h"
#include "../src/upnp_models.h"
#include "../src/upnp_serialize.h"
#include "../src/upnp_usn.h"

void test_usn()
{
	printf("test usn\n");
	upnp_usn_t * usn = upnp_create_usn_with_init("uuid:dummy", NULL);
	printf("udn -- %s\n", usn->udn);
	upnp_free_usn(usn);
}

void test_device()
{
	FILE * fp = fopen("../res/nested_device.xml", "r");
	if (fp == NULL) {
		fprintf(stderr, "error -- no file found\n");
		assert(0);
	}
	char buffer[4096] = {0,};
	fread(buffer, 1, sizeof(buffer), fp);
	fclose(fp);
	upnp_device_t * device = upnp_read_device_xml(buffer);
	assert(device != NULL);

	list_t * usn_list = upnp_device_get_all_usns(device);
	while (usn_list) {
		printf("usn -- %s\n", upnp_usn_to_string((upnp_usn_t*)usn_list->data));
		usn_list = usn_list->next;
	}
}

int main(int argc, char *argv[])
{
	test_usn();
	test_device();
	
    return 0;
}
