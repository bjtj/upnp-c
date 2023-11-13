#include "upnp_serialize.h"
#include <libxml/parser.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

#define VALID_RC(rc) if ((rc) < 0) { fprintf(stderr, "[%s:%d] valid rc -- failed with '%d'\n", __func__, __LINE__, rc); goto done; }

static void _write_device_node(xmlTextWriterPtr writer, upnp_device_t * device);
static void _write_service_node(xmlTextWriterPtr writer, upnp_service_t * service);
static void _write_action_node(xmlTextWriterPtr writer, upnp_action_t * action);
static void _write_argument_node(xmlTextWriterPtr writer, upnp_argument_t * argument);
static void _write_state_variable_node(xmlTextWriterPtr writer, upnp_state_variable_t * state_variable);

static upnp_service_t * s_read_service_node(xmlNode * node)
{
	upnp_service_t * service = upnp_create_service();
	xmlNode * cur_node = node->xmlChildrenNode;
	for (; cur_node; cur_node = cur_node->next) {
		if (cur_node->type == XML_ELEMENT_NODE && xmlStrcmp(cur_node->name, (xmlChar*)"text")) {
			xmlNode * first = cur_node->xmlChildrenNode;
			if (!first || first->next) {
				continue;
			}
			service->properties = property_put(service->properties,
                                         (char*)cur_node->name,
                                         (char*)first->content);
		}
	}

	return service;
}

static upnp_device_t * s_read_device_node(xmlNode * node)
{
	upnp_device_t * device = upnp_create_device();
	xmlNode * cur_node;
	for (cur_node = node->xmlChildrenNode; cur_node; cur_node = cur_node->next) {
		if (cur_node->type == XML_ELEMENT_NODE) {
			if (xmlStrcmp(cur_node->name, (xmlChar*)"deviceList") == 0) {
				xmlNode * device_node = cur_node->xmlChildrenNode;
				for (; device_node; device_node = device_node->next) {
					if (xmlStrcmp(device_node->name, (xmlChar*)"device") == 0) {
						upnp_device_t * child_device = s_read_device_node(device_node);
						device->embedded_devices = list_add(device->embedded_devices, child_device);
					}
				}
			} else if (xmlStrcmp(cur_node->name, (xmlChar*)"serviceList") == 0) {
				xmlNode * service_node = cur_node->xmlChildrenNode;
				for (; service_node; service_node = service_node->next) {
					if (xmlStrcmp(service_node->name, (xmlChar*)"service") == 0) {
						upnp_service_t * service = s_read_service_node(service_node);
						device->services = list_add(device->services, service);
					}
				}
			} else {
				xmlNode * first = cur_node->xmlChildrenNode;
				char * name;
				char * value;
				if (!first || first->next) {
					continue;
				}
				name = (char*)cur_node->name;
				value = (char*)first->content;
				device->properties = property_put(device->properties, name, value);
			}
		}
	}
	return device;
}

upnp_device_t * upnp_read_device_xml(const char * xml)
{
	upnp_device_t * device = NULL;
	xmlDoc * doc;
	xmlNode * root;
	xmlNode * cur_node = NULL;

	if (xml == NULL) {
		return NULL;
	}
	
	doc = xmlReadMemory(xml, strlen(xml), NULL, NULL, 0);
	if (doc == NULL) {
		goto done;
	}
	
	root = xmlDocGetRootElement(doc);
	if (root == NULL) {
		goto done;
	}

	device = upnp_create_device();
	
	for (cur_node = root->xmlChildrenNode; cur_node; cur_node = cur_node->next) {
		if (cur_node->type == XML_ELEMENT_NODE) {
			if (xmlStrcmp(cur_node->name, (xmlChar*)"specVersion") == 0) {
				/* spec version */
			} else if (xmlStrcmp(cur_node->name, (xmlChar*)"device") == 0) {
				device = s_read_device_node(cur_node);
				break;
			}
		}
	}

done:
	
	xmlFreeDoc(doc);
	xmlCleanupParser();
	xmlMemoryDump();
	return device;
}


static upnp_argument_t * s_read_argument(xmlNode * node)
{
	xmlNode * cur_node;
	upnp_argument_t * argument = upnp_create_argument();
	cur_node = node->xmlChildrenNode;
	for (; cur_node; cur_node = cur_node->next) {
		if (xmlStrcmp(cur_node->name, (xmlChar*)"name") == 0) {
			xmlNode * first = cur_node->xmlChildrenNode;
			if (!first || first->next) {
				continue;
			}
			argument->name = strdup((char*)first->content);
		} else if (xmlStrcmp(cur_node->name, (xmlChar*)"relatedStateVariable") == 0) {
			xmlNode * first = cur_node->xmlChildrenNode;
			if (!first || first->next) {
				continue;
			}
			argument->related_state_variable = strdup((char*)first->content);
		} else if (xmlStrcmp(cur_node->name, (xmlChar*)"direction") == 0) {
			xmlNode * first = cur_node->xmlChildrenNode;
			if (!first || first->next) {
				continue;
			}
			argument->direction = ((xmlStrcmp(first->content, (xmlChar*)"in") == 0)
                             ? DIR_IN : DIR_OUT);
		}
	}

	return argument;
}

static upnp_action_t * s_read_action(xmlNode * node)
{
	xmlNode * cur_node = node->xmlChildrenNode;
	upnp_action_t * action = upnp_create_action();
	for (; cur_node; cur_node = cur_node->next) {
		if (xmlStrcmp(cur_node->name, (xmlChar*)"name") == 0) {
			xmlNode * first = cur_node->xmlChildrenNode;
			if (!first || first->next) {
				continue;
			}
			action->name = strdup((char*)first->content);
		} else if (xmlStrcmp(cur_node->name, (xmlChar*)"argumentList") == 0) {
			xmlNode * argument_node = cur_node->xmlChildrenNode;
			for (; argument_node; argument_node = argument_node->next) {
				if (xmlStrcmp(argument_node->name, (xmlChar*)"argument") == 0) {
					upnp_argument_t * argument = s_read_argument(argument_node);
					action->arguments = list_add(action->arguments, argument);
				}
			}
		}
	}

	return action;
}

static upnp_state_variable_t * s_read_state_variable(xmlNode * node)
{
	xmlAttr * attrs;
	xmlNode * cur_node;
	upnp_state_variable_t * state_variable = upnp_create_state_variable();
	cur_node = node->xmlChildrenNode;

	attrs = node->properties;
	for (; attrs; attrs = attrs->next) {
		if ((xmlStrcmp(attrs->name, (xmlChar*)"sendEvents") == 0) &&
        (xmlStrcmp(attrs->xmlChildrenNode->content, (xmlChar*)"yes") == 0)) {
			upnp_state_variable_set_send_events(state_variable, 1);
		} else if ((xmlStrcmp(attrs->name, (xmlChar*)"multicast") == 0) &&
               (xmlStrcmp(attrs->xmlChildrenNode->content, (xmlChar*)"yes") == 0)) {
			upnp_state_variable_set_multicast(state_variable, 1);
		}
	}
	
	for (; cur_node; cur_node = cur_node->next) {
		if (xmlStrcmp(cur_node->name, (xmlChar*)"name") == 0) {
			xmlNode * first = cur_node->xmlChildrenNode;
			if (!first || first->next) {
				continue;
			}
			state_variable->name = strdup((char*)first->content);
		} else if (xmlStrcmp(cur_node->name, (xmlChar*)"dataType") == 0) {
			xmlNode * first = cur_node->xmlChildrenNode;
			if (!first || first->next) {
				continue;
			}
			state_variable->data_type = strdup((char*)first->content);
		} else if (xmlStrcmp(cur_node->name, (xmlChar*)"defaultValue") == 0) {
			xmlNode * first = cur_node->xmlChildrenNode;
			if (!first || first->next) {
				continue;
			}
			state_variable->default_value = strdup((char*)first->content);
		} else if (xmlStrcmp(cur_node->name, (xmlChar*)"allowedValueList") == 0) {
			xmlNode * allowed_node = cur_node->xmlChildrenNode;
			list_t * allowed_list = NULL;
			for (; allowed_node; allowed_node = allowed_node->next) {
				if (xmlStrcmp(allowed_node->name, (xmlChar*)"allowedValue") == 0) {
					xmlNode * first = cur_node->xmlChildrenNode;
					if (!first || first->next) {
						continue;
					}
					allowed_list = list_add(allowed_list, xmlStrdup(first->content));
				}
			}
			upnp_state_variable_set_allowed_list(state_variable, allowed_list);
		}
		/* todo: allowed value range */
	}

	return state_variable;
}

upnp_scpd_t * upnp_read_scpd_xml(const char * xml)
{

	upnp_scpd_t * scpd = NULL;
	xmlDoc * doc;
	xmlNode * root;
	xmlNode * cur_node = NULL;

	doc = xmlReadMemory(xml, strlen(xml), NULL, NULL, 0);
	if (doc == NULL) {
		goto done;
	}
	
	root = xmlDocGetRootElement(doc);
	if (root == NULL) {
		goto done;
	}

	scpd = upnp_create_scpd();

	cur_node = root->xmlChildrenNode;
	for (; cur_node; cur_node = cur_node->next) {
		if (xmlStrcmp(cur_node->name, (xmlChar*)"specVersion") == 0) {
			/*  */
		} else if(xmlStrcmp(cur_node->name, (xmlChar*)"actionList") == 0) {
			xmlNode * action_node = cur_node->xmlChildrenNode;
			for (; action_node; action_node = action_node->next) {
				if (xmlStrcmp(action_node->name, (xmlChar*)"action") == 0) {
					upnp_action_t * action = s_read_action(action_node);
					scpd->actions = list_add(scpd->actions, action);
				}
			}
		} else if (xmlStrcmp(cur_node->name, (xmlChar*)"serviceStateTable") == 0) {
			xmlNode * state_variable_node = cur_node->xmlChildrenNode;
			for (; state_variable_node; state_variable_node = state_variable_node->next) {
				if (xmlStrcmp(state_variable_node->name, (xmlChar*)"stateVariable") == 0) {
					upnp_state_variable_t * state_variable = s_read_state_variable(state_variable_node);
					scpd->state_variables = list_add(scpd->state_variables, state_variable);
				}
			}
		}
	}

done:
	
	xmlFreeDoc(doc);
	xmlCleanupParser();
	xmlMemoryDump();
	
	return scpd;
}

upnp_action_request_t * upnp_read_action_request(const char * xml)
{
	upnp_action_request_t * request = NULL;
	xmlDoc * doc;
	xmlNode * root;
	xmlNode * cur_node = NULL;

	doc = xmlReadMemory(xml, strlen(xml), NULL, NULL, 0);
	if (doc == NULL) {
		goto done;
	}
	
	root = xmlDocGetRootElement(doc);
	if (root == NULL) {
		goto done;
	}

	request = upnp_create_action_request();

	cur_node = root->xmlChildrenNode;
	for (; cur_node; cur_node = cur_node->next) {
		if (xmlStrcmp(cur_node->name, (xmlChar*)"Body") == 0) {
			xmlNode * node = cur_node->xmlChildrenNode;
			for (; node; node = node->next) {
				if (node->type == XML_ELEMENT_NODE) {
					xmlNode * param_node = node->xmlChildrenNode;
					upnp_action_request_set_action_name(request, (char*)node->name);
					/* todo: service type */
					for (; param_node; param_node = param_node->next) {
						if (param_node->type == XML_ELEMENT_NODE) {
							xmlNode * first = param_node->xmlChildrenNode;
							if (!first) {
								upnp_action_request_put(request,
                                        (char*)param_node->name,
                                        "");
								continue;
							}
							if (first->next) {
								continue;
							}
							upnp_action_request_put(request,
                                      (char*)param_node->name,
                                      (char*)first->content);
						}
					}
					break;
				}
			}
		}
	}

done:
	xmlFreeDoc(doc);
	xmlCleanupParser();
	xmlMemoryDump();
	
	return request;
}

upnp_action_response_t * upnp_read_action_response(const char * xml)
{
	upnp_action_response_t * response = NULL;
	xmlDoc * doc;
	xmlNode * root;
	xmlNode * cur_node = NULL;

	doc = xmlReadMemory(xml, strlen(xml), NULL, NULL, 0);
	if (doc == NULL) {
		goto done;
	}
	
	root = xmlDocGetRootElement(doc);
	if (root == NULL) {
		goto done;
	}

	response = upnp_create_action_response();

	cur_node = root->xmlChildrenNode;
	for (; cur_node; cur_node = cur_node->next)
	{
		if (xmlStrcmp(cur_node->name, (xmlChar*)"Body") == 0)
		{
			xmlNode * node = cur_node->xmlChildrenNode;
			for (; node; node = node->next)
			{
				if (node->type == XML_ELEMENT_NODE &&
            xmlStrlen(node->name) > strlen("Response"))
				{
					xmlNode * param_node = node->xmlChildrenNode;
					char action_name[128] = {0,};
					snprintf(action_name, sizeof(action_name), "%.*s",
                   (int)(xmlStrlen(node->name) - strlen("Response")),
                   node->name);
					upnp_action_response_set_action_name(response, action_name);
					/* todo: service type */
					for (; param_node; param_node = param_node->next) {
						if (param_node->type == XML_ELEMENT_NODE) {
							xmlNode * first = param_node->xmlChildrenNode;
							if (!first) {
								upnp_action_response_put(response,
                                         (char*)param_node->name,
                                         "");
								continue;
							}
							if (first->next) {
								continue;
							}
							upnp_action_response_put(response,
                                       (char*)param_node->name,
                                       (char*)first->content);
						}
					}
					break;
				}
			}
		}
	}

done:
	xmlFreeDoc(doc);
	xmlCleanupParser();
	xmlMemoryDump();
	
	return response;
}


char * upnp_write_action_request(upnp_action_request_t * req)
{


	
	char * out = NULL;
	int rc;
	xmlTextWriterPtr writer;
	xmlBufferPtr buf;
	const char * encoding = "utf-8";
	list_t * lst;
	char tmp[1024] = {0,};
	
	buf = xmlBufferCreate();
	writer = xmlNewTextWriterMemory(buf, 0);

	rc = xmlTextWriterStartDocument(writer, NULL, encoding, NULL);
	VALID_RC(rc);

	/* envelope */
	rc = xmlTextWriterStartElement(writer, BAD_CAST "s:Envelope");
	VALID_RC(rc);

	rc = xmlTextWriterWriteAttribute(writer,
                                   BAD_CAST "s:encodingStyle",
                                   BAD_CAST "http://schemas.xmlsoap.org/soap/encoding/");
	VALID_RC(rc);

	rc = xmlTextWriterWriteAttribute(writer,
                                   BAD_CAST "xmlns:s",
                                   BAD_CAST "http://schemas.xmlsoap.org/soap/envelope/");
	VALID_RC(rc);


	/* body */
	rc = xmlTextWriterStartElement(writer, BAD_CAST "s:Body");
	VALID_RC(rc);

	/* action */
	snprintf(tmp, sizeof(tmp), "u:%s", upnp_action_request_get_action_name(req));
	rc = xmlTextWriterStartElement(writer, BAD_CAST tmp);
	VALID_RC(rc);

	rc = xmlTextWriterWriteAttribute(writer,
                                   BAD_CAST "xmlns:u",
                                   BAD_CAST upnp_action_request_get_service_type(req));
	VALID_RC(rc);

	/* params */
	lst = req->params;
	for (; lst; lst = lst->next) {
		name_value_t * nv = (name_value_t*)lst->data;
		/* single element */
		rc = xmlTextWriterWriteFormatElement(writer,
                                         BAD_CAST name_value_get_name(nv),
                                         "%s", name_value_get_value(nv));
		VALID_RC(rc);
	}

	rc = xmlTextWriterEndElement(writer);
	VALID_RC(rc);

	rc = xmlTextWriterEndElement(writer);
	VALID_RC(rc);
	
	rc = xmlTextWriterEndElement(writer);
	VALID_RC(rc);
	
	rc = xmlTextWriterEndDocument(writer);
	VALID_RC(rc);

	out = (char*)xmlStrdup(buf->content);

done:
	xmlFreeTextWriter(writer);
	xmlBufferFree(buf);
	return out;
}


char * upnp_write_action_response(upnp_action_response_t * res)
{

	
	char * out = NULL;
	int rc;
	xmlTextWriterPtr writer;
	xmlBufferPtr buf;
	const char * encoding = "utf-8";
	list_t * lst;
	char tmp[1024] = {0,};
	
	buf = xmlBufferCreate();
	writer = xmlNewTextWriterMemory(buf, 0);

	rc = xmlTextWriterStartDocument(writer, NULL, encoding, NULL);
	VALID_RC(rc);

	/* envelope */
	rc = xmlTextWriterStartElement(writer, BAD_CAST "s:Envelope");
	VALID_RC(rc);

	rc = xmlTextWriterWriteAttribute(writer,
                                   BAD_CAST "s:encodingStyle",
                                   BAD_CAST "http://schemas.xmlsoap.org/soap/encoding/");
	VALID_RC(rc);

	rc = xmlTextWriterWriteAttribute(writer,
                                   BAD_CAST "xmlns:s",
                                   BAD_CAST "http://schemas.xmlsoap.org/soap/envelope/");
	VALID_RC(rc);


	/* body */
	rc = xmlTextWriterStartElement(writer, BAD_CAST "s:Body");
	VALID_RC(rc);

	/* action */
	snprintf(tmp, sizeof(tmp), "u:%sResponse", upnp_action_response_get_action_name(res));
	rc = xmlTextWriterStartElement(writer, BAD_CAST tmp);
	VALID_RC(rc);

	rc = xmlTextWriterWriteAttribute(writer,
                                   BAD_CAST "xmlns:u",
                                   BAD_CAST upnp_action_response_get_service_type(res));
	VALID_RC(rc);

	/* params */
	lst = res->params;
	for (; lst; lst = lst->next) {
		name_value_t * nv = (name_value_t*)lst->data;
		/* single element */
		rc = xmlTextWriterWriteFormatElement(writer,
                                         BAD_CAST name_value_get_name(nv),
                                         "%s", name_value_get_value(nv));
		VALID_RC(rc);
	}

	rc = xmlTextWriterEndElement(writer);
	VALID_RC(rc);

	rc = xmlTextWriterEndElement(writer);
	VALID_RC(rc);
	
	rc = xmlTextWriterEndElement(writer);
	VALID_RC(rc);
	
	rc = xmlTextWriterEndDocument(writer);
	VALID_RC(rc);

	out = (char*)xmlStrdup(buf->content);

done:
	xmlFreeTextWriter(writer);
	xmlBufferFree(buf);
	return out;
}


list_t * upnp_read_propertyset(const char * xml)
{
	list_t * props = NULL;
	xmlDoc * doc;
	xmlNode * root;
	xmlNode * cur_node = NULL;

	if (xml == NULL) {
		return NULL;
	}
	
	doc = xmlReadMemory(xml, strlen(xml), NULL, NULL, 0);
	if (doc == NULL) {
		goto done;
	}
	
	root = xmlDocGetRootElement(doc);
	if (root == NULL) {
		goto done;
	}

	for (cur_node = root->xmlChildrenNode; cur_node; cur_node = cur_node->next) {
		if (cur_node->type == XML_ELEMENT_NODE &&
        xmlStrcmp(cur_node->name, (xmlChar*)"property") == 0)
		{
			xmlNode * property_node = cur_node->xmlChildrenNode;
			for (; property_node; property_node = property_node->next) {
				if (property_node->type == XML_ELEMENT_NODE) {
					char * name = (char*)property_node->name;
					char * value = (property_node->xmlChildrenNode == NULL) ?
						"" : (char*)property_node->xmlChildrenNode->content;
					props = list_add(props, create_name_value_with_namevalue(name, value));
					break;
				}
			}

		}
	}

done:
	
	xmlFreeDoc(doc);
	xmlCleanupParser();
	xmlMemoryDump();
	return props;
}

char * upnp_write_propertyset(list_t * props)
{

	
	char * out = NULL;
	int rc;
	xmlTextWriterPtr writer;
	xmlBufferPtr buf;
	const char * encoding = "utf-8";
	list_t * lst;
	
	buf = xmlBufferCreate();
	writer = xmlNewTextWriterMemory(buf, 0);

	rc = xmlTextWriterStartDocument(writer, NULL, encoding, NULL);
	VALID_RC(rc);

	/* propertyset */
	rc = xmlTextWriterStartElement(writer, BAD_CAST "e:propertyset");
	VALID_RC(rc);

	rc = xmlTextWriterWriteAttribute(writer,
                                   BAD_CAST "xmlns:e",
                                   BAD_CAST "urn:schemas-upnp-org:event-1-0");
	VALID_RC(rc);


	/* property */
	lst = props;
	for (; lst; lst = lst->next) {
		name_value_t * nv = (name_value_t*)lst->data;

		rc = xmlTextWriterStartElement(writer, BAD_CAST "e:property");
		VALID_RC(rc);
		
		/* single element */
		rc = xmlTextWriterWriteFormatElement(writer,
                                         BAD_CAST name_value_get_name(nv),
                                         "%s", name_value_get_value(nv));
		VALID_RC(rc);
	}
	
	rc = xmlTextWriterEndElement(writer);
	VALID_RC(rc);
	
	rc = xmlTextWriterEndDocument(writer);
	VALID_RC(rc);

	out = (char*)xmlStrdup(buf->content);

done:
	xmlFreeTextWriter(writer);
	xmlBufferFree(buf);
	return out;
}

int upnp_read_timeout(const char * timeout)
{
	if (strcmp_ignorecase(timeout, "Second-") == 0) {
		return atoi(timeout + strlen("Second-"));
	}
	return -1;
}

char * upnp_write_device_description(upnp_device_t * device)
{
	char * out = NULL;
	int rc;
	xmlTextWriterPtr writer;
	xmlBufferPtr buf;
	const char * encoding = "utf-8";
	list_t * lst;
	char tmp[1024] = {0,};
	
	buf = xmlBufferCreate();
	writer = xmlNewTextWriterMemory(buf, 0);

	rc = xmlTextWriterStartDocument(writer, NULL, encoding, NULL);
	VALID_RC(rc);

	rc = xmlTextWriterStartElement(writer, BAD_CAST "root");
	VALID_RC(rc);

	_write_device_node(writer, device);

	rc = xmlTextWriterEndElement(writer);
	VALID_RC(rc);
	
	rc = xmlTextWriterEndDocument(writer);
	VALID_RC(rc);

	out = (char*)xmlStrdup(buf->content);

done:
	xmlFreeTextWriter(writer);
	xmlBufferFree(buf);
	return out;
}

char * upnp_write_scpd(upnp_scpd_t * scpd)
{
	char * out = NULL;
	int rc;
	xmlTextWriterPtr writer;
	xmlBufferPtr buf;
	const char * encoding = "utf-8";
	
	buf = xmlBufferCreate();
	writer = xmlNewTextWriterMemory(buf, 0);

	rc = xmlTextWriterStartDocument(writer, NULL, encoding, NULL);
	VALID_RC(rc);

	rc = xmlTextWriterStartElement(writer, BAD_CAST "scpd");
	VALID_RC(rc);
	
	rc = xmlTextWriterWriteAttribute(writer,
                                   BAD_CAST "xmlns",
                                   BAD_CAST "urn:schemas-upnp-org:service-1-0");
	VALID_RC(rc);

	{
		rc = xmlTextWriterStartElement(writer, BAD_CAST "actionList");
		VALID_RC(rc);
		
		list_t * action_node = scpd->actions;
		for (; action_node; action_node = action_node->next) {
			upnp_action_t * action = (upnp_action_t*)action_node->data;
			_write_action_node(writer, action);
		}
		rc = xmlTextWriterEndElement(writer);
		VALID_RC(rc);
	}

	{
		rc = xmlTextWriterStartElement(writer, BAD_CAST "serviceStateTable");
		VALID_RC(rc);
		list_t * state_variable_node = scpd->state_variables;
		for (; state_variable_node; state_variable_node = state_variable_node->next) {
			upnp_state_variable_t * state_variable = (upnp_state_variable_t*)state_variable_node->data;
			_write_state_variable_node(writer, state_variable);
		}
		rc = xmlTextWriterEndElement(writer);
		VALID_RC(rc);
	}

	rc = xmlTextWriterEndElement(writer);
	VALID_RC(rc);
	
	rc = xmlTextWriterEndDocument(writer);
	VALID_RC(rc);

	out = (char*)xmlStrdup(buf->content);

done:
	xmlFreeTextWriter(writer);
	xmlBufferFree(buf);
	return out;
}


static void _write_device_node(xmlTextWriterPtr writer, upnp_device_t * device)
{
	int rc;
	rc = xmlTextWriterStartElement(writer, BAD_CAST "device");
	VALID_RC(rc);

	{
		list_t * property_node = device->properties;
		for (; property_node; property_node = property_node->next) {
			property_t * prop = (property_t*)property_node->data;
			char * name = property_get_name(prop);
			char * value = property_get_value(prop);
			rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST name, "%s", value);
			VALID_RC(rc);
		}
	}

	list_t * service_node = device->services;
	if (service_node) {	
		rc = xmlTextWriterStartElement(writer, BAD_CAST "serviceList");
		VALID_RC(rc);

		for (; service_node; service_node = service_node->next) {
			upnp_service_t * service = (upnp_service_t*)service_node->data;
			_write_service_node(writer, service);
		}

		rc = xmlTextWriterEndElement(writer);
		VALID_RC(rc);
	}

	list_t * embedded_device_node = device->embedded_devices;
	if (embedded_device_node) {
		rc = xmlTextWriterStartElement(writer, BAD_CAST "deviceList");
		VALID_RC(rc);
		for (; embedded_device_node; embedded_device_node = embedded_device_node->next) {
			upnp_device_t * embedded_device = (upnp_device_t*)embedded_device_node->data;
			_write_device_node(writer, embedded_device);
		}
		rc = xmlTextWriterEndElement(writer);
		VALID_RC(rc);
	}

	rc = xmlTextWriterEndElement(writer);
	VALID_RC(rc);
done:
	return;
}


static void _write_service_node(xmlTextWriterPtr writer, upnp_service_t * service)
{

	int rc;
	rc = xmlTextWriterStartElement(writer, BAD_CAST "service");
	VALID_RC(rc);

	{
		list_t * property_node = service->properties;
		for (; property_node; property_node = property_node->next) {
			property_t * prop = (property_t*)property_node->data;
			char * name = property_get_name(prop);
			char * value = property_get_value(prop);
			rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST name, "%s", value);
			VALID_RC(rc);
		}
	}

	rc = xmlTextWriterEndElement(writer);
	VALID_RC(rc);
done:
	return;
}

static void _write_action_node(xmlTextWriterPtr writer, upnp_action_t * action)
{
	int rc;
	rc = xmlTextWriterStartElement(writer, BAD_CAST "action");
	VALID_RC(rc);

	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "name", "%s", action->name);
	VALID_RC(rc);

	{
		rc = xmlTextWriterStartElement(writer, BAD_CAST "argumentList");
		VALID_RC(rc);
		list_t * argument_node = action->arguments;
		for (; argument_node; argument_node = argument_node->next) {
			upnp_argument_t * argument = (upnp_argument_t*)argument_node->data;
			_write_argument_node(writer, argument);
		}
		rc = xmlTextWriterEndElement(writer);
		VALID_RC(rc);
	}
	

	rc = xmlTextWriterEndElement(writer);
	VALID_RC(rc);
done:
	return;
}

static void _write_argument_node(xmlTextWriterPtr writer, upnp_argument_t * argument)
{

	int rc;
	rc = xmlTextWriterStartElement(writer, BAD_CAST "argument");
	VALID_RC(rc);

  rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "name", "%s", argument->name);
	VALID_RC(rc);

	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "direction", "%s", argument->direction == DIR_IN ? "in" : "out");
	VALID_RC(rc);

	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "relatedStateVariable", "%s", argument->related_state_variable);
	VALID_RC(rc);
	
	rc = xmlTextWriterEndElement(writer);
	VALID_RC(rc);
done:
	return;
}

static void _write_state_variable_node(xmlTextWriterPtr writer, upnp_state_variable_t * state_variable)
{
	int rc;
	rc = xmlTextWriterStartElement(writer, BAD_CAST "stateVariable");
	VALID_RC(rc);

	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "name", "%s", state_variable->name);
	VALID_RC(rc);

	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "dataType", "%s", state_variable->data_type);
	VALID_RC(rc);

	rc = xmlTextWriterEndElement(writer);
	VALID_RC(rc);
done:
	return;
}
