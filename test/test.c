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

upnp_device_t * read_device(const char * path)
{
	FILE * fp = fopen(path, "r");
	if (fp == NULL) {
		fprintf(stderr, "error -- no file found\n");
		assert(0);
	}
	char buffer[4096] = {0,};
	fread(buffer, 1, sizeof(buffer), fp);
	fclose(fp);
	return upnp_read_device_xml(buffer);
}

void test_device()
{

	upnp_device_t * device = read_device("../res/nested_device.xml");
	assert(device != NULL);

	list_t * usn_list = upnp_device_get_all_usns(device);
	while (usn_list) {
		printf("usn -- %s\n", upnp_usn_to_string((upnp_usn_t*)usn_list->data));
		usn_list = usn_list->next;
	}
}

void test_search()
{
	printf("test search\n");
	upnp_device_t * device = read_device("../res/nested_device.xml");
	assert(device != NULL);

	assert(upnp_device_get_device_with_type(device, "urn:schemas-upnp-org:device:DimmableLight:1") != NULL);
	assert(upnp_device_get_device_with_type(device, "urn:schemas-upnp-org:device:BinaryLight:0.9") != NULL);
	assert(upnp_device_get_device_with_type(device, "urn:schemas-upnp-org:device:BinaryLight") == NULL);

	assert(upnp_device_get_service_with_type(device, "urn:schemas-upnp-org:service:SwitchPower:1") != NULL);
	assert(upnp_device_get_service_with_type(device, "urn:schemas-upnp-org:service:Dimming:1") != NULL);
	assert(upnp_device_get_service_with_type(device, "urn:schemas-upnp-org:service:Dimming") == NULL);
}

void test_str()
{
	printf("test str\n");
	assert(starts_with("hello world", "hello"));
	assert(ends_with("hello world", "world"));
}

void test_device_description()
{
	printf("test device description\n");
	
	upnp_device_t * device = read_device("../res/nested_device.xml");
	assert(device != NULL);

	char * description = upnp_write_device_description(device);
	assert(upnp_read_device_xml(description) != NULL);

	upnp_device_t * re_device = upnp_read_device_xml(description);
	assert(re_device != NULL);

	list_t * usn_list = upnp_device_get_all_usns(re_device);
	while (usn_list) {
		printf("usn -- %s\n", upnp_usn_to_string((upnp_usn_t*)usn_list->data));
		usn_list = usn_list->next;
	}
	
	free(description);
}

upnp_scpd_t * _read_scpd(const char * filepath)
{
	FILE * fp = fopen(filepath, "r");
	if (fp == NULL) {
		return NULL;
	}
	char buffer[4096] = {0,};
	fread(buffer, 1, sizeof(buffer), fp);
	fclose(fp);
	return upnp_read_scpd_xml(buffer);
}

void _print_scpd(upnp_scpd_t * scpd)
{
	list_t * action_node = scpd->actions;
	for (; action_node; action_node = action_node->next) {
		upnp_action_t * action = (upnp_action_t*)action_node->data;
		printf("action -- %s\n", action->name);
	}

	list_t * state_variable_node = scpd->state_variables;
	for (; state_variable_node; state_variable_node = state_variable_node->next) {
		upnp_state_variable_t * state_variable = (upnp_state_variable_t*)state_variable_node->data;
		printf("state variable -- %s\n", state_variable->name);
	}
}

void test_scpd()
{
	printf("test scpd\n");

	upnp_scpd_t * scpd = _read_scpd("../res/scpd.xml");
	assert(scpd != NULL);

	_print_scpd(scpd);

	char * scpd_xml = upnp_write_scpd(scpd);
	assert(scpd_xml != NULL);
	printf("scpd -- %s\n", scpd_xml);

	upnp_scpd_t * re_scpd = upnp_read_scpd_xml(scpd_xml);
	_print_scpd(re_scpd);

	free(scpd_xml);
}

int main(int argc, char *argv[])
{
	test_usn();
	test_device();
	test_search();
	test_str();
	test_device_description();
	test_scpd();
	
    return 0;
}
